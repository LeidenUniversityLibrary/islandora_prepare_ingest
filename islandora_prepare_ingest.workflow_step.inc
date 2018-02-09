<?php

/**
 * @file
 * Step info.
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
  $info['add_key_regex'] = array(
    'name' => t('Add key with regular expression based value'),
    'type group' => t('Add key(s)'),
    'class name' => 'AddKeyRegExStep',
  );
  $info['add_key_counter'] = array(
    'name' => t('Add key with counter value'),
    'type group' => t('Add key(s)'),
    'class name' => 'AddKeyCounterStep',
  );
  $info['add_keys_extracting_values'] = array(
    'name' => t('Add key(s) by extracting values'),
    'type group' => t('Add key(s)'),
    'class name' => 'AddKeysByExtractingStep',
  );
  $info['modify_key_value'] = array(
    'name' => t('Modify key value'),
    'type group' => t('Modify key(s)'),
    'class name' => 'ModifyKeyValueStep',
  );
  $info['make_directory'] = array(
    'name' => t('Make directory'),
    'type group' => t('Files and directories'),
    'class name' => 'MakeDirectoryStep',
  );
  $info['write_file'] = array(
    'name' => t('Write to file'),
    'type group' => t('Files and directories'),
    'class name' => 'WriteFileStep',
  );
  $info['copy_file'] = array(
    'name' => t('Copy file'),
    'type group' => t('Files and directories'),
    'class name' => 'CopyFileStep',
  );
  $info['group_items'] = array(
    'name' => t('Group items'),
    'type group' => t('Sorting, filtering and grouping'),
    'class name' => 'GroupItemsStep',
  );
  $info['filter_items'] = array(
    'name' => t('Filter items  start'),
    'type group' => t('Sorting, filtering and grouping'),
    'class name' => 'FilterItemsStep',
  );
  $info['end_filter'] = array(
    'name' => t('Filter items end'),
    'type group' => t('Sorting, filtering and grouping'),
    'class name' => 'EndFilterItemsStep',
  );
  $info['parse_xml'] = array(
    'name' => t('Parse XML'),
    'type group' => t('Handling XML'),
    'class name' => 'ParseXmlStep',
  );
  $info['value_from_xml'] = array(
    'name' => t('Add key from XML'),
    'type group' => t('Handling XML'),
    'class name' => 'ValueFromXmlStep',
  );

/*
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

