<?php

/**
 * @file
 * Database interface.
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
 * Select all workflows in the database
 */
function islandora_prepare_ingest_select_workflows($type = NULL, $negate = FALSE) {
  $results = array();
  $select = db_select('islandora_prepare_ingest_workflow', 'workflow')
    ->fields('workflow');
  if ($type) {
    $select = $select->condition('type', $type, $negate?'<>':'=');
  }
  $workflows = $select->execute();
  while ($workflow = $workflows->fetchAssoc()) {
    $results[] = $workflow;
  }
  return $results;
}

/**
 * Get a query for a specific workflow in the database
 */
function islandora_prepare_ingest_select_workflow_query($workflowid) {
  return db_select('islandora_prepare_ingest_workflow', 'workflow')
    ->fields('workflow')
    ->condition('id', $workflowid);
}
/**
 * Select a specific workflow in the database
 */
function islandora_prepare_ingest_select_workflow($workflowid) {
  $workflow = islandora_prepare_ingest_select_workflow_query($workflowid)->execute();
  return $workflow->fetchAssoc();
}

/**
 * Insert a new workflow in the database with the given name and descritpion
 */
function islandora_prepare_ingest_insert_workflow($name, $description, $type = 'partial') {
  $insert = db_insert('islandora_prepare_ingest_workflow')
    ->fields(array(
      'name' => $name,
      'description' => $description,
      'type' => $type,
    ));
  return $insert->execute();
}

/**
 * Copy a workflow in the database with the given name and descritpion
 */
function islandora_prepare_ingest_copy_workflow($tocopyworkflowid) {
  // First copy the workflow itself
  $selquery = db_select('islandora_prepare_ingest_workflow', 'workflow')
    ->fields('workflow', array('name', 'description', 'type'))
    ->condition('id', $tocopyworkflowid);
  $insert = db_insert('islandora_prepare_ingest_workflow')
    ->fields(array('name', 'description', 'type'))
    ->from($selquery);
  $workflowid = $insert->execute();

  islandora_prepare_ingest_copy_steps($tocopyworkflowid, $workflowid);

  return $workflowid; 
}

function islandora_prepare_ingest_max_step_weight($workflowid) {
  $select = db_select('islandora_prepare_ingest_step', 'step')
    ->condition('step.workflow_id',$workflowid);
  $select->addExpression('MAX(weight)');
  $weight = $select->execute()->fetchField(); 

  return $weight + 1;
}

function islandora_prepare_ingest_copy_steps($fromworkflowid, $toworkflowid) {
  $tocopysteps = islandora_prepare_ingest_select_workflow_steps($fromworkflowid);
  $newstepids = array();
  $newweight = islandora_prepare_ingest_max_step_weight($toworkflowid);
  foreach ($tocopysteps as $tocopystep) {
    $insert = db_insert('islandora_prepare_ingest_step')
      ->fields(array(
        'workflow_id' => $toworkflowid,
        'step_name' => $tocopystep['step_name'],
        'step_module' => $tocopystep['step_module'],
        'weight' => $newweight,
      ));
    $newweight += 1;
    $copiedstepid = $insert->execute();
    $newstepids[] = $copiedstepid;
    
    $selquery = db_select('islandora_prepare_ingest_step_config', 'config');
    $selquery->addExpression($copiedstepid,'step_id');
    $selquery = $selquery->fields('config', array('config_key', 'config_value'))
      ->condition('config.step_id',$tocopystep['id']);
    $insert = db_insert('islandora_prepare_ingest_step_config')
      ->fields(array('config_key','config_value','step_id'))
      ->from($selquery);
    $insert->execute();
  }
  return $newstepids;
}

/**
 * Update a workflow with the given id in the database with the new values
 */
function islandora_prepare_ingest_update_workflow($workflowid, array $values) {
  db_update('islandora_prepare_ingest_workflow')
    ->fields($values)
    ->condition('id', $workflowid)
    ->execute();
}

/**
 * Delete a workflow with the given id in the database
 */
function islandora_prepare_ingest_delete_workflow($workflowid) {
  islandora_prepare_ingest_delete_workflow_steps($workflowid);
  db_delete('islandora_prepare_ingest_workflow')
    ->condition('id', $workflowid)
    ->execute();
}

/**
 * Query to select all steps of a specific workflow in the database
 */
function islandora_prepare_ingest_select_workflow_steps_query($workflowid) {
  return db_select('islandora_prepare_ingest_step', 'step')
    ->fields('step')
    ->condition('step.workflow_id',$workflowid)
    ->orderBy('weight');
}

/**
 * Select all steps of a specific workflow in the database
 */
function islandora_prepare_ingest_select_workflow_steps($workflowid) {
  $results = array();
  $steps = islandora_prepare_ingest_select_workflow_steps_query($workflowid)->execute();
  while ($step = $steps->fetchAssoc()) {
    $results[] = $step;
  }
  return $results;
}

/**
 * Insert a step to a workflow in the database with the given name and description
 */
function islandora_prepare_ingest_insert_step($workflowid, $name, $module) {
  $newweight = islandora_prepare_ingest_max_step_weight($workflowid);
  $insert = db_insert('islandora_prepare_ingest_step')
    ->fields(array(
      'workflow_id' => $workflowid,
      'step_name' => $name,
      'step_module' => $module,
      'weight' => $newweight,
    ));
  return $insert->execute();
}

