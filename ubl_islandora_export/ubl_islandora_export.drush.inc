<?php
/**
 * @file
 * Functions for exporting Islandora content via drush.
 */

//module_load_include('inc', 'ubl_check_datastreams', 'includes/ubl_check_datastreams.children');

/**
 * Implements hook_drush_command().
 */
function ubl_islandora_export_drush_command() {
  $items['islandora_export'] = array(
    'description' => 'Exports the metadata and/or datastreams of items from Islandora. You can specify which items to use for the export, like items in specific collection(s), batch set(s) and/or a file with identifiers or a Solr query. Optionally you can specify the type of the source objects you need. Provide an ini file to specify the output format you want.',
    'options' => array(
      'format_file' => 'Mandatory, the absolute filepath to an ini file containing the format of the export. See documentation for specifics.',
      'directory' => 'Mandatory, the absolute filepath to an empty directory to export to.',
      'collection' => 'Optionally, one or more collection IDs, separated by comma\'s.',
      'batchset' => 'Optionally, one or more batchset IDs, separated by comma\'s.',
      'ids_file' => 'Optionally, the absolute filepath to a file containing a list of Islandora identifiers.',
      'solr_query' => 'Optionally, a Solr query to find the items to export. Cannot be combined with collection, batchset or ids_file.',
      'cmodel' => 'Optionally, filters the objects found by collection/batchset/ids_file on the given content models(s), separated by comma\'s.',
    ),
    'aliases' => array('export'),
    'examples' => array(
      'drush --user=admin islandora_export --collection=islandora:root --format_file=/path/to/format.ini --directory=/path/to/empty/directory',
      'drush --user=admin export --batchset=66 --format_file=/path/to/format.ini --directory=/path/to/empty/directory',
      'drush --user=admin export --ids_file=/path/to/idsfile.csv --format_file=/path/to/format.ini --directory=/path/to/empty/directory',
      'drush --user=admin export --solr_query=catch_all_fields_mt:book --cmodel=islandora:bookCModel --format_file=/path/to/format.ini --directory=/path/to/empty/directory',
    ),
  );
  return $items;
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_ubl_islandora_export_islandora_export_validate() {
  $collections = drush_get_option('collection');
  $batchsets = drush_get_option('batchset');
  $idsfile = drush_get_option('ids_file');
  $solrquery = drush_get_option('solr_query');
  $cmodels = drush_get_option('cmodel');
  $formatfile = drush_get_option('format_file');
  $directory = drush_get_option('directory');

  // check directory option
  if (!isset($directory)) {
    return drush_set_error("Argument directory is mandatory!");
  }
  if (!is_dir($directory)) {
    return drush_set_error("directory '$directory' does not exist, should be an absolute path");
  }
  // Check format_file option.
  if (!isset($formatfile)) {
    return drush_set_error("Argument format_file is mandatory!");
  }
  if (!file_exists($formatfile)) {
    return drush_set_error("format_file '$formatfile' does not exist, should be an absolute path");
  }
  $formatconfig = parse_ini_file($formatfile, TRUE);
  if ($formatconfig === FALSE) {
    return drush_set_error("format_file '$formatfile' is not a valid ini file");
  }
  $formatfilename = basename($formatfile);
  if (!isset($formatconfig['exportformat'])) {
    return drush_set_error("format_file '$formatfilename' does not have the mandatory section [exportformat]");
  }
  if (!(isset($formatconfig['exportformat']['type']) && $formatconfig['exportformat']['type'] === 'CSV')) {
    return drush_set_error("format_file '$formatfilename' section [exportformat]: only type CSV is allowed at present");
  }
  if (!(isset($formatconfig['exportformat']['separator']) && strlen($formatconfig['exportformat']['separator']) == 1)) {
    return drush_set_error("format_file '$formatfilename' section [exportformat]: separator is mandatory and should be a single character.");
  }
  if (!(isset($formatconfig['exportformat']['columns']) && is_array($formatconfig['exportformat']['columns']) && count($formatconfig['exportformat']['columns']) > 0)) {
    return drush_set_error("format_file '$formatfilename' section [exportformat]: one or more columns[] are required");
  }
  $columns = $formatconfig['exportformat']['columns'];
  if (isset($formatconfig['exportformat']['columntypes'])) {
    $columntypes = $formatconfig['exportformat']['columntypes'];
    foreach ($columns as $column) {
      $hasdef = FALSE;
      if (isset($formatconfig[$column])) {
        $hasdef = TRUE;
        if (!ubl_islandora_export_validate_ini_column($formatfilename, $formatconfig, $column)) {
          return FALSE;
        }
      }
      foreach ($columntypes as $type => $cmodel) {
        $specificcolumn = $type . ':' . $column;
        if (isset($formatconfig[$specificcolumn])) {
          $hasdef = TRUE;
	  if (!ubl_islandora_export_validate_ini_column($formatfilename, $formatconfig, $specificcolumn)) {
	    return FALSE;
	  }
        }
      }
    }
  }
  else {
    foreach ($columns as $column) {
      if (!ubl_islandora_export_validate_ini_column($formatfilename, $formatconfig, $column)) {
	return FALSE;
      }
    }
  }

  // Check collection option.
  if (isset($collections)) {
    $colarray = explode(',', $collections);
    foreach ($colarray as $collection) {
      $collectionobj = islandora_object_load($collection);
      if (!($collectionobj && in_array('islandora:collectionCModel', $collectionobj->models))) {
	return drush_set_error("Identifier '$collection' does not resolve to a collection");
      }
    }
  }
 
  // Check batchset option.
  if (isset($batchsets)) {
    if (module_load_include('inc', 'islandora_batch', 'includes/db')) {
      $batchsetarray = explode(',', $batchsets);
      foreach ($batchsetarray as $batchset) {
        $itemids = islandora_batch_get_queued_items_for_set($batchset);
        if (!($itemids && count($itemids) > 0)) {
	  return drush_set_error("Identifier '$batchset' does not resolve to a batch set");
        }
      }
    }
    else {
      return drush_set_error("Required module 'islandora_batch' failed to load.");
    }
  }

  // Check ids_file option.
  if (isset($idsfile)) {
    if (!file_exists($idsfile)) {
      return drush_set_error("ids_file '$idsfile' does not exist");
    }
  }

  // Check solr_query option.
  if (isset($solrquery)) {
    if (isset($idsfile) || isset($batchsets) || isset($collections)) {
      return drush_set_error("solr_query cannot be combined with collection, batchset or ids_file.");
    } 
    $parts = explode(':', $solrquery);
    if (count($parts) != 2) {
      return drush_set_error("solr_query should have the following format: solr_field:search_term.");
    }
  }

  return TRUE;
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_ubl_islandora_export_islandora_export() {
  $collections = drush_get_option('collection');
  $batchsets = drush_get_option('batchset');
  $idsfile = drush_get_option('ids_file');
  $solrquery = drush_get_option('solr_query');
  $cmodels = drush_get_option('cmodel');
  $formatfile = drush_get_option('format_file');
  $directory = drush_get_option('directory');

  if (isset($cmodels)) {
    $cmodelarray = explode(',', $cmodels);
  }

  $doneids = array();
  $formatconfig = parse_ini_file($formatfile, TRUE);
  if (isset($collections)) {
    $colarray = explode(',', $collections);
    foreach ($colarray as $collection) {
      $collectionobj = islandora_object_load($collection);
      if ($collectionobj && in_array('islandora:collectionCModel', $collectionobj->models)) {
        ubl_islandora_export_process_children($obj, $cmodels, $doneids);
      }
    }
  }
  if (isset($batchsets)) {
    $batchsetarray = explode(',', $batchsets);
    foreach ($batchsetarray as $batchset) {
      $itemids = islandora_batch_get_queued_items_for_set($batchset);
      foreach ($itemids as $itemid) {
	$obj = islandora_object_load($itemid);
	if ($obj) {
	  if (isset($cmodelarray)) { 
	    if (!array_intersect($cmodelarray, $obj->models)) { 
	      continue;
	    } 
	  } 
	  ubl_islandora_export_object_export($obj, $formatconfig, $directory);
          $doneids[$itemid] = 1;
        }
      }
    }
  }
  if (isset($idsfile)) {
    $ids = islandora_ids_from_file($idsfile);
    foreach ($ids as $itemid) {
      $obj = islandora_object_load($itemid);
      if ($obj) {
        if (isset($cmodelarray)) { 
         if (!array_intersect($cmodelarray, $obj->models)) { 
           continue;
         } 
        } 
        ubl_islandora_export_object_export($obj, $formatconfig, $directory);
        $doneids[$itemid] = 1;
        ubl_islandora_export_process_children($obj, $cmodels, $doneids);
      }
    }
  }
  if (isset($solrquery)) {
    $current = 0;
    $numfound = 1;
    while ($current < $numfound) {
      $qp = new IslandoraSolrQueryProcessor();
      $qp->buildQuery($solrquery);
      $qp->solrLimit = 50;
      $qp->solrStart = $current;
      $qp->executeQuery();
      $r = $qp->islandoraSolrResult;
      $numfound = $r['response']['numFound'];
      if ($current === 0) {
        drush_log(t("Found @numfound objects for query @query", array("@numfound" => $numfound, "@query" => $solrquery)), 'ok');
      }

      $len = count($r['response']['objects']);
      for ($i = 0; $i < $len; $i++) {
        $objid = $r['response']['objects'][$i]['PID']; 
        $obj = islandora_object_load($objid);
        if ($obj) {
          if (isset($cmodelarray)) {
           if (!array_intersect($cmodelarray, $obj->models)) {
             continue;
           }
          }
          ubl_islandora_export_object_export($obj, $formatconfig, $directory);
          $doneids[$objid] = 1;
          ubl_islandora_export_process_children($obj, $cmodels, $doneids);
        }
      }
      $current += $len;
    }
  }
}

function ubl_islandora_export_process_children($obj, $cmodels, &$doneids) {
  if (!$obj) {
    return;
  }
  if (isset($doneids[$obj->id])) {
    return; // Already done this object.
  }
  $doneids[$obj->id] = 1;
  if (in_array('islandora:collectionCModel', $obj->models)) {
    module_load_include('inc', 'islandora', 'includes/utilities');
    list($count, $colobjs) = islandora_basic_collection_get_member_objects($collectionobj, -1, -1);
    foreach ($colobjs as $colobj) {
      $cid = $colobj['object']['value'];
      $cobj = islandora_object_load($cid);
      if ($cobj) {
        if (isset($cmodelarray)) {
          if (!array_intersect($cmodelarray, $cobj->models)) {
            continue;
          }
        }
        ubl_islandora_export_object_export($cobj, $formatconfig, $directory);
        ubl_islandora_export_process_children($cobj, $cmodels, $doneids);
      }
    }
  }
  if (in_array('islandora:bookCModel', $obj->models) && (!isset($cmodelarray) || in_array('islandora:pageCModel', $cmodelarray))) {
    module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
    $pages = islandora_paged_content_get_pages($obj);
    foreach ($pages as $pagepid => $pageinfo) {
      $pageobj = islandora_object_load($pagepid);
      ubl_islandora_export_object_export($pageobj, $formatconfig, $directory);
    }
  }
  if (in_array('islandora:compoundCModel', $obj->models)) {
    $compoundparts = islandora_compound_object_get_parts($obj->id);
    foreach ($compoundparts as $partpid) {
      $pobj = islandora_object_load($partpid);
      if ($pobj) {
        if (isset($cmodelarray)) {
          if (!array_intersect($cmodelarray, $pobj->models)) {
            continue;
          }
        }
        ubl_islandora_export_object_export($pobj, $formatconfig, $directory);
        ubl_islandora_export_process_children($pobj, $cmodels, $doneids);
      }
    }
  }
}

function ubl_islandora_export_object_export($obj, array $formatconfig, $outputdirectory) {
  if ($formatconfig['exportformat']['type'] === 'CSV') {
    $csvfile = rtrim($outputdirectory, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR . 'export.csv';
    $fileexistsalready = is_file($csvfile);
    if ($fileexistsalready) {
      $csvfilehandle = fopen($csvfile, 'a');
    }
    else {
      $csvfilehandle = fopen($csvfile, 'w');
    }
    if ($csvfilehandle === FALSE) {
      return drush_set_error("File $csvfile cannot be opend for writing"); 
    } 
    if (!$fileexistsalready) {
      $separator = $formatconfig['exportformat']['separator'];
      $columns = $formatconfig['exportformat']['columns']; 
      if (fputcsv($csvfilehandle, $columns, $separator) === FALSE) {
        return drush_set_error("Failed to write to file $csvfile");
      } 
    }
    $ok = ubl_islandora_export_object_export_csv($obj, $formatconfig, $outputdirectory, $csvfilehandle);
    if ($ok === FALSE) {
      return FALSE;
    }
  }
  drush_log(t("Exported object @id succesfully", array('@id' => $obj->id)), 'ok');
}

function ubl_islandora_export_object_export_csv($obj, array $formatconfig, $outputdirectory, $csvfilehandle) {
  $separator = $formatconfig['exportformat']['separator'];
  $columnnames = $formatconfig['exportformat']['columns'];

  $columnvalues = array();
  foreach ($columnnames as $columnname) {
    unset($columnconfig);
    if (isset($formatconfig['exportformat']['columntypes'])) {
      $columntypes = $formatconfig['exportformat']['columntypes'];
      $cmodels = $obj->models; 
      foreach ($columntypes as $type => $cmodel) {
        if (in_array($cmodel, $cmodels)) {
          if (isset($formatconfig[$type . ':' . $columnname])) {
            $columnconfig = $formatconfig[$type . ':' . $columnname];
          }
        }
      }
    }
    if (!isset($columnconfig) && isset($formatconfig[$columnname])) {
      $columnconfig = $formatconfig[$columnname];
    }
    if (!isset($columnconfig)) {
      return drush_set_error("No config found for column '$columnname'. Please use a general config or a specific for every type.");
    }
    $value = FALSE;
    if ($columnconfig['type'] === 'string') {
      $value = $columnconfig['string'];
    }
    elseif ($columnconfig['type'] === 'value') { 
      $value = ubl_islandora_export_object_export_value($obj, $columnconfig);
    }
    elseif ($columnconfig['type'] === 'file') {
      $filename = ubl_islandora_export_object_export_file($obj, $formatconfig, $columnconfig, $outputdirectory);
      if ($filename) {
        $value = $filename;
      }
    }
    if ($value === FALSE) {
      $value = '';
    }
    $columnvalues[] = $value;
  }
  if (fputcsv($csvfilehandle, $columnvalues, $separator) === FALSE) {
    return drush_set_error("Failed to write to csv file");
  }
  return TRUE;
}

function ubl_islandora_export_object_export_value($obj, array $columnconfig) {
  $value = FALSE;
  $source = $columnconfig['source'];
  if ($source['type'] === 'property') {
    $value = ubl_islandora_export_object_property($obj, $source['property']);
  }
  elseif ($source['type'] === 'datastream') {
    $dsid = $source['dsid'];
    $ds = $obj[$dsid];
    if ($ds) {
      $extract = $columnconfig['extract'];
      if ($extract['type'] === 'property') {
        switch ($extract['property']) {
          case 'checksum':
            $value = $ds->checksum;
            break;
          case 'checksumtype':
            $value = $ds->checksumType;
            break;
          case 'controlgroup':
            $value = $ds->controlGroup;
            break;
          case 'creationdate':
	    $value = $ds->createdDate->format("Y-m-d");
            break;
	  case 'creationdatetime':
	    $value = $ds->createdDate->format("Y-m-d\TH:i:s");
	    break;
          case 'id':
            $value = $ds->id;
            break;
          case 'label':
            $value = $ds->label;
            break;
          case 'mimetype':
            $value = $ds->mimetype;
            break;
          case 'size':
            $value = $ds->size;
            break;
          case 'state':
            $value = $ds->state;
            break;
          case 'url':
            $value = $ds->url;
            break;
          case 'extension':
            module_load_include('inc', 'islandora', 'includes/mimetype.utils');
            $value = islandora_get_extension_for_mimetype($ds->mimetype);
            break;
        }
      }
      elseif ($extract['type'] === 'xpath') {
        $xpath = $extract['xpath'];
	$content = $ds->content;
	$domdoc = new DOMDocument();
	if ($domdoc->loadXML($content)) {
	  $domxpath = new DOMXPath($domdoc);
          $namespaces = explode(';', $extract['namespaces']);
          foreach ($namespaces as $namespace) {
            list($prefix, $uri) = explode(' ', $namespace, 2);
	    $domxpath->registerNamespace($prefix, $uri);
          }
	  $domnodelist = $domxpath->query($xpath);
	  if ($domnodelist->length > 0) {
	    foreach ($domnodelist as $domnode) {
	      $text = $domnode->textContent;
	      if (isset($text) && strlen($text) > 0) {
		$value = $text;
		break;
	      }
	    }
	  }
	}
      }
    }
  }
  return $value;
}

function ubl_islandora_export_object_property($obj, $property) {
  $value = FALSE;
  switch ($property) {
    case 'creationdate':
      $value = $obj->createdDate->format("Y-m-d");
      break;
    case 'creationdatetime':
      $value = $obj->createdDate->format("Y-m-d\TH:i:s");
      break;
    case 'id':
      $value = $obj->id;
      break;
    case 'label':
      $value = $obj->label;
      break;
    case 'modifydate':
      $value = $obj->lastModifiedDate->format("Y-m-d");
      break;
    case 'modifydatetime':
      $value = $obj->lastModifiedDate->format("Y-m-d\TH:i:s");
      break;
    case 'owner':
      $value = $obj->owner;
      break;
    case 'state':
      $value = $obj->state;
      break;
  }
  return $value;
}

function ubl_islandora_export_object_export_file($obj, array $formatconfig, array $columnconfig, $outputdirectory) {
  $filepath = '';
  $outputdir = rtrim($outputdirectory, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
  if (isset($columnconfig['outputdirectory'])) {
    $dir = ubl_islandora_export_object_export_filepath_part($obj, $formatconfig, $columnconfig['outputdirectory']);
    if ($dir) {
      $filepath = $filepath . rtrim($dir, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
    }
  }
  if (preg_match('#[^a-zA-Z0-9./_-]#', $filepath)) {
    return drush_set_error(t('Filepath @filepath contains illegal characters', array('@filepath' => $filepath)));
  }
  if (!is_dir($outputdir . $filepath)) {
    mkdir($outputdir . $filepath, 0777, TRUE); 
  }
  $filename = ubl_islandora_export_object_export_filepath_part($obj, $formatconfig, $columnconfig['outputfilename']);
  if (!$filename) {
    return FALSE;
  }
  if (preg_match('#[^a-zA-Z0-9._-]#', $filename)) {
    return drush_set_error(t('Filename @filename contains illegal characters', array('@filename' => $filename)));
  }
  $filepath .= $filename;
  if (isset($columnconfig['outputextension'])) {
    $extension = ubl_islandora_export_object_export_filepath_part($obj, $formatconfig, $columnconfig['outputextension']);
    if ($extension) {
      if (preg_match('#[^a-zA-Z0-9._-]#', $extension)) {
	return drush_set_error(t('Extension @extension contains illegal characters', array('@extension' => $extension)));
      }
      $filepath .= '.' . ltrim($extension, '.'); 
    }
  }
  
  $source = $columnconfig['source'];
  $fileexists = is_file($outputdir . $filepath);
  if ($source['type'] === 'property') {
    $value = ubl_islandora_export_object_property($obj, $source['property']);
    if ($value) {
      file_put_contents($outputdir . $filepath, $value);
    }
  }
  elseif ($source['type'] === 'datastream') {
    $dsid = $source['dsid'];
    $ds = $obj[$dsid];
    if ($ds) {
      $ds->getContent($outputdir . $filepath);
    }
    else {
      $filepath = FALSE; // no datastream to write
    }
  }
  if ($filepath !== FALSE && $fileexists) {
    drush_log(t("File '@filepath' is overwritten because it existed already.", array("@filepath" => $filepath)), "warning");
  }
  return $filepath;
}

function ubl_islandora_export_object_export_filepath_part($obj, array $formatconfig, array $outputconfig) {
  $part = FALSE;
  if (isset($outputconfig['string'])) {
    $part = $outputconfig['string'];
  }
  elseif (isset($outputconfig['like'])) {
    $columnname = $outputconfig['like'];
    if (isset($formatconfig[$columnname])) {
      $part = ubl_islandora_export_object_export_value($obj, $formatconfig[$columnname]); 
      if ($part) {
	$part = preg_replace('#[^a-zA-Z0-9._-]+#', '_', $part);
      }
    }
  }
  return $part; 
}

function ubl_islandora_export_validate_ini_column($formatfile, array $formatconfig, $column) {
  if (!isset($formatconfig[$column])) {
    return drush_set_error("format_file '$formatfile' does not have a definition for column '$column'");
  }
  if (!isset($formatconfig[$column]['type'])) {
    return drush_set_error("format_file '$formatfile' section [$column]: type should exist");
  }
  $type = $formatconfig[$column]['type'];
  if ($type !== 'string' && $type !== 'value' && $type !== 'file') {
    return drush_set_error("format_file '$formatfile' section [$column]: type should be 'string', 'value' or 'file'");
  }
  if ($type === 'string') {
    if (!isset($formatconfig[$column]['string'])) {
      return drush_set_error("format_file '$formatfile' section [$column]: string should exist.");
    }
  }
  else {
    if (!isset($formatconfig[$column]['source']['type'])) {
      return drush_set_error("format_file '$formatfile' section [$column]: source[type] should exist");
    }
    $sourcetype = $formatconfig[$column]['source']['type'];
    if ($sourcetype !== 'property' && $sourcetype !== 'datastream') {
      return drush_set_error("format_file '$formatfile' section [$column]: source[type] should be either 'property' or 'datastream'");
    }
    if ($sourcetype === 'property') {
      $property = $formatconfig[$column]['source']['property'];
      if (!isset($property)) {
	return drush_set_error("format_file '$formatfile' section [$column]: source[property] should exist");
      }
      $possibleproperties = array('checksum', 'checksumtype', 'creationdate', 'creationdatetime', 'modifydate', 'modifydatetime', 'id', 'label', 'owner', 'mimetype', 'size', 'state', 'url', 'extension');
      if (!in_array($property, $possibleproperties)) {
	return drush_set_error(t("format_file '$formatfile' section [$column]: source[property] should be one of these: @props",
        array('@props' => implode(',', $possibleproperties))));
      }
    }
    elseif ($sourcetype === 'datastream') {
      $dsid = $formatconfig[$column]['source']['dsid'];
      if (!isset($dsid)) {
	return drush_set_error("format_file '$formatfile' section [$column]: source[dsid] should exist");
      }
    }
    if ($type === 'value') {
      if (isset($formatconfig[$column]['extract'])) {
	$extracttype = $formatconfig[$column]['extract']['type'];
	if (!isset($extracttype) || ($extracttype !== 'xpath' && $extracttype !== 'property')) {
	  return drush_set_error("format_file '$formatfile' section [$column]: extract[type] should have value 'xpath' or 'property'");
	}
	if (!isset($formatconfig[$column]['extract'][$extracttype])) {
	  return drush_set_error("format_file '$formatfile' section [$column]: extract[$extracttype] should exist");
	}
	if (isset($formatconfig[$column]['extract']['xpath'])) {
	  $xpath = $formatconfig[$column]['extract']['xpath'];
	  $doc = new DOMDocument;
	  $domxpath = new DOMXPath($doc);
	  if (isset($formatconfig[$column]['extract']['namespaces'])) {
	    $namespaces = explode(';', $formatconfig[$column]['extract']['namespaces']);
	    foreach ($namespaces as $namespace) {
	      $parts = explode(' ', $namespace);
	      if (count($parts) != 2) {
		return drush_set_error("format_file '$formatfile' section [$column]: extract[namespaces]: wrong format '$namespace', should be 'prefix uri'. Multiple namespaces separated by semicolon ;.");
	      }
	      list($prefix, $uri) = $parts;
	      if (!(filter_var($uri, FILTER_VALIDATE_URL) || (strpos($uri, 'urn:') === 0))) {
		return drush_set_error("format_file '$formatfile' section [$column]: extract[namespaces]: not a URI in '$namespace'");
	      }
	      if (@$domxpath->registerNamespace($prefix, $uri) === FALSE) {
		return drush_set_error("format_file '$formatfile' section [$column]: extract[namespaces]: '$prefix=$uri' is not valid");
	      }
	    }
	  }
	  if (@$domxpath->query($xpath) === FALSE) {
	    return drush_set_error("format_file '$formatfile' section [$column]: extract[xpath]: '$xpath' is not valid");
	  }
	}
      } 
    }
    elseif ($type === 'file') {
      if (!ubl_islandora_export_validate_ini_column_output($formatfile, $formatconfig, $column, 'directory')) {
	return FALSE;
      }
      if (!ubl_islandora_export_validate_ini_column_output($formatfile, $formatconfig, $column, 'filename')) {
	return FALSE;
      }
      if (!ubl_islandora_export_validate_ini_column_output($formatfile, $formatconfig, $column, 'extension')) {
	return FALSE;
      }
    }
  }
  return TRUE;
}

function ubl_islandora_export_validate_ini_column_output($formatfile, array $formatconfig, $column, $type) {
  if (!isset($formatconfig[$column]['output' . $type])) {
    if ($type === 'filename') {
      return drush_set_error("format_file '$formatfile' section [$column]: output$type should exist");
    }
    else {
      return TRUE;
    }
  }
  if (count($formatconfig[$column]['output' . $type]) != 1 || !(isset($formatconfig[$column]['output' . $type]['string']) || isset($formatconfig[$column]['output' . $type]['like']))) {
    return drush_set_error("format_file '$formatfile' section [$column]: either output${type}[string] or output${type}[like] should exist");
  }
  if (isset($formatconfig[$column]['output' . $type]['like'])) {
    $likecol = $formatconfig[$column]['output' . $type]['like'];
    if ($likecol === $column) {
      return drush_set_error("format_file '$formatfile' section [$column]: output${type}[like] refers to itself");
    }
    if (!isset($formatconfig[$likecol])) { 
      return drush_set_error("format_file '$formatfile' does not have a definition for column '$likecol' referred to by section [$column] output${type}[like]");
    } 
    if (!(isset($formatconfig[$likecol]['type']) && $formatconfig[$likecol]['type'] === 'value')) {
      return drush_set_error("format_file '$formatfile' section [$column]: output${type}[like] should refer to a section of type 'value'");
    }
    if (!ubl_islandora_export_validate_ini_column($formatfile, $formatconfig, $likecol)) {
      return FALSE;
    }
  }
  return TRUE;
}

function islandora_ids_from_file($idsfile) {
  $foundids = array();

  $idsstr = file_get_contents($idsfile);
  $fieldseparator = "";
  $ids = str_getcsv($idsstr, "\n");
  foreach ($ids as $index => $id) {
    if (strlen($id) == 0) {
      continue;
    }
    if ($fieldseparator === "") {
      $fieldseparator = "\t";
      $comps = str_getcsv($id, $fieldseparator);
      if (count($comps) > 1) {
        // Fields are separated by a tab.
      }
      else {
        $fieldseparator = ",";
        $comps = str_getcsv($id, $fieldseparator);
        if (count($comps) > 1) {
          // Fields are separated by a comma.
        }
        else {
          $fieldseparator = ";";
          $comps = str_getcsv($id, $fieldseparator);
          if (count($comps) > 1) {
            // Fields are separated by a semicolon.
          }
          else {
            // Fields are not separated.
            $fieldseparator = FALSE;
          }
        }
      }
    }
    if ($fieldseparator !== FALSE) {
      $comps = str_getcsv($id, $fieldseparator);
      if (count($comps) > 1) {
        $id = $comps[0];
      }
    }
    $foundids[] = $id;
  }
  return $foundids;
}

