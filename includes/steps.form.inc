<?php

/**
 * @file
 * steps.form.inc
 */

/**
 * The form-part of the steps of the workflow
 *
 * @param array $form
 *   The Drupal form definition.
 */
function islandora_prepare_ingest_steps($workflowid, $workflowresults, $workflowdata, $type) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

  $path = drupal_get_path('module', 'islandora_prepare_ingest');
  drupal_add_js($path . '/js/islandora_prepare_ingest.js');
  drupal_add_css($path . '/css/islandora_prepare_ingest.css');

  $ischecking = ($type === 'check');
  $istesting = ($type === 'test');
  $canmovesteps = ($type === 'edit');
  $canchangeconfig = ($type === 'edit' || $type === 'activeedit' || $type === 'check' || $type === 'test');
  $isactiveedit = ($type === 'activeedit');
  $isdryrun = ($type === 'dryrun');

  $form = array();

  $groups = array();

  # steps
  $form['steps'] = array(
    '#title' => t('Steps'), 
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  
  # Workflow steps
  $stepsinfo = module_invoke_all('workflow_step_info');
  $workflowsteps = islandora_prepare_ingest_select_workflow_steps($workflowid);
  $j = 0;
  $showdata = FALSE;
  $showfiles = FALSE;
  foreach ($workflowsteps as $workflowstep) {
    $j++;
    $stepname = $workflowstep['step_name'];
    $stepid = $workflowstep['id'];
    $data = isset($workflowdata[$stepid]['data'])?$workflowdata[$stepid]['data']:array();
    $context = isset($workflowdata[$stepid]['context'])?$workflowdata[$stepid]['context']:array();

    $stepconfig = islandora_prepare_ingest_select_workflow_step_configuration($stepid);
    if ($istesting || $isdryrun) {
      $showdata = ($stepsinfo[$stepname]['changes count'] || $stepsinfo[$stepname]['changes keys']);
      $showfiles = ($stepsinfo[$stepname]['changes files']);
    }
    if (array_key_exists('visual', $stepsinfo[$stepname]) && in_array('groupend', $stepsinfo[$stepname]['visual'])) {
      array_pop($groups);
      array_pop($groups);
    }
    $steps =& $form['steps'];
    foreach ($groups as $group) {
      $steps =& $steps[$group];
    }
    $workflowresult = isset($workflowresults[$stepid])?$workflowresults[$stepid]:NULL;
    $steps['step_' . $stepid] = islandora_prepare_ingest_make_base_step_form($workflowstep, $stepsinfo[$stepname], $stepconfig, $workflowresult, $stepid, $stepname, $canmovesteps, $canchangeconfig, $istesting, $ischecking, $isactiveedit);
    if ($ischecking || $istesting) {
      if ($j == count($workflowsteps)) {
        $steps['step_' . $stepid]['#collapsed'] = FALSE;
      }
    }

    if (($istesting || $isdryrun) && $showdata) {
      $islimited = FALSE;
      $header = array();
      $rows = array();
      $limitbytype = array();
      foreach ($data as $d) {
	$type = implode('_', array_keys($d));
	if (!array_key_exists($type, $limitbytype)) {
	  $limitbytype[$type] = 0;
	}
	$limitbytype[$type]++;
	if ($limitbytype[$type] <= 5) {
	 $rows[] = $d;
	}
	else {
	  $islimited = TRUE;
	}
	$header = array_merge($header, array_keys($d));
      }
      $header = array_unique($header);
      $hasdata = (count($header) > 0);
      if (!$hasdata) {
	$header[] = t('No data');
      }
      $table = '<table class="checkdatatable"><tr>';
      if ($hasdata) {
	$table .= '<th></th>';
      }
      foreach ($header as $h) {
	$table .= '<th>' . $h . '</th>';
      }
      $table .= '</tr>';
      foreach ($rows as $i => $row) {
	$table .= '<tr>';
	$table .= '<th>' . $i . '</th>';
	foreach ($header as $h) {
	  $value = (array_key_exists($h, $row)?$row[$h]:'-');
          if (!is_string($value)) {
	    $value = (string)$value;
          }
          if (strlen($value) > 28) {
            $shortvalue = substr($value, 0, 13) . '...' . substr($value, -12);
            $shortvalue = check_plain($shortvalue);
            $value = check_plain($value);
            $value = str_replace("\n", "<BR/>", $value);
	    $table .= '<td><span class="upi_fullvalue">' . $value . '</span><span class="upi_shortvalue">' . $shortvalue . '</span></td>';
          }
          else {
            $value = check_plain($value);
            $table .= '<td>' . $value . '</td>';
          }
	}
	$table .= '</tr>';
      }
      if ($hasdata && $islimited) {
	$table .= '<tr><td colspan="'.(count($header) + 1).'">The data above are only the first 5 samples of each type</td></tr>';
      }

      $table .= '</table>';
      $steps['step_' . $stepid]['datalisting'] = array(
        '#type' => 'fieldset',
        '#title' => t('Data'),
      );
      $steps['step_' . $stepid]['datalisting']['datatable'] = array(
	'#markup' => $table,
      );
    }
    if (($istesting || $isdryrun) && $showfiles) {
      $header = array('number' => '', 'filepath' => t('Filepath'), 'type' => t('Type'));
      $files = array_key_exists('_PIFS', $context)?array_keys($context['_PIFS']):array();
      sort($files);
      if (count($files) > 0) {
	$table = '<table class="checkdatatable"><tr>';
	foreach ($header as $h) {
	  $table .= '<th>' . $h . '</th>';
	}
	$table .= '</tr>';
	$lastdir = '';
	$countperdir = array();
	$islimited = FALSE;
	foreach ($files as $i => $file) {
	  $isdir = pifs_is_dir($file, $context);
	  $parentdir = dirname($file);
	  if (!array_key_exists($parentdir, $countperdir)) {
	    $countperdir[$parentdir] = 0;
	  }
	  $countperdir[$parentdir]++;
	  $showfile = TRUE;
	  while (strlen($parentdir) > 1) {
	    if (!array_key_exists($parentdir, $countperdir)) {
	      break;
	    }
	    if ($countperdir[$parentdir] > 5) {
	      $showfile = FALSE;
	      $islimited = TRUE;
	      break;
	    }
	    $parentdir = dirname($parentdir);
	  }
	  if ($showfile) {
	    $table .= '<tr>';
	    $table .= '<th>' . $i . '</th>';
	    $table .= '<td>' . check_plain($file) . '</td>'; 
	    $table .= '<td>' . ($isdir?t('Directory'):t('File')) . '</td>'; 
	    $table .= '</tr>';
	  }
	}
	if ($islimited) {
	  $table .= '<tr><td colspan="'.(count($header) + 1).'">The file listing is limited to 5 items per directory</td></tr>';
	}

	$table .= '</table>';
        $steps['step_' . $stepid]['filelisting'] = array(
          '#type' => 'fieldset',
          '#title' => t('File listing (virtual)'),
        );
	$steps['step_' . $stepid]['filelisting']['filetable'] = array(
	  '#markup' => $table,
	);
      }
    }
    if (!$steps['step_' . $stepid]['#collapsed']) {
      $len = count($groups);
      $scsteps =& $form['steps'];
      for ($i = 0; $i < $len; $i += 2) {
        $groupname = $groups[$i];
        $scsteps =& $scsteps[$groupname];
        $scsteps['#collapsed'] = FALSE;
        $scsteps =& $scsteps['grouped'];
      }
    }
    if (array_key_exists('visual', $stepsinfo[$stepname]) && in_array('groupstart', $stepsinfo[$stepname]['visual'])) {
      $groups[] = 'step_' . $stepid;
      $groups[] = 'grouped';
      $steps['step_' . $stepid]['grouped'] = array(
        '#type' => 'container',
	'#attributes' => array(
	  'class' => array('grouped_steps'),
	),
        '#weight' => 3,
      );
    }
  }
  return $form;
}



