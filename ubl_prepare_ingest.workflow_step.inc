<?php

/**
 * @file
 * Step info.
 */

// only checks if the proper keys are given by the previous step
define('WORK_EXTENT_ONLY_CHECK_INPUT', 1);

// only checks if the arguments are all filled out.
define('WORK_EXTENT_ONLY_CHECK_FOR_EMPTY_ARGUMENTS', 2);

// only checks if the filled arguments are filled out with the correct values.
define('WORK_EXTENT_ONLY_CHECK_FILLED_ARGUMENTS', 4);

// only checks if all arguments are filled out and have the correct values.
define('WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS', WORK_EXTENT_ONLY_CHECK_FOR_EMPTY_ARGUMENTS | WORK_EXTENT_ONLY_CHECK_FILLED_ARGUMENTS);

// only does a dry run: this means that all steps are done but writing of data is done to a sandbox (internal memory).
define('WORK_EXTENT_ONLY_DRY_RUN', 8);

// only does the actual work
define('WORK_EXTENT_ONLY_DO_ALL', 16);

// only checks if the proper keys are given by the previous step
define('WORK_EXTENT_CHECK_INPUT', WORK_EXTENT_ONLY_CHECK_INPUT);

// checks if the proper keys are given and checks if the filled arguments are filled out with the correct values.
define('WORK_EXTENT_CHECK_FILLED_ARGUMENTS', WORK_EXTENT_CHECK_INPUT | WORK_EXTENT_ONLY_CHECK_FILLED_ARGUMENTS); 

// checks if the proper keys are given and checks if the arguments are all filled out with the correct values.
define('WORK_EXTENT_CHECK_ALL_ARGUMENTS', WORK_EXTENT_CHECK_INPUT | WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS); 

// does the above two levels and does a dry run with testdata.
define('WORK_EXTENT_DRY_RUN_WITH_TESTDATA', WORK_EXTENT_ONLY_CHECK_INPUT | WORK_EXTENT_ONLY_CHECK_FILLED_ARGUMENTS | WORK_EXTENT_ONLY_DRY_RUN);

// does the above two levels and does a dry run.
define('WORK_EXTENT_DRY_RUN', WORK_EXTENT_CHECK_ALL_ARGUMENTS | WORK_EXTENT_ONLY_DRY_RUN);

// checks the proper keys and arguments, and does a full run if this checks out.
define('WORK_EXTENT_DO_ALL', WORK_EXTENT_CHECK_ALL_ARGUMENTS | WORK_EXTENT_ONLY_DO_ALL);


/**
 * Implements hook_workflow_step_info
 */
function ubl_prepare_ingest_workflow_step_info() {
  $info = array();

  $info['read_file_names'] = array(
    'arguments' => array(
      'directory_path' => array(
        'label' => t('Directory path'),
        'description' => t('Specify the absolute path to the directory where the input files are located.'),
        'type' => 'filepath',
      ),
      'filter' => array(
        'label' => t('Filter'),
        'description' => t('For example: *.tif'),
        'type' => 'string',
      ),
      'include_subdirectory_files' => array(
        'label' => t('Include files in subdirectories'),
        'type' => 'boolean',
      ),
      'target_key_name' => array(
        'label' => t('Target key name'),
        'type' => 'key',
      ),
    ),
    'input keys' => array(),
    'output keys' => array('target_key_name'),
    'changes count' => TRUE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
  );
  $info['add_key_regexp'] = array(
    'arguments' => array(
      'source_key_name' => array(
        'label' => t('Source key name'),
        'type' => 'key',
      ),
      'target_key_name' => array(
        'label' => t('Target key name'),
        'type' => 'key',
      ),
      'regexp' => array(
        'label' => t('Regular expression'),
        'type' => 'regexp',
      ),
      'replacement' => array(
        'label' => t('Replacement'),
        'type' => 'string',
      ),
    ),
    'input keys' => array('source_key_name'),
    'output keys' => array('target_key_name'),
    'changes count' => FALSE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
  );
  $info['add_key_template'] = array(
    'arguments' => array(
      'target_key_name' => array(
        'label' => t('Target key name'),
        'type' => 'key',
      ),
      'template' => array(
        'label' => t('Template'),
        'description' => t('Text with keys that will be replaced between curly braces.'),
        'type' => 'template',
      ),
    ),
    'input keys callback' => 'get_input_keys_add_key_template',
    'output keys' => array('target_key_name'),
    'changes count' => FALSE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
  );
  $info['add_key_counter'] = array(
    'arguments' => array(
      'target_key_name' => array(
        'label' => t('Target key name'),
        'type' => 'key',
      ),
      'start' => array(
        'label' => t('Start'),
        'type' => 'number',
        'default_value' => 1,
      ),
      'step' => array(
        'label' => t('Step'),
        'type' => 'number',
        'default_value' => 1,
      ),
      'width' => array(
        'label' => t('Width'),
        'description' => t('The width of the number, it will be prepended by zero\'s to meet this width.'),
        'type' => 'number',
        'default_value' => 1,
      ),
      'foreach_value' => array(
        'label' => t('Foreach value of key name'),
        'description' => t('Optional, start counting for each unique value of this key.'),
        'type' => 'key',
        'optional' => TRUE,
      ),
    ),
    'input keys' => array('foreach_value'),
    'output keys' => array('target_key_name'),
    'changes count' => FALSE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
  );
  $info['extract_keys_template'] = array(
    'arguments' => array(
      'source_key_name' => array(
	'label' => t('Source key name'),
	'type' => 'key',
      ),
      'target_keys_template' => array(
	'label' => t('Target keys template'),
	'description' => t('Template should reflect the value of the source key. The keys are in the place of the extracted value and between curly braces. Example: if the value is "test_1.tif" and you want to extract both the number and the extension, then you can use the following: test_{number}.{extension}'),
	'type' => 'keystemplate',
      ),
    ),
    'input keys' => array('source_key_name'),
    'output keys callback' => 'get_output_keys_extract_keys_template',
    'changes count' => FALSE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
  );
  $info['modify_key_value'] = array(
    'arguments' => array(
      'source_key_name' => array(
	'label' => t('Source key name'),
	'type' => 'key',
      ),
      'type' => array(
	'label' => t('Type'),
	'type' => 'select',
	'options' => array(
	  '' => t('-'),
	  'uppercase' => t('uppercase'),
	  'lowercase' => t('lowercase'),
	  'removehyphen' => t('remove -'),
	  'removeline' => t('remove - and _'),
	  'removespaces' => t('remove spaces'),
	  'trim' => t('trim spaces'),
	  'replacehyphen' => t('replace - with space'),
	  'replaceline' => t('replace - and _ with space'),
          'pad5zeros' => t('pad zero\s to number to get 5 digits'),
	  'striptags' => t('strip tags'),
	  'sha1' => t('SHA1 hash'),
	),
      ),
    ),
    'input keys' => array('source_key_name'),
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
  );
  $info['make_directory'] = array(
    'arguments' => array(
      'directory_key_name' => array(
        'label' => t('Directory key name'),
        'description' => t('This key should hold an absolute path to a not yet existing directory. The parent directory should exist already.'),
        'type' => 'key',
      ),
    ),
    'input keys' => array('directory_key_name'),
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => FALSE,
    'changes files' => TRUE,
  );
  $info['copy_file'] = array(
    'arguments' => array(
      'source_key_name' => array(
        'label' => t('Source key name'),
        'description' => t('The value for this key should hold an absolute path to an existing file.'),
        'type' => 'key',
      ),
      'target_key_name' => array(
        'label' => t('Target key name'),
        'description' => t('This key should hold an absolute path to an existing directory or the absolute path including the new name of the file to copy.'),
        'type' => 'key',
      ),
    ),
    'input keys' => array('source_key_name','target_key_name'),
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => FALSE,
    'changes files' => TRUE,
  );
  $info['write_file'] = array(
    'arguments' => array(
      'content_key_name' => array(
        'label' => t('Content key name'),
        'description' => t('This key should hold the content for the file.'),
        'type' => 'key',
      ),
      'target_key_name' => array(
        'label' => t('Target key name'),
        'description' => t('This key should hold an absolute path to a not existing file.'),
        'type' => 'key',
      ),
    ),
    'input keys' => array('content_key_name','target_key_name'),
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => FALSE,
    'changes files' => TRUE,
  );
  $info['group_items'] = array(
    'arguments' => array(
      'group_key_name' => array(
        'label' => t('Group key name'),
        'description' => t('Every item that has the same value for this key will be grouped to one item.'),
        'type' => 'key',
      ),
    ),
    'input keys' => array('group_key_name'),
    'output keys' => array(),
    'changes count' => TRUE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
  );
  $info['filter_items'] = array(
    'arguments' => array(
      'filter_key_name' => array(
        'label' => t('Filter key name'),
        'description' => t('The key that should be used to filter.'),
        'type' => 'key',
      ),
      'filter_type' => array(
        'label' => t('Filter type'),
        'description' => t('Choose how you want to filter: "key exists" filters if the key has any value, "equals" filters if the value of the filter key name is exactly the same as the filter value, "matches" filters if the value of the filter key name matches the regular expression in the filter value.'),
        'type' => 'select',
        'options' => array('exists' => t('key exists'), 'equals' => t('equals'), 'matches' => t('matches')),
      ),
      'filter_value' => array(
        'label' => t('Filter value'),
        'description' => t('The value to filter by.'),
        'type' => 'string',
        'optional' => array('filter_type' => array('exists')),
      ),
      'negate' => array(
        'label' => t('Negate'),
        'description' => t('If false, values of the filter_key_name key that are equal to filter_value, are included. If true, values of the filter_key_name key that are not equal to filter_value, are included.'),
        'type' => 'boolean',
      ),
    ),
    'input keys' => array('filter_key_name'),
    'output keys' => array(),
    'changes count' => TRUE,
    'changes keys' => FALSE,
    'changes files' => FALSE,
  );
  $info['parse_xml'] = array(
    'arguments' => array(
      'filepath_key_name' => array(
        'label' => t('Filepath key name'),
        'description' => t('The value for this key should hold an absolute path to an existing XML file.'),
        'type' => 'key',
      ),
      'xml_key_name' => array(
        'label' => t('XML key name'),
        'description' => t('The key where the XML should be stored.'),
        'type' => 'key',
      ),
    ),
    'input keys' => array('filepath_key_name'),
    'output keys' => array('xml_key_name'),
    'changes count' => FALSE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
  );
  $info['value_from_xml'] = array(
    'arguments' => array(
      'source_xml_key_name' => array(
        'label' => t('XML key name'),
        'description' => t('The key name where the XML is stored.'),
        'type' => 'key',
      ),
      'target_key_name' => array(
        'label' => t('Target key name'),
        'description' => t('The key that should be used to hold the value.'),
        'type' => 'key',
      ),
      'xpath' => array(
        'label' => t('XPath'),
        'description' => t('An XPath to the needed value.'),
        'type' => 'xpath',
      ),
      'namespaces' => array(
        'label' => t('namespaces'),
        'description' => t('The namespaces; one namespace per line in the following format: prefix=namespace URI'),
        'type' => 'text',
      ),
    ),
    'input keys' => array('source_xml_key_name'),
    'output keys' => array('target_key_name'),
    'changes count' => FALSE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
  );
  $info['xml_parts_from_xml'] = array(
    'arguments' => array(
      'source_xml_key_name' => array(
        'label' => t('XML key name'),
        'description' => t('The key name where the XML is stored.'),
        'type' => 'key',
      ),
      'target_key_name' => array(
        'label' => t('Target key name'),
        'description' => t('The key that should be used to hold the new XML parts.'),
        'type' => 'key',
      ),
      'xpath' => array(
        'label' => t('XPath'),
        'description' => t('An XPath to a repeating element.'),
        'type' => 'xpath',
      ),
      'namespaces' => array(
        'label' => t('namespaces'),
        'description' => t('The namespaces; one namespace per line in the following format: prefix=namespace URI'),
        'type' => 'text',
      ),
      'related_key_name' => array(
        'label' => t('Related key name'),
        'description' => t('Optionally, a key that will be included in the new items so they are related.'),
        'type' => 'key',
        'optional' => TRUE,
      ),
    ),
    'input keys' => array('source_xml_key_name','related_key_name'),
    'output keys' => array('target_key_name','related_key_name'),
    'changes count' => TRUE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
  );
  $info['read_csv_file'] = array(
    'arguments' => array(
      'csv_filepath' => array(
        'label' => t('CSV filepath'),
        'description' => t('Specify the absolute path to the CSV file.'),
        'type' => 'filepath',
      ),
      'column_keys' => array(
        'label' => t('Column keys'),
        'description' => t('Define the output keys of the CSV file, separated by semicolons.'),
        'type' => 'keys',
      ),
      'has_header' => array(
        'label' => t('Has header'),
        'description' => t('Does the CSV file have a header.'),
        'type' => 'boolean',
      ),
    ),
    'input keys' => array(),
    'output keys callback' => 'get_output_keys_read_csv_file',
    'changes count' => TRUE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
  );
  $info['end_filter'] = array(
    'arguments' => array(),
    'input keys' => array(),
    'output keys' => array(),
    'changes count' => TRUE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
  );
  $validateconfig = array(
    'arguments' => array(
      'filepath' => array(
        'label' => t('Filepath'),
        'description' => t('The absolute path to the directory where the import files are placed.'),
        'type' => 'filepath',
      ),
      'namespace' => array(
        'label' => t('Namespace'),
        'description' => t('The namespace of objects to create.'),
        'type' => 'namespace',
      ),
      'collection' => array(
        'label' => t('Collection'),
        'description' => t('The collection to which the generated items should be added.'),
        'type' => 'collection',
      ),
    ),
    'input keys' => array(),
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => FALSE,
    'changes files' => FALSE,
  );
  $info['validate_basic_image'] = $validateconfig;
  $info['validate_large_image'] = $validateconfig;
  $info['validate_book'] = $validateconfig;
  /*$info['validate_newspaper'] = $validateconfig;*/

  return $info;
}

