<?php

/**
 * @file
 * Data cache.
 */

/**
 * Select query for data cache from a specific workflow, step, other id and of type.
 */
function _islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type, $fields = NULL) {
  if (!$fields) {
    $fields = array('item_nr', 'key', 'value');
  }
  $select = db_select('islandora_prepare_ingest_data_cache', 'datacache')
    ->fields('datacache', $fields)
    ->condition('workflow_id', $workflow_id)
    ->condition('step_id', $step_id)
    ->condition('other_id', $other_id)
    ->condition('type', $type);

  return $select;
}

/**
 * Select data cache from a specific workflow, step, other id and of type and returns iterator over this data.
 */
function islandora_prepare_ingest_data_cache_select_iterator($workflow_id, $step_id, $other_id, $type) {
  $select = _islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type);
  $select = $select->orderBy('item_nr');

  return $select->execute();
}

/**
 * Select data cache from a specific workflow, step, other id, of type and within min and max item nr.
 */
function islandora_prepare_ingest_data_cache_select($workflow_id, $step_id, $other_id, $type, $min_item_nr, $max_item_nr) {
  $select = _islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type);
  $select = $select->orderBy('item_nr');
  $select = $select->condition('item_nr', array($min_item_nr, $max_item_nr), 'BETWEEN');
  $dcs = $select->execute();
  $results = array();
  while ($dc = $dcs->fetchAssoc()) {
    $inr = $dc['item_nr'];
    $k = $dc['key'];
    $v = $dc['value'];
    if (!isset($results[$inr])) {
      $results[$inr] = array();
    }
    $results[$inr][$k] = unserialize($v);
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

  $countstr = $select->addExpression('COUNT(DISTINCT item_nr)')->execute()->fetchField();

  return intval($countstr, 10);
}

/**
 * Count data cache items from a specific workflow, step, other id and of type.
 */
function islandora_prepare_ingest_data_cache_per_step_count($workflow_id, $step_id, $other_id, $type) {
  $select = _islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type, array('item_nr'));
  $countstr = $select->addExpression('COUNT(DISTINCT item_nr)')->execute()->fetchField();

  return intval($countstr, 10);
}

/**
 * Count data cache items from a specific workflow, step, other id, of type and key.
 */
function islandora_prepare_ingest_data_cache_per_step_key_count($workflow_id, $step_id, $other_id, $type, $key) {
  $select = _islandora_prepare_ingest_data_cache_select_query($workflow_id, $step_id, $other_id, $type, array('item_nr'));
  $select = $select->condition('key', $key);
  $countstr = $select->addExpression('COUNT(DISTINCT item_nr)')->execute()->fetchField();

  return intval($countstr, 10);
}

/**
 * Insert data cache values.
 */
function islandora_prepare_ingest_data_cache_insert($workflow_id, $step_id, $other_id, $type, $items) {
  $fields = array('workflow_id', 'step_id', 'other_id', 'item_nr', 'timestamp', 'type', 'key', 'value');
  $insert = db_insert('islandora_prepare_ingest_data_cache')->fields($fields);
  $item_nr = 1;
  $timestamp = REQUEST_TIME;
  foreach ($items as $item) {
    foreach ($item as $key => $value) {
      $record = array(
        'workflow_id' => $workflow_id,
        'step_id'     => $step_id,
        'other_id'    => $other_id,
        'type'        => $type,
        'item_nr'     => $item_nr,
        'timestamp'   => $timestamp,
        'key'         => $key,
        'value'       => serialize($value),
      );
      $insert->values($record);
    }
    $record = array(
      'workflow_id' => $workflow_id,
      'step_id'     => $step_id,
      'other_id'    => $other_id,
      'type'        => $type,
      'item_nr'     => $item_nr,
      'timestamp'   => $timestamp,
      'key'         => 'item nr',
      'value'       => serialize($item_nr),
    );
    $insert->values($record);

    $item_nr++;
  }
  $insert->execute();
}

/**
 * Insert single data cache item.
 */
function islandora_prepare_ingest_data_cache_single_insert($workflow_id, $step_id, $other_id, $type, $item_nr, $item) {
  $fields = array('workflow_id', 'step_id', 'other_id', 'item_nr', 'timestamp', 'type', 'key', 'value');
  $insert = db_insert('islandora_prepare_ingest_data_cache')->fields($fields);
  $timestamp = REQUEST_TIME;
  foreach ($item as $key => $value) {
    $record = array(
      'workflow_id' => $workflow_id,
      'step_id'     => $step_id,
      'other_id'    => $other_id,
      'type'        => $type,
      'item_nr'     => $item_nr,
      'timestamp'   => $timestamp,
      'key'         => $key,
      'value'       => serialize($value),
    );
    $insert->values($record);
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

