<?php

/**
 * @file
 * steps/ReadFilenamesStep.inc
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


class ReadFilenamesStep extends PrepareIngestStep {
  
  public function label() {
    $title = t('Add items');
    if (array_key_exists('target_key_name', $this->config) && strlen($this->config['target_key_name']) > 0) {
      $title .= ' ' . t('with key "@key"', array('@key' => $this->config['target_key_name']));
    }
    $title .= ' ' . t('by reading file names');
    if (array_key_exists('directory_path', $this->config) && strlen($this->config['directory_path']) > 0) {
      $title .= ' ' . t('from "@path"', array('@path' => $this->config['directory_path']));
    }
    if (array_key_exists('filter', $this->config) && strlen($this->config['filter']) > 0) {
      $title .= ' ' . t('filtered by "@filter"', array('@filter' => $this->config['filter']));
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'directory_path' => array(
        'label' => t('Directory path'),
        'description' => t('Specify the absolute path to the directory where the input files are located.'),
        'type' => 'filepath',
        'allow_constants' => TRUE,
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
    );
  }
    
  protected function _outputKeyKeys() {
    return array('target_key_name');
  } 
  
  public function changesCount() {
    return TRUE;
  }
  
  public function changesKeys() {
    return TRUE;
  }

  public function changesFiles() {
    return FALSE;
  }

  public function checkConfiguration($context) {
    $results = parent::checkConfiguration($context);
    if ($this->hasConfigKey('directory_path')) {
      $dirpath = $this->config['directory_path'];
      if (substr($dirpath, 0, 1) !== DIRECTORY_SEPARATOR) {
        $results[] = array('text' => t("Argument '@key' should be an absolute filepath, but is '@value'.",
                  array('@key' => $this->arguments()['directory_path']['label'], '@value' => $dirpath)), 'type' => 'argument_error');
      }
      if (!is_dir($dirpath)) {
        $results[] = array('text' => t("Argument '@key' should be an existing filepath, but is '@value'",
                  array('@key' => $this->arguments()['directory_path']['label'], '@value' => $dirpath)), 'type' => 'argument_error');
      }
    }
    if ($results) {
      return $results;
    }

    return FALSE;
  }

  public function dryRun(&$context) {
    if ($this->hasConfigKey('directory_path')) {
      $prevdatastep = $this->_previousDataChangingStep();

      module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $dirpath = $this->config['directory_path'];
      $filter = $this->config['filter'];
      $includedir = $this->config['include_subdirectory_files'];
      $filenames = pifs_full_filenames_from_directory($dirpath, $this->workflowid, $this->otherid, $filter, $includedir);
      $key = $this->config['target_key_name'];

      // first add all existing items
      $itemnr = 1;
      if (isset($prevdatastep)) {
        $prevdatastep->resetDataIterator();
        $item = $prevdatastep->nextDataItem();
        while ($item !== NULL) {
          $this->saveDataItem($item);
          $itemnr = $item['item nr'];
          $item = $prevdatastep->nextDataItem();
        }
        $itemnr++;
      }

      foreach ($filenames as $filename) {
        $item = array($key => $filename, 'item nr' => $itemnr);
        $this->saveDataItem($item);
        $itemnr++;
      }
    }
    return FALSE;
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    // no need to do anything
  }

}