/**
 * Helper functions.
 **/
function ubl_prepare_ingest_label_for_step($stepname, $stepconfig = array()) {
  $stepsinfo = module_invoke_all('workflow_step_info');
  $stepinfo = $stepsinfo[$stepname];
  $labelfunc = ubl_prepare_ingest_callback_for_step($stepname, 'label');
  if ($labelfunc !== FALSE) {
    $label = $labelfunc($stepinfo, $stepconfig);
  }
  if (!$label) {
    $label = $stepname;
  }

  return $label;
}

function ubl_prepare_ingest_callback_for_step($stepname, $callbacktype) {
  $func = FALSE;
  if (function_exists($callbacktype . '_' . $stepname)) {
    $func = $callbacktype . '_' . $stepname;
  }
  return $func;  
}

function fill_data_for_check_input_keys_if_needed(array $info, array $config, array &$data, $extent) {
  if (!(($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL))) {
    $keys = array();
    if (array_key_exists('output keys callback', $info)) {
      $keys = $info['output keys callback']($info, $config);
    }
    elseif (array_key_exists('output keys', $info)) {
      $outputkeys = $info['output keys'];
      foreach ($outputkeys as $ikey) {
        if (array_key_exists($ikey, $config)) {
          $keys[] = $config[$ikey];
        }
      }
    }
    if (count($keys) > 0) {
      if ($info['changes count']) {
        $data[] = array_combine($keys, $keys);
      }
      elseif ($info['changes keys']) {
        foreach ($data as $i => $d) {
          $data[$i] = array_merge($d, array_combine($keys, $keys));
        }
      }
    }
  }
}

function check_input_keys_in_data(array $info, array $config, array $data) {
  $results = array();
  $inputkeys = array();

  if (array_key_exists('input keys callback', $info)) {
    $inputkeys = $info['input keys callback']($info, $config);
    foreach ($inputkeys as $key) {
      $missingcount = 0;
      foreach ($data as $d) {
        if (!array_key_exists($key, $d)) {
           $missingcount++;
        }     
      }     
      if ($missingcount > 0) {
        if (count($data) === $missingcount) {
          $results[] = array('text' => t("Missing key '@key'. This key is not defined in any of the previous steps.", array('@key' => $key)), 'type' => 'key_error');
        }     
        else {
          $results[] = array('text' => t("Missing key '@key'. This key is defined in a previous step but not available for all items. Use a filter before this step to keep the items with the right keys.", array('@key' => $key)), 'type' => 'key_error');
        }     
      }     
    }
  }
  elseif (array_key_exists('input keys', $info)) {
    $inputkeys = $info['input keys'];
    foreach ($inputkeys as $ikey) {
      if (isset($info['arguments'][$ikey]['optional'])) {
        if (is_array($info['arguments'][$ikey]['optional'])) {
          foreach ($info['arguments'][$ikey]['optional'] as $optkey => $optvalues) {
            if (in_array($config[$optkey], $optvalues)) {
              continue 2;
            }
          }
        }
        else {
          if ($info['arguments'][$ikey]['optional']) {
            continue;
          }
        }
      }
      if (!array_key_exists($ikey, $config) || (strlen($config[$ikey]) == 0)) {
	$results[] = array('text' => t('Key @key not defined in configuration.', array('@key' => $ikey)), 'type' => 'config_error');
	continue;
      }
      $key = $config[$ikey];
      $missingcount = 0;
      foreach ($data as $d) {
	if (!array_key_exists($key, $d)) {
	   $missingcount++;
	}
      }
      if ($missingcount > 0) {
	if (count($data) === $missingcount) {
	  $results[] = array('text' => t("Missing key '@key'. This key is not defined in any of the previous steps.", array('@key' => $key)), 'type' => 'key_error');
	}
	else {
	  $results[] = array('text' => t("Missing key '@key'. This key is defined in a previous step but not available for all items. Use a filter before this step to keep the items with the right keys.", array('@key' => $key)), 'type' => 'key_error');
	}
      }
    }
  }
  return $results;
}

