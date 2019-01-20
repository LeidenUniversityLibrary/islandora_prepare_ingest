<?php

/**
 * @file
 * steps/WriteCsvFileStep.inc
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
  
class WriteCsvFileStep extends PrepareIngestFilesStep {

  public function label() {
    $title = t('Write to CSV file');
    if ($this->hasConfigKey('target_filepath')) {
      $title .= ' ' . t('at "@fp"', array('@fp' => $this->config['target_filepath']));
    }
    if ($this->hasConfigKey('header')) {
      $title .= ' ' . t('with header "@header"', array('@header' => $this->config['header']));
      if ($this->hasConfigKey('content')) {
        $title .= ' ' . t('and');
      }
    }
    if ($this->hasConfigKey('content')) {
      if (!$this->hasConfigKey('header')) {
        $title .= ' ' . t('with');
      }
      $title .= ' ' . t('content "@content"', array('@content' => $this->config['content']));
    }
    if ($this->hasConfigKey('delimiter')) {
      $delimiter = $this->config['delimiter'];
      if ($delimiter === "\t") {
        $delimiter = 'tab';
      }
      else {
        $delimiter = "\"$delimiter\"";
      }
      $title .= ' ' . t('delimited by @delimiter', array('@delimiter' => $delimiter));
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
      'target_filepath' => array(
        'label' => t('Target filepath'),
        'description' => t('This should hold an absolute path to a not existing file.'),
        'type' => 'templatestring',
      ),
      'header' => array(
        'label' => t('Header'),
        'description' => t('The names of the columns, seperated by semicolons. Leave empty when no header is needed.'),
        'type' => 'string',
        'allow_constants' => TRUE,
        'optional' => TRUE,
      ),
      'content' => array(
        'label' => t('Content'),
        'description' => t('Template of the values of the columns, seperated by semicolons.'),
        'type' => 'template',
      ),
      'delimiter' => array(
        'label' => t('Delimiter'),
        'type' => 'select',
        'options' => array(
          '' => t('; (semicolon)'),
          ',' => t(', (comma)'),
          "\t" => t('tab'),
        ),
        'optional' => true,
      ),
      'target_exists_action' => array(
        'label' => t('If target exists'),
        'type' => 'select',
        'options' => array(
          'append' => t('append content to target'),
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
    $result += $this->_getInputKeysFromTemplates(array('header'));
    $result += $this->_getInputKeysFromTemplates(array('content'));
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

  public function checkConfiguration($context) {
    $result = parent::checkConfiguration($context);

    if ($this->hasConfigKey('header') && $this->hasConfigKey('content')) {
      $header = str_getcsv($this->config['header'], ';');
      $contenttemplate = str_getcsv($this->config['content'], ';');
      if (count($header) > count($contenttemplate)) {
        $result[] = array('text' => t("Header contains more columns than are defined in the content."), 'type' => 'run_error');
      }
      if (count($header) < count($contenttemplate)) {
        $result[] = array('text' => t("Header contains less columns than are defined in the content."), 'type' => 'run_error');
      }
    }
    return $result;
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('content') && $this->hasConfigKey('target_filepath') && isset($prevdatastep)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $delimiter = $this->hasConfigKey('delimiter')?$this->config['delimiter']:';';
      $header = str_getcsv($this->config['header'], ';');
      $contenttemplate = str_getcsv($this->config['content'], ';');
      $targetfilepathtemplate = $this->config['target_filepath'];

      $filesused = array();
      $results = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $targetfilepath = $this->fillOutTemplate($targetfilepathtemplate, $item);
        $targetdirectory = dirname($targetfilepath);
        if (!pifs_is_dir($targetdirectory, $this->workflowid, $this->stepid, $this->otherid)) {
          $results[] = array('text' => t("Directory '@directory' for file '@file' should exist, but does not.",
                    array('@directory' => $targetdirectory, '@file' => $targetfilepath)), 'type' => 'run_error');
          return $results;
        }
        if (!isset($filesused[$targetfilepath])) {
          $needsheader = $this->hasConfigKey('header');
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
              elseif ($this->config['target_exists_action'] === 'append') {
                $needsheader = FALSE;
              }
            }
          }
          if ($needsheader) {
            $content = $this->csvstring($header, $delimiter); 
            pifs_entry_write_content($targetfilepath, $content, $this->workflowid, $this->stepid, $this->otherid, FALSE);
          }
        }
        $filesused[$targetfilepath] = TRUE;
        $contentarray = array();
        foreach ($contenttemplate as $ctemplate) {
          $contentarray[] = $this->fillOutTemplate($ctemplate, $item);
        }
        $content = $this->csvstring($contentarray, $delimiter);
        pifs_entry_write_content($targetfilepath, $content, $this->workflowid, $this->stepid, $this->otherid, TRUE);
        $item = $prevdatastep->nextDataItem();
      }
    }
    return FALSE;
  }

  private function csvstring($input, $delimiter = ',', $enclosure = '"') {
    // Code thanks to https://gist.github.com/johanmeiring/2894568
    // Open a memory "file" for read/write...
    $fp = fopen('php://temp', 'r+');
    // ... write the $input array to the "file" using fputcsv()...
    fputcsv($fp, $input, $delimiter, $enclosure);
    // ... rewind the "file" so we can read what we just wrote...
    rewind($fp);
    // ... read the entire line into a variable...
    $data = stream_get_contents($fp);
    // ... close the "file"...
    fclose($fp);
    // ... and return the $data to the caller
    return $data;
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
