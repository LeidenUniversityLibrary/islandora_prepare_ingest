<?php
error_reporting(E_ALL);
ini_set("display_errors", 1);


/**
 * @file
 * Handles the compound item creation from csv import.
 */

/**
 * The form for uploading csv.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function ubl_batch_compound_import_form(array $form, array &$form_state) {
  $extensions = array('txt csv');
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $form['file'] = array(
    '#title' => t('Upload csv'),
    '#type' => 'managed_file',
    '#description' => t('Select a file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $extensions[0])),
    '#upload_location' => 'temporary://',
    '#required' => TRUE,
    '#upload_validators' => array(
      'file_validate_extensions' => $extensions,
      'file_validate_size' => array($upload_size * 1024 * 1024),
    ),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Upload CSV'),
  );
  return $form;
}

/**
 * Processes uploaded csv.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function ubl_batch_compound_import_form_submit(array $form, array &$form_state) {
  $file = file_load($form_state['values']['file']);
  $path = drupal_realpath($file->uri);

  ini_set("auto_detect_line_endings", "1");
  $delimit = "\t";
  $parent2child = array();
  $child2parent = array();
  $linenr = 0;
  $rels_predicate = variable_get('islandora_compound_object_relationship', 'isConstituentOf');
  if (($fh = fopen($path, "r")) !== FALSE) {
    while (($data = fgetcsv($fh, 0, $delimit)) !== FALSE) {
      $linenr++;
      if ($linenr == 1) {
        // treat header special
        if (count($data) == 1) {
          $delimit = ",";
          $data = str_getcsv($data[0],$delimit);
        }
        if (count($data) == 1) {
          $delimit = ";";
          $data = str_getcsv($data[0],$delimit);
        }
        $parentid = $data[0];
        $childid = $data[1];
       
        $foundparents = find_fedora_object_ids($parentid); 
        $foundchildren = find_fedora_object_ids($childid); 
        
        if (count($foundparents) != 1 && count($foundchildren) == 0) {
          // is header
          continue;
        }
      }
      
      $parentid = $data[0];
      $childid = $data[1];
     
      $foundparents = find_fedora_object_ids($parentid); 
      $foundchildren = find_fedora_object_ids($childid); 

      if (count($foundchildren) == 0) {
        drupal_set_message(t("Found no children for id @childid", array("@childid" => $childid)), 'warning');
      }

      if ( ! array_key_exists($parentid,$parent2child)) {
        $parent2child[$parentid] = array("childidcount" => 0, "childfoundcount" => 0, "childselcount" => 0, "parent" => $foundparents, "children" => array());
        if (count($foundparents) == 1) {
          $foundparentid = $foundparents[0];
          $foundparent = islandora_object_load($foundparentid);
          $compound = FALSE;
          if (variable_get('islandora_compound_object_compound_children', TRUE)) {
            $compound = TRUE;
          }
          else {
            foreach ($foundparent->models as $value) {
              if ($value == "islandora:compoundCModel") {
                $compound = TRUE;
              }
            }
          }
          if ($compound) {
            $parent2child[$parentid]["singleparentid"] = $foundparentid;
          }
          else {
            drupal_set_message(t("Found for parent with id @parentid a Islandora object (@objectid) that is not an compound object", array("@parentid" => $parentid, "@objectid" => $foundparentid)), 'warning');
          }
        }
      }
      $parent2child[$parentid]["childidcount"]++;
      if ( ! array_key_exists($childid,$parent2child[$parentid]["children"])) {
        $parent2child[$parentid]["children"][$childid] = array_fill_keys($foundchildren,true);
        $parent2child[$parentid]["childfoundcount"] += count($foundchildren);
        $parent2child[$parentid]["childselcount"] += count($foundchildren);

        if (array_key_exists("singleparentid",$parent2child[$parentid])) {
          $foundparentid = $parent2child[$parentid]["singleparentid"];
          foreach ($foundchildren as $foundchild) {
            if ($foundchild === $foundparentid) {
              drupal_set_message(t("Child (@childid) with itself as parent (@parentid) is not allowed", array("@childid" => $childid, "@parentid" => $parentid)), 'error');
              unset($parent2child[$parentid]["singleparentid"]);
            }
          }
        }
        foreach ($foundchildren as $foundchild) {
            if (array_key_exists($foundchild,$child2parent)) {
                drupal_set_message(t("Child (@childid) already belongs to parent (@parentid) so will not be added to parent (@otherparentid)", array("@childid" => $foundchild, "@parentid" => $child2parent[$foundchild], "@otherparentid" => $parentid)), 'warning');
                $parent2child[$parentid]["children"][$childid][$foundchild] = false;
                $parent2child[$parentid]["childselcount"]--;
            }
            else {
              $child2parent[$foundchild] = $parentid;
            }
          $childobject = islandora_object_load($foundchild);
          if ($childobject) {
            $part_of = $childobject->relationships->get(FEDORA_RELS_EXT_URI, $rels_predicate);
            // This object is part of a compound object.
            if (!empty($part_of) && count($part_of) > 0) {
              drupal_set_message(t("Child (@foundchild) already has a parent (@parentid)", array("@foundchild" => $foundchild, "@parentid" => $part_of[0]['object']['value'])), 'warning');
              if ($parent2child[$parentid]["children"][$childid][$foundchild]) {
                $parent2child[$parentid]["children"][$childid][$foundchild] = false;
                $parent2child[$parentid]["childselcount"]--;
              }
            }
          }
        }
      }
    }
    fclose($fh);
  }

  $_SESSION['ubl_batch_compound_parent2child'] = $parent2child;
  $_SESSION['ubl_batch_compound_finished'] = false;

  $form_state['redirect'] = 'ubl_batch_compound/confirm';
}

/**
 * The form for confirming the compound items.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */

