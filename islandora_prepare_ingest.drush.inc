<?php
/**
 * @file
 * Functions for prepare ingest via drush.
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
 * Implements hook_drush_command().
 */
function islandora_prepare_ingest_drush_command() {
  $items['prepare_ingest'] = array(
    'description' => dt('Prepare an ingest.'),
    'options' => array(
      'workflow' => dt('Optional, the workflow id of an active workflow.'),
    ),
    'aliases' => array('ipi'),
    'examples' => array(
      'drush prepare_ingest --workflow=1',
      'drush ipi --workflow=2',
    ),
  );
  return $items;
}

function drush_islandora_prepare_ingest_prepare_ingest_validate() {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  $workflowid = drush_get_option('workflow');

  global $user;
  if ($user->name === '') {
    return drush_set_error(dt("Use this command as a valid user: use --user=YOURUSERNAME"));
  }

  if (isset($workflowid)) {
    $workflow = islandora_prepare_ingest_select_workflow($workflowid);
    if ($workflow === NULL || !$workflow['checked']) {
      return drush_set_error(dt("The workflow option should indicate the workflow id of a checked workflow."));
    }
  }
}

function drush_islandora_prepare_ingest_prepare_ingest() {
  module_load_include('inc', 'islandora_prepare_ingest', 'islandora_prepare_ingest.workflow_step');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/run_workflow');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');

  $workflowid = drush_get_option('workflow');
  if (!isset($workflowid)) {
    $checkedworkflows = array();
    foreach (islandora_prepare_ingest_select_workflows() as $workflow) {
      if (isset($workflow['checked']) && $workflow['checked']) {
        $checkedworkflows[$workflow['id']] = $workflow['name'];
      }
    }
    $workflowid = drush_choice($checkedworkflows, dt("Choose a workflow to run:"));
    if (!$workflowid) {
      return;
    }
  }

  $additionalconfig = NULL;
  if (isset($workflowid)) {
    $workflow = islandora_prepare_ingest_select_workflow($workflowid);
    $stepsinfo =  module_invoke_all('workflow_step_info');
    $workflowsteps = islandora_prepare_ingest_select_workflow_steps($workflowid);

    $additionalconfig = array();
    foreach ($workflowsteps as $workflowstep) {
      $stepname = $workflowstep['step_name'];
      $stepid = $workflowstep['id'];
      $stepinfo = $stepsinfo[$stepname];
      $stepclass = $stepinfo['class name'];
      $stepconfig = islandora_prepare_ingest_select_workflow_step_configuration($stepid);
      $step = new $stepclass($workflowstep, $stepconfig, NULL);
      $stepargs = $step->arguments();
      foreach ($stepargs as $argname => $arginfo) {
        $prompt = $step->promptValueOnPrepareIngest($argname);
        if ($prompt !== FALSE) {
          $newvalue = drush_prompt($prompt, $step->valueForKey($argname));
          $additionalconfig[$stepid][$argname] = $newvalue;
        }
      }
    }
   
    drush_log(dt("Starting full run..."));
    $otherid = islandora_prepare_ingest_random_string(10);
    $steps = array();
    $context = array('drush' => TRUE);
    $results = islandora_prepare_ingest_drush_dry_run_workflow($workflowid, $otherid, $additionalconfig, $context, $steps);
    $haserror = _islandora_prepare_ingest_drush_show_results($results);
    if (!$haserror) {
      $results = islandora_prepare_ingest_drush_prepare_workflow($steps, $context);
      $haserror = _islandora_prepare_ingest_drush_show_results($results);
      if (!$haserror) {
        drush_log(dt("Run finished succesfully"), 'success');
      }
    }
  }
}

function _islandora_prepare_ingest_drush_show_results($results) {
  $haserror = FALSE;
  if (is_array($results) && count($results) > 0) {
    foreach ($results as $stepid => $res) {
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
  return $haserror;
}
