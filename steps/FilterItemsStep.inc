<?php

/**
 * @file
 * steps/FilterItemsStep.inc
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

class FilterItemsStep extends PrepareIngestStep implements PrepareIngestFilteringInterface {
  
  public function label() {
    $title = t('Filter items');
    if ($this->hasConfigKey('filter_key_name')) {
      $title .= ' ' . t('if the value of key "@key"', array('@key' => $this->config['filter_key_name']));
    }
    if ($this->hasConfigKey('filter_type')) {
      if ($this->config['filter_type'] === 'exists') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('does not exist');
        }
        else {
          $title .= ' ' . t('exists');
        }
      }
      elseif ($this->config['filter_type'] === 'equals') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('does not equal');
        }
        else {
          $title .= ' ' . t('equals');
        }
        if ($this->hasConfigKey('filter_value')) {
          $title .= ' ' . '"' . $this->config['filter_value'] . '"';
        }
      }
      elseif ($this->config['filter_type'] === 'matches') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('does not match');
        }
        else {
          $title .= ' ' . t('matches');
        }
        if ($this->hasConfigKey('filter_value')) {
          $title .= ' ' . $this->config['filter_value'];
        }
      }
      elseif ($this->config['filter_type'] === 'item_nrs') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('does not exist or does not contain the item nrs');
        }
        else {
          $title .= ' ' . t('exists and item contains the item nrs');
        }
        if ($this->hasConfigKey('filter_value')) {
          $title .= ' ' . $this->config['filter_value'];
        }
      }
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'filter_key_name' => array(
        'label' => t('Filter key name'),
        'description' => t('The key that should be used to filter.'),
        'type' => 'key',
      ),
      'filter_type' => array(
        'label' => t('Filter type'),
        'description' => t('Choose how you want to filter: "key exists" filters if the key has any value, "key exists and item contains item nrs" filters if key has any value and the item nr is in the filter value, "equals" filters if the value of the filter key name is exactly the same as the filter value, "matches" filters if the value of the filter key name matches the regular expression in the filter value.'),
        'type' => 'select',
        'options' => array(
          'exists' => t('key exists'),
          'item_nrs' => t('key exists and item contains item nrs'),
          'equals' => t('equals'),
          'matches' => t('matches'),
        ),
      ),
      'filter_value' => array(
        'label' => t('Filter value'),
        'description' => t('The value to filter by.'),
        'type' => 'string',
        'allow_constants' => TRUE,
        'optional' => array('filter_type' => array('exists')),
      ),
      'negate' => array(
        'label' => t('Negate'),
        'description' => t('If false, values of the filter_key_name key that are equal to filter_value, are included. If true, values of the filter_key_name key that are not equal to filter_value, are included.'),
        'type' => 'boolean',
      ),
    );
  }

  public function _inputKeyKeys() {
    return array('filter_key_name');
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

  public function checkConfiguration($context) {
    $results = parent::checkConfiguration($context);
    if ($this->hasConfigKey('filter_type')) {
      if ($this->config['filter_type'] === 'exists') {
        // no check needed, filter_value does not matter.
        unset($results['filter_value']);
      }
      elseif ($this->config['filter_type'] === 'equals') {
        if (!$this->hasConfigKey('filter_value')) {
          $results[] = array('text' => t("Filter with type 'equals' should have a value."), 'type' => 'argument_error');
        }
      }
      elseif ($this->config['filter_type'] === 'matches') {
        if (!($this->hasConfigKey('filter_value') && (@preg_match($this->config['filter_value'], "XXX") !== FALSE))) {
          $results[] = array('text' => t("Filter with type 'matches' should have a regular expression, but has '@value'.",
                             array('@value' => $this->config['filter_value'])), 'type' => 'argument_error');
        }
      }
      elseif ($this->config['filter_type'] === 'item_nrs') {
        if (!$this->hasConfigKey('filter_value') || !preg_match('/^[0-9,-]+$/', $this->config['filter_value'])) {
          $results[] = array('text' => t("Filter with type 'contains item number' should have a value containing item numbers, or a range, but has '@value'. Valid values are for example: '1-6', '1,3,5' or '-10,20-'",
                             array('@value' => $this->config['filter_value'])), 'type' => 'argument_error');
        }
      }
    }
    return $results;
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('filter_key_name') && $this->hasConfigKey('filter_type') && isset($prevdatastep)) {
      $filterconfig = $this->filterConfig();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      $currentitemnr = 1;
      while ($item !== NULL) {
        $shouldkeep = $this->shouldKeep($item, $filterconfig);
        if ($shouldkeep) {
          $item['item nr'] = $currentitemnr;
          $this->saveDataItem($item);
          $currentitemnr++;
        }
        $item = $prevdatastep->nextDataItem();
      }
      $prevdatastep->clearDataIterator();
    }
    return FALSE;
  }

  public function filterConfig() {
    $filterconfig = $this->config;
    $type = $this->config['filter_type'];
    $value = $this->config['filter_value'];
    if ($type === 'item_nrs') {
      $ranges = array();
      $parts = explode(',', $value);
      foreach ($parts as $part) {
        $p = explode('-', $part);
        if (count($p) === 1) {
          if (is_numeric($p[0])) {
            $m = intval($p[0]);
            array_push($ranges, array('min' => $m, 'max' => $m));
          }
        }
        elseif (count($p) === 2) {
          $min = is_numeric($p[0])?intval($p[0]):1;
          $max = is_numeric($p[1])?intval($p[1]):PHP_INT_MAX;
          array_push($ranges, array('min' => $min, 'max' => $max));
        }
      }
      $filterconfig['ranges'] = $ranges;
    }
    return $filterconfig;
  }

  public function shouldKeep($item, $filterconfig) {
    $filterkey = $filterconfig['filter_key_name'];
    $type = $filterconfig['filter_type'];
    $value = $filterconfig['filter_value'];
    $negate = $filterconfig['negate'];

    $shouldkeep = FALSE;
    if ($type === 'exists') {
      $shouldkeep = isset($item[$filterkey]);
    }
    elseif ($type === 'equals') {
      $shouldkeep = (isset($item[$filterkey]) && ($item[$filterkey] === $value));
    }
    elseif ($type === 'matches') {
      $shouldkeep = (isset($item[$filterkey]) && preg_match($value, $item[$filterkey]));
    }
    elseif ($type === 'item_nrs') {
      if (isset($item[$filterkey])) {
        $ranges = $filterconfig['ranges'];
        foreach ($ranges as $range) {
          if ($item['item nr'] >= $range['min'] && $item['item nr'] <= $range['max']) {
            $shouldkeep = TRUE;
            break;
          }
        }
      }
    }
    if ($negate) {
      $shouldkeep = !$shouldkeep;
    }
    return $shouldkeep;
  }

  public function verify($context) {
    // don't verify input keys or output keys, because some might be missing but that is okay.
    return array();
  }

  public function prepareIngest(&$context) {
    // no need to do anything
  }

}
