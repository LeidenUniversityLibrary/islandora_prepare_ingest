<?php

function islandora_batch_set_purge_items_page_callback($id, $management = FALSE) {
  return drupal_get_form('islandora_batch_purge_set_form', $id, $management);
}

function islandora_batch_purge_set_form($form, &$form_state, $id, $management) {
  form_load_include($form_state, 'inc', 'islandora_batch', 'includes/menu');
  return confirm_form(
    $form,
    t('Are you sure you want to purge all the items in the set "@set"?', array(
      '@set' => $id,
    )),
    $management ? "admin/reports/islandora_batch_queue/$id" : "islandora_batch/reports/queue/$id"
  );
}

function islandora_batch_purge_set_form_submit(&$form, &$form_state) {
  module_load_include('inc', 'islandora_batch', 'includes/db');
  list($id, $management) = $form_state['build_info']['args'];
  
  /* what items to purge */
  $purged = 0;
  $operations = array();
  $itemids = islandora_batch_get_queued_items_for_set($id);
  foreach ($itemids as $itemid) {
    $operations[] = array('islandora_batch_purge_single_item_operation',array($itemid));
    $purged++;
  }

  $batch = array(
    'finished' => 'islandora_batch_purge_set_finished',
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'title' => format_plural($purged, 'Purging 1 item.', 'Purging @count items.'),
    'file' => drupal_get_path('module', 'ubl_batch_purge_items') . '/includes/purge.inc',
    'operations' => $operations,
  );

  $form_state['redirect'] = array(
    $management ? 'admin/reports/islandora_batch_sets' : 'islandora_batch/reports/set',
  );

  batch_set($batch);

}

function islandora_batch_delete_single_item_operation($itemid, AbstractObject $collection, &$context) {
  if ($object = islandora_object_load($itemid)) {
    module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');

    $context['message'] = t('Deleting item with id @pid', array('@pid' => $itemid));

    $other_parents = islandora_basic_collection_get_other_parents($object, $collection);
    // If only one parent, delete the object.
    if (empty($other_parents)) {
      islandora_delete_object($object);
    }
    else {
      // If more than one parent, only remove parent relationship.
      islandora_basic_collection_remove_from_collection($object, $collection);
    }

    $context['results'][] = $itemid;
  }
  else {
    $context['message'] = t('Item with id @pid already deleted', array('@pid' => $itemid));
  }
}

function islandora_batch_delete_set_finished($success, $results, $operations) {
  ubl_batch_delete_purge_set_finished('Deleted', $success, $results, $operations);
}

function islandora_batch_purge_single_item_operation($itemid, &$context) {
  if ($object = islandora_object_load($itemid)) {
    $context['message'] = t('Purging item with id @pid', array('@pid' => $itemid));

    islandora_delete_object($object);

    $context['results'][] = $itemid;
  }
  else {
    $context['message'] = t('Item with id @pid already purged', array('@pid' => $itemid));
  }
}

function islandora_batch_purge_set_finished($success, $results, $operations) {
  ubl_batch_delete_purge_set_finished('Purged', $success, $results, $operations);
}
function ubl_batch_delete_purge_set_finished($verb, $success, $results, $operations) {
  if ($success) {
    drupal_set_message(format_plural(count($results), $verb.' 1 item.', $verb.' @count items.'));
  }
  else {
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
      '%error_operation' => $error_operation[0],
      '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}
