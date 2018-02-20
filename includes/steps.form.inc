<?php

/**
 * @file
 * steps.form.inc
 *
 *
 *  Copyright 2017-2018 Leiden University Library
 *
 *  This file is part of islandora_prepare_ingest.
 *
 *  islandora_prepare_ingest is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * The form-part of the steps of the workflow
 *
 * @param array $form
 *   The Drupal form definition.
 */
function islandora_prepare_ingest_steps($workflowid, $workflowresults, $otherid) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/db');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

  $path = drupal_get_path('module', 'islandora_prepare_ingest');
  drupal_add_js($path . '/js/islandora_prepare_ingest.js');
  drupal_add_css($path . '/css/islandora_prepare_ingest.css');

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

  $hasdata = ($otherid !== NULL);
  $hasfiles = ($otherid !== NULL);

  $j = 0;
  foreach ($workflowsteps as $workflowstep) {
    $j++;
    $stepname = $workflowstep['step_name'];
    $stepid = $workflowstep['id'];
    $stepinfo = $stepsinfo[$stepname];
    $stepclass = $stepinfo['class name'];
    $stepconfig = islandora_prepare_ingest_select_workflow_step_configuration($stepid);
    $step = new $stepclass($workflowstep, $stepconfig, $otherid);

    $showdata = ($step->changesCount() || $step->changesKeys());
    $showfiles = $step->changesFiles();
    if (in_array('groupend', $step->visual())) {
      array_pop($groups);
      array_pop($groups);
    }
    $steps =& $form['steps'];
    foreach ($groups as $group) {
      $steps =& $steps[$group];
    }
    $workflowresult = isset($workflowresults[$stepid])?$workflowresults[$stepid]:NULL;

    $steps['step_' . $stepid] = islandora_prepare_ingest_make_base_step_form($step, $workflowresult);
    if (($hasdata && $showdata) || ($hasfiles && $showfiles)) {
      if ($j == count($workflowsteps)) {
        $steps['step_' . $stepid]['#collapsed'] = FALSE;
      }
      $steps['step_' . $stepid]['tabs'] = array(
        '#type' => 'actions',
        '#attributes' => array(
          'class' => array('tabs'),
        ),
        '#weight' => -998,
      );
      $steps['step_' . $stepid]['tabs']['show_data'] = array(
        '#type' => 'button',
        '#value' => t('Show data'),
        '#attributes' => array(
          'class' => array('show_data_button'),
        ),
      );
      $steps['step_' . $stepid]['tabs']['edit'] = array(
        '#type' => 'button',
        '#value' => t('Edit'),
        '#attributes' => array(
          'class' => array('edit_button'),
        ),
      );
    }

    $inputkeys = $step->inputKeys();
    $flatinputkeys = array();
    array_walk_recursive($inputkeys, function($value) use(&$flatinputkeys) {
        $flatinputkeys[] = $value;
    });
    $outputkeys = $step->outputKeys();
    $flatoutputkeys = array();
    array_walk_recursive($outputkeys, function($value) use(&$flatoutputkeys) {
        $flatoutputkeys[] = $value;
    });
    if ($hasdata && $showdata) {
      $steps['step_' . $stepid]['datalisting'] = array(
        '#type' => 'fieldset',
        '#title' => t('Data'),
        '#attributes' => array(
          'class' => array('datacache'),
          'data-stepid' => $stepid,
          'data-type' => 1,
          'data-startitemnr' => 1,
          'data-enditemnr' => 10,
          'data-inputkeys' => implode(',', $flatinputkeys),
          'data-outputkeys' => implode(',', $flatoutputkeys),
        ),
      );
    }
    if ($hasfiles && $showfiles) {
      $steps['step_' . $stepid]['filelisting'] = array(
        '#type' => 'fieldset',
        '#title' => t('File listing (virtual)'),
        '#attributes' => array(
          'class' => array('datacache'),
          'data-stepid' => $stepid,
          'data-type' => 2,
          'data-startitemnr' => 1,
          'data-enditemnr' => 10,
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
    if (in_array('groupstart', $step->visual())) {
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



function islandora_prepare_ingest_make_base_step_form($step, $workflowresult) {
  $stepid = $step->id();
  $stepform = array(
    '#title' => $step->label(),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#attributes' => array(
      'class' => array('workflow_step', $step->name()),
    ),
  );
  foreach (array('check', 'dryrun', 'verify', 'prepare') as $type) {
    if (isset($workflowresult[$type]) && count($workflowresult[$type]) > 0) {
      $stepform['#collapsed'] = FALSE;
      foreach ($workflowresult[$type] as $i => $result) {
        if ($i >= 10) {
          break;
        }
        $stepform[$type . '_description' . '_' . $i] = array(
          '#type' => 'item',
          '#markup' => '<SPAN class="' . $result['type'] . '">' . $result['text'] . '</SPAN>',
          '#weight' => -999 + $i,
        );
      }
      if (count($workflowresult[$type]) >= 10) {
        $stepform[$type . '_description' . '_' . count($workflowresult[$type])] = array(
          '#type' => 'item',
          '#markup' => '<SPAN class="warning">Other ' . (count($workflowresult[$type]) - 10) . ' errors suppressed</SPAN>',
          '#weight' => -999 + 11,
        );
      }
    }
  }
  $stepform['fields'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('fields'),
    ),
    '#weight' => 1,
  );
  $stepform['fields']['remove_step_' . $stepid] = array(
    '#type' => 'hidden',
    '#default_value' => '0',
    '#attributes' => array(
      'class' => array('remove_step'),
    ),
  );
  $stepform['fields']['weight_step_' . $stepid] = array(
    '#type' => 'hidden',
    '#default_value' => $step->weight(),
    '#attributes' => array(
      'class' => array('weight_step'),
    ),
  );
  $arguments = $step->arguments();
  foreach ($arguments as $argname => $argvalues) {
    $value = $step->valueForKey($argname);
    $title = $argvalues['label'];
    $stepform['fields']['step_' . $stepid . '_' . $argname] = array(
      '#title' => $title,
      '#description' => array_key_exists('description', $argvalues)?$argvalues['description']:'',
    );
    if ($argvalues['type'] === 'boolean') {
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'select';
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#options'] = array(
        '' => t('Choose value'),
        '0' => t('False'),
        '1' => t('True'),
      );
    }
    elseif ($argvalues['type'] === 'select') {
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'select';
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#options'] = $argvalues['options'];
    }
    elseif ($argvalues['type'] === 'template' || $argvalues['type'] === 'keystemplate' || $argvalues['type'] === 'text') {
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'textarea';
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#cols'] = 100;
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#rows'] = 4;
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#attributes'] = array(
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

        $stepform['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'select';
        $stepform['fields']['step_' . $stepid . '_' . $argname]['#options'] = $options;
      }
      else {
        $stepform['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'textfield';
        $stepform['fields']['step_' . $stepid . '_' . $argname]['#size'] = 60;
      }
    }
    else {
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#type'] = 'textfield';
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#size'] = 60;
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#attributes'] = array(
        'autocomplete' => 'off',
        'autocorrect' => 'off',
        'autocapitalize' => 'off',
        'spellcheck' => 'false',
      );
    }
    $stepform['fields']['step_' . $stepid . '_' . $argname]['#default_value'] = $value;
    if (in_array($argname, array_keys($step->inputKeys()))) {
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#attributes']['class'][] = 'input_key'; 
    }
    if (in_array($argname, array_keys($step->outputKeys()))) {
      $stepform['fields']['step_' . $stepid . '_' . $argname]['#attributes']['class'][] = 'output_key'; 
    }
    $stepform['fields']['step_' . $stepid . '_' . $argname]['#attributes']['class'][] = $argvalues['type'];
  }
  $stepform['buttons'] = array(
    '#type' => 'actions',
    '#attributes' => array(
      'class' => array('buttons'),
    ),
    '#weight' => 2,
  );
  if (!in_array('nomove', $step->visual())) {
    $stepform['buttons']['moveto'] = array(
      '#type' => 'button',
      '#value' => t('Move to'),
      '#attributes' => array(
        'class' => array('moveto_step_button'),
      ),
    );
  }
  if (in_array('ungroup', $step->visual())) {
    $stepform['buttons']['ungroup'] = array(
      '#type' => 'button',
      '#value' => t('Ungroup'),
      '#attributes' => array(
        'class' => array('ungroup_step_button'),
      ),
    );
  }
  if (!in_array('noremove', $step->visual())) {
    $stepform['buttons']['remove'] = array(
      '#type' => 'button',
      '#value' => t('Remove'),
      '#attributes' => array(
        'class' => array('remove_step_button'),
      ),
    );
    if (in_array('groupremove', $step->visual())) {
      $stepform['buttons']['remove']['#attributes']['class'][] = 'groupremove';
    }
  }

  return $stepform;
}

function islandora_prepare_ingest_report_result_summary($workflowid, $workflowresults) {
  // TODO: weghalen?
  $errorcount = 0;
  $warningcount = 0;
  if (is_array($workflowresults) && count($workflowresults) > 0) {
    foreach ($workflowresults as $stepid => $workflowres) {
      if (is_array($workflowres)) {
        foreach ($workflowres as $wftype => $wfres) {
          if (count($wfres) > 0) {
            foreach ($wfres as $res) {
              $type = $res['type'];
              if (strpos($type, 'error') !== FALSE || strpos($type, 'check') !== FALSE) {
                $errorcount++;
              }
              elseif (strpos($type, 'warn') !== FALSE) {
                $warningcount++;
              }
            }
          }
        }
      }
    }
  }
  $haserrors = (($errorcount + $warningcount) > 0);
  if ($errorcount > 0) {
    drupal_set_message(t('Dryrun failed with !count errors. See below for details.', array('!count' => $errorcount)), 'error', FALSE);
  }
  if ($warningcount > 0) {
    drupal_set_message(t('Dryrun failed with !count warnings. See below for details.', array('!count' => $warningcount)), 'warning', FALSE);
  }
  if (!$haserrors) {
    drupal_set_message(t('Dryrun finished successful!'));
  }
  return !$haserrors;
}