function check_arguments(array $info, array $config, $extent) {
  $results = array();
  $outputkeys = array();
  if (array_key_exists('output keys callback', $info)) {
    $outputkeys = $info['output keys callback']($info, $config);
  }
  elseif (array_key_exists('output keys', $info)) {
    $outputkeys = $info['output keys'];
  }
  if (array_key_exists('arguments', $info)) {
    $args = $info['arguments'];
    foreach ($args as $argkey => $argconfig) {
      if (array_key_exists($argkey, $config) && strlen($config[$argkey]) > 0) {
        if ($extent & WORK_EXTENT_ONLY_CHECK_FILLED_ARGUMENTS) {
          $value = $config[$argkey];
          $type = $argconfig['type'];
          switch($type) {
            case "number":
              if (!is_numeric($value)) {
                $results[] = array('text' => t("Argument '@key' should be a numeric value, but is '@value'", 
                  array('@key' => $argkey, '@value' => $value)), 'type' => 'argument_error');
              }
              break;
            case "string":
              // no need to check
              break;
            case "boolean":
              if ($value !== "0" && $value !== "1") {
                $results[] = array('text' => t("Argument '@key' should be a boolean value, but is '@value'", 
                  array('@key' => $argkey, '@value' => $value)), 'type' => 'argument_error');
              }
              break;
            case "template":
              // no need to check
              break;
            case "key":
              if (preg_match("/^[a-zA-Z0-9_-]+$/", $value) === 0) {
                $results[] = array('text' => t("Argument '@key' should be a key value, but is '@value'", 
                  array('@key' => $argkey, '@value' => $value)), 'type' => 'argument_error');
              }
              break;
            case "keys":
              if (preg_match("/^[a-zA-Z0-9_;-]+$/", $value) === 0) {
                $results[] = array('text' => t("Argument '@key' should be one or more key values separated by a semicolon, but is '@value'", 
                  array('@key' => $argkey, '@value' => $value)), 'type' => 'argument_error');
              }
              break;
            case "filepath":
              if (preg_match("/^(?:\/[a-zA-Z0-9_.-]+)+\/?$/", $value) === 0) {
                $results[] = array('text' => t("Argument '@key' should be a filepath, but is '@value'", 
                  array('@key' => $argkey, '@value' => $value)), 'type' => 'argument_error');
              }
              break;
            case "regexp":
              if (@preg_match($value, "XXX") === false) {
                $results[] = array('text' => t("Argument '@key' should be a regular expression, but is '@value'", 
                  array('@key' => $argkey, '@value' => $value)), 'type' => 'argument_error');
              }
              break;
            case "xpath":
              // is there a better way to validate xpath's, without removing the namespaces?
              $doc = new DOMDocument;
              $xpath = new DOMXPath($doc);
              $valuewons = preg_replace('#[^/:\\]\\[]+:#', '', $value);

              if ($xpath->query($valuewons) === false) {
                $results[] = array('text' => t("Argument '@key' should be a valid XPath, but is '@value'.", 
                  array('@key' => $argkey, '@value' => $value, '@type' => gettype($value))), 'type' => 'argument_error');
              }
              break;
            case "collection":
              module_load_include('inc', 'islandora', 'includes/utilities');
              $is_collection = FALSE;
              if (islandora_is_valid_pid($value)) {
                $obj = islandora_object_load($value);
                if (in_array('islandora:collectionCModel', $obj->models)) {
                  $is_collection = TRUE;
                }
              }
              if (!$is_collection) {
                $results[] = array('text' => t("Argument '@key' should be a valid PID of a collection, but is '@value'.", 
                  array('@key' => $argkey, '@value' => $value, '@type' => gettype($value))), 'type' => 'argument_error');
              }
              break;
            case "namespace":
              module_load_include('inc', 'islandora', 'includes/utilities');
              if (preg_match('/^([A-Za-z0-9-\.]+)$/', $value) === 0 || !islandora_namespace_accessible($value)) {
                $results[] = array('text' => t("Argument '@key' should be a valid namespace, but is '@value'.", 
                  array('@key' => $argkey, '@value' => $value, '@type' => gettype($value))), 'type' => 'argument_error');
              }
              break;
          }
        }
      }
      else {
        if (isset($argconfig['optional'])) {
          if (is_array($argconfig['optional'])) {
            foreach ($argconfig['optional'] as $optkey => $optvalues) {
              if (in_array($config[$optkey], $optvalues)) {
                continue 2;
              }
            }
          }
          else {
            if ($argconfig['optional']) {
              continue;
            }
          }
        }
        if (in_array($argkey, $outputkeys) || ($extent & WORK_EXTENT_ONLY_CHECK_FOR_EMPTY_ARGUMENTS)) {
          $results[$argkey] = array('text' => t('Missing argument \'@argname\'', array('@argname' => $argconfig['label'])), 'type' => 'argument_error');
        }
      }
    } 
  }
  else {
    $results[] = array('text' => t('Missing arguments in info.'), 'type' => 'argument_error');
  }
  return $results;
}

/**
 * read_file_names
 **/
function label_read_file_names(array $info, array $config) {
  $title = t('Add items by reading file names');
  if (array_key_exists('directory_path', $config) && strlen($config['directory_path']) > 0) {
    $title .= ' ' . t('from "@path"', array('@path' => $config['directory_path']));
  }
  if (array_key_exists('filter', $config) && strlen($config['filter']) > 0) {
    $title .= ' ' . t('filtered by "@filter"', array('@filter' => $config['filter']));
  }
  if (array_key_exists('target_key_name', $config) && strlen($config['target_key_name']) > 0) {
    $title .= ' ' . t('and insert item with key "@key"', array('@key' => $config['target_key_name']));
  }
  $title .= '.';

  return $title;
}

function work_read_file_names(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (array_key_exists('directory_path', $config) && strlen($config['directory_path']) > 0) {
      if (substr($config['directory_path'], 0, 1) !== DIRECTORY_SEPARATOR) {
        $results[] = array('text' => t("Argument '@key' should be an absolute filepath, but is '@value'.",
                  array('@key' => $info['arguments']['directory_path']['label'], '@value' => $config['directory_path'])), 'type' => 'argument_error');
      }
      if (!is_dir($config['directory_path'])) {
        $results[] = array('text' => t("Argument '@key' should be an existing filepath, but is '@value'",
                  array('@key' => $info['arguments']['directory_path']['label'], '@value' => $config['directory_path'])), 'type' => 'argument_error');
      }
    }
    if (count($results) > 0) {
      return $results;
    }
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    // do the actual work.
    if (array_key_exists('directory_path', $config) && strlen($config['directory_path']) > 0) {
      module_load_include('inc', 'ubl_prepare_ingest', 'includes/pifs');
      $dirpath = $config['directory_path'];
      $filter = $config['filter'];
      $includedir = $config['include_subdirectory_files'];
      $filenames = pifs_full_filenames_from_directory($dirpath, $context, $filter, $includedir);
      $key = $config['target_key_name'];
      foreach ($filenames as $filename) {
	$data[] = array($key => $filename);
      }
    }
  }
  fill_data_for_check_input_keys_if_needed($info, $config, $data, $extent);

  return FALSE;
}

/**
 * add_key_regexp
 **/
function label_add_key_regexp(array $info, array $config) {
  $title = '';
  if (array_key_exists('target_key_name', $config) && strlen($config['target_key_name']) > 0) {
    $title .= ' ' . t('named "@key"', array('@key' => $config['target_key_name']));
  }
  if (array_key_exists('source_key_name', $config) && strlen($config['source_key_name']) > 0) {
    $title .= ' ' . t('with value from key "@key"', array('@key' => $config['source_key_name']));
  }
  if (array_key_exists('regexp', $config) && strlen($config['regexp']) > 0) {
    $title .= ' ' . t('by replacing "@regexp"', array('@regexp' => $config['regexp']));
  }
  if (array_key_exists('replacement', $config) && strlen($config['replacement']) > 0) {
    $title .= ' ' . t('with "@replacement"', array('@replacement' => $config['replacement']));
  }
  
  if (strlen($title) == 0) {
    $title = t('Add key with regular expression based value');
  }
  else {
    $title = t('Add key') . ' ' . $title;
  }
  $title .= '.';

  return $title;
}

function work_add_key_regexp(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (count($results) > 0) {
      return $results;
    }
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    $targetkey = $config['target_key_name'];
    $sourcekey = $config['source_key_name'];
    $regexp = $config['regexp'];
    $replacement = $config['replacement'];

    foreach ($data as $current => $d) {
      if (array_key_exists($sourcekey, $d)) {
        $newvalue = preg_replace($regexp, $replacement, $d[$sourcekey]);
        $data[$current][$targetkey] = $newvalue; 
      }
    }
  }
  fill_data_for_check_input_keys_if_needed($info, $config, $data, $extent);

  return FALSE;
}

/**
 * add_key_template
 **/
function label_add_key_template(array $info, array $config) {
  $title = '';
  if (array_key_exists('target_key_name', $config) && strlen($config['target_key_name']) > 0) {
    $title .= ' ' . t('named "@key"', array('@key' => $config['target_key_name']));
  }
  if (array_key_exists('template', $config) && strlen($config['template']) > 0) {
    $template = $config['template'];
    $title .= ' ' . t('with value from template "@template"', array('@template' => $template));
  }

  if (strlen($title) == 0) {
    $title = t('Add key with template based value');
  }
  else {
    $title = t('Add key') . ' ' . $title;
  }
  $title .= '.';

  return $title;
}

function get_input_keys_add_key_template(array $info, array $config) {
  if (array_key_exists('template', $config) && strlen($config['template']) > 0) {
    $template = $config['template'];
    $inputkeys = array();
    if (preg_match_all('/{([a-zA-Z0-9_-]+)}/', $template, $inputkeys)) {
      return array_unique($inputkeys[1]);
    }
  } 
  return array();
}

function work_add_key_template(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (count($results) > 0) {
      return $results;
    }
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    $targetkey = $config['target_key_name'];
    $template = $config['template'];
    $inputkeys = get_input_keys_add_key_template($info, $config);
    $labels = array_map(function($inputkey) { return '{' . $inputkey . '}'; }, $inputkeys);

    foreach ($data as $current => $d) {
      $newvalues = array_map(function($inputkey) use($d) { return $d[$inputkey]; }, $inputkeys);
      $newvalue = str_replace($labels, $newvalues, $template);
      $data[$current][$targetkey] = $newvalue;
    }
  }
  fill_data_for_check_input_keys_if_needed($info, $config, $data, $extent);

  return FALSE;
}

/**
 * add_key_counter
 **/
function label_add_key_counter(array $info, array $config) {
  $title = '';
  if (array_key_exists('target_key_name', $config) && strlen($config['target_key_name']) > 0) {
    $title .= ' ' . t('named "@key"', array('@key' => $config['target_key_name']));
  }
  if (array_key_exists('start', $config) && strlen($config['start']) > 0) {
    $title .= ' ' . t('from "@start"', array('@start' => $config['start']));
  }
  if (array_key_exists('step', $config) && strlen($config['step']) > 0) {
    $title .= ' ' . t('with increments of "@step"', array('@step' => $config['step']));
  }
  if (array_key_exists('width', $config) && strlen($config['width']) > 0) {
    $title .= ' ' . t('and width "@width"', array('@width' => $config['width']));
  }
  if (array_key_exists('foreach_value', $config) && strlen($config['foreach_value']) > 0) {
    $title .= ' ' . t('and start for each unique value of "@key"', array('@key' => $config['foreach_value']));
  }

  if (strlen($title) == 0) {
    $title = t('Add key with counter value');
  }
  else {
    $title = t('Add counter with key') . ' ' . $title;
  }
  $title .= '.';

  return $title;
}

