<?php

/**
 * @file
 * steps/CopyFileStep.inc
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

class CopyFileStep extends PrepareIngestFilesStep {
  
  public function label() {
    $title = t('Copy file');
    if ($this->hasConfigKey('source_filepath')) {
      $title .= ' ' . t('from "@fp"', array('@fp' => $this->config['source_filepath']));
    }
    if ($this->hasConfigKey('target_filepath')) {
      $title .= ' ' . t('to "@fp"', array('@fp' => $this->config['target_filepath']));
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
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
    );
  }
 
  public function inputKeys() {
    return $this->_getInputKeysFromTemplates(array('source_filepath', 'target_filepath'));
  }

  public function outputKeys() {
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
    if ($this->hasConfigKey('source_filepath') && $this->hasConfigKey('target_filepath') && isset($prevdatastep)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $sourcetemplate = $this->config['source_filepath'];
      $targettemplate = $this->config['target_filepath'];

      $results = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $sourcefilepath = $this->fillOutTemplate($sourcetemplate, $item);
        if (!pifs_is_file($sourcefilepath, $this->workflowid, $this->stepid, $this->otherid)) {
          $results[] = array('text' => t("File '@file' does not exist.",
                    array('@file' => $sourcefilepath)), 'type' => 'run_error');
        }
        $targetfilepath = $this->fillOutTemplate($targettemplate, $item);
        if ((substr($targetfilepath, -1) === DIRECTORY_SEPARATOR) || pifs_is_dir($targetfilepath, $this->workflowid, $this->stepid, $this->otherid)) {
          $targetfilepath = $targetfilepath . DIRECTORY_SEPARATOR . basename($sourcefilepath);
        }
        if (pifs_is_file($targetfilepath, $this->workflowid, $this->stepid, $this->otherid)) {
          $results[] = array('text' => t("File '@file' already exists.",
                    array('@file' => $targetfilepath)), 'type' => 'run_error');
        }
        $targetdirectory = dirname($targetfilepath);
        if (!pifs_is_dir($targetdirectory, $this->workflowid, $this->stepid, $this->otherid)) {
          $results[] = array('text' => t("Directory '@directory' for file '@file' should exist, but does not.",
                    array('@directory' => $targetdirectory, '@file' => $targetfilepath)), 'type' => 'run_error');
        }
        if (count($results) > 0) {
          return $results;
        }
        pifs_entry_copy($sourcefilepath, $targetfilepath, $this->workflowid, $this->stepid, $this->otherid);

        //$this->saveDataItem($item);

        $item = $prevdatastep->nextDataItem();
      }
    }
    return FALSE;
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    pifs_persist_entries($this->workflowid, $this->stepid, $this->otherid);
  }
}
