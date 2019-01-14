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
    if ($this->hasConfigKey('target_exists_action')) {
      if ($this->config['target_exists_action'] === 'error') {
        $title .= ', ' . t('fail if file exists already');
      }
      elseif ($this->config['target_exists_action'] === 'overwrite') {
        $title .= ', ' . t('overwrite if file exists already');
      }
      elseif ($this->config['target_exists_action'] === 'skip_item') {
        $title .= ', ' . t('skip items in next steps if file exists already');
      }
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
      'target_exists_action' => array(
        'label' => t('If target exists'),
        'type' => 'select',
        'options' => array(
          '' => t('append content to target'),
          'overwrite' => t('overwrite with content'),
          'error' => t('fail with message'),
          'skip_item' => t('skip item in all next steps'),
        ),
        'optional' => true,
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

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('content_key_name') && $this->hasConfigKey('target_filepath') && isset($prevdatastep)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $contentkey = $this->config['content_key_name'];
      $targetfilepathtemplate = $this->config['target_filepath'];

      $results = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $content = isset($item[$contentkey])?$item[$contentkey]:'';
        $targetfilepath = $this->fillOutTemplate($targetfilepathtemplate, $item);
        $targetdirectory = dirname($targetfilepath);
        if (!pifs_is_dir($targetdirectory, $this->workflowid, $this->stepid, $this->otherid)) {
          $results[] = array('text' => t("Directory '@directory' for file '@file' should exist, but does not.",
                    array('@directory' => $targetdirectory, '@file' => $targetfilepath)), 'type' => 'run_error');
          return $results;
        }
        if (pifs_file_exists($targetfilepath, $this->workflowid, $this->stepid, $this->otherid)) {
          if (pifs_is_dir($targetfilepath, $this->workflowid, $this->stepid, $this->otherid)) {
            $results[] = array('text' => t("Cannot write file: a directory '@directory' already exists at that location.",
                      array('@directory' => $targetfilepath)), 'type' => 'run_error');
            return $results;
          }
          if ($this->hasConfigKey('target_exists_action')) {
            if ($this->config['target_exists_action'] === 'error') {
              $results[] = array('text' => t("Cannot write to file '@file' because it already exists.",
                        array('@file' => $targetfilepath)), 'type' => 'run_error');
              return $results;
            }
            elseif ($this->config['target_exists_action'] === 'skip_item') {
              $prevdatastep->skipDataItemInNextSteps($item);
              $item = $prevdatastep->nextDataItem();
              continue;
            }
          }
        }
        $do_append = true;
        if ($this->hasConfigKey('target_exists_action')) {
          if ($this->config['target_exists_action'] === 'overwrite') {
            $do_append = false;
          }
        }
        pifs_entry_write_content($targetfilepath, $content, $this->workflowid, $this->stepid, $this->otherid, $do_append);

        $item = $prevdatastep->nextDataItem();
      }
    }
    return FALSE;
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

    $exists_action = 'append';
    if ($this->hasConfigKey('target_exists_action')) {
      if ($this->config['target_exists_action'] === 'error') {
        $exists_action = 'error';
      }
      else if ($this->config['target_exists_action'] === 'overwrite') {
        $exists_action = 'overwrite';
      }
    }
    pifs_persist_entries($this->workflowid, $this->stepid, $this->otherid, $exists_action);
  }
}