function work_add_key_counter(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (count($results) > 0) {
      return $results;
    }
    if (array_key_exists('step', $config) && strlen($config['step']) > 0) {
      if (intval($config['step']) == 0) {
        $result[] = array('text' => t("Argument '@key' should be a number (not zero), but is '@value'.",
                  array('@key' => $info['arguments']['step']['label'], '@value' => $config['step'])), 'type' => 'argument_error');
      }
    }
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    $targetkey = $config['target_key_name'];
    $start = intval($config['start']);
    $step = intval($config['step']);
    $width = intval($config['width']);
    $foreachkey = $config['foreach_value'];
    $format = '%0' . $width . 'd';
    $counter = array();
    foreach ($data as $current => $d) {
      $foreachvalue = isset($d[$foreachkey])?$d[$foreachkey]:'_GLOB_COUNTER_';
      if (!array_key_exists($foreachvalue, $counter)) {
        $counter[$foreachvalue] = $start; 
      }
      $newvalue = sprintf($format, $counter[$foreachvalue]);
      $counter[$foreachvalue] += $step;
      $data[$current][$targetkey] = $newvalue;
    }
  }
  if (!(($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL))) {
    // add some test items to data
  }
  fill_data_for_check_input_keys_if_needed($info, $config, $data, $extent);

  return FALSE;
}

/**
* extract_keys_template
**/
function label_extract_keys_template(array $info, array $config) {
  $title = t('Add key(s) using template');
  if (array_key_exists('target_keys_template', $config) && strlen($config['target_keys_template']) > 0) {
    $title .= ' ' . t('"@template"', array('@template' => $config['target_keys_template']));
  }
  if (array_key_exists('source_key_name', $config) && strlen($config['source_key_name']) > 0) {
    $title .= ' ' . t('from value of key "@key"', array('@key' => $config['source_key_name']));
  }
  return $title;
}

function work_extract_keys_template(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (count($results) > 0) {
      return $results;
    }
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    $sourcekey = $config['source_key_name'];
    $targetkeystemplate = $config['target_keys_template'];
    $outputkeys = get_output_keys_extract_keys_template($info, $config);
    $templateregexp = preg_replace('/\\\{[a-zA-Z0-9_-]+\\\}/', '(.+)', preg_quote($targetkeystemplate, '/'));
    foreach ($data as $current => $d) {
      if (array_key_exists($sourcekey, $d)) {
	$outputvalues = array();
	if (preg_match('/' . $templateregexp . '/', $d[$sourcekey], $outputvalues)) {
	  array_shift($outputvalues);
	  $output = array_combine($outputkeys, $outputvalues);
	  foreach ($output as $key => $value) {
	    $data[$current][$key] = $value;
	  }
	}
      }
    }
  }
  fill_data_for_check_input_keys_if_needed($info, $config, $data, $extent);

  return FALSE;
}

function get_output_keys_extract_keys_template(array $info, array $config) {
  if (array_key_exists('target_keys_template', $config) && strlen($config['target_keys_template']) > 0) {
    $template = $config['target_keys_template'];
    $outputkeys = array();
    if (preg_match_all('/{([a-zA-Z0-9_-]+)}/', $template, $outputkeys)) {
      return array_unique($outputkeys[1]);
    }
  }
  return array();
}

function label_modify_key_value(array $info, array $config) {
  $title = '';

  if (array_key_exists('type', $config) && strlen($config['type']) > 0) {
    $type = $config['type'];

    switch ($type) {
      case 'uppercase':
	$title = t('Uppercase');
	break;
      case 'lowercase':
	$title = t('Lowercase');
	break;
      case 'removehyphen':
	$title = t('Remove - from');
	break;
      case 'removeline':
	$title = t('Remove - and _ from');
	break;
      case 'removespaces':
	$title = t('Remove spaces from');
	break;
      case 'replacehyphen':
	$title = t('Replace - with space in');
	break;
      case 'replaceline':
	$title = t('Replace - and _ with space in');
	break;
      case 'striptags':
	$title = t('Strip tags from');
	break;
      case 'sha1':
	$title = t('SHA1 hash');
	break;
      case 'trim':
	$title = t('Trim spaces of');
	break;
      case 'pad5zeros':
	$title = t('Pad zero\'s to 5 digits');
	break;
    }
  }
  else {
    $title = t('Change');
  }
  if (array_key_exists('source_key_name', $config) && strlen($config['source_key_name']) > 0) {
    $title .= ' ' . t('the value of key @key', array('@key' => $config['source_key_name']));
  }
  else {
    $title .= ' ' . t('the value of a key');
  }
  return $title;
}

function work_modify_key_value(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (count($results) > 0) {
      return $results;
    }
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    $sourcekey = $config['source_key_name'];
    $targetkey = $sourcekey;
    $type = $config['type'];

    switch ($type) {
      case 'uppercase':
	foreach ($data as $current => $d) {
	  if (array_key_exists($sourcekey, $d)) {
	    $data[$current][$targetkey] = strtoupper($d[$sourcekey]);
	  }
	}
	break;
      case 'lowercase':
	foreach ($data as $current => $d) {
	  if (array_key_exists($sourcekey, $d)) {
	    $data[$current][$targetkey] = strtolower($d[$sourcekey]);
	  }
	}
	break;
      case 'removehyphen':
	foreach ($data as $current => $d) {
	  if (array_key_exists($sourcekey, $d)) {
	    $data[$current][$targetkey] = str_replace('-', '', $d[$sourcekey]);
	  }
	}
	break;
      case 'removeline':
	foreach ($data as $current => $d) {
	  if (array_key_exists($sourcekey, $d)) {
	    $data[$current][$targetkey] = str_replace(array('-', '_'), '', $d[$sourcekey]);
	  }
	}
	break;
      case 'removespaces':
	foreach ($data as $current => $d) {
	  if (array_key_exists($sourcekey, $d)) {
	    $data[$current][$targetkey] = str_replace(' ', '', $d[$sourcekey]);
	  }
	}
	break;
      case 'replacehyphen':
	foreach ($data as $current => $d) {
	  if (array_key_exists($sourcekey, $d)) {
	    $data[$current][$targetkey] = str_replace('-', ' ', $d[$sourcekey]);
	  }
	}
	break;
      case 'replaceline':
	foreach ($data as $current => $d) {
	  if (array_key_exists($sourcekey, $d)) {
	    $data[$current][$targetkey] = str_replace(array('-', '_'), ' ', $d[$sourcekey]);
	  }
	}
	break;
      case 'striptags':
	$title = t('Strip tags from');
	foreach ($data as $current => $d) {
	  if (array_key_exists($sourcekey, $d)) {
	    $data[$current][$targetkey] = strip_tags($d[$sourcekey]);
	  }
	}
	break;
      case 'sha1':
	$title = t('SHA1 hash');
	foreach ($data as $current => $d) {
	  if (array_key_exists($sourcekey, $d)) {
	    $data[$current][$targetkey] = sha1($d[$sourcekey]);
	  }
	}
	break;
      case 'trim':
	foreach ($data as $current => $d) {
	  if (array_key_exists($sourcekey, $d)) {
	    $data[$current][$targetkey] = trim($d[$sourcekey]);
	  }
	}
	break;
      case 'pad5zeros':
	foreach ($data as $current => $d) {
	  if (array_key_exists($sourcekey, $d)) {
	    $data[$current][$targetkey] = sprintf("%05d", $d[$sourcekey]);
	  }
	}
	break;
    }

  }
  return FALSE;
}

/**
 * make_directory
 **/
function label_make_directory(array $info, array $config) {
  $title = t('Make directory');
  if (array_key_exists('directory_key_name', $config) && strlen($config['directory_key_name']) > 0) {
    $title .= ' ' . t('based on the value of key "@key"', array('@key' => $config['directory_key_name']));
  }
  $title .= '.';

  return $title;
}

function work_make_directory(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (array_key_exists('directory_key_name', $config) && strlen($config['directory_key_name']) > 0) {
      if (is_file($config['directory_key_name'])) {
        $results[] = array('text' => t("Argument '@key' should be a non-existing directory, but a file exists at '@value'.",
                  array('@key' => $info['arguments']['directory_key_name']['label'], '@value' => $config['directory_key_name'])), 'type' => 'argument_error');
      }
      if (is_dir($config['directory_key_name'])) {
        $results[] = array('text' => t("Argument '@key' should be a non-existing directory, but a directory exists at '@value'.",
                  array('@key' => $info['arguments']['directory_key_name']['label'], '@value' => $config['directory_key_name'])), 'type' => 'argument_error');
      }
    }
    if (count($results) > 0) {
      return $results;
    }
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    module_load_include('inc', 'ubl_prepare_ingest', 'includes/pifs');
    $sourcekey = $config['directory_key_name'];
    foreach ($data as $current => $d) {
      $dirname = $d[$sourcekey]; 
      $parentdir = dirname($dirname);
      if (pifs_file_exists($dirname, $context) && !pifs_is_dir($dirname, $context)) {
        $results[] = array('text' => t("'@directory' in key '@key' already exists but is not a directory.",
                  array('@directory' => $dirname, '@key' => $sourcekey)), 'type' => 'run_error');
      }
      if (!pifs_is_dir($parentdir, $context)) {
        $results[] = array('text' => t("Parent directory '@directory' of directory in key '@key' should exist, but does not.",
                  array('@directory' => $parentdir, '@key' => $sourcekey)), 'type' => 'run_error');
      }
      if (count($results) > 0) {
        return $results;
      }
      if ($extent & WORK_EXTENT_ONLY_DRY_RUN) {
        pifs_mkdir($dirname, $context);
      }
      if ($extent & WORK_EXTENT_ONLY_DO_ALL) {
        if (!is_dir($dirname)) {
          mkdir($dirname);
        }
      }
    }    
  }
  fill_data_for_check_input_keys_if_needed($info, $config, $data, $extent);

  return FALSE;
}

/**
 * copy_file
 **/
