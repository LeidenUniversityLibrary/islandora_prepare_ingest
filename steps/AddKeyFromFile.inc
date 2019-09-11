<?php

/**
 * @file
 * steps/AddKeyFromFile.inc
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


class AddKeyFromFile extends PrepareIngestStep {
  
  public function label() {
    $title = t('Add key');
    if ($this->hasConfigKey('target_key_name')) {
      $title .= ' ' . $this->config['target_key_name'];
    }
    $title .= ' ' . t('from file');
    if ($this->hasConfigKey('filepath')) {
      $title .= ' ' . t('located at "@fp"', array('@fp' => $this->config['filepath']));
    }
    $title .= '.';
    return $title;
  }

  public function arguments() {
    return array(
      'target_key_name' => array(
        'label' => t('Target key name'),
        'type' => 'key',
      ),
      'filepath' => array(
        'label' => t('Filepath'),
        'description' => t('This should hold an absolute path to an existing file.'),
        'type' => 'templatestring',
      ),
    );
  }

  public function inputKeys() {
    return $this->_getInputKeysFromTemplates(array('filepath'));
  }

  public function _outputKeyKeys() {
    return array('target_key_name');
  }

  public function changesCount() {
    return FALSE;
  }

  public function changesKeys() {
    return TRUE;
  }

  public function changesFiles() {
    return FALSE;
  }

  public function dryRun(&$context) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('filepath') && $this->hasConfigKey('target_key_name') && isset($prevdatastep)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

      $filepathtemplate = $this->config['filepath'];
      $targetkey = $this->config['target_key_name'];

      $results = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $filepath = $this->fillOutTemplate($filepathtemplate, $item);
        if (!pifs_is_file($filepath, $this->workflowid, $this->stepid, $this->otherid)) {
          $results[] = array('text' => t("File '@file' does not exist.",
                  array('@file' => $filepath)), 'type' => 'run_error');
        }
        $content = pifs_entry_read_content($filepath, $this->workflowid, $this->stepid, $this->otherid);

        $item[$targetkey] = $content;
        $this->saveDataItem($item);

        $item = $prevdatastep->nextDataItem();
      }
      if (count($results) > 0) {
        return $results;
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