function islandora_prepare_ingest_make_base_step_form($workflowstep, $stepinfo, $stepconfig, $workflowresult, $stepid, $stepname, $canmovesteps, $canchangeconfig, $istesting, $ischecking, $isactiveedit) {
  $newstep = array(
    '#title' => islandora_prepare_ingest_label_for_step($stepname, $stepconfig),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#attributes' => array(
      'class' => array('workflow_step', $stepname),
    ),
  );
  if (isset($workflowresult) && is_array($workflowresult) && count($workflowresult) > 0) {
    $newstep['#collapsed'] = FALSE;
    foreach ($workflowresult as $i => $result) {
      if ($i >= 10) {
        break;
      }
      $newstep['check_description' . '_' . $i] = array(
        '#type' => 'item',
        '#markup' => '<SPAN class="' . $result['type'] . '">' . $result['text'] . '</SPAN>',
      );
    }
    if (count($workflowresult) >= 10) {
      $newstep['check_description' . '_' . count($workflowresult)] = array(
        '#type' => 'item',
        '#markup' => '<SPAN class="warning">Other ' . (count($workflowresult) - 10) . ' errors suppressed</SPAN>',
      );
    }
  }
  if ($canmovesteps) {
    $newstep['fields'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('fields'),
      ),
      '#weight' => 1,
    );
    $newstep['fields']['remove_step_' . $stepid] = array(
      '#type' => 'hidden',
      '#default_value' => '0',
      '#attributes' => array(
        'class' => array('remove_step'),
      ),
    );
    $newstep['fields']['weight_step_' . $stepid] = array(
      '#type' => 'hidden',
      '#default_value' => $workflowstep['weight'],
      '#attributes' => array(
        'class' => array('weight_step'),
      ),
    );
  }
  if ($canchangeconfig) {
    $arguments = $stepinfo['arguments'];
    foreach ($arguments as $argname => $argvalues) {
      $value = '';
      if (array_key_exists($argname, $stepconfig)) {
        $value = $stepconfig[$argname];
      }
      elseif (array_key_exists('default_value', $argvalues)) {
        $value = $argvalues['default_value'];
      }
      
      if ($istesting || $ischecking) {
        if (strlen($value) > 0) {
          continue; // skip filled in arguments when testing or checking
        }
        if (array_key_exists('optional', $argvalues)) {
          if (is_array($argvalues['optional'])) {
            foreach ($argvalues['optional'] as $optkey => $optvalues) {
              if (in_array($stepconfig[$optkey], $optvalues)) {
                continue 2;
              }
            }
          }
          else {
            if ($argvalues['optional']) {
              continue;
            }
          }
        }
        $newstep['#collapsed'] = FALSE;
      }
      if ($isactiveedit) {
        if (strlen($value) == 0) {
          $newstep['#collapsed'] = FALSE;
        }
      }
      $title = $argvalues['label'];
      if ($istesting || $ischecking) {
        $title .= ' ' . t('(please provide a test value)');  
      }
      $newstep['fields']['step_' . $stepid . '_' . $argname] = array(
        '#title' => $title,
        '#description' => array_key_exists('description', $argvalues)?$argvalues['description']:'',
      );
      if ($argvalues['type'] === 'boolean') {
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'select';
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#options'] = array(
          '' => t('Choose value'),
          '0' => t('False'),
          '1' => t('True'),
        );
      }
      elseif ($argvalues['type'] === 'select') {
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'select';
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#options'] = $argvalues['options'];
      }
      elseif ($argvalues['type'] === 'template' || $argvalues['type'] === 'keystemplate' || $argvalues['type'] === 'text') {
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'textarea';
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#cols'] = 100;
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#rows'] = 4;
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#attributes'] = array(
          'autocomplete' => 'off',
          'autocorrect' => 'off',
          'autocapitalize' => 'off',
          'spellcheck' => 'false',
        );
      }
      elseif ($argvalues['type'] === 'collection') {
        module_load_include('inc', 'islandora_solution_pack_collection', 'includes/utilities');
        $collections = islandora_basic_collection_get_collections();
        if (count($collections) < 100) {
          $map_options = function($o) {
            return filter_xss($o['label']);
          };
          $options = array('' => t('Choose a collection'));
          $options = array_merge($options, array_map($map_options, islandora_basic_collection_get_collections()));
 
          $newstep['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'select';
          $newstep['fields']['step_' . $stepid . '_' . $argname]['#options'] = $options;
        }
        else {
          $newstep['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'textfield';
          $newstep['fields']['step_' . $stepid . '_' . $argname]['#size'] = 60;
        }
      }
      else {
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'textfield';
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#size'] = 60;
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#attributes'] = array(
          'autocomplete' => 'off',
          'autocorrect' => 'off',
          'autocapitalize' => 'off',
          'spellcheck' => 'false',
        );
      }
      $newstep['fields']['step_' . $stepid . '_' . $argname]['#default_value'] = $value;
      if (array_key_exists('input keys', $stepinfo) && in_array($argname, $stepinfo['input keys'])) {
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#attributes']['class'][] = 'input_key'; 
      }
      if (array_key_exists('output keys', $stepinfo) && in_array($argname, $stepinfo['output keys'])) {
        $newstep['fields']['step_' . $stepid . '_' . $argname]['#attributes']['class'][] = 'output_key'; 
      }
      $newstep['fields']['step_' . $stepid . '_' . $argname]['#attributes']['class'][] = $argvalues['type'];
    }
  }
    if ($canmovesteps) {
      $newstep['buttons'] = array(
        '#type' => 'actions',
        '#attributes' => array(
          'class' => array('buttons'),
        ),
        '#weight' => 2,
      );
      if (!(array_key_exists('visual', $stepinfo) && in_array('nomove', $stepinfo['visual']))) {
        $newstep['buttons']['moveto'] = array(
          '#type' => 'button',
          '#value' => t('Move to'),
          '#attributes' => array(
            'class' => array('moveto_step_button'),
          ),
        );
      }
      if (array_key_exists('visual', $stepinfo) && in_array('ungroup', $stepinfo['visual'])) {
        $newstep['buttons']['ungroup'] = array(
          '#type' => 'button',
          '#value' => t('Ungroup'),
          '#attributes' => array(
            'class' => array('ungroup_step_button'),
          ),
        );
      }
      if (!(array_key_exists('visual', $stepinfo) && in_array('noremove', $stepinfo['visual']))) {
        $newstep['buttons']['remove'] = array(
          '#type' => 'button',
          '#value' => t('Remove'),
          '#attributes' => array(
            'class' => array('remove_step_button'),
          ),
        );
        if (array_key_exists('visual', $stepinfo) && in_array('groupremove', $stepinfo['visual'])) {
          $newstep['buttons']['remove']['#attributes']['class'][] = 'groupremove';
        }
      }
    }

  return $newstep;
}

