<?php

/**
 * @file
 * Data cache.
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
 * Select query for data cache from a specific workflow, step, other id, of type and within min and max item nr.
 */
function _islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type) {
  $select = db_select('islandora_prepare_ingest_data_cache', 'datacache')
    ->fields('datacache', array('value'))
    ->condition('workflow_id', $workflow_id)
    ->condition('step_id', $step_id)
    ->condition('other_id', $other_id)
    ->condition('type', $type);

  return $select;
}

/**
 * Select data cache from a specific workflow, step, other id, of type and within min and max item nr.
 */
function islandora_prepare_ingest_data_cache_select($workflow_id, $step_id, $other_id, $type, $min_item_nr, $max_item_nr) {
  $select = _islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type);
  $select = $select->condition('item_nr', array($min_item_nr, $max_item_nr), 'BETWEEN');
  $dcs = $select->execute();
  $results = array();
  while ($dc = $dcs->fetchAssoc()) {
    $results[] = $dc['value'];
  }
  return $results;
}

/**
 * Count data cache items from a specific workflow, step, other id and of type.
 */
function islandora_prepare_ingest_data_cache_count($workflow_id, $step_id, $other_id, $type) {
  $select = _islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type);
  $countstr = $select->countQuery()->execute()->fetchField();

  return intval($countstr, 10);
}

/**
 * Insert data cache values.
 */
function islandora_prepare_ingest_data_cache_insert($workflow_id, $step_id, $other_id, $type, $values) {
  $fields = array('workflow_id', 'step_id', 'other_id', 'item_nr', 'timestamp', 'type', 'value');
  $insert = db_insert('islandora_prepare_ingest_data_cache')->fields($fields);
  $item_nr = 1;
  $timestamp = REQUEST_TIME;
  foreach ($values as $value) {
    $record = array(
      'workflow_id' => $workflow_id,
      'step_id'     => $step_id,
      'other_id'    => $other_id,
      'type'        => $type,
      'item_nr'     => $item_nr,
      'timestamp'   => $timestamp,
      'value'       => $value,
    );
    $insert->values($record);
    $item_nr++;
  }
  $insert->execute();
}

/**
 * Delete all old data cache values with a specific other id or that are more than 1 day old.
 */
function islandora_prepare_ingest_data_cache_delete_old($other_id = NULL) {
  if ($other_id !== NULL) {
    db_delete('islandora_prepare_ingest_data_cache')
      ->condition('other_id', $other_id)
      ->execute();
  }
  $timestamp = REQUEST_TIME - (24*60*60);
  db_delete('islandora_prepare_ingest_data_cache')
    ->condition('timestamp', $timestamp, '<')
    ->execute();
}

