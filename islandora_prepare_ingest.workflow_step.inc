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
    'name' => t('Add items by reading filenames'),
    'type group' => t('Add items'),
    'class name' => 'ReadFilenamesStep',
  );
  $info['add_key_template'] = array(
    'name' => t('Add key with template based value'),
    'type group' => t('Add key'),
    'class name' => 'AddKeyTemplateStep',
  );
  $info['add_key_regex'] = array(
    'name' => t('Add key with regular expression based value'),
    'type group' => t('Add key'),
    'class name' => 'AddKeyRegExStep',
  );
  $info['add_key_counter'] = array(
    'name' => t('Add key with counter value'),
    'type group' => t('Add key'),
    'class name' => 'AddKeyCounterStep',
  );
  $info['add_keys_extracting_values'] = array(
    'name' => t('Add key(s) by extracting values'),
    'type group' => t('Add key'),
    'class name' => 'AddKeysByExtractingStep',
  );
  $info['modify_key_value'] = array(
    'name' => t('Modify key value'),
    'type group' => t('Modify key'),
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
    'name' => t('Add key with parsed XML'),
    'type group' => t('Handling XML'),
    'class name' => 'ParseXmlStep',
  );
  $info['value_from_xml'] = array(
    'name' => t('Add key from XML'),
    'type group' => array(t('Handling XML'), t('Add key')),
    'class name' => 'ValueFromXmlStep',
  );
  $info['xml_parts_from_xml'] = array(
    'name' => t('Add items by extracting XML parts from XML'),
    'type group' => array(t('Handling XML'), t('Add items')),
    'class name' => 'ExtractPartsFromXmlStep',
  );
  $info['read_csv_file'] = array(
    'name' => t('Add items by reading CSV file'),
    'type group' => t('Add items'),
    'class name' => 'ReadCsvFileStep',
  );
  $info['visual_group_start'] = array(
    'name' => t('Visual group  start'), // 2 spaces for sorting.
    'type group' => t('Visual'),
    'class name' => 'VisualGroupStartStep',
  );
  $info['visual_group_end'] = array(
    'name' => t('Visual group end'),
    'type group' => t('Visual'),
    'class name' => 'VisualGroupEndStep',
  );
  $info['sort_items'] = array(
    'name' => t('Sort items'),
    'type group' => t('Sorting, filtering and grouping'),
    'class name' => 'SortItemsStep',
  );
  $info['transform_xml'] = array(
    'name' => t('Transform XML'),
    'type group' => t('Handling XML'),
    'class name' => 'TransformXmlStep',
  );
  $info['validate_xml'] = array(
    'name' => t('Validate XML'),
    'type group' => t('Handling XML'),
    'class name' => 'ValidateXmlStep',
  );

/*
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

