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
    $operations[] = array(
      'islandora_prepare_ingest_dry_run_step_batch_operation',
      array(
        $workflowstep,
        $stepsinfo,
        $otherid,
        $additionalconfig,
      ),
    );
  }

  return $operations;
}

function islandora_prepare_ingest_dry_run_clear_cache_operation($otherid, &$context) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

  $context['message'] = t('Clearing cache');
  # clear old cached data.
  islandora_prepare_ingest_data_cache_delete_old($otherid);
  islandora_prepare_ingest_pifs_delete_old($otherid);
}

function islandora_prepare_ingest_dry_run_step_batch_operation($workflowstep, $stepsinfo, $otherid, $additionalconfig, &$context) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');

  if (!isset($context['results']['_workflowid'])) {
    $context['results']['_workflowid'] = $workflowstep['workflow_id'];
    $context['results']['_otherid'] = $otherid;
    $context['results']['_by_type'] = array();
  }
  if (empty($context['sandbox'])) {
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
    islandora_prepare_ingest_constants_add_to_stepconfig($stepconfig, $context);

    $prevstep = isset($context['results']['_steps'])?$context['results']['_steps'][count($context['results']['_steps']) -1]:NULL;
    $step = new $stepclass($workflowstep, $stepconfig, $otherid, $prevstep);
    $context['results']['_steps'][] = $step;
    $context['sandbox']['_step'] = $step;

    $stepresult = $step->checkConfiguration($context);
    if ($stepresult) {
      $context['results']['_by_type'][$step->id()]['check'] = $stepresult;
    }
    if (islandora_prepare_ingest_has_error($stepresult)) {
      $context['results']['_has_error'] = TRUE;
      return;
    }
  }
  else {
    $step = $context['sandbox']['_step'];
  }
  if (isset($context['results']['_has_error']) && $context['results']['_has_error']) {
    // Don't do dry run of the step if previous steps had any errors.
    return;
  }
  $context['message'] = t("Doing step !stepnr: !steplabel", array('!stepnr' => $step->weight(), '!steplabel' => $step->label()));
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
  if (isset($results['_by_type'], $results['_workflowid'])) {
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
}

function islandora_prepare_ingest_drush_dry_run_workflow($workflowid, $otherid, array $additionalconfig) {
  //module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  //module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');
  //module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');
  //module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

  if ($otherid === NULL) {
    $otherid = islandora_prepare_ingest_random_string(10);
  }

  $operations = islandora_prepare_ingest_dry_run_operations($workflowid, $otherid, $additionalconfig);

  $batch = array(
    'title' => t('Full run'),
    'operations' => $operations,
    'finished' => 'islandora_prepare_ingest_drush_dry_run_step_batch_finished',
    'file' => drupal_get_path('module', 'islandora_prepare_ingest') . '/includes/run_workflow.inc',
  );

  batch_set($batch);

  $batch = &batch_get();
  $batch['progressive'] = FALSE;

  drush_backend_batch_process();
}
/*
function islandora_prepare_ingest_drush_full_run_operation(&$context) {
  $steps = $context['results']['_steps'];
  if (empty($context['sandbox'])) {
    $context['sandbox']['drush'] = TRUE;
    $context['sandbox']['current'] = 0;
    $context['sandbox']['maxsteps'] = count($steps);
  }
  if (empty($steps) || (isset($context['results']['_has_error']) && $context['results']['_has_error'])) {
    $context['finished'] = 1;
    return;
  }

  $step = $steps[$context['sandbox']['current']];
  $context['sandbox']['current']++;

  $stepresult = $step->prepareIngest($context);

  if ($stepresult) {
    $context['results']['_by_type'][$step->id()]['prepare'] = $stepresult;
  }
  if (islandora_prepare_ingest_has_error($stepresult)) {
    $context['results']['_has_error'] = TRUE;
  }

  $context['finished'] = $context['sandbox']['current'] / $context['sandbox']['maxsteps'];
}
*/
function islandora_prepare_ingest_drush_dry_run_step_batch_finished($success, $results, $operations) {
  $haserror = FALSE;
  if (is_array($results['_by_type']) && count($results['_by_type']) > 0) {
    foreach ($results['_by_type'] as $stepid => $res) {
      foreach ($res as $type => $infos) {
        foreach ($infos as $info) {
          if (strpos($info['type'], 'error') !== FALSE || strpos($info['type'], 'check') !== FALSE) {
            drush_log($info['text'], 'error');
            $haserror = TRUE;
          }
          elseif (strpos($info['type'], 'warn') !== FALSE) {
            drush_log($info['text'], 'warning');
          }
          else {
           drush_log($info['text'], 'ok');
          }
        }
      }
    }
  }
  if (count($results['_steps']) == 0) {
    drush_log(dt("Workflow contains no steps"), 'error');
  }
  else if (!$haserror) {
    drush_log(dt("Dry run finished succesfully, continuing with prepare ingest."), "ok");
  }
  else {
    drush_log(dt("Full run failed."), 'error');
    if (isset($results['_otherid'])) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

      islandora_prepare_ingest_data_cache_delete_old($results['_otherid']);
    }
  }
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
