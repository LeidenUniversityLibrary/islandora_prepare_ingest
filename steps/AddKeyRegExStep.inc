<?php

/**
 * @file
 * steps/AddKeyRegExStep.inc
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

class AddKeyRegExStep extends PrepareIngestStep {
  
  public function label() {
    $title = '';
    if ($this->hasConfigKey('target_key_name')) {
      $title .= ' ' . t('named "@key"', array('@key' => $this->config['target_key_name']));
    }
    if ($this->hasConfigKey('source_key_name')) {
      $title .= ' ' . t('with value from key "@key"', array('@key' => $this->config['source_key_name']));
    }
    if ($this->hasConfigKey('regexp')) {
      $title .= ' ' . t('by replacing "@regexp"', array('@regexp' => $this->config['regexp']));
    }
    if ($this->hasConfigKey('replacement')) {
      $title .= ' ' . t('with "@replacement"', array('@replacement' => $this->config['replacement']));
    }
  
    if (strlen($title) == 0) {
      $title = t('Add key with regular expression based value');
    }
    else {
      $title = t('Add key') . ' ' . $title;
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'source_key_name' => array(
        'label' => t('Source key name'),
        'type' => 'key',
      ),
      'target_key_name' => array(
        'label' => t('Target key name'),
        'type' => 'key',
      ),
      'regexp' => array(
        'label' => t('Regular expression'),
        'type' => 'regexp',
        'allow_constants' => TRUE,
      ),
      'replacement' => array(
        'label' => t('Replacement'),
        'type' => 'templatestring',
        'allow_constants' => TRUE,
      ),
    );
  }

  public function inputKeys() {
    $result = $this->_getInputKeysFromTemplates(array('replacement'));
    if ($this->hasConfigKey('source_key_name')) {
      $result['source_key_name'] = array($this->config['source_key_name']);
    }
    else {
      $result['source_key_name'] = array();
    }
    return $result;
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
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('target_key_name') && $this->hasConfigKey('source_key_name')
       && $this->hasConfigKey('regexp') && $this->hasConfigKey('replacement') && isset($prevdatastep)) {
      $targetkey = $this->config['target_key_name'];
      $sourcekey = $this->config['source_key_name'];
      $regexp = $this->config['regexp'];
  
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $replacement = $this->fillOutTemplate($this->config['replacement'], $item);
        $newvalue = preg_replace($regexp, $replacement, $item[$sourcekey]);
        $item[$targetkey] = $newvalue;
        $this->saveDataItem($item);
        $item = $prevdatastep->nextDataItem();
      }
    }
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    // no need to do anything
  }

}
