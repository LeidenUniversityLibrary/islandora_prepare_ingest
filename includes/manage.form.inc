<?php

/**
 * @file
 * islandora_prepare_ingest_manage.form.inc
 *
 *
 *  Copyright 2017-2018 Leiden University Library
 *
 *  This file is part of islandora_prepare_ingest.
 *
 *  islandora_prepare_ingest is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * The form for managing the workflows
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_prepare_ingest_manage_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');

  $type = array('full', 'partial');
  if (isset($_SESSION['filterbytype'])) {
    if ($_SESSION['filterbytype'] === 'all') {
      $type = NULL;
    }
    else {
      $type = explode(',', $_SESSION['filterbytype']);
    }
  }
  $workflows = islandora_prepare_ingest_select_workflows($type);
  $options = array();
  $checkedfilter = NULL;
  if (isset($_SESSION['filterbychecked']) && $_SESSION['filterbychecked'] !== 'all') {
    $checkedfilter = ($_SESSION['filterbychecked'] === 'checked');
  }
  foreach ($workflows as $workflow) {
    if ($checkedfilter === NULL || $workflow['checked'] == $checkedfilter) {
      $options[$workflow['id']] = array(
        'name' => l($workflow['name'], 'admin/islandora/prepare_ingest/edit/' . $workflow['id']),
        'type' => $workflow['type'],
        'checked' => $workflow['checked']?t('Checked'):'',
        'copy' => l(t('copy'),'admin/islandora/prepare_ingest/copy/' . $workflow['id']),
        'export' => l(t('export'),'admin/islandora/prepare_ingest/export/' . $workflow['id']),
      );
    }
  }
  $form['filter'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter workflows'),
    '#collapsible' => TRUE,
    '#collapsed' => ((!isset($_SESSION['filterbytype']) || $_SESSION['filterbytype'] === 'full,partial') && (!isset($_SESSION['filterbychecked']) || $_SESSION['filterbychecked'] === 'all')),
  );
  $form['filter']['bytype'] = array(
    '#type' => 'select',
    '#title' => t('Filter by type'),
    '#options' => array(
      'full,partial' => t('Full and partial workflows'),
      'full' => t('Full workflows only'),
      'partial' => t('Partial workflows only'),
      'all' => t('All workflows'),
    ),
    '#default_value' => isset($_SESSION['filterbytype']) ? $_SESSION['filterbytype'] : 0,
  );
  $form['filter']['bychecked'] = array(
    '#type' => 'select',
    '#title' => t('Filter by checked'),
    '#options' => array(
      'all' => t('Checked and unchecked'),
      'checked' => t('Checked only'),
      'unchecked' => t('Unchecked only'),
    ),
    '#default_value' => isset($_SESSION['filterbychecked']) ? $_SESSION['filterbychecked'] : 0,
  );
  $form['filter']['filter'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
    '#submit' => array('islandora_prepare_ingest_manage_filter'),
  );
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
    '#submit' => array('islandora_prepare_ingest_manage_delete_selected_workflows'),
  );

  $form['new_workflow'] = array(
    '#type' => 'submit',
    '#value' => t('Add New Workflow'),
    '#submit' => array('islandora_prepare_ingest_manage_add_workflow'),
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
    '#upload_location' => 'public://islandora_prepare_ingest_import',
    '#upload_validators' => array(
      'file_validate_extensions' => array('json'),
    ),
  );
  $form['import']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
    '#submit' => array('islandora_prepare_ingest_manage_upload_import'),
  );

  $form['config'] =  array(
    '#type' => 'fieldset',
    '#title' => t('Configuration'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['config']['per_run_data'] =  array(
    '#type' => 'checkbox',
    '#title' => t('Store workflow run data per run'),
    '#description' => t('If enabled, stores the data of a workflow run '
      . 'separately, so the same workflow can be run at the same time by '
      . 'different users, but this will increase database load. '
      . '<BR/>'
      . 'If disabled, the data of a workflow run can be seen by all users of '
      . 'the workflow.'),
    '#default_value' => variable_get('islandora_prepare_ingest_per_run_data', TRUE),
  );
  $form['config']['keep_cache_hours'] = array(
    '#title' => t('Empty cache if older than'),
    '#type' => 'textfield',
    '#default_value' => variable_get('islandora_prepare_ingest_keep_cache_hours', 24),
    '#size' => 5,
  );
  $form['config']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#submit' => array('islandora_prepare_ingest_manage_save_configuration'),
    '#validate' => array('islandora_prepare_ingest_manage_validate_configuration'),
  );

  return $form;
}

function islandora_prepare_ingest_manage_delete_selected_workflows(array $form, array &$form_state) {
  if (array_key_exists("table", $form_state["values"])) {
    foreach ($form_state["values"]["table"] as $workflowid => $delete) {
      if ($delete) {
        islandora_prepare_ingest_delete_workflow($workflowid);
      }
    }
  }
}

function islandora_prepare_ingest_manage_add_workflow(array $form, array &$form_state) {
  $form_state['redirect'] = 'admin/islandora/prepare_ingest/make';
}

function islandora_prepare_ingest_manage_filter(array $form, array &$form_state) {
  $_SESSION['filterbytype'] = isset($form_state['input']['bytype']) ? $form_state['input']['bytype'] : NULL;
  $_SESSION['filterbychecked'] = isset($form_state['input']['bychecked']) ? $form_state['input']['bychecked'] : NULL;
}

/**
 * Submit handler for uploading a file.
 *
 * @param array $form
 *   An array representing a Drupal form.
 * @param array $form_state
 *   An array containing the Drupal form state.
 */