function label_copy_file(array $info, array $config) {
  $title = t('Copy file');
  if (array_key_exists('source_key_name', $config) && strlen($config['source_key_name']) > 0) {
    $title .= ' ' . t('from the value of key "@key"', array('@key' => $config['source_key_name']));
  }
  if (array_key_exists('target_key_name', $config) && strlen($config['target_key_name']) > 0) {
    $title .= ' ' . t('to the value of key "@key"', array('@key' => $config['target_key_name']));
  }
  $title .= '.';

  return $title;
}

function work_copy_file(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (array_key_exists('target_key_name', $config) && strlen($config['target_key_name']) > 0) {
      if (is_file($config['target_key_name'])) {
        $results[] = array('text' => t("Argument '@key' points to a location where a file exists, at '@value'.",
                  array('@key' => $info['arguments']['target_key_name']['label'], '@value' => $config['target_key_name'])), 'type' => 'argument_error');
      }
    }
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_DRY_RUN) {
    module_load_include('inc', 'ubl_prepare_ingest', 'includes/pifs');
    $sourcekey = $config['source_key_name'];
    $targetkey = $config['target_key_name'];
    foreach ($data as $current => $d) {
      $sourcefile = $d[$sourcekey];
      if (!pifs_is_file($sourcefile, $context)) {
	$results[] = array('text' => t("File '@file' in key '@key' does not exist.",
		  array('@file' => $sourcefile, '@key' => $sourcekey)), 'type' => 'run_error');
      }
      $targetlocation = $d[$targetkey];
      if ((substr($targetlocation, -1) === DIRECTORY_SEPARATOR) || pifs_is_dir($targetlocation, $context)) {
        $targetfile = $targetlocation . DIRECTORY_SEPARATOR . basename($sourcefile);
      }
      else {
        $targetfile = $targetlocation;
      }
      if (pifs_is_file($targetfile, $context)) {
        $results[] = array('text' => t("File '@file' in key '@key' already exists.",
                  array('@file' => $targetfile, '@key' => $targetkey)), 'type' => 'run_error');
      }
      $targetdirectory = dirname($targetfile);
      if (!pifs_is_dir($targetdirectory, $context)) {
        $results[] = array('text' => t("Directory '@directory' for file '@file' in key '@key' should exist, but does not.",
                  array('@directory' => $targetdirectory, '@file' => $targetfile, '@key' => $targetkey)), 'type' => 'run_error');
      }
      if (count($results) > 0) {
        return $results;
      }
      pifs_copy($sourcefile, $targetfile, $context);
    }
  }
  if ($extent & WORK_EXTENT_ONLY_DO_ALL) {
    $sourcekey = $config['source_key_name'];
    $targetkey = $config['target_key_name'];
    foreach ($data as $current => $d) {
      $sourcefile = $d[$sourcekey];
      if (!is_file($sourcefile)) {
        $results[] = array('text' => t("File '@file' in key '@key' does not exist.",
                  array('@file' => $sourcefile, '@key' => $sourcekey)), 'type' => 'run_error');
      }
      $targetlocation = $d[$targetkey];
      if ((substr($targetlocation, -1) === DIRECTORY_SEPARATOR) || is_dir($targetlocation)) {
        $targetfile = $targetlocation . DIRECTORY_SEPARATOR . basename($sourcefile);
      }
      else {
        $targetfile = $targetlocation;
      }
      if (is_file($targetfile)) {
        $results[] = array('text' => t("File '@file' in key '@key' already exists.",
                  array('@file' => $targetfile, '@key' => $targetkey)), 'type' => 'run_error');
      }
      $targetdirectory = dirname($targetfile);
      if (!is_dir($targetdirectory)) {
        $results[] = array('text' => t("Directory '@directory' for file '@file' in key '@key' should exist, but does not.",
                  array('@directory' => $targetdirectory, '@file' => $targetfile, '@key' => $targetkey)), 'type' => 'run_error');
      }
      if (count($results) > 0) {
        return $results;
      }
      $success = @link($sourcefile, $targetfile);
      if (!$success) {
        @copy($sourcefile, $targetfile);
      }
    }
  }  
  fill_data_for_check_input_keys_if_needed($info, $config, $data, $extent);

  return FALSE;
}

/**
 * write_file
 **/
function label_write_file(array $info, array $config) {
  $title = t('Write to file');
  if (array_key_exists('content_key_name', $config) && strlen($config['content_key_name']) > 0) {
    $title .= ' ' . t('the content of key "@key"', array('@key' => $config['content_key_name']));
  }
  if (array_key_exists('target_key_name', $config) && strlen($config['target_key_name']) > 0) {
    $title .= ' ' . t('located at the value of key "@key"', array('@key' => $config['target_key_name']));
  }
  $title .= '.';

  return $title;
}

function work_write_file(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_DRY_RUN) {
    module_load_include('inc', 'ubl_prepare_ingest', 'includes/pifs');
    $contentkey = $config['content_key_name'];
    $targetkey = $config['target_key_name'];
    foreach ($data as $current => $d) {
      $content = $d[$contentkey];
      $targetfile = $d[$targetkey];
      $targetdirectory = dirname($targetfile);
      if (!pifs_is_dir($targetdirectory, $context)) {
        $results[] = array('text' => t("Directory '@directory' for file '@file' in key '@key' should exist, but does not.",
                  array('@directory' => $targetdirectory, '@file' => $targetfile, '@key' => $targetkey)), 'type' => 'run_error');
      }
      if (count($results) > 0) {
        return $results;
      }
      if (pifs_entry_exists($targetfile, $context)) {
        if (pifs_entry_is_file($targetfile, $context)) {
          pifs_append_content($targetfile, $content, $context);
        }
        else {
          $results[] = array('text' => t("Cannot write file: a directory '@directory' already exists at that location.",
                    array('@directory' => $targetfile)), 'type' => 'run_error');
          return $results;
        }
      }
      else {
        pifs_write_content($targetfile, $content, $context);
      }
    }
  }
  if ($extent & WORK_EXTENT_ONLY_DO_ALL) {
    $contentkey = $config['content_key_name'];
    $targetkey = $config['target_key_name'];
    foreach ($data as $current => $d) {
      $content = $d[$contentkey];
      $targetfile = $d[$targetkey];
      $targetdirectory = dirname($targetfile);
      if (!is_dir($targetdirectory)) {
        $results[] = array('text' => t("Directory '@directory' for file '@file' in key '@key' should exist, but does not.",
                  array('@directory' => $targetdirectory, '@file' => $targetfile, '@key' => $targetkey)), 'type' => 'run_error');
      }
      if (count($results) > 0) {
        return $results;
      }
      $fh = fopen($targetfile, "a"); // always append to file
      fwrite($fh, $content);
      fclose($fh);
    }
  }
  fill_data_for_check_input_keys_if_needed($info, $config, $data, $extent);

  return FALSE;
}

/**
 * group_items
 **/
function label_group_items(array $info, array $config) {
  $title = t('Group items');
  if (array_key_exists('group_key_name', $config) && strlen($config['group_key_name']) > 0) {
    $title .= ' ' . t('by the value of key "@key"', array('@key' => $config['group_key_name']));
  }
  $title .= '.';

  return $title;
}

function work_group_items(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (count($results) > 0) {
      return $results;
    }
  }
  // do this for every other extent
  if (isset($config['group_key_name'])) {
    $groupkey = $config['group_key_name'];

    $grouparray = array();
    foreach ($data as $current => $d) {
      $group = $d[$groupkey];
      if (array_key_exists($group, $grouparray)) {
        $newarray = array_merge($grouparray[$group], $d);
        $grouparray[$group] = $newarray;
      }
      else {
        $grouparray[$group] = $d;
      }
    }
    array_splice($data, 0); // Remove all items from the data array
    foreach ($grouparray as $k => $d) {
      $data[] = $d;
    }
  }

  return FALSE;
}

/**
 * filter_items
 **/
function label_filter_items(array $info, array $config) {
  $title = t('Filter items');
  if (array_key_exists('filter_key_name', $config) && strlen($config['filter_key_name']) > 0) {
    $title .= ' ' . t('if the value of key "@key"', array('@key' => $config['filter_key_name']));
  }
  if (array_key_exists('filter_type', $config) && strlen($config['filter_type']) > 0) {
    if ($config['filter_type'] === 'exists') {
      if (array_key_exists('negate', $config) && $config['negate']) {
        $title .= ' ' . t('does not exist');
      }
      else {
        $title .= ' ' . t('exists');
      }
    }
    elseif ($config['filter_type'] === 'equals') {
      if (array_key_exists('negate', $config) && $config['negate']) {
        $title .= ' ' . t('does not equal');
      }
      else {
        $title .= ' ' . t('equals');
      }
      if (array_key_exists('filter_value', $config) && strlen($config['filter_value']) > 0) {
        $title .= ' ' . '"' . $config['filter_value'] . '"';
      }
    }
    elseif ($config['filter_type'] === 'matches') {
      if (array_key_exists('negate', $config) && $config['negate']) {
        $title .= ' ' . t('does not match');
      }
      else {
        $title .= ' ' . t('matches');
      }
      if (array_key_exists('filter_value', $config) && strlen($config['filter_value']) > 0) {
        $title .= ' ' . $config['filter_value'];
      }
    }
  }
  $title .= '.';

  return $title;
}

