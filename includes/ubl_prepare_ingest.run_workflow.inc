<?php

/**
 * @file
 * ubl_prepare_ingest.run_workflow.inc
 */

/**
 * Run the steps of a workflow
 *
 * @param $workflowid
 *   A workflow ID.
 * @param $extent
 *   The extent of the work to do during the run of the workflow.
 * @param array $additionalconfig
 *   Any additional configuration, associated by step id..
 */

function ubl_prepare_ingest_run_workflow($workflowid, array &$workflowdata, $extent, array $additionalconfig = NULL) {
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/db');
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/constants');

  $results = array();
  $data = array();
  $context = array();

  # Workflow steps
  $stepsinfo = module_invoke_all('workflow_step_info');
  $workflowsteps = ubl_prepare_ingest_select_workflow_steps($workflowid);
  foreach ($workflowsteps as $workflowstep) {
    $stepname = $workflowstep['step_name'];
    $stepid = $workflowstep['id'];
    $stepconfig = ubl_prepare_ingest_select_workflow_step_configuration($stepid);
    if (isset($additionalconfig[$stepid])) {
      foreach ($additionalconfig[$stepid] as $k => $v) {
        $stepconfig[$k] = $v;
      }
    }
    $stepconfig = ubl_prepare_ingest_constants_add_to_stepconfig($stepconfig, $context);
    $stepfunc = ubl_prepare_ingest_callback_for_step($stepname, 'work');
    $results[$stepid] = $stepfunc($stepsinfo[$stepname], $stepconfig, $data, $context, $extent);

    $workflowdata[$stepid]['data'] = $data;
    $workflowdata[$stepid]['context'] = $context;
  }
  return $results;
}


function ubl_prepare_ingest_full_run_workflow($workflowid) {
  module_load_include('inc', 'ubl_prepare_ingest', 'ubl_prepare_ingest.workflow_step');
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/db');

  $extent = WORK_EXTENT_DO_ALL;

  $data = array();
  $context = array();

  $haserrors = FALSE;

  # Workflow steps
  $stepsinfo = module_invoke_all('workflow_step_info');
  $workflowsteps = ubl_prepare_ingest_select_workflow_steps($workflowid);
  foreach ($workflowsteps as $workflowstep) {
    $stepname = $workflowstep['step_name'];
    $stepid = $workflowstep['id'];
    $stepconfig = ubl_prepare_ingest_select_workflow_step_configuration($stepid);
    $stepfunc = ubl_prepare_ingest_callback_for_step($stepname, 'work');
    $results = $stepfunc($stepsinfo[$stepname], $stepconfig, $data, $context, $extent);

    if (is_array($results) && count($results) > 0) {
      foreach ($results as $result) {
        if (strpos($result['type'], 'error') !== FALSE) {
          drush_log($result['text'], 'error');
          $haserrors = TRUE;
        }
        elseif (strpos($result['type'], 'warn') !== FALSE) {
          drush_log($result['text'], 'warning');
        }
        else {
          drush_log($result['text'], 'notice');
        }
      }
    }
  }
  if (!$haserrors) {
    drush_log(t('Prepare ingest completed successfully!'), 'success');
  }
}