function islandora_prepare_ingest_manage_upload_import($form, $form_state) {
  $file = file_save_upload('workflow_upload_import', array('file_validate_extensions' => array('json')), 'public://islandora_prepare_ingest_import');
  if ($file) {
    $path = drupal_realpath($file->uri);
    $json = file_get_contents($path);
    if ($json === FALSE) {
      drupal_set_message(t("Cannot get contents of file"), 'error');
      return;
    }
    $isimported = islandora_prepare_ingest_insert_new_workflow_from_json($json);
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
function islandora_prepare_ingest_edit_workflow_form(array $form, array &$form_state, $workflowid = FALSE) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/run_workflow');

  $form = array();

  $workflow = array();
  if ($workflowid !== FALSE) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
    $workflow = islandora_prepare_ingest_select_workflow($workflowid);

    $workflowresults = isset($_SESSION['workflowresults'])?$_SESSION['workflowresults']:array();
    if (variable_get('islandora_prepare_ingest_per_run_data', TRUE)) {
      $otherid = isset($_SESSION['otherid'])?$_SESSION['otherid']:NULL;
    }
    else {
      $otherid = "web$workflowid";
    }

    if (isset($_SESSION['workflowresults'])) {
      islandora_prepare_ingest_report_result_summary($workflowresults);
      // there should be a better way to prevent this from displaying twice...
      unset($_SESSION['workflowresults']);
    }

    $form['workflowid'] = array(
      '#type' => 'hidden',
      '#value' => $workflowid,
    );
    $form['otherid'] = array(
      '#type' => 'hidden',
      '#value' => $otherid,
    );
  }

  # name, description and type
  $iseditable = TRUE;
  if ($iseditable) {
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
  }
  else {
    $form['workflowinfo'] = array(
      '#title' => $workflow['name'],
      '#description' => $workflow['description'],
      '#type' => 'fieldset',
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );
  }

  if ($workflowid) {
    # steps
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/steps.form');
    $form += islandora_prepare_ingest_steps($workflowid, $workflowresults, $otherid);

    # Add another step
    $options = array();
    $stepsinfo = module_invoke_all('workflow_step_info');
    foreach ($stepsinfo as $stepname => $stepinfo) {
      $typegroup = isset($stepinfo['type group'])?$stepinfo['type group']:t('Other');
      if (is_array($typegroup)) {
        foreach ($typegroup as $tg) {
          $options[$tg][$stepname] = $stepinfo['name'];
        }
      }
      else {
        $options[$typegroup][$stepname] = $stepinfo['name'];
      }
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
    $workflows = islandora_prepare_ingest_select_workflows();
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

  # Back
  $form['back'] = array(
    '#type' => 'submit',
    '#value' => t('Back'),
    '#submit' => array('islandora_prepare_ingest_edit_workflow_form_back'),
    '#id' => 'back_button',
  );

  # Save
  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save workflow'),
    '#id' => 'save_workflow_button',
  );

  if ($workflowid) {
    # Test
    $form['dryrun'] = array(
      '#type' => 'submit',
      '#value' => t('Dryrun workflow'),
      '#submit' => array('islandora_prepare_ingest_edit_workflow_form_dryrun'),
      '#id' => 'dryrun_workflow_button',
    );
    # Hide save button when there is nothing to save
    $form['save']['#attributes']['style'] = 'display: none;';
  }
  return $form;
}

function islandora_prepare_ingest_edit_workflow_form_validate(array $form, array &$form_state) {
  $name = $form_state['values']['workflowname'];

  if (!$name) {
    form_set_error('workflowname', t('A workflow needs a name.'));
  }
}

function islandora_prepare_ingest_edit_workflow_form_submit(array $form, array &$form_state) {
  // save workflow.
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');

  unset($_SESSION['workflowresults']);
  if (variable_get('islandora_prepare_ingest_per_run_data', TRUE)) {
    unset($_SESSION['otherid']);
  }

  $name = $form_state['values']['workflowname'];
  $description = $form_state['values']['workflowdescription'];
  $type = $form_state['values']['workflowtype'];

  if (array_key_exists('workflowid', $form_state['values'])) {
    $workflowid = $form_state['values']['workflowid'];
    islandora_prepare_ingest_update_workflow($workflowid,array(
      'name' => $name,
      'description' => $description,
      'type' => $type,
    ));
  }
  else {
    $workflowid = islandora_prepare_ingest_insert_workflow($name, $description, $type);
    $form_state['redirect'] = 'admin/islandora/prepare_ingest/edit/' . $workflowid;
  }
  $updates = array();
  foreach ($form_state['values'] as $key => $value) {
    if (strpos($key, 'remove_step_') === 0) {
      if ($value) {
        $stepidstr = substr($key, strlen('remove_step_'));
        if (($stepid = filter_var($stepidstr, FILTER_VALIDATE_INT)) !== FALSE) {
           islandora_prepare_ingest_delete_step($stepid, $workflowid);
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
          islandora_prepare_ingest_update_step($stepid, $workflowid, array('weight' => $newweight));
        }
      }
    }
  }
  foreach ($updates as $stepid => $update) {
    if (!(array_key_exists('_removed', $update) && $update['_removed'])) {
      $currentstepconfig = islandora_prepare_ingest_select_workflow_step_configuration($stepid);
      foreach ($update as $key => $value) {
        if (array_key_exists($key, $currentstepconfig)) {
          if (strcmp($value, $currentstepconfig[$key]) != 0) {
            islandora_prepare_ingest_update_step_configuration($stepid, $key, $value);
          }
        }
        else {
          islandora_prepare_ingest_insert_step_configuration($stepid, $key, $value);
        }
      }
    }
  }

  if ($form_state['clicked_button']['#id'] === 'add_step_button') {
    $stepname = $form_state['values']['which_step'];
    if ($workflowid && $stepname) {
      islandora_prepare_ingest_insert_step($workflowid, $stepname, '');
    }
  }
  if ($form_state['clicked_button']['#id'] === 'add_workflow_steps_button') {
    $selectedworkflowid = $form_state['values']['which_workflow_steps'];
    if ($workflowid && $selectedworkflowid) {
      islandora_prepare_ingest_copy_steps($selectedworkflowid, $workflowid);
    }
  }
}

function islandora_prepare_ingest_edit_workflow_form_back(array $form, array &$form_state) {
  unset($_SESSION['workflowresults']);
  unset($_SESSION['otherid']);

  $form_state['redirect'] = 'admin/islandora/prepare_ingest';
}

function islandora_prepare_ingest_edit_workflow_form_dryrun(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/run_workflow');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');

  $workflowid = $form_state['values']['workflowid'];
  if (variable_get('islandora_prepare_ingest_per_run_data', TRUE)) {
    $otherid = islandora_prepare_ingest_random_string(10);
  }
  else {
    $otherid = $form_state['values']['otherid'];
  }
  $_SESSION['otherid'] = $otherid;
  unset($_SESSION['workflowresults']);
  islandora_prepare_ingest_dry_run_workflow($workflowid, $otherid);
}

