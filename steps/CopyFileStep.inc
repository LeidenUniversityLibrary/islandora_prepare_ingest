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
    if ($this->hasConfigKey('action')) {
      switch ($this->config['action']) {
        case '':
          $title = t('Copy file');
          break;
        case 'copy_dir':
          $title = t('Copy directory');
          break;
        case 'move_file':
          $title = t('Move file');
          break;
        case 'move_dir':
          $title = t('Move directory');
          break;
      }
    }
    if ($this->hasConfigKey('source_filepath')) {
      $title .= ' ' . t('from "@fp"', array('@fp' => $this->config['source_filepath']));
    }
    if ($this->hasConfigKey('target_filepath')) {
      $title .= ' ' . t('to "@fp"', array('@fp' => $this->config['target_filepath']));
    }
    if ($this->hasConfigKey('source_skip_missing')) {
      if ($this->config['source_skip_missing'] === '1') {
        $title .= ' ' . t('and skip missing source files');
      }
      elseif ($this->config['source_skip_missing'] === 'skip_item') {
        $title .= ' ' . t('and skip items with missing source files in next steps');
      }
    }
    if ($this->hasConfigKey('target_exists_action')) {
      if ($this->config['target_exists_action'] === 'keep_old') {
        $title .= ' ' . t('but leave target if it exists');
      }
      else if ($this->config['target_exists_action'] === 'keep_new') {
        $title .= ' ' . t('but overwrite target if it exists');
      }
      else if ($this->config['target_exists_action'] === 'skip_item') {
        $title .= ' ' . t('and skip items in next steps if target exists');
      }
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'action' => array(
        'label' => t('Copy or move'),
        'description' => t('Copy or move a file or a directory.'),
        'type' => 'select',
        'options' => array(
          '' => t('Copy file'),
          'copy_dir' => t('Copy directory'),
          'move_file' => t('Move file'),
          'move_dir' => t('Move directory'),
        ),
        'optional' => true,
      ),
      'source_filepath' => array(
        'label' => t('Source filepath'),
        'description' => t('This should hold an absolute path to an existing file.'),
        'type' => 'templatestring',
      ),
      'source_skip_missing' => array(
        'label' => t('Source missing'),
        'description' => t('What to do if source file is missing.'),
        'type' => 'select',
        'options' => array(
          '' => t('fail with message'),
          '1' => t('skip missing source files'),
          'skip_item' => t('skip item in all next steps'),
        ),
        'optional' => true,
      ),
      'target_filepath' => array(
        'label' => t('Target filepath'),
        'description' => t('This should hold an absolute path to an existing directory or the absolute path including the new name of the file to copy.'),
        'type' => 'templatestring',
      ),
      'target_exists_action' => array(
        'label' => t('Target exists'),
        'type' => 'select',
        'options' => array(
          '' => t('fail with message'),
          'keep_old' => t('leave the target unchanged'),
          'keep_new' => t('replace the target with the source file'),
          'skip_item' => t('skip item in all next steps'),
        ),
        'optional' => true,
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
    if ($this->hasConfigKey('source_filepath') && $this->hasConfigKey('target_filepath')) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $sourcetemplate = $this->config['source_filepath'];
      $targettemplate = $this->config['target_filepath'];
      $skipmissing = $this->config['source_skip_missing'];
      $action = isset($this->config['action'])?$this->config['action']:'';

      $results = array();
      if (isset($prevdatastep)) {
        $prevdatastep->resetDataIterator();
        $item = $prevdatastep->nextDataItem();
      }
      else {
        // Do the loop at least once, so this step can be used as the first step of a workflow.
        $item = array('_dummy' => 'dummy item');
      }
      while ($item !== NULL) {
        $sourcefilepath = $this->fillOutTemplate($sourcetemplate, $item);
        $sourceisdir = pifs_is_dir($sourcefilepath, $this->workflowid, $this->stepid, $this->otherid);
        $sourceisfile = pifs_is_file($sourcefilepath, $this->workflowid, $this->stepid, $this->otherid);
        if ((($action === '' || $action === 'move_file') && !$sourceisfile)
            ||
            (($action === 'copy_dir' || $action === 'move_dir') && !$sourceisdir)) {
          if ($skipmissing === '1') {
            $item = isset($prevdatastep)?$prevdatastep->nextDataItem():NULL;
            continue;
          }
          elseif ($skipmissing === 'skip_item') {
            if (isset($prevdatastep)) {
              $prevdatastep->skipDataItemInNextSteps($item);
              $item = $prevdatastep->nextDataItem();
            }
            else {
              $item = NULL;
            }
            continue;
          }
          else {
            $type = ($action === '' || $action === 'move_file')?t('File'):t('Directory');
            $results[] = array('text' => t("@type '@file' does not exist.",
                      array('@type' => $type, '@file' => $sourcefilepath)), 'type' => 'run_error');
          }
        }
        $targetfilepath = $this->fillOutTemplate($targettemplate, $item);
        if ((substr($targetfilepath, -1) === DIRECTORY_SEPARATOR) || pifs_is_dir($targetfilepath, $this->workflowid, $this->stepid, $this->otherid)) {
          $targetfilepath = $targetfilepath . DIRECTORY_SEPARATOR . basename($sourcefilepath);
        }
        if ($action === 'copy_dir' || $action === 'move_dir') {
          if (pifs_is_file($targetfilepath, $this->workflowid, $this->stepid, $this->otherid)) {
            $results[] = array('text' => t("Target '@file' is a file but should be a directory.",
                      array('@type' => $type, '@file' => $sourcefilepath)), 'type' => 'run_error');
          }
        }
        $targetisfile = pifs_is_file($targetfilepath, $this->workflowid, $this->stepid, $this->otherid);
        $targetisdir = pifs_is_dir($targetfilepath, $this->workflowid, $this->stepid, $this->otherid);
        if ((($action === '' || $action === 'move_file') && $targetisfile)
            ||
            (($action === 'copy_dir' || $action === 'move_dir') && $targetisdir)) {
          $report_error = true;
          if ($this->hasConfigKey('target_exists_action')) {
            if ($this->config['target_exists_action'] === 'keep_old') {
              $item = isset($prevdatastep)?$prevdatastep->nextDataItem():NULL;
              continue;
            }
            elseif ($this->config['target_exists_action'] === 'skip_item') {
              if (isset($prevdatastep)) {
                $prevdatastep->skipDataItemInNextSteps($item);
                $item = $prevdatastep->nextDataItem();
              }
              else {
                $item = NULL;
              }
              continue;
            }
            elseif ($this->config['target_exists_action'] === 'keep_new') {
              $report_error = false;
            }
          }
          if ($report_error)  {
            $results[] = array('text' => t("File '@file' already exists.",
                      array('@file' => $targetfilepath)), 'type' => 'run_error');
          }
        }
        $targetdirectory = dirname($targetfilepath);
        if (!pifs_is_dir($targetdirectory, $this->workflowid, $this->stepid, $this->otherid)) {
          $results[] = array('text' => t("Directory '@directory' for '@file' should exist, but does not.",
                    array('@directory' => $targetdirectory, '@file' => $targetfilepath)), 'type' => 'run_error');
        }
        if (count($results) > 0) {
          return $results;
        }
        if ($action === '' || $action === 'move_file') {
          pifs_entry_copy($sourcefilepath, $targetfilepath, $this->workflowid, $this->stepid, $this->otherid);
        }
        else if ($action === 'copy_dir' || $action === 'move_dir') {
          pifs_entry_recursive_copy($sourcefilepath, $targetfilepath, $this->workflowid, $this->stepid, $this->otherid);
        }
        if ($action === 'move_file') {
          pifs_entry_remove($sourcefilepath, $this->workflowid, $this->stepid, $this->otherid);
        }
        else if ($action === 'move_dir') {
          pifs_entry_recursive_remove($sourcefilepath, $this->workflowid, $this->stepid, $this->otherid);
        }

        //$this->saveDataItem($item);

        $item = isset($prevdatastep)?$prevdatastep->nextDataItem():NULL;
      }
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
