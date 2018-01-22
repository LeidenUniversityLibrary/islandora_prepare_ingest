<?php

/**
 * @file
 * Step info.
 */

/**
 * Implements hook_workflow_step_info
 */
function islandora_prepare_ingest_workflow_step_info() {
  $info = array();

  $info['define_constant'] = array(
    'name' => t('Define constant'),
    'type group' => ' ' . t('Constants'),
    'class name' => 'DefineConstantStep',
  );
  $info['read_file_names'] = array(
    'name' => t('Read filenames'),
    'type group' => t('Add items from files'),
    'class name' => 'ReadFilenamesStep',
  );
  $info['add_key_template'] = array(
    'name' => t('Add key with template based value'),
    'type group' => t('Add key(s)'),
    'class name' => 'AddKeyTemplateStep',
  );
  $info['write_file'] = array(
    'name' => t('Write to file'),
    'type group' => t('Files and directories'),
    'class name' => 'WriteFileStep',
  );

/*
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
        'allow_constants' => TRUE,
      ),
      'replacement' => array(
        'label' => t('Replacement'),
        'type' => 'string',
        'allow_constants' => TRUE,
      ),
    ),
    'input keys' => array('source_key_name'),
    'output keys' => array('target_key_name'),
    'changes count' => FALSE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
    'type group' => t('Add key(s)'),
  );
*/
/*
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
        'allow_constants' => TRUE,
      ),
      'step' => array(
        'label' => t('Step'),
        'type' => 'number',
        'default_value' => 1,
        'allow_constants' => TRUE,
      ),
      'width' => array(
        'label' => t('Width'),
        'description' => t('The width of the number, it will be prepended by zero\'s to meet this width.'),
        'type' => 'number',
        'default_value' => 1,
        'allow_constants' => TRUE,
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
    'type group' => t('Add key(s)'),
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
        'allow_constants' => TRUE,
      ),
    ),
    'input keys' => array('source_key_name'),
    'output keys callback' => 'get_output_keys_extract_keys_template',
    'changes count' => FALSE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
    'type group' => t('Add key(s)'),
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
          'xmlencode' => t('XML encode'),
          'xmldecode' => t('XML decode'),
	),
      ),
    ),
    'input keys' => array('source_key_name'),
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
    'type group' => t('Modify key(s)'),
  );
  $info['make_directory'] = array(
    'arguments' => array(
      'directory_name' => array(
        'label' => t('Directory name'),
        'description' => t('This should hold an absolute path to a not yet existing directory. The parent directory should exist already.'),
        'type' => 'templatestring',
      ),
    ),
    'input keys callback' => 'get_input_keys_from_template',
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => FALSE,
    'changes files' => TRUE,
    'type group' => t('Files and directories'),
  );
  $info['copy_file'] = array(
    'arguments' => array(
      'source_filepath' => array(
        'label' => t('Source filepath'),
        'description' => t('This should hold an absolute path to an existing file.'),
        'type' => 'templatestring',
      ),
      'target_filepath' => array(
        'label' => t('Target filepath'),
        'description' => t('This should hold an absolute path to an existing directory or the absolute path including the new name of the file to copy.'),
        'type' => 'templatestring',
      ),
    ),
    'input keys callback' => 'get_input_keys_from_template',
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => FALSE,
    'changes files' => TRUE,
    'type group' => t('Files and directories'),
  );
  $info['write_file'] = array(
    'arguments' => array(
      'content_key_name' => array(
        'label' => t('Content key name'),
        'description' => t('This key should hold the content for the file.'),
        'type' => 'key',
      ),
      'target_filepath' => array(
        'label' => t('Target filepath'),
        'description' => t('This should hold an absolute path to a not existing file.'),
        'type' => 'templatestring',
      ),
    ),
    'input keys' => array('content_key_name'),
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => FALSE,
    'changes files' => TRUE,
    'type group' => t('Files and directories'),
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
    'type group' => t('Sorting, filtering and grouping'),
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
        'allow_constants' => TRUE,
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
    'type group' => t('Sorting, filtering and grouping'),
  );
  $info['parse_xml'] = array(
    'arguments' => array(
      'filepath' => array(
        'label' => t('Filepath'),
        'description' => t('This should hold an absolute path to an existing XML file.'),
        'type' => 'templatestring',
      ),
      'xml_key_name' => array(
        'label' => t('XML key name'),
        'description' => t('The key where the XML should be stored.'),
        'type' => 'key',
      ),
    ),
    'input keys callback' => 'get_input_keys_from_template',
    'output keys' => array('xml_key_name'),
    'changes count' => FALSE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
    'type group' => t('Handling XML'),
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
        'allow_constants' => TRUE,
      ),
      'namespaces' => array(
        'label' => t('namespaces'),
        'description' => t('The namespaces; one namespace per line in the following format: prefix=namespace URI'),
        'type' => 'text',
        'allow_constants' => TRUE,
      ),
    ),
    'input keys' => array('source_xml_key_name'),
    'output keys' => array('target_key_name'),
    'changes count' => FALSE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
    'type group' => t('Handling XML'),
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
        'allow_constants' => TRUE,
      ),
      'namespaces' => array(
        'label' => t('namespaces'),
        'description' => t('The namespaces; one namespace per line in the following format: prefix=namespace URI'),
        'type' => 'text',
        'allow_constants' => TRUE,
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
    'type group' => t('Handling XML'),
  );
  $info['read_csv_file'] = array(
    'arguments' => array(
      'csv_filepath' => array(
        'label' => t('CSV filepath'),
        'description' => t('Specify the absolute path to the CSV file.'),
        'type' => 'filepath',
        'allow_constants' => TRUE,
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
    'type group' => t('Add items from files'),
  );
  $info['end_filter'] = array(
    'arguments' => array(),
    'input keys' => array(),
    'output keys' => array(),
    'changes count' => TRUE,
    'changes keys' => TRUE,
    'changes files' => FALSE,
    'type group' => t('Sorting, filtering and grouping'),
  );
  $info['visual_group_start'] = array(
    'arguments' => array(
      'description' => array(
        'label' => t('Description'),
        'description' => t('A description of this visual grouping.'),
        'type' => 'template',
        'allow_constants' => TRUE,
      ),
    ),
    'input keys' => array(),
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => FALSE,
    'changes files' => FALSE,
    'visual' => array('groupstart','groupremove','ungroup'),
    'type group' => ' ' . t('Visual'),
  );
  $info['visual_group_end'] = array(
    'arguments' => array(),
    'input keys' => array(),
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => FALSE,
    'changes files' => FALSE,
    'visual' => array('groupend','nomove','noremove'),
    'type group' => ' ' . t('Visual'),
  );
  $info['sort_items'] = array(
    'arguments' => array(
      'order_key_1' => array(
        'label' => t('Order key 1'),
        'description' => t('Order by the value of this key.'),
        'type' => 'key',
      ),
      'order_type_1' => array(
        'label' => t('Order type 1'),
        'description' => t('How to order the value of this key.'),
        'type' => 'select',
        'options' => array(
          'strcmp' => t('string compare'),
          'strcasecmp' => t('string compare case-insensitive'),
          'intcmp' => t('integer compare'),
          'strcmprev' => t('string compare reverse'),
          'strcasecmprev' => t('string compare case-insensitive reverse'),
          'intcmprev' => t('integer compare reverse'),
        ),
      ),
      'order_key_2' => array(
        'label' => t('Order key 2'),
        'description' => t('Optional, order by the value of this key.'),
        'type' => 'key',
        'optional' => TRUE,
      ),
      'order_type_2' => array(
        'label' => t('Order type 2'),
        'description' => t('Optional, how to order the value of this key.'),
        'type' => 'select',
        'options' => array(
          'strcmp' => t('string compare'),
          'strcasecmp' => t('string compare case-insensitive'),
          'intcmp' => t('integer compare'),
          'strcmprev' => t('string compare reverse'),
          'strcasecmprev' => t('string compare case-insensitive reverse'),
          'intcmprev' => t('integer compare reverse'),
        ),
        'optional' => TRUE,
      ),
    ),
    'input keys' => array('order_key_1','order_key_2'),
    'output keys' => array(),
    'changes count' => TRUE,
    'changes keys' => FALSE,
    'changes files' => FALSE,
    'type group' => t('Sorting, filtering and grouping'),
  );
  $info['transform_xml'] = array(
    'arguments' => array(
      'xml_key_name' => array(
        'label' => t('XML key'),
        'description' => t('The input XML. The value of this key is either an absolute filepath to an XML file or contains XML itself.'),
        'type' => 'key',
      ),
      'xsl_key_name' => array(
        'label' => t('XSLT key'),
        'description' => t('The input XSLT. The value of this key is either an absolute filepath to an XSLT file or contains XSLT itself.'),
        'type' => 'key',
      ),
      'target_filepath' => array(
        'label' => t('Target filepath'),
        'description' => t('This should hold an absolute path to a not existing file where the output XML file will be written.'),
        'type' => 'templatestring',
      ),
    ),
    'input keys' => array('xml_key_name', 'xsl_key_name'),
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => FALSE,
    'changes files' => TRUE,
    'type group' => t('Handling XML'),
  );
  $info['validate_xml'] = array(
    'arguments' => array(
      'xml_key_name' => array(
        'label' => t('XML key'),
        'description' => t('Validate the XML. The value of this key is either an absolute filepath to an XML file or contains XML itself.'),
        'type' => 'key',
      ),
      'validate_dtd' => array(
        'label' => t('Validate based on DTD'),
        'description' => t('The XML should contain a DTD for this to work.'),
        'type' => 'boolean',
      ),
      'schema_key_name' => array(
        'label' => t('Schema key'),
        'description' => t('Optionally, validate against a schema. The value of this key is either an absolute filepath to a schema file or contains a schema itself.'),
        'type' => 'key',
        'optional' => TRUE,
      ),
    ),
    'input keys' => array('xml_key_name', 'schema_key_name'),
    'output keys' => array(),
    'changes count' => FALSE,
    'changes keys' => FALSE,
    'changes files' => FALSE,
    'type group' => t('Handling XML'),
   );
   $validateconfig = array(
    'arguments' => array(
      'filepath' => array(
        'label' => t('Filepath'),
        'description' => t('The absolute path to the directory where the import files are placed.'),
        'type' => 'filepath',
        'allow_constants' => TRUE,
      ),
      'namespace' => array(
        'label' => t('Namespace'),
        'description' => t('The namespace of objects to create.'),
        'type' => 'namespace',
        'allow_constants' => TRUE,
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
    'type group' => t('Validation'),
  );
  $info['validate_basic_image'] = $validateconfig;
  $info['validate_large_image'] = $validateconfig;
  $info['validate_book'] = $validateconfig;
*/
  /*$info['validate_newspaper'] = $validateconfig;*/

  return $info;
}

