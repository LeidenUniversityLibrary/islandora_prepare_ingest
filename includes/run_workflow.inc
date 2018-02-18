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
    }
    if (islandora_prepare_ingest_has_error($stepresult)) {
      $hasconfigerror = TRUE;
      break;
    }
    if (!$hasconfigerror) {
      $stepresult = $step->dryRun($context);
      if ($stepresult) {
        $results[$stepid]['dryrun'] = $stepresult;
      }
      if (islandora_prepare_ingest_has_error($stepresult)) {
        break;
      }
      $stepresult = $step->verify($context);
      if ($stepresult) {
        $results[$stepid]['verify'] = $stepresult;
      }
      if (islandora_prepare_ingest_has_error($stepresult)) {
        break;
      }
      if ($isfullrun) {
        $stepresult = $step->prepareIngest($context);
        if ($stepresult) {
          $results[$stepid]['prepare'] = $stepresult;
        }
        if (islandora_prepare_ingest_has_error($stepresult)) {
          break;
        }
      }
    }
  }
  return $results;
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