function ubl_batch_compound_import_confirm_form(array $form, array &$form_state) {
  $parent2child = $_SESSION['ubl_batch_compound_parent2child'];
  $options = array();
  $defaultvalues = array();
  if (count($form_state["build_info"]["args"]) == 1) {
    $parentid = $form_state["build_info"]["args"][0];

    drupal_set_title(t("Generate compound: viewing parent \"@parentid\"", array("@parentid" => $parentid)));

    foreach ($parent2child[$parentid]["children"] as $cid => $foundchildrenids) {
      if (count($foundchildrenids) == 0) {
        $rowid = $parentid . '//' . $cid . '//' . 'none';
        $options[$rowid]['childid'] = check_plain($cid);
        $options[$rowid]['foundchildren'] = t('No child found');
        $options[$rowid]['#disabled'] = TRUE;
      } 
      else {
        foreach ($foundchildrenids as $foundchildid => $isenabled) {
          $rowid =  $parentid . '//' . $cid . '//' . $foundchildid;
          $options[$rowid]['childid'] = check_plain($cid);
          $options[$rowid]['foundchildren'] = array(
            'data' => array(
               '#type' => 'link',
               '#title' => filter_xss($foundchildid),
               '#href' => "islandora/object/{$foundchildid}",
            ),
          );
          $defaultvalues[$rowid] = $isenabled;
        }
      }
    }
    $table = array(
      '#type' => 'tableselect',
      '#header' => array(
         'childid' => t('Child ID in CSV'),
         'foundchildren' => t('Found Children'),
      ),
      '#options' => $options,
      '#default_value' => $defaultvalues,
      '#empty' => t('No data available.'),
      '#disabled' => ($_SESSION['ubl_batch_compound_finished']),
    );

    $form['child_table'] = $table;

    $submittitle = t('Save and back');
    if ($_SESSION['ubl_batch_compound_finished']) {
      $submittitle = t('Back');
    }
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => $submittitle,
    );
  }
  else {
    foreach ($parent2child as $pid => $values) {
      $options[$pid]['parentid'] = check_plain($pid);
      if (array_key_exists('singleparentid',$values)) {
        $foundparent = $values["singleparentid"];
        $options[$pid]['parentitem'] = array(
          'data' => array(
            '#type' => 'link',
            '#title' => filter_xss($foundparent),
            '#href' => "islandora/object/{$foundparent}",
          ),
        );
      }
      else {
        $options[$pid]['parentitem'] = t('New parent');
      }
      $options[$pid]['childids'] = format_plural($values['childidcount'], "1 child in CSV", "@count children in CSV");
      if ($values['childfoundcount'] !== $values['childselcount']) {
        $options[$pid]['foundchildren'] = t("@selcount selected of @foundcount found", array("@selcount" => $values['childselcount'], "@foundcount" => $values['childfoundcount']));
      }
      else {
        $options[$pid]['foundchildren'] = format_plural($values['childfoundcount'], "1 child found", "@count children found");
      }
      $options[$pid]['view'] = array(
        'data' => array(
          '#type' => 'link',
          '#title' => t("View"),
          '#href' => "ubl_batch_compound/confirm/{$pid}",
        ),
      );
  
      if ($values['childfoundcount'] == 0) {
        $defaultvalues[$pid] = false;
        $options[$pid]['#disabled'] = true;
      }
      else {
        $defaultvalues[$pid] = true;
      }
    }
    $table = array(
      '#type' => 'tableselect',
      '#header' => array(
         'parentid' => t('Parent ID in CSV'),
         'parentitem' => t('Parent item'),
         'childids' => t('Child IDs in CSV'),
         'foundchildren' => t('Found Children'),
         'view' => '',
      ),
      '#options' => $options,
      '#default_value' => $defaultvalues,
      '#empty' => t('No data available.'),
      '#disabled' => ($_SESSION['ubl_batch_compound_finished']),
    );

    $form['parent_table'] = $table;
  
    if ( ! $_SESSION['ubl_batch_compound_finished']) {
      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Generate compound objects'),
      );
    }
  }

  return $form;
}

