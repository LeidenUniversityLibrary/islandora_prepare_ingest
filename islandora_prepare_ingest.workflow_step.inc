<?php

/**
 * @file
 * Step info.
 *
 *
 *  Copyright 2017-2019 Leiden University Library
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
  $info['add_key_regexp'] = array(
    'name' => t('Add key with regular expression based value'),
    'type group' => t('Add key'),
    'class name' => 'AddKeyRegExStep',
  );
  $info['add_key_counter'] = array(
    'name' => t('Add key with counter value'),
    'type group' => t('Add key'),
    'class name' => 'AddKeyCounterStep',
  );
  $info['extract_keys_template'] = array(
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
    'name' => t('Filter  items'),
    'type group' => t('Sorting, filtering and grouping'),
    'class name' => 'FilterItemsStep',
  );
  $info['end_filter'] = array(
    'name' => t('Filter end'),
    'type group' => t('Sorting, filtering and grouping'),
    'class name' => 'EndFilterItemsStep',
  );
  $info['parse_xml'] = array(
    'name' => t('Add key with parsed XML'),
    'type group' => array(t('Handling XML'), t('Add key')),
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
    'type group' => array(t('Validation'), t('Handling XML')),
    'class name' => 'ValidateXmlStep',
  );
  $info['validate_basic_image'] = array(
    'name' => t('Validate Basic Image'),
    'type group' => t('Validation'),
    'class name' => 'ValidateBasicImageStep',
  );
  $info['validate_large_image'] = array(
    'name' => t('Validate Large Image'),
    'type group' => t('Validation'),
    'class name' => 'ValidateLargeImageStep',
  );
  $info['validate_book'] = array(
    'name' => t('Validate Book'),
    'type group' => t('Validation'),
    'class name' => 'ValidateBookStep',
  );
  $info['validate_pdf'] = array(
    'name' => t('Validate PDF'),
    'type group' => t('Validation'),
    'class name' => 'ValidatePDFStep',
  );
  $info['validate_compound'] = array(
    'name' => t('Validate Compound'),
    'type group' => t('Validation'),
    'class name' => 'ValidateCompoundStep',
  );
  $info['filter_by_file'] = array(
    'name' => t('Filter by file'),
    'type group' => t('Sorting, filtering and grouping'),
    'class name' => 'FilterByFileStatsStep',
  );
  $info['add_key_using_matching_item'] = array(
    'name' => t('Add key using matching item'),
    'type group' => t('Add key'),
    'class name' => 'AddKeyUsingMatchingItemStep',
  );
  $info['write_to_csv_file'] = array(
    'name' => t('Write to CSV file'),
    'type group' => t('Files and directories'),
    'class name' => 'WriteCsvFileStep',
  );
  $info['extract_archive'] = array(
    'name' => t('Extract an archive'),
    'type group' => t('Files and directories'),
    'class name' => 'ExtractArchiveStep',
  );
  $info['add_key_from_file'] = array(
    'name' => t('Add key from file'),
    'type group' => t('Add key'),
    'class name' => 'AddKeyFromFile',
  );

  return $info;
}

