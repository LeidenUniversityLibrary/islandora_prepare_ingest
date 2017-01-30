<?php

/**
 * @file
 * Function to retrieve the children and missing datastreams info
 */


/**
 * Function that retrieves an associative array of all children of the parent.
 */
function ubl_check_datastreams_retrieve_all_child_ids($parent, &$childids) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_batch', 'includes/db');

  if ($parent &&  in_array('islandora:collectionCModel', $parent->models)) {
    list($count, $colobjs) = islandora_basic_collection_get_member_objects($parent, -1, -1);
    foreach ($colobjs as $colobj) {
      $cid = $colobj['object']['value'];
      if (!isset($childids[$cid])) {
	$obj = islandora_object_load($cid);
	if ($obj) {
	  $childids[$cid] = $cid;
	  ubl_check_datastreams_retrieve_all_child_ids($obj, $childids);
        }
      }
    }
  }
  if ($parent &&  in_array('islandora:compoundCModel', $parent->models)) {
    $compoundparts = islandora_compound_object_get_parts($parent->id);
    foreach ($compoundparts as $partpid) {
      if (!isset($childids[$partpid])) {
	$obj = islandora_object_load($partpid);
	if ($obj) {
	  $childids[$partpid] = $partpid;
	  ubl_check_datastreams_retrieve_all_child_ids($obj, $childids);
        }
      }
    }
  }
  if ($parent &&  in_array('islandora:bookCModel', $parent->models)) {
    $pages = islandora_paged_content_get_pages($parent);
    foreach ($pages as $pagepid => $pageinfo) {
      if (!isset($childids[$pagepid])) {
	$childids[$pagepid] = $pagepid;
      }
    }
  }
  return $childids;
}

/**
 * Function that returns an associative array by cmodel, dsid and count.
 */
function ubl_check_datastreams_dsids_by_cmodel($childids) {
  $dsidsbycmodel = array();
  foreach ($childids as $childid) {
    $child = islandora_object_load($childid);
    if ($child) {
      $cmodels = ubl_check_datastreams_get_cmodel_string($child);

      $info = ubl_check_datastreams_datastream_property_info($child);
      foreach ($info as $dsid => $dsinfo) {
        if (!isset($dsidsbycmodel[$cmodels])) {
          $dsidsbycmodel[$cmodels] = array();
        }
        if (!isset($dsidsbycmodel[$cmodels][$dsid])) {
          $dsidsbycmodel[$cmodels][$dsid] = array();
        }
        if (!isset($dsidsbycmodel[$cmodels][$dsid]['total'])) {
          $dsidsbycmodel[$cmodels][$dsid]['total'] = 0;
        }
        if (!isset($dsidsbycmodel[$cmodels][$dsid]['found'])) {
          $dsidsbycmodel[$cmodels][$dsid]['found'] = 0;
        }
        $dsidsbycmodel[$cmodels][$dsid]['total']++;
        $dsidsbycmodel[$cmodels][$dsid]['isrequired'] = $dsinfo['isrequired'];
        if ($dsinfo['isavailable']) {
          $dsidsbycmodel[$cmodels][$dsid]['found']++;
        }
      }
      $child->refresh();
      unset($child);
    }
  }
  return $dsidsbycmodel;
}

/**
 * Function that returns a CSV formatted string of islandora IDs and DSIDs.
 */
function ubl_check_datastreams_make_csv_content($objectid, $objecttype, $cmodel2dsids) {
  $csv = '';
  $childids = array();
  if ($objecttype === 'batch') {
    $itemids = islandora_batch_get_queued_items_for_set($objectid);
    foreach ($itemids as $itemid) {
      $childids[$itemid] = $itemid;
    }
  }
  else {
    $parent = islandora_object_load($objectid);
    ubl_check_datastreams_retrieve_all_child_ids($parent, $childids);
  }
  foreach ($childids as $childid) {
    foreach ($cmodel2dsids as $filtercmodel => $dsids) {
      foreach ($dsids as $filterdsid) {
        $child = islandora_object_load($childid);
        if ($child) {
          $info = ubl_check_datastreams_datastream_property_info($child);
          if (!(isset($info[$filterdsid]['isavailable']) && $info[$filterdsid]['isavailable'])) {
            $cmodels = ubl_check_datastreams_get_cmodel_string($child);
            if ($filtercmodel === $cmodels) {
              $csv .= $child->id . ";" . $filterdsid . ";\n";
            }
          }
          $child->refresh();
          unset($child);
        }
      }
    }
  }
  return $csv;
}

/**
 * Helper function to get the cmodels as a string.
 */
function ubl_check_datastreams_get_cmodel_string($object) {
  $cmodels = implode(',', $object->models);
  $cmodels = str_replace(',fedora-system:FedoraObject-3.0', '', $cmodels);
  $cmodels = str_replace('fedora-system:FedoraObject-3.0,', '', $cmodels);

  return $cmodels;
}
