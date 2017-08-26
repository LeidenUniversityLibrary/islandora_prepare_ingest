<?php

class ReadFilenamesStep extends PrepareIngestStep {
  
  public function label() {
    $title = t('Add items by reading file names');
    if (array_key_exists('directory_path', $this->config) && strlen($this->config['directory_path']) > 0) {
      $title .= ' ' . t('from "@path"', array('@path' => $this->config['directory_path']));
    }
    if (array_key_exists('filter', $this->config) && strlen($this->config['filter']) > 0) {
      $title .= ' ' . t('filtered by "@filter"', array('@filter' => $this->config['filter']));
    }
    if (array_key_exists('target_key_name', $this->config) && strlen($this->config['target_key_name']) > 0) {
      $title .= ' ' . t('and insert item with key "@key"', array('@key' => $this->config['target_key_name']));
    }
    $title .= '.';

    return $title;
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

  public function dryRun() {
    if ($this->hasConfigKey('directory_path')) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $dirpath = $this->config['directory_path'];
      $filter = $this->config['filter'];
      $includedir = $this->config['include_subdirectory_files'];
      $filenames = pifs_full_filenames_from_directory($dirpath, $context, $filter, $includedir);
      $key = $this->config['target_key_name'];
      foreach ($filenames as $filename) {
        $items[] = array($key => $filename);
      }
      islandora_prepare_ingest_data_cache_insert($workflow_id, $step_id, $other_id, 2, $items);
    }
  }

  public function verify() {
    return parent::verify();
  }

  public function prepareIngest() {
    // no need to do anything
  }

}
