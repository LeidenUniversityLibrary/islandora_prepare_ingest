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
  }

  $additionalconfig = NULL;
  if (isset($workflowid)) {
    $workflow = islandora_prepare_ingest_select_workflow($workflowid);
    $stepsinfo =  module_invoke_all('workflow_step_info');
    $workflowsteps = islandora_prepare_ingest_select_workflow_steps($workflowid);

    foreach ($workflowsteps as $workflowstep) {
      $stepname = $workflowstep['step_name'];
      $stepid = $workflowstep['id'];
      $stepinfo = $stepsinfo[$stepname];
      $stepclass = $stepinfo['class name'];
      $stepconfig = islandora_prepare_ingest_select_workflow_step_configuration($stepid);
      $step = new $stepclass($workflowstep, $stepconfig, $otherid);
      $stepargs = $step->arguments();
      foreach ($stepargs as $argname => $arginfo) {
        $prompt = $step->promptValueOnPrepareIngest($argname);
        if ($prompt !== FALSE) {
          $newvalue = drush_prompt($prompt, $step->valueForKey($argname));
          if (!is_array($additionalconfig)) {
            $additionalconfig = array();
          }
          $additionalconfig[$stepid][$argname] = $newvalue;
        }
      }
    }
   
    drush_log(dt("Starting full run..."));
    $results = islandora_prepare_ingest_full_run_workflow($workflowid, $additionalconfig);
    if (is_array($results) && count($results) > 0) {
      foreach ($results as $stepid => $res) {
        foreach ($res as $type => $error) {
          drupal_log($error['text'], 'error');
        }
      }
    }
    else {
      drush_log(dt("Run finished succesfully"), 'success');
    }
  }
}
