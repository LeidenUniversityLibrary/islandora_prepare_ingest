<?php

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

  public function checkConfiguration() {
    $results = parent::checkConfiguration();
    if ($results) {
      return $results;
    }
    if ($this->hasConfigKey('directory_path')) {
      $dirpath = $this->config['directory_path'];
      if (substr($dirpath, 0, 1) !== DIRECTORY_SEPARATOR) {
        $results[] = array('text' => t("Argument '@key' should be an absolute filepath, but is '@value'.",
                  array('@key' => $this->info['arguments']['directory_path']['label'], '@value' => $dirpath)), 'type' => 'argument_error');
      }
      if (!is_dir($dirpath)) {
        $results[] = array('text' => t("Argument '@key' should be an existing filepath, but is '@value'",
                  array('@key' => $this->info['arguments']['directory_path']['label'], '@value' => $dirpath)), 'type' => 'argument_error');
      }
    }
    if ($results) {
      return $results;
    }

    return FALSE;
  }

  public function dryRun(&$context) {
    if ($this->hasConfigKey('directory_path')) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $dirpath = $this->config['directory_path'];
      $filter = $this->config['filter'];
      $includedir = $this->config['include_subdirectory_files'];
      $filenames = pifs_full_filenames_from_directory($dirpath, $this->workflowid, $this->otherid, $filter, $includedir);
      $key = $this->config['target_key_name'];
      $items = array();
      foreach ($filenames as $filename) {
        $items[] = array($key => $filename);
      }
      islandora_prepare_ingest_data_cache_insert($this->workflowid, $this->stepid, $this->otherid, 1, $items);
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
