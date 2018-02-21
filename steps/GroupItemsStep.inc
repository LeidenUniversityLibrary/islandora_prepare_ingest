<?php

/**
 * @file
 * steps/GroupItemsStep.inc
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

class GroupItemsStep extends PrepareIngestStep {
  
  public function label() {
    $title = t('Group items');
    if ($this->hasConfigKey('group_key_name')) {
      $title .= ' ' . t('by the value of key "@key"', array('@key' => $this->config['group_key_name']));
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'group_key_name' => array(
        'label' => t('Group key name'),
        'description' => t('Every item that has the same value for this key will be grouped to one item.'),
        'type' => 'key',
      ),
    );
  }

  public function _inputKeyKeys() {
    return array('group_key_name');
  }

  public function outputKeys() {
    return array();
  }

  public function changesCount() {
    return TRUE;
  }

  public function changesKeys() {
    return TRUE;
  }

  public function changesFiles() {
    return FALSE;
  }

  public function dryRun(&$context) {
    // TODO: do grouping in the sql query
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('group_key_name') && isset($prevdatastep)) {
      $groupkey = $this->config['group_key_name'];

      $grouparray = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $group = $item[$groupkey];
        if (array_key_exists($group, $grouparray)) {
          $newarray = array_merge($grouparray[$group], $item);
          $grouparray[$group] = $newarray;
        }
        else {
          $grouparray[$group] = $item;
        }

        $item = $prevdatastep->nextDataItem();
      }
      $itemnr = 1;
      foreach ($grouparray as $group => $item) {
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
