<?php

/**
 * @file
 * active_workflows.form.inc
 */

/**
 * The form for setting up one prepare ingest workflow
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_prepare_ingest_active_workflows_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');

  $workflows = islandora_prepare_ingest_select_workflows('active');

  if (count($workflows) > 0) {
    $form['active_workflows'] = array(
      '#title' => t('Active workflows'),
      '#type' => 'fieldset',
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );
    $options = array();
    foreach ($workflows as $workflow) {
      $options[$workflow['id']] = array(
	'name' => l($workflow['name'],'admin/islandora/islandora_prepare_ingest/active_workflow/' . $workflow['id'] . '/edit'),
      );
    }
    $form['active_workflows']['table'] = array(
      '#type' => 'tableselect',
      '#header' => array(
	'name' => t('Name'),
      ),
      '#options' => $options,
    );

    $form['active_workflows']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete selected workflow(s)'),
      '#submit' => array('islandora_prepare_ingest_prepare_delete_selected_workflows'),
    );
  }

  $form['add_workflow'] = array(
    '#title' => t('Add workflow'),
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $options = array();
  $workflows = islandora_prepare_ingest_select_workflows('full');
  if (count($workflows) > 0) {
    $options[''] = t('Choose workflow');
    foreach ($workflows as $workflow) {
      if ($workflow['checked']) {
        $options[$workflow['id']] = $workflow['name'];
      }
    }
    $form['add_workflow']['choosen_workflow'] = array(
      '#type' => 'select',
      '#options' => $options,
    );
    $form['add_workflow']['new_workflow'] = array(
      '#type' => 'submit',
      '#value' => t('Add New Workflow'),
      '#submit' => array('islandora_prepare_ingest_manage_add_active_workflow'),
    );
  }
  else {
    $form['add_workflow']['choosen_workflow'] = array(
      '#type' => 'item',
      '#description' => t('Make sure at least one full workflow is available'),
    );
  }
  return $form;
}

function islandora_prepare_ingest_manage_add_active_workflow(array $form, array &$form_state) {
  $oldworkflowid = $form_state['values']['choosen_workflow'];
  if ($oldworkflowid) {
    $newworkflowid = islandora_prepare_ingest_copy_workflow($oldworkflowid);
    islandora_prepare_ingest_update_workflow($newworkflowid, array('type' => 'active'));

    $form_state['redirect'] = 'admin/islandora/islandora_prepare_ingest/active_workflow/' . $newworkflowid . '/edit';
  }
}

function islandora_prepare_ingest_active_workflow_form(array $form, array &$form_state, $workflowid, $action) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');

  $form = array();
  $workflow = islandora_prepare_ingest_select_workflow($workflowid);

  $form['workflowid'] = array(
    '#type' => 'hidden',
    '#value' => $workflowid,
  );

  if ($action === 'activeedit') {
    # name, description
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

    module_load_include('inc', 'islandora_prepare_ingest', 'includes/steps.form');
    $workflowresults = isset($_SESSION['workflowresults'])?$_SESSION['workflowresults']:array();
    $workflowdata = isset($_SESSION['workflowdata'])?$_SESSION['workflowdata']:array();
    $form += islandora_prepare_ingest_steps($workflowid, $workflowresults, $workflowdata, $action);

    # back
    $form['back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
      '#submit' => array('islandora_prepare_ingest_active_workflow_form_back'),
    );
    # Save
    $form['save'] = array(
      '#type' => 'submit',
      '#value' => t('Save workflow'),
      '#id' => 'save',
    );
    # Dry run
    $form['check'] = array(
      '#type' => 'submit',
      '#value' => t('Dry run'),
      '#id' => 'dryrun',
    );
  }
  elseif ($action === 'dryrun') {
    # name, description and type
    $form['workflowinfo'] = array(
      '#title' => $workflow['name'],
      '#description' => $workflow['description'],
      '#type' => 'fieldset',
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );

    module_load_include('inc', 'islandora_prepare_ingest', 'includes/steps.form');
    $workflowresults = isset($_SESSION['workflowresults'])?$_SESSION['workflowresults']:array();
    $workflowdata = isset($_SESSION['workflowdata'])?$_SESSION['workflowdata']:array();
    $form += islandora_prepare_ingest_steps($workflowid, $workflowresults, $workflowdata, $action);

    # back
    $form['back'] = array(
      '#type' => 'submit',
      '#value' => t('Back'),
      '#submit' => array('islandora_prepare_ingest_active_workflow_form_back_dryrun'),
    );
  }

  return $form;
}

function islandora_prepare_ingest_active_workflow_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/run_workflow');

  $name = $form_state['values']['workflowname'];
  $description = $form_state['values']['workflowdescription'];
  $workflowid = $form_state['values']['workflowid'];

  islandora_prepare_ingest_update_workflow($workflowid,array(
    'name' => $name,
    'description' => $description,
  ));
  $updates = array();
  foreach ($form_state['values'] as $key => $value) {
    if (strpos($key, 'step_') === 0) {
      $parts = explode('_', $key, 3);
      if (count($parts) == 3) {
	$stepidstr = $parts[1];
	$configkey = $parts[2];
	if (($stepid = filter_var($stepidstr, FILTER_VALIDATE_INT)) !== FALSE) {
	  if (strcmp($form['steps']['step_' . $stepid]['fields'][$key]['#default_value'] , $value) != 0) {
	    $updates[$stepid][$configkey] = $value;
	  }
	}
      }
    }
  }
  foreach ($updates as $stepid => $update) {
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

  
  if ($form_state['clicked_button']['#id'] === 'dryrun') {
    $workflowdata = array();
    $workflowresults = islandora_prepare_ingest_run_workflow($workflowid, $workflowdata, WORK_EXTENT_DRY_RUN);
    islandora_prepare_ingest_report_result_summary($workflowid, 'dryrun', $workflowresults);
    $_SESSION['workflowresults'] = $workflowresults;
    $_SESSION['workflowdata'] = $workflowdata;
    $form_state['redirect'] = 'admin/islandora/islandora_prepare_ingest/active_workflow/' . $workflowid . '/dryrun';
  }
}

function islandora_prepare_ingest_active_workflow_form_back(array $form, array &$form_state) {
  unset($_SESSION['workflowresults']);
  unset($_SESSION['workflowdata']);
  $form_state['redirect'] = 'admin/islandora/islandora_prepare_ingest/active_workflows';
}

function islandora_prepare_ingest_active_workflow_form_back_dryrun(array $form, array &$form_state) {
  unset($_SESSION['workflowresults']);
  unset($_SESSION['workflowdata']);
  $workflowid = $form_state['values']['workflowid'];
  $form_state['redirect'] = 'admin/islandora/islandora_prepare_ingest/active_workflow/' . $workflowid . '/edit';
}

function islandora_prepare_ingest_prepare_delete_selected_workflows(array $form, array &$form_state) {
  if (array_key_exists("table", $form_state["values"])) {
    foreach ($form_state["values"]["table"] as $workflowid => $delete) {
      if ($delete) {
        islandora_prepare_ingest_delete_workflow($workflowid);
      }
    }
  }
}