function islandora_prepare_ingest_copy_workflow_form(array $form, array &$form_state, $workflowid) {
  $form += islandora_prepare_ingest_edit_workflow_form($form, $form_state, $workflowid);

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
    '#submit' => array('islandora_prepare_ingest_edit_workflow_form_copy_workflow'),
    '#validate' => array('islandora_prepare_ingest_edit_workflow_form_validate'),
  );
  unset($form['dryrun']);

  return $form;
}

function islandora_prepare_ingest_edit_workflow_form_copy_workflow(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');

  $name = $form_state['values']['workflowname'];
  $description = $form_state['values']['workflowdescription'];

  if (array_key_exists('workflowid', $form_state['values'])) {
    $oldworkflowid = $form_state['values']['workflowid'];
    $newworkflowid = islandora_prepare_ingest_copy_workflow($oldworkflowid);

    islandora_prepare_ingest_update_workflow($newworkflowid, array('name' => $name, 'description' => $description));

    $form_state['redirect'] = 'admin/islandora/prepare_ingest/edit/' . $newworkflowid;
  }
}

function islandora_prepare_ingest_export_workflow_form(array $form, array &$form_state, $workflowid) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');

  $form = array();
  $workflowjson = islandora_prepare_ingest_workflow_json($workflowid);

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
    //'#submit' => array('islandora_prepare_ingest_export_workflow_form_submit'),
  );

  return $form;
}