function work_filter_items(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    // do not check input keys, because some might be missing and that is okay
  }     
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (array_key_exists('filter_type', $config) && strlen($config['filter_type']) > 0) {
      if ($config['filter_type'] === 'exists') {
        // no check needed, filter_value does not matter.
        unset($results['filter_value']);
      } 
      elseif ($config['filter_type'] === 'equals') {
        if (!(array_key_exists('filter_value', $config) && strlen($config['filter_value']) > 0)) {
          $results[] = array('text' => t("Filter with type 'equals' should have a value."), 'type' => 'argument_error');
        }
      } 
      elseif ($config['filter_type'] === 'matches') {
        if (!(array_key_exists('filter_value', $config) && (@preg_match($config['filter_value'], "XXX") !== FALSE))) {
          $results[] = array('text' => t("Filter with type 'matches' should have a regular expression, but has '@value'.",
                             array('@value' => $config['filter_value'])), 'type' => 'argument_error');
        }
      } 
    }
    if (count($results) > 0) {
      return $results;
    }   
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    $filterkey = $config['filter_key_name'];
    $type = $config['filter_type'];
    $value = $config['filter_value'];
    $negate = $config['negate'];
    
    $keep = array();
    $remove = array();
    $newdata = array();
    $newindex = 0;
    foreach ($data as $current => $d) {
      $shouldkeep = FALSE;
      if ($type === 'exists') {
        $shouldkeep = is_array($d) && array_key_exists($filterkey, $d);
      }
      elseif ($type === 'equals') {
        $shouldkeep = (is_array($d) && array_key_exists($filterkey, $d) && ($d[$filterkey] === $value));
      }
      elseif ($type === 'matches') {
        $shouldkeep = (is_array($d) && array_key_exists($filterkey, $d) && preg_match($value, $d[$filterkey]));
      }
      if ($negate) {
        $shouldkeep = !$shouldkeep;
      } 
      if ($shouldkeep) {
        $keep[$current] = $newindex;
        $newdata[$newindex] = $d;
        $newindex++;
      }
      else {
        $remove[$current] = $d;
      }
    }
    // Remove all items from the data array and replace by newdata
    array_splice($data, 0, count($data), $newdata); 

    if (!array_key_exists('_filters', $context)) {
      $context['_filters'] = array();
    }
    $context['_filters'][] = array(
      'keep' => $keep,
      'remove' => $remove,
    );
  }

  if (!(($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL))) {
    $filterkey = $config['filter_key_name'];
    $negate = $config['negate'];
    
    $keep = array();
    $remove = array();
    $newdata = array();
    $newindex = 0;
    foreach ($data as $current => $d) {
      $shouldkeep = FALSE;
      $shouldkeep = array_key_exists($filterkey, $d);
      if ($negate) {
        $shouldkeep = !$shouldkeep;
      } 
      if ($shouldkeep) {
        $keep[$current] = $newindex;
        $newdata[$newindex] = $d;
        $newindex++;
      }
      else {
        $remove[$current] = $d;
      }
    }
    // Remove all items from the data array and replace by newdata
    array_splice($data, 0, count($data), $newdata); 

    if (!array_key_exists('_filters', $context)) {
      $context['_filters'] = array();
    }
    $context['_filters'][] = array(
      'keep' => $keep,
      'remove' => $remove,
    );
  }

  return FALSE;
}

/**
 * parse_xml
 **/
function label_parse_xml(array $info, array $config) {
  $title = t('Parse XML file');
  if (array_key_exists('filepath_key_name', $config) && strlen($config['filepath_key_name']) > 0) {
    $title .= ' ' . t('located at the value of key "@key"', array('@key' => $config['filepath_key_name']));
  }
  if (array_key_exists('xml_key_name', $config) && strlen($config['xml_key_name']) > 0) {
    $title .= ' ' . t('and store the XML in key "@key"', array('@key' => $config['xml_key_name']));
  }
  $title .= '.';

  return $title; 
}

function work_parse_xml(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (count($results) > 0) {
      return $results;
    }   
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    module_load_include('inc', 'ubl_prepare_ingest', 'includes/pifs');
    $filepathkey = $config['filepath_key_name'];
    $xmlkey = $config['xml_key_name'];
    foreach ($data as $current => $d) {
      $filepath = $d[$filepathkey];
      if (!pifs_is_file($filepath, $context)) {
        $results[] = array('text' => t("File '@file' in key '@key' does not exist.",
                  array('@file' => $filepath, '@key' => $filepathkey)), 'type' => 'run_error');
      }
      $domdoc = new DOMDocument();
      if (is_file($filepath)) {
        if (!$domdoc->load($filepath)) {
          $results[] = array('text' => t("File '@file' in key '@key' is not a valid XML document.",
                    array('@file' => $filepath, '@key' => $filepathkey)), 'type' => 'run_error');
        }
      }
      else {
        $contentxml = pifs_read_content($filepath, $context);
        if (!$domdoc->loadXML($contentxml)) {
          $results[] = array('text' => t("File '@file' in key '@key' is not a valid XML document.",
                    array('@file' => $filepath, '@key' => $filepathkey)), 'type' => 'run_error');
        }
      }
      if (count($results) > 0) {
        return $results;
      }
      $data[$current][$xmlkey] = $domdoc->saveXML();
    }
  }
  fill_data_for_check_input_keys_if_needed($info, $config, $data, $extent);

  return FALSE;
}

/**
 * value_from_xml
 **/
function label_value_from_xml(array $info, array $config) {
  $title = t('Add key from XML');
  if (array_key_exists('source_xml_key_name', $config) && strlen($config['source_xml_key_name']) > 0) {
    $title .= ' ' . t('at key "@key"', array('@key' => $config['source_xml_key_name']));
  }
  if (array_key_exists('xpath', $config) && strlen($config['xpath']) > 0) {
    $title .= ' ' . t('located by XPath "@value"', array('@value' => $config['xpath']));
  }
  if (array_key_exists('namespaces', $config) && strlen($config['namespaces']) > 0) {
    $namespaces = _parse_namespaces_string($config['namespaces']);
    if (count($namespaces) > 0) {
      $title .= ' ' . format_plural(count($namespaces), 'with namespace prefix:', 'with namespace prefixes:');
      foreach ($namespaces as $prefix => $nsuri) {
        $title .= ' ' . $prefix;
      }
    }
  }
  if (array_key_exists('target_key_name', $config) && strlen($config['target_key_name']) > 0) {
    $title .= ' ' . t('and store it in key "@key"', array('@key' => $config['target_key_name']));
  }

  $title .= '.';

  return $title; 
}

function _parse_namespaces_string($nsstr) {
  $namespaces = array();
  $nslines = explode("\n", $nsstr);
  foreach ($nslines as $nsline) {
    $nsline = trim($nsline); 
    if (strlen($nsline) > 0) {
      $parts = explode("=", $nsline);
      if (count($parts) == 2) {
	$prefix = trim($parts[0]);
	$nsuri = trim($parts[1]);
	if (strlen($prefix) > 0 && strlen($nsuri) > 0) {
	  $namespaces[$prefix] = $nsuri;
	}
      }
    }
  }
  return $namespaces;
}

function work_value_from_xml(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (count($results) > 0) {
      return $results;
    }
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    $xmlkey = $config['source_xml_key_name'];
    $targetkey = $config['target_key_name'];
    $xpath = $config['xpath'];
    $namespaces = _parse_namespaces_string($config['namespaces']);
    foreach ($data as $current => $d) {
      $xml = $d[$xmlkey];
      $domdoc = new DOMDocument();
      $domdoc->loadXML($xml);
      $domxpath = new DOMXPath($domdoc);
      foreach ($namespaces as $prefix => $nsuri) {
        $domxpath->registerNamespace($prefix, $nsuri);
      } 
      $domnodelist = $domxpath->query($xpath);
      if ($domnodelist === FALSE) {
        $result[] = array('text' => t("Malformed XPath at '@key' with '@value'.",
                  array('@key' => $info['arguments']['step']['label'], '@value' => $config['xpath'])), 'type' => 'argument_error');
        return $result;
      }
      if ($domnodelist->length > 0) {
        foreach ($domnodelist as $domnode) {
          $text = $domnode->textContent;
          if (isset($text) && strlen($text) > 0) {
            $data[$current][$targetkey] = $text;
            break;
          }
        }
      }
    } 
  }
  fill_data_for_check_input_keys_if_needed($info, $config, $data, $extent);

  return FALSE;
}

/**
 * xml_parts_from_xml
 **/
function label_xml_parts_from_xml(array $info, array $config) {
  $title = t('Add items by extracting XML parts from XML');
  if (array_key_exists('source_xml_key_name', $config) && strlen($config['source_xml_key_name']) > 0) {
    $title .= ' ' . t('at key "@key"', array('@key' => $config['source_xml_key_name']));
  }
  if (array_key_exists('xpath', $config) && strlen($config['xpath']) > 0) {
    $title .= ' ' . t('located by XPath "@value"', array('@value' => $config['xpath']));
  }
  if (array_key_exists('namespaces', $config) && strlen($config['namespaces']) > 0) {
    $namespaces = _parse_namespaces_string($config['namespaces']);
    if (count($namespaces) > 0) {
      $title .= ' ' . format_plural(count($namespaces), 'with namespace prefix:', 'with namespace prefixes:');
      foreach ($namespaces as $prefix => $nsuri) {
        $title .= ' ' . $prefix;
      }
    }
  }
  if (array_key_exists('target_key_name', $config) && strlen($config['target_key_name']) > 0) {
    $title .= ' ' . t('and insert item with key "@key"', array('@key' => $config['target_key_name']));
  }
  if (array_key_exists('related_key_name', $config) && strlen($config['related_key_name']) > 0) {
    $title .= ' ' . t('and related key "@key"', array('@key' => $config['related_key_name']));
  }

  $title .= '.';

  return $title; 
}