/**
 * Update a step with the given id in the database with the new values
 */
function islandora_prepare_ingest_update_step($stepid, $workflowid, array $values) {
  db_update('islandora_prepare_ingest_step')
    ->fields($values)
    ->condition('id', $stepid)
    ->condition('workflow_id', $workflowid)
    ->execute();
}

/**
 * Delete a step with the given id in the database
 */
function islandora_prepare_ingest_delete_step($stepid, $workflowid) {
  db_delete('islandora_prepare_ingest_step')
    ->condition('id', $stepid)
    ->condition('workflow_id', $workflowid)
    ->execute();
  islandora_prepare_ingest_delete_step_configuration($stepid);
}

/**
 * Delete all steps from a workflow with the given id in the database
 */
function islandora_prepare_ingest_delete_workflow_steps($workflowid) {
  $steps = islandora_prepare_ingest_select_workflow_steps($workflowid);
  foreach ($steps as $step) {
    $stepid = $step['id'];
    islandora_prepare_ingest_delete_step($stepid, $workflowid);
  }
}

/**
 * Select step configuration of a specific step in the database
 */
function islandora_prepare_ingest_select_workflow_step_configuration($stepid) {
  $results = array();
  $configs = db_select('islandora_prepare_ingest_step_config', 'config')
    ->fields('config')
    ->condition('step_id',$stepid)
    ->execute();
  while ($keyvalue = $configs->fetchAssoc()) {
    $results[$keyvalue['config_key']] = $keyvalue['config_value'];
  }
  return $results;
}

/**
 * Insert a configuration to a step in the database with key and value 
 */
function islandora_prepare_ingest_insert_step_configuration($stepid, $key, $value) {
  $insert = db_insert('islandora_prepare_ingest_step_config')
    ->fields(array(
      'step_id' => $stepid,
      'config_key' => $key,
      'config_value' => $value,
    ));
  return $insert->execute();
}

/**
 * Update a specific configuration for a key of a step with the given id in the database with the new value
 */
function islandora_prepare_ingest_update_step_configuration($stepid, $key, $value) {
  db_update('islandora_prepare_ingest_step_config')
    ->fields(array(
     'config_value' => $value,
    ))
    ->condition('step_id', $stepid)
    ->condition('config_key', $key)
    ->execute();
}

/**
 * Delete the step configuration with the given id in the database
 */
function islandora_prepare_ingest_delete_step_configuration($stepid) {
  db_delete('islandora_prepare_ingest_step_config')
    ->condition('step_id', $stepid)
    ->execute();
}

/**
 * Return the workflow data as JSON
 */
function islandora_prepare_ingest_workflow_json($workflowid) {
  $result = array('version' => '1.0');
  $workflowdata = islandora_prepare_ingest_select_workflow($workflowid);
  unset($workflowdata['id']);
  $result = array_merge($result, $workflowdata); 
  $steps = islandora_prepare_ingest_select_workflow_steps($workflowid);
  $result['steps'] = array();
  foreach ($steps as $step) {
    $stepid =  $step['id'];
    unset($step['id']);
    unset($step['workflow_id']);

    $stepconfig = islandora_prepare_ingest_select_workflow_step_configuration($stepid);
    $step['config'] = $stepconfig;
    $result['steps'][] = $step;
  }

  return drupal_json_encode($result);
}

/**
 * Insert a workflow based on workflow data as JSON
 */
function islandora_prepare_ingest_insert_new_workflow_from_json($json) {
  $data = drupal_json_decode($json);
  
  if ($data === NULL) {
    return FALSE;
  }
  if ($data['version'] !== '1.0') {
    return FALSE;
  }

  $wfname = $data['name'];
  $wfdesc = $data['description'];
  $wftype = $data['type'];
  $wfsteps = $data['steps']; 

  if (!(is_string($wfname) && strlen($wfname) > 0)) {
    return FALSE;
  }
  if ($wftype !== 'partial' && $wftype !== 'full' && $wftype !== 'active') {
    return FALSE;
  }
  if (!is_array($wfsteps)) {
    return FALSE;
  }
  $steps = array();
  usort($wfsteps, function($a, $b) { return ($a['weight'] < $b['weight'])?-1:1; });
  foreach ($wfsteps as $wfstep) {
    $stepmodule = $wfstep['step_module']; 
    $stepname = $wfstep['step_name']; 
    $stepconfig = $wfstep['config']; 
    if (!(is_string($stepname) && strlen($stepname) > 0)) {
      return FALSE;
    }
    if (!is_array($stepconfig)) {
      return FALSE;
    }
    $steps[] = array('step_name' => $stepname, 'step_module' => $stepmodule, 'config' => $stepconfig);
  }
  
  $wfid = islandora_prepare_ingest_insert_workflow($wfname, $wfdesc, $wftype);
  foreach ($steps as $step) {
    $stepname = $step['step_name'];
    $stepmodule = $step['step_module'];
    $stepid = islandora_prepare_ingest_insert_step($wfid, $stepname, $stepmodule);
    foreach ($step['config'] as $key => $value) {
      islandora_prepare_ingest_insert_step_configuration($stepid, $key, $value); 
    }
  }

  return TRUE;
}
