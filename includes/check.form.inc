<?php

/**
 * @file
 * check.form.inc
 */

/**
 * The form for making a workflow
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_prepare_ingest_check_workflow_form(array $form, array &$form_state, $workflowid, $istesting) {
  $form = array();

  $workflow = array();
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  $workflow = islandora_prepare_ingest_select_workflow($workflowid);

  $form['workflowid'] = array(
    '#type' => 'hidden',
    '#value' => $workflowid,
  );
 
  # name, description and type
  $form['workflowinfo'] = array(
    '#title' => $workflow['name'],
    '#description' => $workflow['description'],
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  # steps
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/steps.form');
  $workflowresults = isset($_SESSION['workflowresults'])?$_SESSION['workflowresults']:array();
  $workflowdata = isset($_SESSION['workflowdata'])?$_SESSION['workflowdata']:array();
  $form += islandora_prepare_ingest_steps($workflowid, $workflowresults, $workflowdata, $istesting?'test':'check');
 
  # back
  $form['back'] = array(
    '#type' => 'submit',
    '#value' => t('Edit workflow'),
    '#submit' => array('islandora_prepare_ingest_check_workflow_form_back'),
  );
  # Test workflow
  $form['test_workflow'] = array(
    '#type' => 'submit',
    '#value' => t('Test workflow'),
    '#id' => 'test_workflow',
  );

  return $form;
}

function islandora_prepare_ingest_check_workflow_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/run_workflow');

  $workflowid = $form_state['values']['workflowid'];
  if ($form_state['clicked_button']['#id'] === 'test_workflow') {
    $testdata = array();
    foreach ($form_state['values'] as $k => $v) {
      if (strpos($k, 'step_') === 0) {
        $parts = explode('_', $k, 3);
        if (count($parts) == 3) {
          $stepidstr = $parts[1];
          $configkey = $parts[2];
          if (($stepid = filter_var($stepidstr, FILTER_VALIDATE_INT)) !== FALSE) {
            $testdata[$stepid][$configkey] = $v;
          }
        }
      }
    }
    $workflowdata = array();
    $otherid = islandora_prepare_ingest_random_string(10);
    $workflowresults = islandora_prepare_ingest_run_workflow($workflowid, $otherid, WORK_EXTENT_DRY_RUN, $testdata);
    islandora_prepare_ingest_report_result_summary($workflowid, 'test', $workflowresults);
    $_SESSION['workflowresults'] = $workflowresults;
    $_SESSION['otherid'] = $otherid;

    $form_state['redirect'] = 'admin/islandora/islandora_prepare_ingest/generic_workflow/test/' . $form_state['values']['workflowid'];
  }
}

function islandora_prepare_ingest_check_workflow_form_back(array $form, array &$form_state) {
  unset($_SESSION['workflowresults']);
  unset($_SESSION['workflowdata']);
  $form_state['redirect'] = 'admin/islandora/islandora_prepare_ingest/generic_workflow/edit/' . $form_state['values']['workflowid'];
}
