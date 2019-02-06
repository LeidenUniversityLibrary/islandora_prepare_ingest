<?php

/**
 * @file
 * steps/SortItemsStep.inc
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

class SortItemsStep extends PrepareIngestStep {
  
  public function label() {
    $title = t('Sort items');
    if ($this->hasConfigKey('order_type_1')) {
      if (substr($this->config['order_type_1'], -3) === 'rev') {
        $title .= ' ' . t('reversed');
      }
    }
    if ($this->hasConfigKey('order_key_1')) {
      $title .= ' ' . t('by the value of key "@key"', array('@key' => $this->config['order_key_1']));
    }
    $title .= $this->_label_key_as_part('order_type_1');
    if ($this->hasConfigKey('order_key_2')) {
      $title .= t(', then sort');
      if ($this->hasConfigKey('order_type_2')) {
        if (substr($this->config['order_type_2'], -3) === 'rev') {
          $title .= ' ' . t('reversed');
        }
      }
      $title .= ' ' . t('by the value of key "@key"', array('@key' => $this->config['order_key_2']));
      $title .= $this->_label_key_as_part('order_type_2');
    }
    $title .= '.';

    return $title;
  }
 
  private function _label_key_as_part($key) {
    $part = '';
 
    if ($this->hasConfigKey($key)) {
      switch ($this->config[$key]) {
        case 'strcmp':
        case 'strcmprev':
          $part .= ' ' . t('as text');
          break;
        case 'strcasecmp':
        case 'strcasecmprev':
          $part .= ' ' . t('as text case-insensitive');
          break;
        case 'intcmp':
        case 'intcmprev':
          $part .= ' ' . t('as number');
          break;
      }
    }
    return $part;
  }

  public function arguments() {
    $sortoptions = array(
      'strcmp' => t('string compare'),
      'strcasecmp' => t('string compare case-insensitive'),
      'intcmp' => t('integer compare'),
      'strcmprev' => t('string compare reverse'),
      'strcasecmprev' => t('string compare case-insensitive reverse'),
      'intcmprev' => t('integer compare reverse'),
    );
    return array(
      'order_key_1' => array(
        'label' => t('Order key 1'),
        'description' => t('Order by the value of this key.'),
        'type' => 'key',
      ),
      'order_type_1' => array(
        'label' => t('Order type 1'),
        'description' => t('How to order the value of this key.'),
        'type' => 'select',
        'options' => $sortoptions,
      ),
      'order_key_2' => array(
        'label' => t('Order key 2'),
        'description' => t('Optional, order by the value of this key.'),
        'type' => 'key',
        'optional' => TRUE,
      ),
      'order_type_2' => array(
        'label' => t('Order type 2'),
        'description' => t('Optional, how to order the value of this key.'),
        'type' => 'select',
        'options' => $sortoptions,
        'optional' => TRUE,
      ),
    );
  }

  protected function _inputKeyKeys() {
    return array('order_key_1', 'order_key_2');
  }

  public function outputKeys() {
    return array();
  }

  public function changesCount() {
    return TRUE;
  }

  public function changesKeys() {
    return FALSE;
  }

  public function changesFiles() {
    return FALSE;
  }

  public function dryRun(&$context) {
    // TODO: do sorting in the sql query
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('order_key_1') && isset($prevdatastep)) {
      // First get all items.
      $allitems = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $allitems[] = $item;
        $item = $prevdatastep->nextDataItem();
      }
      $prevdatastep->clearDataIterator();

      // Then sort the items.
      $intcmp = function($a, $b) {
        $inta = intval($a);
        $intb = intval($b);
        return ($inta - $intb);
      };
      $key1 = $this->config['order_key_1'];
      $factor1 = 1;
      switch ($this->config['order_type_1']) {
        case 'strcmprev':
          $factor1 = -1;
          // leave out the break and fall through
        case 'strcmp':
          $cmp1 = 'strcmp';
          break;
        case 'strcasecmprev':
          $factor1 = -1;
          // leave out the break and fall through
        case 'strcasecmp':
          $cmp1 = 'strcasecmp';
          break;
        case 'intcmprev':
          $factor1 = -1;
          // leave out the break and fall through
        case 'intcmp':
          $cmp1 = $intcmp;
          break;
      }
      if ($this->hasConfigKey('order_key_2')) {
        $key2 = $this->config['order_key_2'];
        $factor2 = 1;
        switch ($this->config['order_type_2']) {
          case 'strcmprev':
            $factor2 = -1;
            // leave out the break and fall through
          case 'strcmp':
            $cmp2 = 'strcmp';
            break;
          case 'strcasecmprev':
            $factor2 = -1;
            // leave out the break and fall through
          case 'strcasecmp':
            $cmp2 = 'strcasecmp';
            break;
          case 'intcmprev':
            $factor2 = -1;
            // leave out the break and fall through
          case 'intcmp':
            $cmp2 = $intcmp;
            break;
        }
        usort($allitems, function($a, $b) use ($cmp1, $key1, $factor1, $cmp2, $key2, $factor2) {
          $result1 = ($factor1 * $cmp1($a[$key1], $b[$key1]));
          if ($result1 === 0) {
            return ($factor2 * $cmp2($a[$key2], $b[$key2]));
          }
          return $result1;
        });
      }
      else {
        usort($allitems, function($a, $b) use ($cmp1, $key1, $factor1) {
          return ($factor1 * $cmp1($a[$key1], $b[$key1]));
        });
      }

      // And save the items.
      $itemnr = 1;
      foreach ($allitems as $item) {
        $item['item nr'] = $itemnr;
        $this->saveDataItem($item);
        $itemnr++;
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
