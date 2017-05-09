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

function islandora_prepare_ingest_run_workflow($workflowid, $otherid, $extent, array $additionalconfig = NULL) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

  $results = array();
  $data = array();
  $context = array();

  # clear old cached data.
  islandora_prepare_ingest_data_cache_delete_old($otherid);

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

    $jsonvalues = array();
    foreach ($data as $current => $d) {
      $d['item nr'] = $current;
      $jsonvalues[] = json_encode($d);
    }
    islandora_prepare_ingest_data_cache_insert($workflowid, $stepid, $otherid, 1, $jsonvalues);

    if (array_key_exists('_PIFS', $context)) {
      $files = $context['_PIFS'];
      sort($files);
      $jsonfiles = array();
      foreach ($files as $current => $d) {
        $d['file nr'] = $current;
        $jsonfiles[] = json_encode($d);
      } 
      islandora_prepare_ingest_data_cache_insert($workflowid, $stepid, $otherid, 2, $jsonfiles);
    }
  }
  return $results;
}

function islandora_prepare_ingest_full_run_workflow($workflowid) {
  module_load_include('inc', 'islandora_prepare_ingest', 'workflow_step');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');

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
    $stepconfig = islandora_prepare_ingest_constants_add_to_stepconfig($stepconfig, $context);
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

function islandora_prepare_ingest_random_string($length, $keyspace = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ') {
  $str = '';
  $max = mb_strlen($keyspace, '8bit') - 1;
  for ($i = 0; $i < $length; ++$i) {
    $str .= $keyspace[rand(0, $max)];
  }
  return $str;
}
