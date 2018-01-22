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

function islandora_prepare_ingest_dry_run_workflow($workflowid, $otherid = NULL, array $additionalconfig = NULL) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

  $results = array();
  $data = array();
  $context = array();
  $hasconfigerror = FALSE;

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

    //$results[$stepid]['label'] = $step->label();
  
    if ($otherid) {
      $stepresult = $step->checkConfiguration();
      if ($stepresult) {
        $results[$stepid]['check'] = $stepresult;
        $hasconfigerror = TRUE;
      }
      if (!$hasconfigerror) {
        $step->dryRun($context);
        $stepresult = $step->verify($context);
        if ($stepresult) {
          $results[$stepid]['verify'] = $stepresult;
        }
      }
    } 
  }
  return $results;
}

function islandora_prepare_ingest_full_run_workflow($workflowid, $otherid = NULL) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

  if ($otherid === NULL) {
    $otherid = islandora_prepare_ingest_random_string(10);
    $results = islandora_prepare_ingest_dry_run_workflow($workflowid, $otherid);
    if ($results) {
      // something went wrong...
      return $results;
    }
  }
  $count = islandora_prepare_ingest_data_cache_count($workflow_id, $other_id);
  if ($count === 0) {
    // no data for this workflow at all 
    $results[] = array(
      array(
        'text' => t("Workflow @workflowid does not have any data for run '@otherid'",
                     array('@workflowid' => $workflow_id, '@otherid' => $otherid)),
        'type' => 'run_error'
      ),
    );
  }
  # Workflow steps
  $stepsinfo = module_invoke_all('workflow_step_info');
  $workflowsteps = islandora_prepare_ingest_select_workflow_steps($workflowid);
  foreach ($workflowsteps as $workflowstep) {
    $stepname = $workflowstep['step_name'];
    $stepinfo = $stepsinfo[$stepname];
    $stepclass = $stepinfo['class name'];
    $stepid = $workflowstep['id'];
    $stepconfig = islandora_prepare_ingest_select_workflow_step_configuration($stepid);
    $stepconfig = islandora_prepare_ingest_constants_add_to_stepconfig($stepconfig, $context);

    $step = new $stepclass($stepinfo, $stepconfig, $workflowid, $stepid, $otherid);

    $stepresult = $step->prepareIngest($context);
    if ($stepresult) {
      $results[$stepid] = $stepresult;
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