function islandora_prepare_ingest_export_workflow_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');

  $workflowid = $form_state['values']['workflowid'];
  $workflowjson = islandora_prepare_ingest_workflow_json($workflowid);
  $workflow = islandora_prepare_ingest_select_workflow($workflowid);
  $wfname = preg_replace('/[^a-zA-Z0-9]+/', '_', $workflow['name']);
  $filepath = "temporary://$wfname.json";
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

function islandora_prepare_ingest_ajax_addstep_handler($workflowid, $stepname, $modulename = '') {
  if ($workflowid && $stepname) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/steps.form');

    $stepid = islandora_prepare_ingest_insert_step($workflowid, $stepname, '');

    $workflowstep = NULL;
    $workflowsteps = islandora_prepare_ingest_select_workflow_steps($workflowid);
    foreach ($workflowsteps as $wfstep) {
      if ($wfstep['id'] === $stepid) {
        $workflowstep = $wfstep;
      }
    }

    $stepsinfo = module_invoke_all('workflow_step_info');
    $stepinfo = $stepsinfo[$stepname];
    $stepclass = $stepinfo['class name'];
    $step = new $stepclass($workflowstep, array(), NULL);

    $form = drupal_get_form('islandora_prepare_ingest_ajax_addstep_form', $step);
    return $form;
  }
  return array();
}

function islandora_prepare_ingest_ajax_addstep_form($form, &$form_state, $step) {
  $form = array(
    '#title' => t('Steps'),
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $stepconfig = array();
  $form['onestep'] = islandora_prepare_ingest_make_base_step_form($step, array());
  $form['onestep']['#collapsed'] = FALSE;

  return $form;
}

function islandora_prepare_ingest_ajax_addstepsgroup_handler($workflowid, $stepsfromworkflowid) {
  if ($workflowid && $stepsfromworkflowid) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/steps.form');

    $stepids = islandora_prepare_ingest_copy_steps($stepsfromworkflowid, $workflowid);
    $id2workflowstep = array();
    $workflowsteps = islandora_prepare_ingest_select_workflow_steps($workflowid);
    foreach ($workflowsteps as $wfstep) {
      $id2workflowstep[$wfstep['id']] = $wfstep;
    }
    $stepsinfo = module_invoke_all('workflow_step_info');

    $cfgs = array();
    foreach ($stepids as $stepid) {
      $step = $id2workflowstep[$stepid];
      $stepname = $step['step_name'];
      $cfgs[$stepid] = array(
        'workflowstep' => $step,
        'stepinfo' => $stepsinfo[$stepname],
        'stepname' => $stepname,
      );
    }
    $form = drupal_get_form('islandora_prepare_ingest_ajax_addstepsgroup_form', $cfgs);

    return $form;
  }
  return array();
}

function islandora_prepare_ingest_ajax_addstepsgroup_form($form, &$form_state, $cfgs) {
  $form = array(
    '#title' => t('Steps'),
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  foreach ($cfgs as $stepid => $cfg) {
    $stepconfig = islandora_prepare_ingest_select_workflow_step_configuration($stepid);
    $form['step_'.$stepid] = islandora_prepare_ingest_make_base_step_form($cfg['workflowstep'], $cfg['stepinfo'], $stepconfig, NULL, $stepid, $cfg['stepname']);
    $form['step_'.$stepid]['#collapsed'] = FALSE;
  }

  return $form;
}

function islandora_prepare_ingest_ajax_datacache_handler() {
  $query = drupal_get_query_parameters();
  $workflowid = isset($query['workflowid'])?$query['workflowid']:0;
  $stepid = isset($query['stepid'])?$query['stepid']:0;
  $otherid = isset($query['otherid'])?$query['otherid']:0;
  $type = isset($query['type'])?$query['type']:0;
  $startitemnr = isset($query['startitemnr'])?$query['startitemnr']:0;
  $enditemnr = isset($query['enditemnr'])?$query['enditemnr']:0;
  if ($workflowid && $stepid && $otherid && $type) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

    $count = islandora_prepare_ingest_data_cache_per_step_count($workflowid, $stepid, $otherid, $type);
    $result = islandora_prepare_ingest_data_cache_select($workflowid, $stepid, $otherid, $type, $startitemnr, $enditemnr);
    return '{ "count" : ' . $count . ', "list" : ' . json_encode(array_values($result)) . ' }';
  }
  return "{ }";
}

