<?php

/**
 * @file
 * steps/ModifyKeyValueStep.inc
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

class ModifyKeyValueStep extends PrepareIngestStep {
  
  public function label() {
    $title = '';
  
    if ($this->hasConfigKey('type')) {
      $type = $this->config['type'];
  
      switch ($type) {
        case 'uppercase':
          $title = t('Uppercase');
          break;
        case 'lowercase':
          $title = t('Lowercase');
          break;
        case 'removehyphen':
          $title = t('Remove - from');
          break;
        case 'removeline':
          $title = t('Remove - and _ from');
          break;
        case 'removespaces':
          $title = t('Remove spaces from');
          break;
        case 'replacehyphen':
          $title = t('Replace - with space in');
          break;
        case 'replaceline':
          $title = t('Replace - and _ with space in');
          break;
        case 'striptags':
          $title = t('Strip tags from');
          break;
        case 'sha1':
          $title = t('SHA1 hash');
          break;
        case 'trim':
          $title = t('Trim spaces of');
          break;
        case 'pad5zeros':
          $title = t('Pad zero\'s to 5 digits');
          break;
        case 'xmlencode':
          $title = t('XML encode');
          break;
        case 'xmldecode':
          $title = t('XML decode');
          break;
      }
    }
    else {
      $title = t('Modify');
    }
    if ($this->hasConfigKey('source_key_name')) {
      $title .= ' ' . t('the value of key "@key"', array('@key' => $this->config['source_key_name']));
    }
    else {
      $title .= ' ' . t('the value of a key');
    }
    return $title;
  }

  public function arguments() {
    return array(
      'source_key_name' => array(
        'label' => t('Source key name'),
        'type' => 'key',
      ),
      'type' => array(
        'label' => t('Type'),
        'type' => 'select',
        'options' => array(
          '' => t('-'),
          'uppercase' => t('uppercase'),
          'lowercase' => t('lowercase'),
          'removehyphen' => t('remove -'),
          'removeline' => t('remove - and _'),
          'removespaces' => t('remove spaces'),
          'trim' => t('trim spaces'),
          'replacehyphen' => t('replace - with space'),
          'replaceline' => t('replace - and _ with space'),
          'pad5zeros' => t('pad zero\s to number to get 5 digits'),
          'striptags' => t('strip tags'),
          'sha1' => t('SHA1 hash'),
          'xmlencode' => t('XML encode'),
          'xmldecode' => t('XML decode'),
        ),
      ),
    );
  }

  public function _inputKeyKeys() {
    return array('source_key_name');
  }

  public function _outputKeyKeys() {
    return array('source_key_name');
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
    if ($this->hasConfigKey('source_key_name') && $this->hasConfigKey('type') && isset($prevdatastep)) {
      $sourcekey = $this->config['source_key_name'];
      $type = $this->config['type'];
      $methodname = "_modify_key_value_" . $type;

      if (method_exists($this, $methodname)) {
        $prevdatastep->resetDataIterator();
        $item = $prevdatastep->nextDataItem();
        while ($item !== NULL) {
          $newvalue = $this->$methodname($item[$sourcekey]);
          $item[$sourcekey] = $newvalue;
          $this->saveDataItem($item);
          $item = $prevdatastep->nextDataItem();
        }
      }
    }
    return FALSE;
  }

  private function _modify_key_value_uppercase($value) {
    return strtoupper($value);
  }

  private function _modify_key_value_lowercase($value) {
    return strtolower($value);
  }

  private function _modify_key_value_removehyphen($value) {
    return str_replace('-', '', $value);
  }

  private function _modify_key_value_removeline($value) {
    return str_replace(array('-', '_'), '', $value);
  }

  private function _modify_key_value_removespaces($value) {
    return str_replace(' ', '', $value);
  }

  private function _modify_key_value_replacehyphen($value) {
    return str_replace('-', ' ', $value);
  }

  private function _modify_key_value_replaceline($value) {
    return str_replace(array('-', '_'), ' ', $value);
  }

  private function _modify_key_value_striptags($value) {
    return strip_tags($value);
  }

  private function _modify_key_value_sha1($value) {
    return sha1($value);
  }

  private function _modify_key_value_trim($value) {
    return trim($value);
  }

  private function _modify_key_value_pad5zeros($value) {
    return sprintf("%05d", $value);
  }

  private function _modify_key_value_xmlencode($value) {
    return htmlspecialchars($value, ENT_XML1 | ENT_QUOTES, 'UTF-8');
  }

  private function _modify_key_value_xmldecode($value) {
    return htmlspecialchars_decode($value, ENT_XML1 | ENT_QUOTES);
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    // no need to do anything
  }

}
