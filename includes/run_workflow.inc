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

function islandora_prepare_ingest_dry_run_workflow($workflowid, $otherid) {
  return _islandora_prepare_ingest_run_workflow($workflowid, FALSE, $otherid);
}

function islandora_prepare_ingest_full_run_workflow($workflowid, array $additionalconfig = NULL) {
  return _islandora_prepare_ingest_run_workflow($workflowid, TRUE, NULL, $additionalconfig);
}

function _islandora_prepare_ingest_run_workflow($workflowid, $isfullrun = FALSE, $otherid = NULL, array $additionalconfig = NULL) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

  $context = array();
  $results = array();
  $data = array();
  $hasconfigerror = FALSE;

  if ($otherid === NULL) {
    $otherid = islandora_prepare_ingest_random_string(10);
  }

  # clear old cached data.
  islandora_prepare_ingest_data_cache_delete_old($otherid);

  # Workflow steps
  $prevstep = NULL;
  $stepsinfo = module_invoke_all('workflow_step_info');
  $workflowsteps = islandora_prepare_ingest_select_workflow_steps($workflowid);
  foreach ($workflowsteps as $workflowstep) {
    $stepname = $workflowstep['step_name'];
    $stepinfo = $stepsinfo[$stepname];
    $stepclass = $stepinfo['class name'];
    $stepid = $workflowstep['id'];
    $stepconfig = islandora_prepare_ingest_select_workflow_step_configuration($stepid);
    if (isset($additionalconfig[$stepid])) {
      foreach ($additionalconfig[$stepid] as $k => $v) {
        $stepconfig[$k] = $v;
      }
    }
    $stepconfig = islandora_prepare_ingest_constants_add_to_stepconfig($stepconfig, $context);

    $step = new $stepclass($workflowstep, $stepconfig, $otherid, $prevstep);
    $prevstep = $step;

    $stepresult = $step->checkConfiguration();
    if ($stepresult) {
      $results[$stepid]['check'] = $stepresult;
      $hasconfigerror = TRUE;
      break;
    }
    if (!$hasconfigerror) {
      $stepresult = $step->dryRun($context);
      if ($stepresult) {
        $results[$stepid]['dryrun'] = $stepresult;
        break;
      }
      $stepresult = $step->verify($context);
      if ($stepresult) {
        $results[$stepid]['verify'] = $stepresult;
        break;
      }
      if ($isfullrun) {
        $stepresult = $step->prepareIngest($context);
        if ($stepresult) {
          $results[$stepid]['prepare'] = $stepresult;
          break;
        }
      }
    }
  }
  return $results;
}

function islandora_prepare_ingest_random_string($length, $keyspace = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ') {
  $str = '';
  $max = mb_strlen($keyspace, '8bit') - 1;
  for ($i = 0; $i < $length; ++$i) {
    $str .= $keyspace[rand(0, $max)];
  }
  return $str;
}