function work_xml_parts_from_xml(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (count($results) > 0) {
      return $results;
    }
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    $xmlkey = $config['source_xml_key_name'];
    $targetkey = $config['target_key_name'];
    $xpath = $config['xpath'];
    $namespaces = _parse_namespaces_string($config['namespaces']);
    $relatedkey = isset($config['related_key_name'])?$config['related_key_name']:'';
    $olddata = $data; // make a copy and iterate over the copy, because items will be added to $data
    foreach ($olddata as $current => $d) {
      $xml = $d[$xmlkey];
      $domdoc = new DOMDocument();
      if ($domdoc->loadXML($xml) === FALSE) {
        $result[] = array('text' => t("Failed to parse xml at '@key' with '@value'.",
                  array('@key' => $info['arguments']['step']['label'], '@value' => $config['source_xml_key_name'])), 'type' => 'argument_error');
        return $result;
      }
      $domxpath = new DOMXPath($domdoc);
      foreach ($namespaces as $prefix => $nsuri) {
        $domxpath->registerNamespace($prefix, $nsuri);
      } 
      $domnodelist = $domxpath->query($xpath);
      if ($domnodelist === FALSE) {
        $result[] = array('text' => t("Malformed XPath at '@key' with '@value'.",
                  array('@key' => $info['arguments']['step']['label'], '@value' => $config['xpath'])), 'type' => 'argument_error');
        return $result;
      }
      if ($domnodelist->length > 0) {
        foreach ($domnodelist as $domnode) {
          $newdomdoc = new DOMDocument;
          $newnode = $newdomdoc->importNode($domnode, true);
          if ($newnode) {
            $newdomdoc->appendChild($newnode);
            $newd = array();
            $newd[$targetkey] = $newdomdoc->saveXML();
            if ($relatedkey) {
              $newd[$relatedkey] = $d[$relatedkey];
            }
            $data[] = $newd;
          }
        }
      }
    } 
  }
  fill_data_for_check_input_keys_if_needed($info, $config, $data, $extent);

  return FALSE;
}

/**
 * read_csv_file
 **/
function label_read_csv_file(array $info, array $config) {
  $title = t('Add items by reading CSV file');
  if (array_key_exists('csv_filepath', $config) && strlen($config['csv_filepath']) > 0) {
    $title .= ' ' . t('located at "@value"', array('@value' => $config['csv_filepath']));
  }
  if (array_key_exists('column_keys', $config) && strlen($config['column_keys']) > 0) {
    $title .= ' ' . t('and place the values in the keys "@value"', array('@value' => $config['column_keys']));
  }
  if (array_key_exists('has_header', $config) && $config['has_header']) {
    $title .= ' ' . t('but skip header');
  }
  $title .= '.';

  return $title;
}

function work_read_csv_file(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);
    if (count($results) > 0) {
      return $results;
    }
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    if (array_key_exists('csv_filepath', $config) && strlen($config['csv_filepath']) > 0) {
      $csvfilepath = $config['csv_filepath'];
      $columnkeys = explode(';', $config['column_keys']);
      $hasheader = $config['has_header'];
      
      ini_set("auto_detect_line_endings", "1");
      $linenr = 0;
      $delimit = "\t";
      if (($fh = fopen($csvfilepath, "r")) !== FALSE) {
	while (($csvdata = fgetcsv($fh, 0, $delimit)) !== FALSE) {
	  $linenr++;
	  if ($linenr == 1) {
	    // Treat header special.
	    if (count($csvdata) == 1) {
	      $delimit = ",";
	      $csvdata = str_getcsv($csvdata[0], $delimit);
	    }
	    if (count($csvdata) == 1) {
	      $delimit = ";";
	      $csvdata = str_getcsv($csvdata[0], $delimit);
	    }
	    if ($hasheader) {
	      // This is a header, so skip it.
	      continue;
	    }
	  }
	  $d = array();
	  foreach ($columnkeys as $columnindex => $columnkey) {
            if (isset($csvdata[$columnindex])) {
	      $d[$columnkey] = $csvdata[$columnindex]; 
            }
            else {
	      $d[$columnkey] = ''; 
            }
	  }
	  $data[] = $d;
	}
      }
    }
  }
  fill_data_for_check_input_keys_if_needed($info, $config, $data, $extent);

  return FALSE;
}

function get_output_keys_read_csv_file(array $info, array $config) {
  if (array_key_exists('column_keys', $config) && strlen($config['column_keys']) > 0) {
    $columnkeys = $config['column_keys'];
    $outputkeys = explode(';', $columnkeys);
    if (count($outputkeys) > 0) {
      return $outputkeys;
    }
  }
  return array();
}

/**
 * end_filter
 **/
function label_end_filter(array $info, array $config) {
  return t('Filter: end.');
}

function work_end_filter(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);

    if (count($results) > 0) {
      return $results;
    }
  }
  // do this for every other extent
  if (TRUE) {
    if (!(array_key_exists('_filters', $context) && count($context['_filters']) > 0)) {
        $results[] = array('text' => t("No filter to end"), 'type' => 'run_error');
    }
    if (count($results) > 0) {
      return $results;
    }
    $newdata = array();
    $filter = array_pop($context['_filters']);
    $keep = $filter['keep'];
    $remove = $filter['remove'];
    foreach ($keep as $toindex => $fromindex) {
      if (array_key_exists($fromindex, $data)) {
        $newdata[$toindex] = $data[$fromindex];
      }
    }
    foreach ($remove as $toindex => $d) {
      $newdata[$toindex] = $d;
    }
    // remove the data that was there when the filter began and replace it with
    // the filtered-out data + the data used in the filter itself.
    // Any data that was added in the filter will remain.
    array_splice($data, 0, count($keep), $newdata);
  }

  return FALSE;
}

/**
 * validate_basic_image_cmodel
 **/
function _label_validate_flat_structure($type, array $info, array $config) {
  $title = t('Validate the @type structure', array('@type' => $type));
  if (array_key_exists('filepath', $config) && strlen($config['filepath']) > 0) {
    $title .= ' ' . t('at "@value"', array('@value' => $config['filepath']));
  }
  $title .= '.';

  return $title;
}

function _work_validate_flat_structure(array $models, array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);

    if (count($results) > 0) {
      return $results;
    }
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    module_load_include('inc', 'islandora', 'includes/utilities');
    module_load_include('inc', 'ubl_prepare_ingest', 'includes/pifs');
   
    $importdirectory = rtrim($config['filepath'], DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
    $dsreqs = islandora_get_datastreams_requirements_from_models($models);
    $requiredmime2ds = array();
    $optionalmime2ds = array();
    foreach ($dsreqs as $dsid => $dsreq) {
      $optional = $dsreq['optional'];
      $mimes = $dsreq['mime'];
      foreach ($mimes as $mime) {
        if ($dsid === 'OBJ') {
          $requiredmime2ds[$mime] = $dsid;
        }
        else {
          $optionalmime2ds[$mime] = $dsid;
        }
      }
    }
    $mime_detect = new MimeDetect();
    $importpathdepth = count(explode(DIRECTORY_SEPARATOR, $importdirectory));
    $rightdepth = $importpathdepth + 1;
    $filenames = pifs_full_filenames_from_directory($importdirectory, $context, '', TRUE, TRUE);
    $isgood = (count($filenames) > 0);
    $groupedfilenames = array();
    foreach ($filenames as $filename) {
      $filepathdepth = count(explode(DIRECTORY_SEPARATOR, $filename));
      if (($filepathdepth > $rightdepth) || pifs_is_dir($filename, $context)) {
        $results[] = array('text' => t("@type '@file' found that should not be there", array('@type' => pifs_is_dir($filename, $context)?t('Directory'):t('File'), '@file' => $filename)), 'type' => 'run_error');
        $isgood = FALSE;
      }
      $info = pathinfo($filename);
      $mime = $mime_detect->getMimetype($filename); 
      $groupedfilenames[$info['filename']][$mime] = $filename; 
    }
    if (count($results) > 0) {
      return $results;
    }
    $stats = array();
    foreach ($groupedfilenames as $group => $mime2filename) {
      $requiredfound = $requiredmime2ds;
      $optionalfound = array();
      $otherfound = array();
      foreach ($mime2filename as $mime => $filename) {
        if (array_key_exists($mime, $requiredmime2ds)) {
          $founddsid = $requiredmime2ds[$mime];
          $requiredfound = array_diff($requiredfound, array($founddsid));
        }
        elseif (array_key_exists($mime, $optionalmime2ds)) {
          $optionalfound[] = $mime;
        }
        else {
          $otherfound[] = $mime;
        }
        if (!array_key_exists($mime, $stats)) {
          $stats[$mime] = 0;
        }
        $stats[$mime]++;
      }
      if (count($requiredfound) > 0) {
        $missingmime = implode(' ' . t('or') . ' ', array_keys($requiredfound));
        $results[] = array('text' => t("Missing required datastream with mime @mime for object named '@name'.", array('@mime' => $missingmime, '@name' => $group)), 'type' => 'run_error');
        $isgood = FALSE;
      }
      if (count($otherfound) > 0) {
        $results[] = array('text' => t("Other datastream found with mime type '@mime' for object named '@name'.", array('@mime' => $otherfound[0], '@name' => $group)), 'type' => 'run_error');
      }
    }
    $text = t("@objectcount objects found", array('@objectcount' => count($groupedfilenames)));
    if (count($stats) > 0) {
      if ($isgood) {
	$results[] = array('text' => t("The structure appears to be valid!"), 'type' => 'validate_info');
      }
      $text .= ' ' . t('with');
      $i = 1;
      foreach ($stats as $mime => $count) {
        $text .= (($i === 1)?' ':(($i === count($stats))?' and ':', ')) . t('@count @mime datastreams', array('@count' => $count, '@mime' => $mime));
        $i++;
      }
    }
    $text .= '.';
    $results[] = array('text' => $text, 'type' => 'validate_info'); 
    if ($isgood && ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
      global $base_url;
      global $user;

      $text = t('To ingest do the following:') . "\n";
      $text .= 'drush -v --root=' . DRUPAL_ROOT;
      $text .= ' --uri=' . $base_url . '/';
      $text .= ' --user=' . $user->name;
      $text .= ' islandora_batch_scan_preprocess';
      $text .= ' --namespace=' . $config['namespace'];
      $text .= ' --parent=' . $config['collection'];
      $text .= ' --content_models=' . implode(',', $models);
      $text .= ' --type=directory';
      $text .= ' --target=' . $config['filepath'];
      $text .= "\n" . t('Followed by:') . "\n";
      $text .= 'drush -v --root=' . DRUPAL_ROOT;
      $text .= ' --uri=' . $base_url . '/';
      $text .= ' --user=' . $user->name;
      $text .= ' islandora_batch_ingest';

      $results[] = array('text' => $text, 'type' => 'ingest_info');
    }
    return $results;
  }

  return FALSE;
}