function islandora_prepare_ingest_report_result_summary($workflowid, $reporttype, $workflowresults) {
  $errorcount = 0;
  $warningcount = 0;
  if (is_array($workflowresults) && count($workflowresults) > 0) {
    foreach ($workflowresults as $stepid => $workflowres) {
      if (is_array($workflowres)) {
        foreach ($workflowres as $wfres) {
          $type = $wfres['type'];
          if (strpos($type, 'error') !== FALSE) {
            $errorcount++;
          }
          elseif (strpos($type, 'warning') !== FALSE) {
            $warningcount++;
          }
        }
      }
    }
  }
  $action = (($reporttype === 'check')?t('Checks'):(($reporttype === 'test')?t('Test'):t('Dry run')));
  if ($errorcount > 0) {
    drupal_set_message(t('@action failed with !count errors. See below for details.', array('@action' => $action, '!count' => $errorcount)), 'error', FALSE);
  }
  if ($warningcount > 0) {
    drupal_set_message(t('@action failed with !count warnings. See below for details.', array('@action' => $action, '!count' => $warningcount)), 'warning', FALSE);
  }
  if (($errorcount + $warningcount) == 0) {
    drupal_set_message(t('@action finished successful!', array('@action' => $action)));
    if ($reporttype === 'check') {
      drupal_set_message(t('You can test this workflow now. Fill out the missing values and press the test workflow button below.'), 'status', FALSE);
    }
    if ($reporttype === 'dryrun') {
      global $user;
      drupal_set_message(t('You can run this workflow now for real on the commandline:'), 'status', FALSE);
      drupal_set_message(t('drush -v --user=!user prepare_ingest --workflow=!workflowid', array('!workflowid' => $workflowid, '!user' => $user->name)), 'status', FALSE);
    }
  }
  return (($errorcount + $warningcount) == 0);
}
