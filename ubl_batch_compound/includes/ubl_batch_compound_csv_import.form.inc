<?php

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
  $form['use_solr_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Optional, use Solr field for search'),
    '#description' => t('Specify the Solr field that should be used to search the compound and children by. For example use "mods_relatedItem_otherFormat_identifier_local_ms" to only search by identifier local.'),
    '#size' => 80,
    '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
    '#default_value' => '',
  );
  $form['exact_match'] = array(
    '#type' => 'checkbox',
    '#title' => t('exact matches only'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Preview compounds'),
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
  $solrfield = $form_state['values']['use_solr_field'];
  $exactmatch = $form_state['values']['exact_match'];

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
        // Treat header special.
        if (count($data) == 1) {
          $delimit = ",";
          $data = str_getcsv($data[0], $delimit);
        }
        if (count($data) == 1) {
          $delimit = ";";
          $data = str_getcsv($data[0], $delimit);
        }
        $parentid = $data[0];
        $childid = $data[1];

        $foundparents = find_fedora_object_ids($parentid, $exactmatch, $solrfield);
        $foundchildren = find_fedora_object_ids($childid, $exactmatch, $solrfield);

        if (count($foundparents) != 1 && count($foundchildren) == 0) {
          // This is a header, so skip it.
          continue;
        }
      }

      $parentid = $data[0];
      $childid = $data[1];

      $foundparents = find_fedora_object_ids($parentid, $exactmatch, $solrfield);
      $foundchildren = find_fedora_object_ids($childid, $exactmatch, $solrfield);

      if (count($foundchildren) == 0) {
        drupal_set_message(t("Found no children for id @childid", array("@childid" => $childid)), 'warning');
      }

      if (!array_key_exists($parentid, $parent2child)) {
        $parent2child[$parentid] = array(
          "childidcount" => 0,
          "childfoundcount" => 0,
          "childselcount" => 0,
          "parent" => $foundparents,
          "enabled" => TRUE,
          "children" => array(),
        );
        if (count($foundparents) == 1) {
          $foundparentid = $foundparents[0];
          $foundparent = islandora_object_load($foundparentid);
          $compound = FALSE;
          if ($foundparent) {
            if (variable_get('islandora_compound_object_compound_children', TRUE)) {
              // Has setting that only compounds are allowed to have children.
              foreach ($foundparent->models as $value) {
                if ($value == "islandora:compoundCModel") {
                  $compound = TRUE;
                }
              }
            }
            else {
              // Has setting that all objects may have child objects.
              $compound = TRUE;
            }
            if ($compound) {
              $parent2child[$parentid]["singleparentid"] = $foundparentid;
              $parent2child[$parentid]["usesingleparent"] = TRUE;
            }
            else {
              drupal_set_message(t("Found for parent with id @parentid a Islandora object (@objectid) that is not an compound object", array("@parentid" => $parentid, "@objectid" => $foundparentid)), 'warning');
            }
          }
        }
      }
      $parent2child[$parentid]["childidcount"]++;
      if (!array_key_exists($childid, $parent2child[$parentid]["children"])) {
        $parent2child[$parentid]["children"][$childid] = array_fill_keys($foundchildren, TRUE);
        $parent2child[$parentid]["childfoundcount"] += count($foundchildren);
        $parent2child[$parentid]["childselcount"] += count($foundchildren);

        if (array_key_exists("singleparentid", $parent2child[$parentid])
           && (array_key_exists("usesingleparent", $parent2child[$parentid]) && $parent2child[$parentid]["usesingleparent"])) {
          $foundparentid = $parent2child[$parentid]["singleparentid"];
          foreach ($foundchildren as $foundchild) {
            if ($foundchild === $foundparentid) {
              drupal_set_message(t("Child (@childid) with itself as parent (@parentid) is not allowed", 
                array(
                  "@childid" => $childid,
                  "@parentid" => $parentid,
                )), 'error');
              $parent2child[$parentid]["usesingleparent"] = FALSE;
            }
          }
        }
        foreach ($foundchildren as $foundchild) {
          if (array_key_exists($foundchild, $child2parent)) {
            drupal_set_message(t("Child (@foundchild, found for @childid) already belongs to parent (@parentid) so will not be added to parent (@otherparentid)",
              array(
                "@foundchild" => $foundchild,
                "@childid" => $childid,
                "@parentid" => $child2parent[$foundchild],
                "@otherparentid" => $parentid,
              )), 'warning');
            $parent2child[$parentid]["children"][$childid][$foundchild] = FALSE;
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
              drupal_set_message(t("Child (@foundchild, found for @childid) already has a parent (@parentid)",
                array(
                  "@foundchild" => $foundchild,
                  "@childid" => $childid,
                  "@parentid" => $part_of[0]['object']['value'],
                )), 'warning');
              if ($parent2child[$parentid]["children"][$childid][$foundchild]) {
                $parent2child[$parentid]["children"][$childid][$foundchild] = FALSE;
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
  $_SESSION['ubl_batch_compound_finished'] = FALSE;

  $form_state['redirect'] = 'ubl_batch_compound/confirm';
}

/**
 * The form for confirming the compound items.
 */
function ubl_batch_compound_import_confirm_form(array $form, array &$form_state) {
  $parent2child = $_SESSION['ubl_batch_compound_parent2child'];
  $options = array();
  $defaultvalues = array();
  if (count($form_state["build_info"]["args"]) == 1) {
    $parentid = $form_state["build_info"]["args"][0];

    drupal_set_title(t("Generate compound: viewing parent '@parentid'", array("@parentid" => $parentid)));

    foreach ($parent2child[$parentid]["children"] as $cid => $foundchildrenids) {
      if (count($foundchildrenids) == 0) {
        $rowid = "$parentid//$cid//none";
        $options[$rowid]['childid'] = check_plain($cid);
        $options[$rowid]['foundchildren'] = t('No child found');
        $options[$rowid]['#disabled'] = TRUE;
      }
      else {
        foreach ($foundchildrenids as $foundchildid => $isenabled) {
          $rowid = $parentid . '//' . $cid . '//' . $foundchildid;
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

    $form['parentid'] = array(
      '#type' => 'hidden',
      '#value' => $parentid,
    );

    if (!$_SESSION['ubl_batch_compound_finished']) {
      if (array_key_exists('singleparentid', $parent2child[$parentid])) {
        $form['usesingleparent'] = array(
          '#type' => 'checkbox',
          '#title' => t("Use existing object @parentid as parent", array("@parentid" => $parent2child[$parentid]['singleparentid'])),
          '#default_value' => $parent2child[$parentid]['usesingleparent'],
          '#return_value' => $parent2child[$parentid]['singleparentid'],
        );
      }
    }

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
      if (array_key_exists('singleparentid', $values) && (array_key_exists('usesingleparent', $values) && $values['usesingleparent'])) {
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
        if ($_SESSION['ubl_batch_compound_finished']) {
          $options[$pid]['parentitem'] = t('No parent made');
        }
        else {
          $options[$pid]['parentitem'] = t('New parent');
        }
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
          '#attributes' => array('class' => array('viewlinkclass')),
        ),
      );

      if ($values['childfoundcount'] == 0) {
        $defaultvalues[$pid] = FALSE;
        $options[$pid]['#disabled'] = TRUE;
      }
      else {
        $defaultvalues[$pid] = $values["enabled"];
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

    if (!$_SESSION['ubl_batch_compound_finished']) {
      drupal_add_js(drupal_get_path('module', 'ubl_batch_compound') . '/js/ubl_batch_compound.js');
    }

    $form['redirecturl'] = array(
      '#type' => 'hidden',
      '#default_value' => 'xxxxx',
    );
    $form['parent_table'] = $table;

    if (!$_SESSION['ubl_batch_compound_finished']) {
      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Generate compound objects'),
      );
    }
  }

  return $form;
}

/**
 * Submitting the form for confirming the compound items.
 */
function ubl_batch_compound_import_confirm_form_submit(array $form, array &$form_state) {

  if (array_key_exists("redirecturl", $form_state["values"]) && $form_state["values"]["redirecturl"] !== 'xxxxx') {
    if (array_key_exists("parent_table", $form_state["values"])) {
      foreach ($form_state["values"]["parent_table"] as $parentid => $isneeded) {
        $_SESSION['ubl_batch_compound_parent2child'][$parentid]['enabled'] = $isneeded;
      }
    }
    $form_state['redirect'] = $form_state["values"]["redirecturl"];
    return;
  }
  if ($_SESSION['ubl_batch_compound_finished']) {
    // Do not change anything if already finished.
    if (array_key_exists("child_table", $form_state["values"])) {
      $form_state['redirect'] = 'ubl_batch_compound/confirm';
    }
    return;
  }
  $operations = array();

  if (array_key_exists("parent_table", $form_state["values"])) {
    foreach ($form_state["values"]["parent_table"] as $parentid => $isneeded) {
      $_SESSION['ubl_batch_compound_parent2child'][$parentid]['enabled'] = $isneeded;
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
    $parentid = $form_state["values"]["parentid"];

    if (array_key_exists("usesingleparent", $form_state["values"])) {
      $parent2child[$parentid]["usesingleparent"] = ($form_state["values"]["usesingleparent"] !== 0);
    }
    foreach ($form_state["values"]["child_table"] as $childpath => $isneeded) {
      $parts = explode("//", $childpath);
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

/**
 * Make the compound for one parentid and the children that were selected.
 */
function ubl_batch_compound_batch_set_children($parentid, &$context) {
  module_load_include('inc', 'islandora_compound_object', 'includes/manage.form');
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora');

  $parent2child = $_SESSION['ubl_batch_compound_parent2child'];
  $oneparent2children = $parent2child[$parentid];
  $parent = FALSE;
  if (array_key_exists('singleparentid', $oneparent2children) && (array_key_exists('usesingleparent', $oneparent2children) && $oneparent2children['usesingleparent'])) {
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
  if ($parent === FALSE) {
    if (count($childids) > 0) {
      $firstchildid = $childids[0];

      $firstchild = islandora_object_load($firstchildid);
      if ($firstchild) {
        $collection = islandora_object_load('islandora:compound_collection');
        $tuque = islandora_get_tuque_connection();
        $parent = $tuque->repository->constructObject(islandora_get_namespace($firstchild->id));
        $parent->label = $parentid;
        $parent->models = 'islandora:compoundCModel';
        islandora_basic_collection_add_to_collection($parent, $collection);
        $tuque->repository->ingestObject($parent);
      }
    }
    if ($parent === FALSE) {
      return;
    }
    $_SESSION['ubl_batch_compound_parent2child'][$parentid]['singleparentid'] = $parent->id;
    $_SESSION['ubl_batch_compound_parent2child'][$parentid]['usesingleparent'] = TRUE;
  }

  $context['message'] = t('Generating compound object @parentid', array('@parentid' => $parent->id));

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

/**
 * Finish making the compounds.
 */
function ubl_batch_compound_batch_set_children_finished() {
  $_SESSION['ubl_batch_compound_finished'] = TRUE;
}

/* helper functions */

/**
 * Finding the Fedora object IDs of objects by identifier.
 */
function find_fedora_object_ids($identifier, $exactmatch = FALSE, $query_field = FALSE) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora_solr_search', 'includes/query_processor');
  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');

  static $alreadyfound = array();

  if (islandora_is_valid_pid($identifier)) {
    // This looks like a islandora object id.
    if (array_key_exists($identifier, $alreadyfound)) {
      return $alreadyfound[$identifier];
    }

    $obj = islandora_object_load($identifier);

    if ($obj) {
      $alreadyfound[$identifier] = array($identifier);

      return $alreadyfound[$identifier];
    }
  }
  static $qp = NULL;
  static $qpc = 0;
  if (is_null($qp) || $qpc > 10) {
    // Get a fresh copy the first time and after each 10 queries.
    // Apparently there are limitations on the amount of queries it can handle.
    $qp = new IslandoraSolrQueryProcessor();
    $qpc = 0;
  }
  $qpc++;

  $escapedid = islandora_solr_lesser_escape($identifier);
  if ($query_field) {
    $search = "$query_field:$escapedid";
  }
  else {
    $search = "catch_all_fields_mt:$escapedid";
  }

  if (array_key_exists($search, $alreadyfound)) {
    return $alreadyfound[$search];
  }

  $qp->buildAndExecuteQuery($search);

  $r = $qp->islandoraSolrResult;
  $numfound = $r['response']['numFound'];

  $objectids = array();
  for ($i = 0; $i < $numfound; $i++) {
    $objid = $r['response']['objects'][$i]['PID'];
    
    if ($exactmatch) {
      $hasexactmatch = FALSE;
      $solrdoc = $r['response']['objects'][$i]['solr_doc'];
      if ($query_field) {
        if ($solrdoc[$query_field] === $identifier) {
          $hasexactmatch = TRUE;
        }
      }
      else {
        foreach ($solrdoc as $solrkey => $solrval) {
          if (is_array($solrval)) {
            foreach ($solrval as $sv) {
              if ($sv === $identifier) {
                $hasexactmatch = TRUE;
                break 2;
              }
            }
          }
          else {
            if ($solrval === $identifier) {
              $hasexactmatch = TRUE;
              break;
            }
          }
        }
      }
      if (!$hasexactmatch) {
        continue;
      }
    }

    if ($objid) {
      $objectids[] = $objid;
    }
  }

  $alreadyfound[$search] = $objectids;

  return $objectids;
}