function label_validate_basic_image(array $info, array $config) {
  return _label_validate_flat_structure(t('basic images'), $info, $config);
}

function work_validate_basic_image(array $info, array $config, &$data, &$context, $extent) {
  return _work_validate_flat_structure(array('islandora:sp_basic_image'), $info, $config, $data, $context, $extent);
}

/**
 * validate_large_image_cmodel
 **/
function label_validate_large_image(array $info, array $config) {
  return _label_validate_flat_structure(t('large images'), $info, $config);
}

function work_validate_large_image(array $info, array $config, &$data, &$context, $extent) {
  return _work_validate_flat_structure(array('islandora:sp_large_image_cmodel'), $info, $config, $data, $context, $extent);
}

/**
 * validate_book_cmodel
 **/
function label_validate_book(array $info, array $config) {
  $title = t('Validate the books structure');
  if (array_key_exists('filepath', $config) && strlen($config['filepath']) > 0) {
    $title .= ' ' . t('at "@value"', array('@value' => $config['filepath']));
  }
  $title .= '.';

  return $title;
}

function work_validate_book(array $info, array $config, &$data, &$context, $extent) {
  if ($extent & WORK_EXTENT_ONLY_CHECK_INPUT) {
    $results = check_input_keys_in_data($info, $config, $data);
    if (count($results) > 0) {
      return $results;
    }
  }
  if ($extent & WORK_EXTENT_ONLY_CHECK_ALL_ARGUMENTS) {
    $results = check_arguments($info, $config, $extent);

    if (count($results) > 0) {
      return $results;
    }
  }
  if (($extent & WORK_EXTENT_ONLY_DRY_RUN) || ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
    module_load_include('inc', 'islandora', 'includes/utilities');
    module_load_include('inc', 'ubl_prepare_ingest', 'includes/pifs');

    $importdirectory = $config['filepath'];
    $bookrequiredds2mimes = array();
    $bookoptionalds2mimes = array(
      'MODS' => array('application/xml', 'text/xml'),
      'TN'   => array('image/jpeg', 'image/png', 'image/gif'),
      'PDF'  => array('application/pdf'),
    );
    
    /* this does not work (yet)
    $bookmodels = array("islandora:bookCModel");
    $bookdsreqs = islandora_get_datastreams_requirements_from_models($bookmodels);
    foreach ($bookdsreqs as $dsid => $dsreq) {
      $optional = $dsreq['optional'];
      $mimes = $dsreq['mime'];
      foreach ($mimes as $mime) {
        if (TRUE) { // nothing is required at book level
          $bookoptionalmime2ds[$mime] = $dsid;
        }
        else {
          $bookrequiredmime2ds[$mime] = $dsid;
        }
      }
    }
    */
    $pagerequiredds2mimes = array(
      'OBJ' => array('image/tiff', 'image/jpeg', 'image/jp2'),
    );
    $pageoptionalds2mimes = array(
      'MODS'  => array('application/xml', 'text/xml'),
      'JP2'   => array('image/jp2'),
      'JPG'   => array('image/jpeg'),
      'TN'    => array('image/png', 'image/gif', 'image/jpeg'),
      'OCR'   => array('text/plain'),
      'HOCR'  => array('text/html', 'application/xml', 'text/xml', 'application/xhtml+xml'),
      'ALTO'  => array('application/xml', 'text/xml'),
    );
    /* this does not wrok (yet)
    $pagemodels = array("islandora:pageCModel");
    $pagedsreqs = islandora_get_datastreams_requirements_from_models($pagemodels);
    foreach ($pagedsreqs as $dsid => $dsreq) {
      $optional = $dsreq['optional'];
      $mimes = $dsreq['mime'];
      foreach ($mimes as $mime) {
        if ($dsid === 'OBJ') { // only OBJ is required at page level
          $pagerequiredmime2ds[$mime] = $dsid;
        }
        else {
          $pageoptionalmime2ds[$mime] = $dsid;
        }
      }
    }
    */
    $mime_detect = new MimeDetect();
    $filenames = pifs_full_filenames_from_directory($importdirectory, $context, '', TRUE, TRUE);
    $booksarray = array();
    foreach ($filenames as $filename) {
      $relpath = preg_replace('#^' . $importdirectory . '/*#', '', $filename);
      $relpathparts = explode(DIRECTORY_SEPARATOR, $relpath);
      $bookname = $relpathparts[0];
      if (pifs_is_dir($filename, $context)) {
        if (count($relpathparts) == 1) {
	    $booksarray[$bookname] = array();
        }
        elseif (count($relpathparts) == 2) {
          $pagenr = $relpathparts[1];
          if (preg_match('/^[0-9]+$/', $pagenr) !== 1) {
            $results[] = array('text' => t("Pages should be numeric only, but '@directory' found.", array('@directory' => $filename)), 'type' => 'validate_error');
          }
          $booksarray[$bookname][$pagenr] = array();
        }
        else {
	  $results[] = array('text' => t("Directory '@directory' found that should not be there", array('@directory' => $filename)), 'type' => 'validate_error');
        }
      }
      else {
        if (count($relpathparts) == 2) {
          // book
          $allowedfile = FALSE;
          $name = pathinfo($filename, PATHINFO_FILENAME);
          if (array_key_exists($name, $bookrequiredds2mimes)) {
            $mime = $mime_detect->getMimetype($filename);
            if (in_array($mime, $bookrequiredds2mimes[$name])) {
              $allowedfile = TRUE;
            }
          }
          if (array_key_exists($name, $bookoptionalds2mimes)) {
            $mime = $mime_detect->getMimetype($filename);
            if (in_array($mime, $bookoptionalds2mimes[$name])) {
              $allowedfile = TRUE;
            }
          }
          if (!$allowedfile) {
	    $results[] = array('text' => t("File '@file' of mime '@mime' found that should not be there", array('@file' => $filename, '@mime' => $mime)), 'type' => 'validate_error');
          }
        }
        elseif (count($relpathparts) == 3) {
          // page
          $allowedfile = FALSE;
          $name = pathinfo($filename, PATHINFO_FILENAME);
          $mime = '';
          if (array_key_exists($name, $pagerequiredds2mimes)) {
            $mime = $mime_detect->getMimetype($filename);
            if (in_array($mime, $pagerequiredds2mimes[$name])) {
              $allowedfile = TRUE;
              $booksarray[$bookname][$pagenr][$name] = $filename; 
            }
          }
          if (array_key_exists($name, $pageoptionalds2mimes)) {
            $mime = $mime_detect->getMimetype($filename);
            if (in_array($mime, $pageoptionalds2mimes[$name])) {
              $allowedfile = TRUE;
              $booksarray[$bookname][$pagenr][$name] = $filename; 
            }
          }
          if (!$allowedfile) {
            if (!$mime) {
              $mime = $mime_detect->getMimetype($filename);
            }
            $results[] = array('text' => t("File '@file' of mime '@mime' found that should not be there", array('@file' => $filename, '@mime' => $mime)), 'type' => 'validate_error');
          }
        }
        else {
	  $results[] = array('text' => t("File '@file' found that should not be there", array('@file' => $filename)), 'type' => 'validate_error');
        }
      }
    }
    $isbookstructure = ((count($booksarray) > 0) && (count($results) === 0));
    $stats = array();
    $pagecount = 0;
    foreach ($booksarray as $book => $pages) {
      foreach ($pages as $page => $ds2file) {
        $pagecount++;
        $notfound = array_diff_key($pagerequiredds2mimes, $ds2file);
        if (count($notfound) > 0) {
	  $results[] = array('text' => t("Missing required datastream @ds for page @page of book '@book'.", array('@ds' => key($notfound), '@page' => $page, '@book' => $book)), 'type' => 'validate_error');
          $isbookstructure = FALSE;
        }
        foreach ($ds2file as $ds => $file) {
          if (!array_key_exists($ds, $stats)) {
            $stats[$ds] = 0;
          }
          $stats[$ds]++;
        }
      }
    }
    $text = t("@bookcount books found", array('@bookcount' => count($booksarray)));
    if (count($booksarray) > 0) {
      if ($isbookstructure) {
        $results[] = array('text' => t("The structure appears to be valid!"), 'type' => 'validate_info');
      }
      $text .= ' ' . t('with @pagecount pages total', array('@pagecount' => $pagecount));
      $i = 1;
      if (count($stats) > 0) {
        $text .= ' ' . t('and');
        foreach ($stats as $mime => $count) {
          $text .= (($i === 1)?' ':(($i === count($stats))?' and ':', ')) . t('@count @mime datastreams', array('@count' => $count, '@mime' => $mime));
          $i++;
        }
      }
      else {
        $text .= ' ' . t('but without any datastreams');
      }
    }
    $text .= '.';
    $results[] = array('text' => $text, 'type' => 'validate_info');

    if ($isbookstructure && ($extent & WORK_EXTENT_ONLY_DO_ALL)) {
      global $base_url;
      global $user;

      $text = t('To ingest do the following:') . "\n";
      $text .= 'drush -v --root=' . DRUPAL_ROOT;
      $text .= ' --uri=' . $base_url . '/';
      $text .= ' --user=' . $user->name;
      $text .= ' islandora_book_batch_preprocess';
      $text .= ' --namespace=' . $config['namespace'];
      $text .= ' --parent=' . $config['collection'];
      $text .= ' --content_models=islandora:bookCModel';
      $text .= ' --type=directory';
      $text .= ' --target=' . $config['filepath'];
      $text .= "\n" . t('Followed by:') . "\n";
      $text .= 'drush -v --root=' . DRUPAL_ROOT;
      $text .= ' --uri=' . $base_url . '/';
      $text .= ' --user=' . $user->name;
      $text .= ' islandora_batch_ingest';

      $results[] = array('text' => $text, 'type' => 'ingest_info');
    }
    return $results;
  }
}

/**
 * validate_newspaper
 **/
function label_validate_newspaper(array $info, array $config) {

}

function work_validate_newspaper(array $info, array $config, &$data, &$context, $extent) {

}