function ubl_batch_compound_import_confirm_form_submit(array $form, array &$form_state) {

  if ($_SESSION['ubl_batch_compound_finished']) {
    // do not change anything if already finished
    if (array_key_exists("child_table",$form_state["values"])) {
      $form_state['redirect'] = 'ubl_batch_compound/confirm';
    }
    return; 
  }
  $operations = array();

  if (array_key_exists("parent_table",$form_state["values"])) {
    foreach ($form_state["values"]["parent_table"] as $parentid => $isneeded) {
      if ($isneeded) {
        $operations[] = array('ubl_batch_compound_batch_set_children', array($parentid));
      }
    }
    if (count($operations) > 0) {
      $batch = array(
        'operations' => $operations,
        'title' => t("Generating compound objects"),
        'init_message' => t("Preparing ..."),
        'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
        'error_message' => t('An error has occurred.'),
        'file' => drupal_get_path('module', 'ubl_batch_compound') . '/includes/ubl_batch_compound_csv_import.form.inc',
        'finished' => 'ubl_batch_compound_batch_set_children_finished',
      );
      batch_set($batch);
    }
  }
  else {
    $parent2child = $_SESSION['ubl_batch_compound_parent2child'];

    foreach ($form_state["values"]["child_table"] as $childpath => $isneeded) {
      $parts = explode("//",$childpath);
      if (count($parts) == 3) {
        $parentid = $parts[0];
        $childcvsid = $parts[1];
        $childid = $parts[2];
        $parent2child[$parentid]["children"][$childcvsid][$childid] = ($isneeded !== 0);
      }
    }
    $parent2child[$parentid]["childselcount"] = 0;
    foreach ($parent2child[$parentid]["children"] as $childs) {
      foreach ($childs as $cid => $enabled) {
        if ($enabled) {
          $parent2child[$parentid]["childselcount"]++;
        }
      }
    }

    $_SESSION['ubl_batch_compound_parent2child'] = $parent2child;

    $form_state['redirect'] = 'ubl_batch_compound/confirm';
  }
}

