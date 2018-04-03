<?php

/**
 * @file
 * run_workflow.inc
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
  //module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');
  //module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');
  //module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

  $operations = islandora_prepare_ingest_dry_run_operations($workflowid, $otherid);

  $batch = array(
    'title' => t('Dry run'),
    'operations' => $operations,
    'finished' => 'islandora_prepare_ingest_dry_run_step_batch_finished',
    'file' => drupal_get_path('module', 'islandora_prepare_ingest') . '/includes/run_workflow.inc',
  );

  batch_set($batch);
}

function islandora_prepare_ingest_dry_run_operations($workflowid, $otherid, $additionalconfig = array()) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');

  # Workflow steps
  $prevstep = NULL;
  $stepsinfo = module_invoke_all('workflow_step_info');
  $workflowsteps = islandora_prepare_ingest_select_workflow_steps($workflowid);

  $operations = array();
  $operations[] = array(
    'islandora_prepare_ingest_dry_run_clear_cache_operation',
    array(
      $otherid,
    ),
  );
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

    $operations[] = array(
      'islandora_prepare_ingest_dry_run_step_batch_operation',
      array(
        $step,
      ),
    );
  }

  return $operations;
}

function islandora_prepare_ingest_dry_run_clear_cache_operation($otherid, &$context) {
  $context['message'] = t('Clearing cache');
  # clear old cached data.
  islandora_prepare_ingest_data_cache_delete_old($otherid);
  islandora_prepare_ingest_pifs_delete_old($otherid);
}

function islandora_prepare_ingest_dry_run_step_batch_operation($step, &$context) {
  $context['message'] = t("Doing step !stepnr '!steplabel'", array('!stepnr' => $step->weight(), '!steplabel' => $step->label()));
  if (!isset($context['results']['_workflowid'])) {
    $context['results']['_workflowid'] = $step->workflowId();
    $context['results']['_by_type'] = array();
  }
  if (empty($context['sandbox'])) {
    $stepresult = $step->checkConfiguration($context);
    if ($stepresult) {
      $context['results']['_by_type'][$step->id()]['check'] = $stepresult;
    }
    if (islandora_prepare_ingest_has_error($stepresult)) {
      $context['results']['_has_error'] = TRUE;
      return;
    }
  }
  if (isset($context['results']['_has_error']) && $context['results']['_has_error']) {
    // Don't do dry run of the step if previous steps had any errors.
    return;
  }
  $stepresult = $step->dryRun($context);
  if ($stepresult) {
    if (isset($context['results']['_by_type'][$step->id()]['dryrun'])) {
      $context['results']['_by_type'][$step->id()]['dryrun'] = $context['results']['_by_type'][$step->id()]['dryrun'] + $stepresult;
    }
    else {
      $context['results']['_by_type'][$step->id()]['dryrun'] = $stepresult;
    }
  }
  if (islandora_prepare_ingest_has_error($stepresult)) {
    $context['results']['_has_error'] = TRUE;
    return;
  }
  if (!isset($context['finished']) || ($context['finished'] >= 1)) {
    $stepresult = $step->verify($context);
    if ($stepresult) {
      $context['results']['_by_type'][$step->id()]['verify'] = $stepresult;
    }
    if (islandora_prepare_ingest_has_error($stepresult)) {
      $context['results']['_has_error'] = TRUE;
      return;
    }
  }
}

function islandora_prepare_ingest_dry_run_step_batch_finished($success, $results, $operations) {
  $_SESSION['workflowresults'] = $results['_by_type'];

  $haserrors = (isset($results['_has_error']) && $results['_has_error']);

  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  $workflowid = $results['_workflowid']; 
  if ($haserrors) {
    islandora_prepare_ingest_update_workflow($workflowid, array('checked' => 0));
  }
  else {
    islandora_prepare_ingest_update_workflow($workflowid, array('checked' => 1));
  }
}

function islandora_prepare_ingest_drush_dry_run_workflow($workflowid, $otherid, array $additionalconfig, &$context, array &$dryrunsteps) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

  $context = array('drush' => TRUE);
  $results = array();

  if ($otherid === NULL) {
    $otherid = islandora_prepare_ingest_random_string(10);
  }

  # clear old cached data.
  islandora_prepare_ingest_data_cache_delete_old($otherid);
  islandora_prepare_ingest_pifs_delete_old($otherid);

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

    drush_log(dt("Doing step !stepnr '!steplabel'", array('!stepnr' => $step->weight(), '!steplabel' => $step->label())), 'notice');

    $results[$stepid] = array();
    if (_islandora_prepare_ingest_run_workflow_step($step, $context, $results[$stepid])) {
      $dryrunsteps[] = $step;
    }
    else {
      break;
    }
  }
  return $results;
}

function islandora_prepare_ingest_drush_prepare_workflow($steps, &$context) {
  $results = array();
  foreach ($steps as $step) {
    $stepresult = $step->prepareIngest($context);
    if ($stepresult) {
      $results[$stepid]['prepare'] = $stepresult;
    }
    if (islandora_prepare_ingest_has_error($stepresult)) {
      break;
    }
  }
  return $results;
}

function _islandora_prepare_ingest_run_workflow_step($step, &$context, &$results) {
  $stepresult = $step->checkConfiguration($context);
  if ($stepresult) {
    $results['check'] = $stepresult;
  }
  if (islandora_prepare_ingest_has_error($stepresult)) {
    return FALSE;
  }
  $stepresult = $step->dryRun($context);
  if ($stepresult) {
    $results['dryrun'] = $stepresult;
  }
  if (islandora_prepare_ingest_has_error($stepresult)) {
    return FALSE;
  }
  $stepresult = $step->verify($context);
  if ($stepresult) {
    $results['verify'] = $stepresult;
  }
  if (islandora_prepare_ingest_has_error($stepresult)) {
    return FALSE;
  }
  return TRUE; 
}

function islandora_prepare_ingest_has_error($stepresult) {
  if ($stepresult) {
    foreach ($stepresult as $sr) {
      if (strpos($sr['type'], 'error') !== FALSE) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

function islandora_prepare_ingest_random_string($length, $keyspace = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ') {
  $str = '';
  $max = mb_strlen($keyspace, '8bit') - 1;
  for ($i = 0; $i < $length; ++$i) {
    $str .= $keyspace[rand(0, $max)];
  }
  return $str;
}
