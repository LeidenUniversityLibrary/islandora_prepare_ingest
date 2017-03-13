<?php
error_reporting(E_ALL);
ini_set("display_errors", 1);


/**
 * @file
 * ubl_prepare_ingest_manage.form.inc
 */

/**
 * The form for managing the workflows
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function ubl_prepare_ingest_manage_form(array $form, array &$form_state) {
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/db');

  $workflows = ubl_prepare_ingest_select_workflows('active', TRUE);

  $options = array();
  foreach ($workflows as $workflow) {
    $options[$workflow['id']] = array(
      'name' => l($workflow['name'], 'admin/islandora/ubl_prepare_ingest/manage/edit/' . $workflow['id']),
      'type' => $workflow['type'],
      'checked' => $workflow['checked']?t('Checked'):'',
      'copy' => l(t('copy'),'admin/islandora/ubl_prepare_ingest/manage/copy/' . $workflow['id']),
      'export' => l(t('export'),'admin/islandora/ubl_prepare_ingest/manage/export/' . $workflow['id']),
    );
  }
  $form['table'] = array(
    '#type' => 'tableselect',
    '#header' => array(
      'name' => t('Workflows'),
      'type' => t('Type'),
      'checked' => t('Checked'),
      'copy' => '',
      'export' => '',
    ),
    '#options' => $options,
    '#empty' => t('No workflows available.'),
  );

  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete selected workflow(s)'),
    '#submit' => array('ubl_prepare_ingest_manage_delete_selected_workflows'),
  );

  $form['new_workflow'] = array(
    '#type' => 'submit',
    '#value' => t('Add New Workflow'),
    '#submit' => array('ubl_prepare_ingest_manage_add_workflow'),
  );

  $form['import'] = array(
    '#type' => 'fieldset',
    '#title' => t('Import workflow'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['import']['workflow_upload_import'] = array(
    '#type' => 'file',
    '#title' => t('Upload a JSON file to import'),
    '#upload_location' => 'public://ubl_prepare_ingest_import',
    '#upload_validators' => array(
      'file_validate_extensions' => array('json'),
    ),
  );
  $form['import']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#submit' => array('ubl_prepare_ingest_manage_upload_import'),
  );

  return $form;
}

function ubl_prepare_ingest_manage_delete_selected_workflows(array $form, array &$form_state) {
  if (array_key_exists("table", $form_state["values"])) {
    foreach ($form_state["values"]["table"] as $workflowid => $delete) {
      if ($delete) {
        ubl_prepare_ingest_delete_workflow($workflowid);
      }
    }
  }
}

function ubl_prepare_ingest_manage_add_workflow(array $form, array &$form_state) {
  $form_state['redirect'] = 'admin/islandora/ubl_prepare_ingest/manage/make';
}

/**
 * Submit handler for uploading a file.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function ubl_prepare_ingest_manage_upload_import($form, $form_state) {
  $file = file_save_upload('workflow_upload_import', array('file_validate_extensions' => array('json')), 'public://ubl_prepare_ingest_import');
  if ($file) {
    $path = drupal_realpath($file->uri);
    $json = file_get_contents($path);
    if ($json === FALSE) {
      drupal_set_message(t("Cannot get contents of file"), 'error');
      return;
    }
    $isimported = ubl_prepare_ingest_insert_new_workflow_from_json($json);
    if (!$isimported) {
      $errortype = json_last_error(); 
      if ($errortype === JSON_ERROR_NONE) {
        drupal_set_message(t("Imported file is JSON, but does not contain valid workflow data."), 'error');
      }
      else {
        drupal_set_message(t("Imported file is not valid JSON: @error.", array('@error' => json_last_error_msg())), 'error');
      }
      return;
    }
    drupal_set_message(t("The file @filename was imported successfully.",
      array(
        '@filename' => $file->filename,
      )
    ));
  }
}


/**
 * The form for making a workflow
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function ubl_prepare_ingest_edit_workflow_form(array $form, array &$form_state, $workflowid = FALSE) {

  $form = array();

  $workflow = array();
  if ($workflowid !== FALSE) {
    module_load_include('inc', 'ubl_prepare_ingest', 'includes/db');
    $workflow = ubl_prepare_ingest_select_workflow($workflowid);

    $form['workflowid'] = array(
      '#type' => 'hidden',
      '#value' => $workflowid,
    );
  }
 
  # name, description and type
  $form['workflowinfo'] = array(
    '#title' => t('Workflow'),
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['workflowinfo']['workflowname'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => array_key_exists('name', $workflow)?$workflow['name']:'',
    '#size' => 60,
  );
  $form['workflowinfo']['workflowdescription'] = array(
    '#title' => t('Description'),
    '#type' => 'textarea',
    '#default_value' => array_key_exists('description', $workflow)?$workflow['description']:'',
    '#size' => 60,
  );
  $form['workflowinfo']['workflowtype'] = array(
    '#title' => t('Type'),
    '#type' => 'select',
    '#options' => array('partial' => t('partial'), 'full' => t('full')),
    '#default_value' => array_key_exists('type', $workflow)?$workflow['type']:'partial',
  );

  if ($workflowid) {
    # steps
    module_load_include('inc', 'ubl_prepare_ingest', 'includes/ubl_prepare_ingest.steps.form');
    $workflowresults = isset($_SESSION['workflowresults'])?$_SESSION['workflowresults']:array();
    $workflowdata = isset($_SESSION['workflowdata'])?$_SESSION['workflowdata']:array();
    $form += ubl_prepare_ingest_steps($workflowid, $workflowresults, $workflowdata, 'edit');

    # Add another step
    $options = array();
    $stepsinfo = module_invoke_all('workflow_step_info');
    foreach ($stepsinfo as $stepname => $stepinfo) {
      $typegroup = isset($stepinfo['type group'])?$stepinfo['type group']:t('Other');
      $options[$typegroup][$stepname] = ubl_prepare_ingest_label_for_step($stepname);
    }
    ksort($options);
    foreach ($options as $k => $v) {
      asort($v);
      $options[$k] = $v;
    }

    $form['steps']['add_step'] = array(
      '#type' => 'fieldset',
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );
    $form['steps']['add_step']['which_step'] = array(
      '#type' => 'select',
      '#options' => $options,
    );
    $form['steps']['add_step']['add_button'] = array(
      '#type' => 'submit',
      '#value' => t('Add step'),
      '#id' => 'add_step_button',
    );
    # Add steps from workflow
    $options = array();
    $workflows = ubl_prepare_ingest_select_workflows('active', TRUE);
    foreach ($workflows as $workflow) {
      $options[$workflow['id']] = $workflow['name'] . ' (' . $workflow['type'] . ')';
    }
    $form['steps']['add_step']['which_workflow_steps'] = array(
      '#type' => 'select',
      '#options' => $options,
    );
    $form['steps']['add_step']['add_workflow_steps_button'] = array(
      '#type' => 'submit',
      '#value' => t('Add workflow steps'),
      '#id' => 'add_workflow_steps_button',
    );
  }
 
  # back
  $form['back'] = array(
    '#type' => 'submit',
    '#value' => t('Back'),
    '#submit' => array('ubl_prepare_ingest_edit_workflow_form_back'),
  );
  # Check
  $form['check'] = array(
    '#type' => 'submit',
    '#value' => t('Check workflow'),
    '#submit' => array('ubl_prepare_ingest_edit_workflow_form_check'),
    '#id' => 'check_workflow_button',
  );
  # Save
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save workflow'),
  );
  return $form;
}

function ubl_prepare_ingest_edit_workflow_form_validate(array $form, array &$form_state) {
  $name = $form_state['values']['workflowname'];

  if (!$name) {
    form_set_error('workflowname',t('Needs value'));
  }
}

function ubl_prepare_ingest_edit_workflow_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/db');

  unset($_SESSION['workflowresults']);
  unset($_SESSION['workflowdata']);

  $name = $form_state['values']['workflowname'];
  $description = $form_state['values']['workflowdescription'];
  $type = $form_state['values']['workflowtype'];

  if (array_key_exists('workflowid', $form_state['values'])) {
    $workflowid = $form_state['values']['workflowid'];
    ubl_prepare_ingest_update_workflow($workflowid,array(
      'name' => $name,
      'description' => $description,
      'type' => $type,
    ));
  }
  else {
    $workflowid = ubl_prepare_ingest_insert_workflow($name, $description, $type);  
    $form_state['redirect'] = 'admin/islandora/ubl_prepare_ingest/manage/edit/' . $workflowid;
  }
  $updates = array();
  foreach ($form_state['values'] as $key => $value) {
    if (strpos($key, 'remove_step_') === 0) {
      if ($value) {
        $stepidstr = substr($key, strlen('remove_step_'));
        if (($stepid = filter_var($stepidstr, FILTER_VALIDATE_INT)) !== FALSE) {
           ubl_prepare_ingest_delete_step($stepid, $workflowid);
           $updates[$stepid]['_removed'] = TRUE;
        } 
      }
    }
    elseif (strpos($key, 'step_') === 0) {
      $parts = explode('_', $key, 3);
      if (count($parts) == 3) {
        $stepidstr = $parts[1];
        $configkey = $parts[2];
        if (($stepid = filter_var($stepidstr, FILTER_VALIDATE_INT)) !== FALSE) {
          $updates[$stepid][$configkey] = $value; 
        }
      }
    }
    elseif (strpos($key, 'weight_step_') === 0) {
      $stepidstr = substr($key, strlen('weight_step_'));
      if (($stepid = filter_var($stepidstr, FILTER_VALIDATE_INT)) !== FALSE) {
        if (strpos($value, 'new_weight=') === 0) {
          $newweight = str_replace('new_weight=', '', $value);
          ubl_prepare_ingest_update_step($stepid, $workflowid, array('weight' => $newweight)); 
        }
      }
    }
  }
  foreach ($updates as $stepid => $update) {
    if (!(array_key_exists('_removed', $update) && $update['_removed'])) {
      $currentstepconfig = ubl_prepare_ingest_select_workflow_step_configuration($stepid);
      foreach ($update as $key => $value) {
        if (array_key_exists($key, $currentstepconfig)) {
          if (strcmp($value, $currentstepconfig[$key]) != 0) {
            ubl_prepare_ingest_update_step_configuration($stepid, $key, $value);
          }
        }
        else {
          ubl_prepare_ingest_insert_step_configuration($stepid, $key, $value);
        }
      } 
    }
  }

  if ($form_state['clicked_button']['#id'] === 'add_step_button') {
    $stepname = $form_state['values']['which_step'];
    if ($workflowid && $stepname) {
      ubl_prepare_ingest_insert_step($workflowid, $stepname, '');
    }
  }
  if ($form_state['clicked_button']['#id'] === 'add_workflow_steps_button') {
    $selectedworkflowid = $form_state['values']['which_workflow_steps'];
    if ($workflowid && $selectedworkflowid) {
      ubl_prepare_ingest_copy_steps($selectedworkflowid, $workflowid);
    }
  }
}

function ubl_prepare_ingest_edit_workflow_form_back(array $form, array &$form_state) {
  unset($_SESSION['workflowresults']);
  unset($_SESSION['workflowdata']);

  $form_state['redirect'] = 'admin/islandora/ubl_prepare_ingest/manage';
}

function ubl_prepare_ingest_edit_workflow_form_check(array $form, array &$form_state) {
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/ubl_prepare_ingest.run_workflow');
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/db');

  $workflowid = $form_state['values']['workflowid'];
  $workflowdata = array();
  $workflowresults = ubl_prepare_ingest_run_workflow($workflowid, $workflowdata, WORK_EXTENT_CHECK_FILLED_ARGUMENTS); 
  $_SESSION['workflowresults'] = $workflowresults; 
  $_SESSION['workflowdata'] = $workflowdata; 
  if (ubl_prepare_ingest_report_result_summary($workflowid, 'check', $workflowresults)) {
    ubl_prepare_ingest_update_workflow($workflowid, array('checked' => 1));
    $form_state['redirect'] = 'admin/islandora/ubl_prepare_ingest/manage/check/' . $workflowid;
  }
  else {
    ubl_prepare_ingest_update_workflow($workflowid, array('checked' => 0));
  }
}

function ubl_prepare_ingest_copy_workflow_form(array $form, array &$form_state, $workflowid) {
  $form += ubl_prepare_ingest_edit_workflow_form($form, $form_state, $workflowid);

  unset($form['steps']);
  unset($form['check']);
  unset($form['save']);
  
  $form['back']['#value'] = t('Cancel');
  
  $origname = $form['workflowinfo']['workflowname']['#default_value'];
  $form['workflowinfo']['#description'] = t('You are about to make a copy of workflow %origname. Fill out a new name and description below and then press %copybutton to make the copy.', array('%origname' => $origname, '%copybutton' => t('Copy workflow')));

  $copypostfix = t('copy');
  $newname = preg_replace_callback(
    "/$copypostfix(?: ([0-9]+))?$/",
    function($match) use ($copypostfix) {
      if (array_key_exists(1, $match)) {
        $nr = $match[1] + 1;
      }
      else {
        $nr = 2;
      };
      return $copypostfix.' '.$nr;
    },
    $origname
  );
  if ($newname === NULL || $origname === $newname) {
    $newname = $origname . ' ' . $copypostfix;
  }

  $form['workflowinfo']['workflowname']['#default_value'] = $newname;

  # Copy
  $form['edit_workflow']['copy'] = array(
    '#type' => 'submit',
    '#value' => t('Copy workflow'),
    '#submit' => array('ubl_prepare_ingest_edit_workflow_form_copy_workflow'),
    '#validate' => array('ubl_prepare_ingest_edit_workflow_form_validate'),
  );

  return $form;
}

function ubl_prepare_ingest_edit_workflow_form_copy_workflow(array $form, array &$form_state) {
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/db');

  $name = $form_state['values']['workflowname'];
  $description = $form_state['values']['workflowdescription'];

  if (array_key_exists('workflowid', $form_state['values'])) {
    $oldworkflowid = $form_state['values']['workflowid'];
    $newworkflowid = ubl_prepare_ingest_copy_workflow($oldworkflowid);

    ubl_prepare_ingest_update_workflow($newworkflowid, array('name' => $name, 'description' => $description));

    $form_state['redirect'] = 'admin/islandora/ubl_prepare_ingest/manage/edit/' . $newworkflowid;
  }
}

function ubl_prepare_ingest_export_workflow_form(array $form, array &$form_state, $workflowid) {
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/db');

  $form = array();
  $workflowjson = ubl_prepare_ingest_workflow_json($workflowid);

  $form['workflowid'] = array(
    '#type' => 'hidden',
    '#value' => $workflowid,
  );
  $form['workflow_export'] = array(
    '#title' => t('Workflow export'),
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['workflow_export']['json'] = array(
    '#title' => t('JSON'),
    '#type' => 'textarea',
    '#default_value' => $workflowjson,
    '#attributes' => array('readonly' => 'readonly'),
    '#size' => 60,
  );
  $form['workflow_export']['download'] = array(
    '#type' => 'submit',
    '#value' => t('Download JSON export'),
    //'#submit' => array('ubl_prepare_ingest_export_workflow_form_submit'),
  );

  return $form;
}

function ubl_prepare_ingest_export_workflow_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/db');

  $workflowid = $form_state['values']['workflowid'];
  $workflowjson = ubl_prepare_ingest_workflow_json($workflowid);
  $filepath = "temporary://workflow.json";
  $file = file_save_data($workflowjson, $filepath, FILE_EXISTS_REPLACE);
  $headers = array(
    'Content-Type' => 'application/json',
    'Content-Disposition' => 'attachment; filename="' . $file->filename . '"',
    'Content-Length' => strlen($workflowjson),
  );

  if (strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE')) {
    $headers['Cache-Control'] = 'must-revalidate, post-check=0, pre-check=0';
    $headers['Pragma'] = 'public';
  }
  else {
    $headers['Pragma'] = 'no-cache';
  }

  file_transfer($file->uri, $headers);

  file_delete($filepath);
}

