<?php

/**
 * @file
 * steps/FilterByFileStatsStep.inc
 *  
 *  
 *  Copyright 2019 Leiden University Library
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

class FilterByFileStatsStep extends FilterItemsStep implements PrepareIngestFilteringInterface {

  public function label() {
    $title = t('Filter by file');
    if ($this->hasConfigKey('filter_key_name')) {
      $title .= ' ' . t('located at "@key"', array('@key' => $this->config['filter_key_name']));
    }
    if ($this->hasConfigKey('filter_type')) {
      if ($this->config['filter_type'] === 'file_exists') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('if file does not exist');
        }
        else {
          $title .= ' ' . t('if file exists');
        }
      }
      elseif ($this->config['filter_type'] === 'dir_exists') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('if directory does not exist');
        }
        else {
          $title .= ' ' . t('if directory exists');
        }
      }
      elseif ($this->config['filter_type'] === 'larger_size') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('if it is smaller than');
        }
        else {
          $title .= ' ' . t('if it is larger than');
        }
        if ($this->hasConfigKey('filter_value')) {
          $title .= ' ' . $this->config['filter_value'];
        }
      }
      elseif ($this->config['filter_type'] === 'larger_file') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('if it is smaller than file at');
        }
        else {
          $title .= ' ' . t('if it is larger than file at');
        }
        if ($this->hasConfigKey('filter_value')) {
          $title .= ' ' . '"' . $this->config['filter_value'] . '"';
        }
      }
      elseif ($this->config['filter_type'] === 'older_date') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('if it is modified before');
        }
        else {
          $title .= ' ' . t('if it is modified after');
        }
        if ($this->hasConfigKey('filter_value')) {
          $title .= ' ' . $this->config['filter_value'];
        }
      }
      elseif ($this->config['filter_type'] === 'older_file') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('if it is modified before file at');
        }
        else {
          $title .= ' ' . t('if it is modified after file at');
        }
        if ($this->hasConfigKey('filter_value')) {
          $title .= ' ' . '"' . $this->config['filter_value'] . '"';
        }
      }
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'filter_key_name' => array(
        'label' => t('Filepath'),
        'description' => t('This should hold an absolute path.'),
        'type' => 'templatestring',
      ),
      'filter_type' => array(
        'label' => t('Filter type'),
        'description' => t('Choose how you want to filter: existance, size or modification date.'),
        'type' => 'select',
        'options' => array(
          'file_exists' => t('file exists'),
          'dir_exists' => t('directory exists'),
          'larger_size' => t('file size is larger than size'),
          'larger_file' => t('file is larger in size than other file'),
          'older_date' => t('file modified after date'),
          'older_file' => t('file modified after other file'),
        ),
      ),
      'filter_value' => array(
        'label' => t('Filter value'),
        'description' => t('The value to filter by.'),
        'type' => 'templatestring',
        'allow_constants' => TRUE,
        'optional' => array('filter_type' => array('file_exists', 'dir_exists')),
      ),
      'negate' => array(
        'label' => t('Negate'),
        'description' => t('If true, the items that do not meet the filter will be included.'),
        'type' => 'boolean',
      ),
    );
  }

  public function inputKeys() {
    return $this->_getInputKeysFromTemplates(array('filter_key_name', 'filter_value'));
  }

  public function outputKeys() {
    return array();
  }

  public function changesCount() {
    return TRUE;
  }

  public function changesKeys() {
    return FALSE;
  }

  public function changesFiles() {
    return FALSE;
  }

  public function checkConfiguration($context) {
    $results = parent::checkConfiguration($context);
    if ($this->hasConfigKey('filter_type')) {
      if ($this->config['filter_type'] === 'file_exists') {
        // no check needed, filter_value does not matter.
        unset($results['filter_value']);
      }
      elseif ($this->config['filter_type'] === 'dir_exists') {
        // no check needed, filter_value does not matter.
        unset($results['filter_value']);
      }
      elseif (in_array($this->config['filter_type'], array('larger_size', 'larger_file', 'older_date', 'older_file'))) {
        if (!$this->hasConfigKey('filter_value')) {
          $results[] = array('text' => t("Filter with type '@type' should have a value.", array('@type' => $this->config['filter_type'])), 'type' => 'argument_error');
        }
      }
      elseif ($this->config['filter_type'] === 'larger_size') {
        if (preg_match('/^\d+\s*(?:B|bytes?|kB|KB|MB|KiB|MiB)$', $this->hasConfigKey('filter_value')) === 0) {
          $results[] = array('text' => t("Filter with type '@type' should have a value like '20 bytes', '4KB' or '15MB. Allowed units are B, bytes, kB, KB, KiB, MB and MiB.", array('@type' => $this->config['filter_type'])), 'type' => 'argument_error');
        }
      }
      elseif ($this->config['filter_type'] === 'older_date') {
        if (preg_match('/^\d\d\d\d-\d\d-\d\d(?:T\d\d:\d\d:\d\d)?$', $this->hasConfigKey('filter_value')) === 0) {
          $results[] = array('text' => t("Filter with type '@type' should have a date formatted as 2019-01-14 or 2019-01-14T16:43:21.", array('@type' => $this->config['filter_type'])), 'type' => 'argument_error');
        }
      }
    }
    return $results;
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('filter_key_name') && $this->hasConfigKey('filter_type') && isset($prevdatastep)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

      $filterconfig = $this->filterConfig();

      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      $currentitemnr = 1;
      while ($item !== NULL) {
        $shouldkeep = $this->shouldKeep($item, $filterconfig);

        if ($shouldkeep) {
          $item['item nr'] = $currentitemnr;
          $this->saveDataItem($item);
          $currentitemnr++;
        }
        $item = $prevdatastep->nextDataItem();
      }
      $prevdatastep->clearDataIterator();
    }
    return FALSE;
  }

  public function shouldKeep($item, $filterconfig) {
    $filterkey = $this->fillOutTemplate($filterconfig['filter_key_name'], $item);
    $type = $filterconfig['filter_type'];
    $value = $this->fillOutTemplate($filterconfig['filter_value'], $item);
    $negate = $filterconfig['negate'];

    $shouldkeep = FALSE;
    if ($type === 'file_exists') {
      $shouldkeep = pifs_is_file($filterkey, $this->workflowid, $this->stepid, $this->otherid);
    }
    elseif ($type === 'dir_exists') {
      $shouldkeep = pifs_is_dir($filterkey, $this->workflowid, $this->stepid, $this->otherid);
    }
    elseif ($type === 'larger_size') {
      $units = array(
        'B' => 1,
        'bytes' => 1,
        'kB' => 1000,
        'KB' => 1024,
        'MB' => 1000 * 1000,
        'KiB' => 1024,
        'MiB' => 1024 * 1024,
      );
      $sizevalue = preg_replace_callback('/^(\d+)\s*(B|bytes?|kB|KB|MB|KiB|MiB)$/', function ($matches) use ($units) {
        return $matches[1] * $units[$matches[2]];
      }, $value);
      $sizevalue = filter_var($sizevalue, FILTER_VALIDATE_INT, array('options' => array('min_range' => 0)));
      if ($sizevalue === FALSE) {
        $results[] = array('text' => t("Size '@size' is not a valid size.",
          array('@size' => $value)), 'type' => 'run_error');
      }
      $sizefile = pifs_filesize($filterkey, $this->workflowid, $this->stepid, $this->otherid);
      if ($sizefile === FALSE) {
        $results[] = array('text' => t("File '@file' does not exist, so cannot check size.",
          array('@file' => $filterkey)), 'type' => 'run_error');
      }
      if (count($results) > 0) {
        return $results;
      }
      $shouldkeep = ($sizefile > $sizevalue);
    }
    elseif ($type === 'larger_file') {
      $sizefile1 = pifs_filesize($filterkey, $this->workflowid, $this->stepid, $this->otherid);
      if ($sizefile1 === FALSE) {
        $results[] = array('text' => t("File '@file' does not exist, so cannot check size.",
          array('@file' => $filterkey)), 'type' => 'run_error');
      }
      $sizefile2 = pifs_filesize($value, $this->workflowid, $this->stepid, $this->otherid);
      if ($sizefile2 === FALSE) {
        $results[] = array('text' => t("File '@file' does not exist, so cannot check size.",
          array('@file' => $value)), 'type' => 'run_error');
      }
      if (count($results) > 0) {
        return $results;
      }
      $shouldkeep = ($sizefile1 > $sizefile2);
    }
    elseif ($type === 'older_date') {
      $mdatevalue = strtotime($value);
      if ($mdatevalue === FALSE || $mdatevalue === -1) {
        $results[] = array('text' => t("Date '@date' is not a valid date.",
          array('@value' => $value)), 'type' => 'run_error');
      }
      $mdatefile = pifs_filemtime($filterkey, $this->workflowid, $this->stepid, $this->otherid);
      if ($mdatefile === FALSE) {
        $results[] = array('text' => t("File '@file' does not exist, so is does not have a modification date.",
          array('@file' => $filterkey)), 'type' => 'run_error');
      }
      if (count($results) > 0) {
        return $results;
      }
      $shouldkeep = ($mdatefile > $mdatevalue);
    }
    elseif ($type === 'older_file') {
      $mdatefile1 = pifs_filemtime($filterkey, $this->workflowid, $this->stepid, $this->otherid);
      if ($mdatefile1 === FALSE) {
        $results[] = array('text' => t("File '@file' does not exist, so is does not have a modification date.",
          array('@file' => $filterkey)), 'type' => 'run_error');
      }
      $mdatefile2 = pifs_filemtime($value, $this->workflowid, $this->stepid, $this->otherid);
      if ($mdatefile2 === FALSE) {
        $results[] = array('text' => t("File '@file' does not exist, so is does not have a modification date.",
          array('@file' => $value)), 'type' => 'run_error');
      }
      if (count($results) > 0) {
        return $results;
      }
      $shouldkeep = ($mdatefile1 > $mdatefile2);
    }
    if ($negate) {
      $shouldkeep = !$shouldkeep;
    }
    return $shouldkeep;
  }

  public function verify($context) {
    // don't verify input keys or output keys, because some might be missing but that is okay.
    return array();
  }

  public function prepareIngest(&$context) {
    // no need to do anything
  }

}
