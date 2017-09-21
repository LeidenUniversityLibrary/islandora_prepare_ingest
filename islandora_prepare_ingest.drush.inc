<?php
/**
 * @file
 * Functions for prepare ingest via drush.
 *
 *
 *  Copyright 2017 Leiden University Library
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
    'description' => 'Prepare an ingest.',
    'options' => array(
      'workflow' => 'The workflow id of an active workflow.',
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

  if (preg_match("/^\d+$/", $workflowid) != 1) {
    return drush_set_error("The workflow option should be a number indicating the workflow id of an active workflow.");
  }
  $workflow = islandora_prepare_ingest_select_workflow($workflowid);
  if ($workflow === NULL || $workflow['type'] !== 'active') {
    return drush_set_error("The workflow option should indicate the workflow id of an active workflow.");
  }
}

function drush_islandora_prepare_ingest_prepare_ingest() {
  module_load_include('inc', 'islandora_prepare_ingest', 'islandora_prepare_ingest.workflow_step');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/run_workflow');

  $workflowid = drush_get_option('workflow');

  islandora_prepare_ingest_full_run_workflow($workflowid);
}
