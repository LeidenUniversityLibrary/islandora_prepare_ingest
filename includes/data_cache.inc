<?php

/**
 * @file
 * Data cache.
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
 * Select query for data cache from a specific workflow, step, other id and of type.
 */
function islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type, $fields = array()) {
  $select = db_select('islandora_prepare_ingest_data_cache', 'datacache');
  if ($fields !== NULL) {
    $select->fields('datacache', $fields);
  }
  $select->condition('workflow_id', $workflow_id)
    ->condition('other_id', $other_id)
    ->condition('dctype', $type);
  if ($step_id !== NULL) {
    $select->condition('step_id', $step_id);
  }

  return $select;
}

/**
 * Select data cache from a specific workflow, step, other id and of type and returns iterator over this data.
 */
function islandora_prepare_ingest_data_cache_select_iterator($workflow_id, $step_id, $other_id, $type) {
  $select = islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type, array('item_nr', 'dckey', 'dcvalue'));
  $select = $select->orderBy('item_nr');

  return $select->execute();
}

/**
 * Select data cache from a specific workflow, step, other id, of type and within min and max item nr.
 */
function islandora_prepare_ingest_data_cache_select($workflow_id, $step_id, $other_id, $type, $min_item_nr, $max_item_nr) {
  $select = islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type, array('item_nr', 'dckey', 'dcvalue'));
  $select = $select->orderBy('item_nr');
  $select = $select->condition('item_nr', array($min_item_nr, $max_item_nr), 'BETWEEN');
  $dcs = $select->execute();
  $results = array();
  while ($dc = $dcs->fetchAssoc()) {
    $inr = $dc['item_nr'];
    $k = $dc['dckey'];
    $v = $dc['dcvalue'];
    if (!isset($results[$inr])) {
      $results[$inr] = array();
    }
    $results[$inr][$k] = unserialize($v);
  }
  return $results;
}

/**
 * Select data cache from a specific workflow, other id, of type, and with a specific key and value.
 **/
function islandora_prepare_ingest_data_cache_select_key_value($workflow_id, $other_id, $type, $key, $value, $startmatch = FALSE) {
  $servalue = db_like(serialize($value));
  if ($startmatch) {
    $servalue = preg_replace(array('/^s:\d+:/','/";$/'), array('s:%:', '%'), $servalue);
  }
  $select = islandora_prepare_ingest_data_cache_select_query($workflow_id, NULL, $other_id, $type, array('item_nr', 'step_id', 'dckey', 'dcvalue'));
  $select = $select->orderBy('item_nr');
  $select = $select->condition('dckey', $key, 'LIKE');
  $select = $select->condition('dcvalue', $servalue, 'LIKE');
  $dcs = $select->execute();
  $results = array();
  while ($dc = $dcs->fetchAssoc()) {
    $results[] = array(
      'step_id' => $dc['step_id'],
      'item_nr' => $dc['item_nr'],
      'key' => $dc['dckey'],
      'value' => unserialize($dc['dcvalue']),
    );
  }
  return $results;
}

/**
 * Count data cache items from a specific workflow, step, other id and of type.
 */
function islandora_prepare_ingest_data_cache_count($workflow_id, $other_id) {
  $fields = array('item_nr');
  $select = db_select('islandora_prepare_ingest_data_cache', 'datacache')
    ->fields('datacache', $fields)
    ->condition('workflow_id', $workflow_id)
    ->condition('other_id', $other_id);

  $select->addExpression('COUNT(DISTINCT item_nr)');
  $countstr = $select->execute()->fetchField();

  return intval($countstr, 10);
}

/**
 * Count data cache items from a specific workflow, step, other id and of type.
 */
function islandora_prepare_ingest_data_cache_per_step_count($workflow_id, $step_id, $other_id, $type) {
  $select = islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type, NULL);
  $r = $select->addExpression('COUNT(DISTINCT item_nr)');
  $countstr = $select->execute()->fetchField();
  return intval($countstr, 10);
}

/**
 * Count data cache items from a specific workflow, step, other id, of type and key.
 */
function islandora_prepare_ingest_data_cache_per_step_key_count($workflow_id, $step_id, $other_id, $type, $key) {
  $select = islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type, NULL);
  $select = $select->condition('dckey', $key);
  $r = $select->addExpression('COUNT(DISTINCT item_nr)');
  $countstr = $select->execute()->fetchField();
  return intval($countstr, 10);
}

/**
 * Count data cache items from a specific workflow, for all steps by type.
 */
