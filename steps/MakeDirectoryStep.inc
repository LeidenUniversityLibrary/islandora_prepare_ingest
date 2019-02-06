<?php

/**
 * @file
 * steps/MakeDirectoryStep.inc
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

class MakeDirectoryStep extends PrepareIngestFilesStep {
  
  public function label() {
    $title = t('Make directory');
    if ($this->hasConfigKey('directory_name')) {
      $title .= ' ' . t('named "@name"', array('@name' => $this->config['directory_name']));
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'directory_name' => array(
        'label' => t('Directory name'),
        'description' => t('The absolute path to a directory that will be created. The parent directory should exist already.'),
        'type' => 'templatestring',
      ),
    );
  }

  public function inputKeys() {
    return $this->_getInputKeysFromTemplates(array('directory_name'));
  }

  public function _outputKeyKeys() {
    return array();
  }

  public function changesCount() {
    return FALSE;
  }

  public function changesKeys() {
    return FALSE;
  }

  public function changesFiles() {
    return TRUE;
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('directory_name') && isset($prevdatastep)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $dirnametemplate = $this->config['directory_name'];
  
      $results = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $dirname = $this->fillOutTemplate($dirnametemplate, $item);
        $parentdir = dirname($dirname);
        if (pifs_file_exists($dirname, $this->workflowid, $this->stepid, $this->otherid) && !pifs_is_dir($dirname, $this->workflowid, $this->stepid, $this->otherid)) {
          $results[] = array('text' => t("'@directory' already exists but is not a directory.",
                    array('@directory' => $dirname)), 'type' => 'run_error');
        }
        if (!pifs_is_dir($parentdir, $this->workflowid, $this->stepid, $this->otherid)) {
          $results[] = array('text' => t("Parent directory '@parentdir' of directory '@dir' should exist, but does not.",
                    array('@parentdir' => $parentdir, '@dir' => $dirname)), 'type' => 'run_error');
        }
        if (count($results) > 0) {
          return $results;
        }
        pifs_entry_mkdir($dirname, $this->workflowid, $this->stepid, $this->otherid);

        //$this->saveDataItem($item);

        $item = $prevdatastep->nextDataItem();
      }
      $prevdatastep->clearDataIterator();
    }
    return FALSE;
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
    pifs_persist_entries($this->workflowid, $this->stepid, $this->otherid);
  }
}
