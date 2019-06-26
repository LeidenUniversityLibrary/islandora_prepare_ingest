<?php

/**
 * @file
 * steps/ExtractArchiveStep.inc
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

class ExtractArchiveStep extends PrepareIngestFilesStep {

  public function label() {
    $title = t('Extract archive');
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
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'source_filepath' => array(
        'label' => t('Source filepath'),
        'description' => t('This should hold an absolute path to an existing archive, in Zip, tar, bzip2, LZF, Rar or ZLib format.'),
        'type' => 'templatestring',
      ),
      'source_skip_missing' => array(
        'label' => t('Source missing'),
        'description' => t('What to do if source archive is missing.'),
        'type' => 'select',
        'options' => array(
          '' => t('fail with message'),
          '1' => t('skip missing source archives'),
          'skip_item' => t('skip item in all next steps'),
        ),
        'optional' => true,
      ),
      'target_filepath' => array(
        'label' => t('Target filepath'),
        'description' => t('This should hold an absolute path to an existing directory in which the archive is extracted.'),
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
    if ($this->hasConfigKey('source_filepath') && $this->hasConfigKey('target_filepath')) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $sourcetemplate = $this->config['source_filepath'];
      $targettemplate = $this->config['target_filepath'];
      $skipmissing = $this->config['source_skip_missing'];

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
        if (!pifs_is_file($sourcefilepath, $this->workflowid, $this->stepid, $this->otherid)) {
          if ($skipmissing === '1') {
            $item = isset($prevdatastep)?$prevdatastep->nextDataItem():NULL;
            continue;
          }
          elseif ($skipmissing === 'skip_item') {
            if (isset($item['item nr']) && isset($prevdatastep)) {
              $prevdatastep->skipDataItemInNextSteps($item);
            }
            $item = isset($prevdatastep)?$prevdatastep->nextDataItem():NULL;
            continue;
          }
          else {
            $results[] = array('text' => t("Archive '@file' does not exist.",
                      array('@file' => $sourcefilepath)), 'type' => 'run_error');
          }
        }
        $targetfilepath = $this->fillOutTemplate($targettemplate, $item);
        if (!pifs_is_dir($targetfilepath, $this->workflowid, $this->stepid, $this->otherid)) {
          $results[] = array('text' => t("Directory '@directory' does not exist.",
                    array('@directory' => $targetfilepath)), 'type' => 'run_error');
        }
        if (count($results) > 0) {
          return $results;
        }
        $result = pifs_archive_extract_make_entries($sourcefilepath, $targetfilepath, $this->workflowid, $this->stepid, $this->otherid);
        if ($result === FALSE) {
          $results[] = array('text' => t("Cannot extract archive '@archive'.",
                    array('@archive' => $sourcefilepath)), 'type' => 'run_error');
          return $results;
        }

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