function islandora_prepare_ingest_data_cache_counts($workflow_id, $other_id) {
  $fields = array('step_id', 'dctype');
  $select = db_select('islandora_prepare_ingest_data_cache', 'datacache')
    ->fields('datacache', $fields)
    ->condition('workflow_id', $workflow_id)
    ->condition('other_id', $other_id)
    ->groupBy('step_id')
    ->groupBy('dctype');
  $select->addExpression('COUNT(*)', 'count');
  $data = $select->execute();

  $results = array();
  while ($dc = $data->fetchAssoc()) {
    $results[$dc['step_id']][$dc['dctype']] = $dc['count'];
  }
  return $results;
}

/**
 * Insert data cache values.
 */
function islandora_prepare_ingest_data_cache_insert($workflow_id, $step_id, $other_id, $type, $items) {
  $fields = array('workflow_id', 'step_id', 'other_id', 'item_nr', 'dctimestamp', 'dctype', 'dckey', 'dcvalue');
  $insert = db_insert('islandora_prepare_ingest_data_cache')->fields($fields);
  $item_nr = 1;
  $timestamp = REQUEST_TIME;
  foreach ($items as $item) {
    foreach ($item as $key => $value) {
      $record = array(
        'workflow_id' => $workflow_id,
        'step_id'     => $step_id,
        'other_id'    => $other_id,
        'item_nr'     => $item_nr,
        'dctimestamp' => $timestamp,
        'dctype'      => $type,
        'dckey'       => $key,
        'dcvalue'     => serialize($value),
      );
      $insert->values($record);
    }
    if (!isset($item['item nr'])) {
      $record = array(
        'workflow_id' => $workflow_id,
        'step_id'     => $step_id,
        'other_id'    => $other_id,
        'item_nr'     => $item_nr,
        'dctimestamp' => $timestamp,
        'dctype'      => $type,
        'dckey'       => 'item nr',
        'dcvalue'     => serialize($item_nr),
      );
      $insert->values($record);
    }
    $item_nr++;
  }
  $insert->execute();
}

/**
 * Insert single data cache item.
 */
function islandora_prepare_ingest_data_cache_single_insert($workflow_id, $step_id, $other_id, $type, $item_nr, $item) {
  $fields = array('workflow_id', 'step_id', 'other_id', 'item_nr', 'dctimestamp', 'dctype', 'dckey', 'dcvalue');
  $insert = db_insert('islandora_prepare_ingest_data_cache')->fields($fields);
  $timestamp = REQUEST_TIME;
  foreach ($item as $key => $value) {
    $record = array(
      'workflow_id' => $workflow_id,
      'step_id'     => $step_id,
      'other_id'    => $other_id,
      'item_nr'     => $item_nr,
      'dctimestamp' => $timestamp,
      'dctype'      => $type,
      'dckey'       => $key,
      'dcvalue'     => serialize($value),
    );
    $insert->values($record);
  }
  if (!isset($item['item nr'])) {
    $record = array(
      'workflow_id' => $workflow_id,
      'step_id'     => $step_id,
      'other_id'    => $other_id,
      'item_nr'     => $item_nr,
      'dctimestamp' => $timestamp,
      'dctype'      => $type,
      'dckey'       => 'item nr',
      'dcvalue'     => serialize($item_nr),
    );
    $insert->values($record);
  }
  $insert->execute();
}

/**
 * Update single data cache item.
 */
function islandora_prepare_ingest_data_cache_single_update($workflow_id, $step_id, $other_id, $type, $item_nr, $item) {
  $fields = array('workflow_id', 'step_id', 'other_id', 'item_nr', 'dctimestamp', 'dctype', 'dckey', 'dcvalue');
  $timestamp = REQUEST_TIME;
  foreach ($item as $key => $value) {
    $update = db_update('islandora_prepare_ingest_data_cache')
     ->fields(array(
        'workflow_id' => $workflow_id,
        'step_id'     => $step_id,
        'other_id'    => $other_id,
        'item_nr'     => $item_nr,
        'dctimestamp' => $timestamp,
        'dctype'      => $type,
        'dckey'       => $key,
        'dcvalue'     => serialize($value),
      ))
     ->condition('workflow_id', $workflow_id)
     ->condition('step_id', $step_id)
     ->condition('other_id', $other_id)
     ->condition('dctype', $type)
     ->condition('item_nr', $item_nr)
     ->condition('dckey', $key, 'LIKE');
    $update->execute();
  }
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
    ->condition('dctimestamp', $timestamp, '<')
    ->execute();
}

