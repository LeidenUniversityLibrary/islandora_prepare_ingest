<?php

/**
 * @file
 * run_workflow.inc
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

function islandora_prepare_ingest_run_workflow($workflowid, array &$workflowdata, $extent, array $additionalconfig = NULL) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');

  $results = array();
  $data = array();
  $context = array();

  # Workflow steps
  $stepsinfo = module_invoke_all('workflow_step_info');
  $workflowsteps = islandora_prepare_ingest_select_workflow_steps($workflowid);
  foreach ($workflowsteps as $workflowstep) {
    $stepname = $workflowstep['step_name'];
    $stepid = $workflowstep['id'];
    $stepconfig = islandora_prepare_ingest_select_workflow_step_configuration($stepid);
    if (isset($additionalconfig[$stepid])) {
      foreach ($additionalconfig[$stepid] as $k => $v) {
        $stepconfig[$k] = $v;
      }
    }
    $stepconfig = islandora_prepare_ingest_constants_add_to_stepconfig($stepconfig, $context);
    $stepfunc = islandora_prepare_ingest_callback_for_step($stepname, 'work');
    $results[$stepid] = $stepfunc($stepsinfo[$stepname], $stepconfig, $data, $context, $extent);

    $limiteddata = array();
    $limitbytype = array();
    foreach ($data as $d) {
      $type = implode('_', array_keys($d));
      if (!array_key_exists($type, $limitbytype)) {
        $limitbytype[$type] = 0;
      }
      $limitbytype[$type]++;
      if ($limitbytype[$type] <= 6) {
        $limiteddata[] = $d;
      }
    }

    $workflowdata[$stepid]['data'] = $limiteddata;
    $workflowdata[$stepid]['context'] = array_key_exists('_PIFS', $context)?array('_PIFS' => $context['_PIFS']):array();
  }
  return $results;
}


function islandora_prepare_ingest_full_run_workflow($workflowid) {
  module_load_include('inc', 'islandora_prepare_ingest', 'workflow_step');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');

  $extent = WORK_EXTENT_DO_ALL;

  $data = array();
  $context = array();

  $haserrors = FALSE;

  # Workflow steps
  $stepsinfo = module_invoke_all('workflow_step_info');
  $workflowsteps = islandora_prepare_ingest_select_workflow_steps($workflowid);
  foreach ($workflowsteps as $workflowstep) {
    $stepname = $workflowstep['step_name'];
    $stepid = $workflowstep['id'];
    $stepconfig = islandora_prepare_ingest_select_workflow_step_configuration($stepid);
    $stepfunc = islandora_prepare_ingest_callback_for_step($stepname, 'work');
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


