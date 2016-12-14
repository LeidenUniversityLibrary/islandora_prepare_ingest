<?php
error_reporting(E_ALL);
ini_set("display_errors", 1);


/**
 * @file
 * ubl_prepare_ingest.steps.form.inc
 */

/**
 * The form-part of the steps of the workflow
 *
 * @param array $form
 *   The Drupal form definition.
 */
function ubl_prepare_ingest_steps($workflowid, $workflowresults, $workflowdata, $type) {
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/db');
  module_load_include('inc', 'ubl_prepare_ingest', 'includes/pifs');

  $ischecking = ($type === 'check');
  $istesting = ($type === 'test');
  $canmovesteps = ($type === 'edit');
  $canchangeconfig = ($type === 'edit' || $type === 'check' || $type === 'test');
  $isdryrun = ($type === 'dryrun');

  $form = array();

  # steps
  $form['steps'] = array(
    '#title' => t('Steps'), 
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  
  # Workflow steps
  $stepsinfo = module_invoke_all('workflow_step_info');
  $workflowsteps = ubl_prepare_ingest_select_workflow_steps($workflowid);
  $j = 0;
  $showdata = FALSE;
  $showfiles = FALSE;
  foreach ($workflowsteps as $workflowstep) {
    $j++;
    $showcontent = FALSE;
    $stepname = $workflowstep['step_name'];
    $stepid = $workflowstep['id'];
    $data = isset($workflowdata[$stepid]['data'])?$workflowdata[$stepid]['data']:array();
    $context = isset($workflowdata[$stepid]['context'])?$workflowdata[$stepid]['context']:array();

    $stepconfig = ubl_prepare_ingest_select_workflow_step_configuration($stepid);
    if ($istesting || $isdryrun) {
      $showdata = ($stepsinfo[$stepname]['changes count'] || $stepsinfo[$stepname]['changes keys']);
      $showfiles = ($stepsinfo[$stepname]['changes files']);
    }
    $form['steps']['step_' . $stepid] = array(
      '#title' => ubl_prepare_ingest_label_for_step($stepname, $stepconfig),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#attributes' => array(
        'class' => array('workflow_step'),
      ),
    );
    if (isset($workflowresults[$stepid]) && is_array($workflowresults[$stepid])) {
      $showcontent = TRUE;
      foreach ($workflowresults[$stepid] as $i => $result) {
        if ($i >= 10) {
          break;
        }
	$form['steps']['step_' . $stepid]['check_description' . '_' . $i] = array(
	  '#type' => 'item',
	  '#markup' => '<SPAN class="' . $result['type'] . '">' . $result['text'] . '</SPAN>',
	);
      }
      if (count($workflowresults[$stepid]) >= 10) {
	$form['steps']['step_' . $stepid]['check_description' . '_' . count($workflowresults[$stepid])] = array(
	  '#type' => 'item',
	  '#markup' => '<SPAN class="warning">Other ' . (count($workflowresults[$stepid]) - 10) . ' errors suppressed</SPAN>',
	);
      }
    }
    if ($ischecking || $istesting) {
      if ($j == count($workflowsteps)) {
        $showcontent = TRUE;
      }
    }
    if ($canmovesteps) {
      $form['steps']['step_' . $stepid]['fields'] = array(
	'#type' => 'container',
	'#attributes' => array(
	  'class' => array('fields'),
	),
      );
      $form['steps']['step_' . $stepid]['fields']['remove_step_' . $stepid] = array(
	'#type' => 'hidden',
	'#default_value' => '0',
	'#attributes' => array(
	  'class' => array('remove_step'),
	),
      );
      $form['steps']['step_' . $stepid]['fields']['weight_step_' . $stepid] = array(
	'#type' => 'hidden',
	'#default_value' => $workflowstep['weight'],
	'#attributes' => array(
	  'class' => array('weight_step'),
	),
      );
    }
    if ($canchangeconfig) {
     $arguments = $stepsinfo[$stepname]['arguments'];
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
	 $showcontent = TRUE;
       }
       $title = $argvalues['label'];
       if ($istesting || $ischecking) {
         $title .= ' ' . t('(please provide a test value)');  
       }
       $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname] = array(
	 '#title' => $title,
	 '#description' => array_key_exists('description', $argvalues)?$argvalues['description']:'',
       );
       if ($argvalues['type'] === 'boolean') {
	 $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'select';
	 $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#options'] = array(
	   '' => t('Choose value'),
	   '0' => t('False'),
	   '1' => t('True'),
	 );
       }
       elseif ($argvalues['type'] === 'select') {
	 $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'select';
	 $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#options'] = $argvalues['options'];
       }
       elseif ($argvalues['type'] === 'template' || $argvalues['type'] === 'text') {
	 $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'textarea';
	 $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#cols'] = 100;
	 $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#rows'] = 4;
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
  
           $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'select';
           $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#options'] = $options;
         }
         else {
           $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'textfield';
           $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#size'] = 60;
         }
       }
       else {
	 $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'textfield';
	 $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#size'] = 60;
       }
       $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#default_value'] = $value;
       if (array_key_exists('input keys', $stepsinfo[$stepname]) && in_array($argname, $stepsinfo[$stepname]['input keys'])) {
	 $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#attributes']['class'][] = 'input_key'; 
       }
       if (array_key_exists('output keys', $stepsinfo[$stepname]) && in_array($argname, $stepsinfo[$stepname]['output keys'])) {
	 $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#attributes']['class'][] = 'output_key'; 
       }
       $form['steps']['step_' . $stepid]['fields']['step_' . $stepid . '_' . $argname]['#attributes']['class'][] = $argvalues['type'];
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
	$header += array_flip($d);
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
	  if (strlen($value) > 45) {
	    $value = substr($value, 0, 20) . '...' . substr($value, -23);
	  }
	  $value = check_plain($value);
	  $table .= '<td>' . $value . '</td>';
	}
	$table .= '</tr>';
      }
      if ($hasdata && $islimited) {
	$table .= '<tr><td colspan="'.(count($header) + 1).'">The data above are only the first 5 samples of each type</td></tr>';
      }

      $table .= '</table>';
      $form['steps']['step_' . $stepid]['datalisting'] = array(
        '#type' => 'fieldset',
        '#title' => t('Data'),
      );
      $form['steps']['step_' . $stepid]['datalisting']['datatable'] = array(
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
        $form['steps']['step_' . $stepid]['filelisting'] = array(
          '#type' => 'fieldset',
          '#title' => t('File listing (virtual)'),
        );
	$form['steps']['step_' . $stepid]['filelisting']['filetable'] = array(
	  '#markup' => $table,
	);
      }
    }
    $form['steps']['step_' . $stepid]['#collapsed'] = !$showcontent;
    if ($canmovesteps) {
      $form['steps']['step_' . $stepid]['buttons'] = array(
	'#type' => 'actions',
	'#attributes' => array(
	  'class' => array('buttons'),
	),
	'moveup' => array(
	  '#type' => 'button',
	  '#value' => t('Move up'),
	  '#attributes' => array(
	    'class' => array('moveup_step_button'),
	  ),
	),
	'movedown' => array(
	  '#type' => 'button',
	  '#value' => t('Move down'),
	  '#attributes' => array(
	    'class' => array('movedown_step_button'),
	  ),
	),
	'remove' => array(
	  '#type' => 'button',
	  '#value' => t('Remove'),
	  '#attributes' => array(
	    'class' => array('remove_step_button'),
	  ),
	),
      );
    }
  }
  return $form;
}

function ubl_prepare_ingest_report_result_summary($workflowid, $reporttype, $workflowresults) {
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