<?php
error_reporting(E_ALL);
ini_set("display_errors", 1);

/**
 * @file
 * Function to retrieve the children and missing datastreams info
 */

module_load_include('inc', 'islandora', 'includes/utilities');
module_load_include('module', 'islandora_solution_pack_compound', 'islandora_compound_object');
module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
module_load_include('inc', 'islandora_batch', 'includes/db');


function ubl_check_datastreams_get_all_children($parent) {
  $children = array();
  if ($parent &&  in_array('islandora:collectionCModel', $parent->models)) {
    list($count, $colobjs) = islandora_basic_collection_get_member_objects($parent, -1, -1);
    foreach ($colobjs as $colobj) {
      $cid = $colobj['object']['value'];
      $obj = islandora_object_load($cid);
      if ($obj) {
	$children[$cid] = $obj;
        $otherchildren = ubl_check_datastreams_get_all_children($obj);
        if (count($otherchildren) > 0) {
          $children = array_merge($children, $otherchildren);
        }
      }
    }
  }
  if ($parent &&  in_array('islandora:compoundCModel', $parent->models)) {
    $compoundparts = islandora_compound_object_get_parts($parent->id);
    foreach ($compoundparts as $partpid) {
      $obj = islandora_object_load($partpid);
      if ($obj) {
        $children[$partpid] = $obj;
        $otherchildren = ubl_check_datastreams_get_all_children($obj);
        if (count($otherchildren) > 0) {
          $children = array_merge($children, $otherchildren);
        }
      }
    }
  }
  if ($parent &&  in_array('islandora:bookCModel', $parent->models)) {
    $pages = islandora_paged_content_get_pages($parent);
    foreach ($pages as $pagepid => $pageinfo) {
      $obj = islandora_object_load($pagepid);
      $children[$pagepid] = $obj; 
    }
  }
  return $children;
}

function ubl_check_datastreams_dsids_by_cmodel($children) {
  $dsidsbycmodel = array();
  foreach ($children as $child) {
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
  }
  return $dsidsbycmodel;
}

function ubl_check_datastreams_make_csv_content($objectid, $objecttype, $cmodel2dsids) {
  $csv = '';
  if ($objecttype === 'batch') {
    $children = array();
    $itemids = islandora_batch_get_queued_items_for_set($objectid);
    foreach ($itemids as $itemid) {
      $children[$itemid] = islandora_object_load($itemid);
    }
  }
  else {
    $parent = islandora_object_load($objectid);
    $children = ubl_check_datastreams_get_all_children($parent);
  }
  foreach ($children as $child) {
    foreach ($cmodel2dsids as $filtercmodel => $dsids) {
      foreach ($dsids as $filterdsid) {
	$info = ubl_check_datastreams_datastream_property_info($child);
	if (!(isset($info[$filterdsid]['isavailable']) && $info[$filterdsid]['isavailable'])) {
	  $cmodels = ubl_check_datastreams_get_cmodel_string($child);
	  if ($filtercmodel === $cmodels) {
	    $csv .= $child->id . ";" . $filterdsid . ";\n";
	  }
	}
      }
    }
  }
  return $csv;
}

function ubl_check_datastreams_get_cmodel_string($object) {
  $cmodels = implode(',', $object->models);
  $cmodels = str_replace(',fedora-system:FedoraObject-3.0', '', $cmodels);
  $cmodels = str_replace('fedora-system:FedoraObject-3.0,', '', $cmodels);

  return $cmodels;
}
