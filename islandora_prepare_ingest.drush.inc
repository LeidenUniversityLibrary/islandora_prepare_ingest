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
    if ($workflow === NULL) {
      return drush_set_error(dt("The workflow option should indicate the workflow id of a checked workflow."));
    }
  }
}

function drush_islandora_prepare_ingest_prepare_ingest() {
  module_load_include('inc', 'islandora_prepare_ingest', 'islandora_prepare_ingest.workflow_step');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/run_workflow');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');

  $workflowid = drush_get_option('workflow');
  if (!isset($workflowid)) {
    $allworkflows = islandora_prepare_ingest_select_workflows();
    $checkedworkflows = array();
    foreach ($allworkflows as $workflow) {
      if (isset($workflow['checked']) && $workflow['checked']) {
        $checkedworkflows[$workflow['id']] = dt("Workflow !id: !name", array('!name' => $workflow['name'], '!id' => $workflow['id']));
      }
    }
    $checkedworkflows['other'] = dt('Other workflows');
    $workflowid = drush_choice($checkedworkflows, dt("Choose a workflow to run:"));
    if ($workflowid === 'other') {
      $otherworkflows = array();
      foreach ($allworkflows as $workflow) {
        if (!(isset($workflow['checked']) && $workflow['checked'])) {
          $otherworkflows[$workflow['id']] = dt("Workflow !id: !name", array('!name' => $workflow['name'], '!id' => $workflow['id']));
        }
      }
      $workflowid = drush_choice($otherworkflows, dt("Choose a workflow to run:"));
    }
    if (!$workflowid) {
      return;
    }
  }

  $additionalconfig = NULL;
  $context = array();
  if (isset($workflowid)) {
    $workflow = islandora_prepare_ingest_select_workflow($workflowid);

    if (!$workflow['checked']) {
      if (!drush_confirm(dt("Workflow '@name' is not checked. Are you sure you want to use it?", array('@name' => $workflow['name'])))) {
        return;
      }
    }

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

    drush_log(dt("Starting full run..."), 'ok');
    if (variable_get('islandora_prepare_ingest_per_run_data', TRUE)) {
      $otherid = islandora_prepare_ingest_random_string(10);
    }
    else {
      $otherid = "drush$workflowid";
    }
    islandora_prepare_ingest_drush_dry_run_workflow($workflowid, $otherid, $additionalconfig);

    if (islandora_prepare_ingest_data_cache_count($workflowid, $otherid) > 0) {
      $context = array();
      $context['sandbox']['drush'] = TRUE;

      # Workflow steps for prepare ingest
      $haserror = FALSE;
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
        islandora_prepare_ingest_constants_add_to_stepconfig($stepconfig, $context);
        $step = new $stepclass($workflowstep, $stepconfig, $otherid, $prevstep);
        $prevstep = $step;

        $stepresult = $step->prepareIngest($context);

        if ($stepresult) {
          foreach ($stepresult as $sr) {
            if (strpos($sr['type'], 'error') !== FALSE || strpos($sr['type'], 'check') !== FALSE) {
              drush_log($sr['text'], 'error');
              $haserror = TRUE;
            }
            elseif (strpos($sr['type'], 'warn') !== FALSE) {
              drush_log($sr['text'], 'warning');
            }
            else {
             drush_log($sr['text'], 'ok');
            }
          }
        }
      }
      if (!$haserror) {
        drush_log(dt("Run finished succesfully"), 'success');
      }
    }
  }
}