function ubl_batch_compound_batch_set_children($parentid, &$context) {
  module_load_include('inc','islandora_compound_object','includes/manage.form');
  module_load_include('inc','islandora_basic_collection','includes/utilities');
  module_load_include('inc','islandora','includes/utilities');
  module_load_include('inc','islandora');

  $parent2child = $_SESSION['ubl_batch_compound_parent2child'];
  $oneparent2children = $parent2child[$parentid];
  $parent = false;
  if (array_key_exists('singleparentid',$oneparent2children)) {
    $parent = islandora_object_load($oneparent2children['singleparentid']);
  }
  $childids = array();
  foreach ($oneparent2children["children"] as $childs) {
    foreach ($childs as $cid => $enabled) {
      if ($enabled) {
        $childids[] = $cid;
      }
    }
  }
  if ($parent === false) {
    if (count($childids) > 0) {
      $firstchildid = $childids[0];

      $firstchild = islandora_object_load($firstchildid);
      if ($firstchild) {
        //$collectionids = islandora_basic_collection_get_parent_pids($firstchild);
        //$collection = islandora_object_load($collectionids[0]); 
        $collection = islandora_object_load('islandora:compound_collection'); 
        $tuque = islandora_get_tuque_connection();
        $parent = $tuque->repository->constructObject(islandora_get_namespace($firstchild->id));
        $parent->label = "New compound object";
        $parent->models = 'islandora:compoundCModel';
        islandora_basic_collection_add_to_collection($parent,$collection);
        $tuque->repository->ingestObject($parent);
      }
    }
    if ($parent === false) {
      return;
    }
    $_SESSION['ubl_batch_compound_parent2child'][$parentid]['singleparentid'] = $parent->id;
  }

  $context['message'] = t('Generating compound object @parentid', array( '@parentid' => $parent->id));

  $child_objects = array();
  foreach ($childids as $childid) {
    $child_object = islandora_object_load($childid);
    if ($child_object) {
      $child_objects[] = $child_object;
    }
  }
  islandora_compound_object_add_parent($child_objects, array($parent->id));

  islandora_compound_object_update_parent_thumbnail($parent);
}

function ubl_batch_compound_batch_set_children_finished() {
  $_SESSION['ubl_batch_compound_finished'] = true;
}

/* helper functions */


function find_fedora_object_ids($identifier,$query_field = "catch_all_fields_mt") {
  module_load_include('inc','islandora_solr_search','includes/query_processor');
  module_load_include('inc','islandora_solr_search','includes/utilities');
  //module_load_include('inc','islandora');

  if (preg_match('/^[a-z]+:\d+$/', $identifier) === 1) {
    // looks like a islandora object id
    if (array_key_exists($identifier,$alreadyfound)) {
      return $alreadyfound[$identifier]; 
    }

    $obj = islandora_object_load($identifier);

    if ($obj) {
      $alreadyfound[$identifier] = array($identifier);

      return $alreadyfound[$identifier]; 
    }
  }
  static $qp = NULL;
  if (is_null($qp)) {
    $qp = new IslandoraSolrQueryProcessor();
  }
  static $alreadyfound = array();

  $escapedid = islandora_solr_lesser_escape($identifier);
  $search = "$query_field:$escapedid";

  if (array_key_exists($search,$alreadyfound)) {
    return $alreadyfound[$search]; 
  }

  $qp->buildAndExecuteQuery($search); 

  $r = $qp->islandoraSolrResult;

  $numFound = $r['response']['numFound'];

  $objectids = array();
  for ($i = 0; $i < $numFound; $i++) {
    $objid = $r['response']['objects'][$i]['PID'];
    //$obj = islandora_object_load($objid);

    if ($objid) {
      $objectids[] = $objid;
    }
  }

  $alreadyfound[$search] = $objectids;

  return $objectids;
}


