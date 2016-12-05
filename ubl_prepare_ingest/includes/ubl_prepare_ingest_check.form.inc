<?php
error_reporting(E_ALL);
ini_set("display_errors", 1);


/**
 * @file
 * ubl_prepare_ingest_check.form.inc
 */

/**
 * The form for making a workflow
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function ubl_prepare_ingest_check_workflow_form(array $form, array &$form_state, $workflowid, $istesting) {
  $form = array();

  $workflow = array();
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/db');
  $workflow = ubl_prepare_ingest_select_workflow($workflowid);

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
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/ubl_prepare_ingest.steps.form');
  $workflowresults = isset($_SESSION['workflowresults'])?$_SESSION['workflowresults']:array();
  $workflowdata = isset($_SESSION['workflowdata'])?$_SESSION['workflowdata']:array();
  $form += ubl_prepare_ingest_steps($workflowid, $workflowresults, $workflowdata, $istesting?'test':'check');
 
  # back
  $form['back'] = array(
    '#type' => 'submit',
    '#value' => t('Edit workflow'),
    '#submit' => array('ubl_prepare_ingest_check_workflow_form_back'),
  );
  # Test workflow
  $form['test_workflow'] = array(
    '#type' => 'submit',
    '#value' => t('Test workflow'),
    '#id' => 'test_workflow',
  );

  return $form;
}

function ubl_prepare_ingest_check_workflow_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/ubl_prepare_ingest.run_workflow');

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
    $workflowresults = ubl_prepare_ingest_run_workflow($workflowid, $workflowdata, WORK_EXTENT_DRY_RUN, $testdata);
    ubl_prepare_ingest_report_result_summary($workflowid, 'test', $workflowresults);
    $_SESSION['workflowresults'] = $workflowresults;
    $_SESSION['workflowdata'] = $workflowdata;

    $form_state['redirect'] = 'admin/islandora/ubl_prepare_ingest/manage/test/' . $form_state['values']['workflowid'];
  }
}

function ubl_prepare_ingest_check_workflow_form_back(array $form, array &$form_state) {
  unset($_SESSION['workflowresults']);
  unset($_SESSION['workflowdata']);
  $form_state['redirect'] = 'admin/islandora/ubl_prepare_ingest/manage/edit/' . $form_state['values']['workflowid'];
}