function islandora_prepare_ingest_ajax_files_handler() {
  $query = drupal_get_query_parameters();
  $workflowid = isset($query['workflowid'])?$query['workflowid']:0;
  $stepid = isset($query['stepid'])?$query['stepid']:0;
  $otherid = isset($query['otherid'])?$query['otherid']:0;
  $startitemnr = isset($query['startitemnr'])?$query['startitemnr']:0;
  $enditemnr = isset($query['enditemnr'])?$query['enditemnr']:0;
  if ($workflowid && $stepid && $otherid) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

    if (isset($query['path'])) {
      $entry = pifs_entry_for_filepath($query['path'], $workflowid, $stepid, $otherid);
      $result = NULL;
      if ($entry !== NULL) {
        if ($entry['valuetype'] & PIFS_IS_DIR) {
          $result = array('type' => 'directory');
        }
        else {
          if ($entry['valuetype'] & PIFS_FROM_ARCHIVE) {
            $result = array(
              'type' => 'file',
              'content' => pifs_archive_read_content_from_file($entry, $workflowid, $stepid, $otherid, 100000),
              'archivefilepath' => pifs_archive_get_archivepath($entry),
            );
          }
          elseif ($entry['valuetype'] & PIFS_HAS_CONTENT) {
            $result = array('type' => 'file', 'content' => $entry['filevalue']);
          }
          elseif ($entry['valuetype'] & PIFS_HAS_REAL) {
            $result = array('type' => 'file', 'realfilepath' => $entry['filevalue']);
          }
        }
        $result['removed'] = ($entry['valuetype'] & PIFS_IS_REMOVED)?TRUE:FALSE;
      }
      return json_encode($result);
    }
    else {
      $count = pifs_count_of_all_filepaths_for_entries($workflowid, $stepid, $otherid);
      $result = pifs_filepaths_for_entries($workflowid, $stepid, $otherid, $startitemnr, $enditemnr);
      return '{ "count" : ' . $count . ', "list" : ' . json_encode(array_values($result)) . ' }';
    }
  }
  return "{ }";
}

function islandora_prepare_ingest_ajax_delivery_handler($page_callback_result) {
  // Only render content and validates return
  $content = '';
  if (is_int($page_callback_result)) {
    // error in page callback result, fallback to delivering no content
  }
  else if (is_string($page_callback_result)) {
    $content = $page_callback_result;
  }
  else if (is_array($page_callback_result)) {
    $content = drupal_render($page_callback_result);
  }
  print $content;
  drupal_page_footer();
}

function islandora_prepare_ingest_report_result_summary($workflowresults) {
  $errorcount = 0;
  $warningcount = 0;
  if (is_array($workflowresults) && count($workflowresults) > 0) {
    foreach ($workflowresults as $stepid => $workflowres) {
      if (is_array($workflowres)) {
        foreach ($workflowres as $wftype => $wfres) {
          if (count($wfres) > 0) {
            foreach ($wfres as $res) {
              $type = $res['type'];
              if (strpos($type, 'error') !== FALSE || strpos($type, 'check') !== FALSE) {
                $errorcount++;
              }
              elseif (strpos($type, 'warn') !== FALSE) {
                $warningcount++;
              }
            }
          }
        }
      }
    }
  }
  $haserrors = (($errorcount + $warningcount) > 0);
  if ($errorcount > 0) {
    drupal_set_message(t('Dryrun failed with !count errors. See below for details.', array('!count' => $errorcount)), 'error', FALSE);
  }
  if ($warningcount > 0) {
    drupal_set_message(t('Dryrun failed with !count warnings. See below for details.', array('!count' => $warningcount)), 'warning', FALSE);
  }
  if (!$haserrors) {
    drupal_set_message(t('Dryrun finished successful!'));
  }
  return !$haserrors;
}

function islandora_prepare_ingest_manage_validate_configuration($form, &$form_state) {
  if (preg_match('/^[1-9][0-9]{0,2}$/', $form_state['values']['keep_cache_hours']) !== 1) {
    form_set_error('keep_cache_hours', t('Empty cache if older than should be a positive whole number below 1000'));
  }
}

function islandora_prepare_ingest_manage_save_configuration($form, &$form_state) {
  variable_set('islandora_prepare_ingest_per_run_data', $form_state['values']['per_run_data']);
  variable_set('islandora_prepare_ingest_keep_cache_hours', $form_state['values']['keep_cache_hours']);
}
