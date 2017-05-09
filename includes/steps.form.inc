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
function islandora_prepare_ingest_steps($workflowid, $workflowresults, $type) {
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
      $steps['step_' . $stepid]['datalisting'] = array(
        '#type' => 'fieldset',
        '#title' => t('Data'),
        '#attributes' => array(
          'class' => array('datacache'),
          'data-stepid' => $stepid,
          'data-type' => 1,
          'data-minitemnr' => 0,
          'data-maxitemnr' => 10,
        ),
      );
    }
    if (($istesting || $isdryrun) && $showfiles) {
      $steps['step_' . $stepid]['filelisting'] = array(
        '#type' => 'fieldset',
        '#title' => t('File listing (virtual)'),
        '#attributes' => array(
          'class' => array('datacache'),
          'data-stepid' => $stepid,
          'data-type' => 2,
          'data-minitemnr' => 0,
          'data-maxitemnr' => 10,
        ),
      );
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
