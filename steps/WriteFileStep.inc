<?php

/**
 * @file
 * steps/WriteFileStep.inc
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
  
class WriteFileStep extends PrepareIngestFilesStep {

  public function label() {
    $title = t('Write');
    if ($this->hasConfigKey('content_key_name')) {
      $title .= ' ' . t('the content of key "@key"', array('@key' => $this->config['content_key_name']));
    }
    $title .= ' ' . t('to file');
    if ($this->hasConfigKey('target_filepath')) {
      $title .= ' ' . t('located at "@fp"', array('@fp' => $this->config['target_filepath']));
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'content_key_name' => array(
        'label' => t('Content key name'),
        'description' => t('This key should hold the content for the file.'),
        'type' => 'key',
      ),
      'target_filepath' => array(
        'label' => t('Target filepath'),
        'description' => t('This should hold an absolute path to a not existing file.'),
        'type' => 'templatestring',
      ),
    );
  }

  public function inputKeys() {
    $result = $this->_getInputKeysFromTemplates(array('target_filepath'));
    if ($this->hasConfigKey('content_key_name')) {
      $result['content_key_name'] = array($this->config['content_key_name']);
    }
    return $result;
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

  public function checkConfiguration() {
    return parent::checkConfiguration();
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('content_key_name') && $this->hasConfigKey('target_filepath') && isset($prevdatastep)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $contentkey = $this->config['content_key_name'];
      $targetfilepathtemplate = $this->config['target_filepath'];
  
      $fileentries = $this->_previousStepFileEntries();

      $results = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $content = $item[$contentkey];
        $targetfilepath = $this->fillOutTemplate($targetfilepathtemplate, $item);
        $targetdirectory = dirname($targetfilepath);
        if (!pifs_is_dir($targetdirectory, $this->workflowid, $this->otherid)) {
          $results[] = array('text' => t("Directory '@directory' for file '@file' should exist, but does not.",
                    array('@directory' => $targetdirectory, '@file' => $targetfilepath)), 'type' => 'run_error');
        }
        if (count($results) > 0) {
          return $results;
        }
        $isappended = FALSE;
        if (pifs_entry_exists($targetfilepath, $this->workflowid, $this->otherid)) {
          if (pifs_entry_is_file($targetfilepath, $this->workflowid, $this->otherid)) {
            $entry = pifs_entry_append_content($targetfilepath, $content, $this->workflowid, $this->stepid, $this->otherid, $item['item nr']);
            $isappended = TRUE;
          }
          else {
            $results[] = array('text' => t("Cannot write file: a directory '@directory' already exists at that location.",
                      array('@directory' => $targetfilepath)), 'type' => 'run_error');
            return $results;
          }
        }
        else {
          $entry = pifs_entry_write_content($targetfilepath, $content, $this->workflowid, $this->stepid, $this->otherid, $item['item nr']);
        }
        if (isset($fileentries[$entry['filepath']]) && !$isappended) {
          $fileentries[$entry['filepath']]['content'] .= $entry['content']; 
        }
        else {
          $fileentries[$entry['filepath']] = $entry;
        }

        $this->saveDataItem($item);

        $item = $prevdatastep->nextDataItem();
      }
      $this->_saveFileEntries($fileentries);
    }
    return FALSE;
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    $this->_prepareIngestFilesUsingFunction('pifs_write_content_to_file');
  }
}
