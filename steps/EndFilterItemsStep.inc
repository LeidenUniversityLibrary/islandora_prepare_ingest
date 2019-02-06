<?php

/**
 * @file
 * steps/EndFilterItemsStep.inc
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

class EndFilterItemsStep extends PrepareIngestStep {
  
  public function label() {
    return t('Filter items end');
  }

  public function arguments() {
    return array(
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
    $startfilterdatastep = $this->_startFilterDataChangingStep();
    if ($startfilterdatastep === NULL) {
      $results[] = array('text' => t("Did not find an accompanying filter step for this end filter"), 'type' => 'check_error');
    }
    return $results;
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    // First search for the accompanying FilterItemStep:
    $startfilterdatastep = $this->_startFilterDataChangingStep();
    // If found, check if the filter is working and check if the items between the filter start and end have changed
    if (isset($startfilterdatastep) && $startfilterdatastep->hasConfigKey('filter_key_name') && $startfilterdatastep->hasConfigKey('filter_type') && isset($prevdatastep)) {
      $prevstartfilterdatastep = $startfilterdatastep->_previousDataChangingStep();
      $prevstartfilterdatastep->resetDataIterator();
      if ($startfilterdatastep === $prevdatastep) {
        // the filter only contains non-data-changing steps, so just use the data from before the filter
        $item = $prevstartfilterdatastep->nextDataItem(); 
        while ($item !== NULL) {
          $this->saveDataItem($item);
          $item = $prevstartfilterdatastep->nextDataItem(); 
        }
      }
      else {
        // the filter does contain data-changing steps, so do the filtering again on it's children but replace the filtered items with the items of the previous step: 
        $filterkey = $startfilterdatastep->config['filter_key_name'];
        $type = $startfilterdatastep->config['filter_type'];
        $value = $startfilterdatastep->config['filter_value'];
        $negate = $startfilterdatastep->config['negate'];
  
        $prevdatastep->resetDataIterator();
        $currentitemnr = 1;
        $item = $prevstartfilterdatastep->nextDataItem();
        $newitem = $prevdatastep->nextDataItem();
        while ($item !== NULL) {
          $shouldkeep = $this->_shouldKeep($item, $type, $filterkey, $value, $negate);
          if ($shouldkeep) {
            // this item was filtered, so replace these filtered items up to a non-filtered item.
            // first find the last filtered item of this part
            $lastfiltereditem = $item;
            $item = $prevstartfilterdatastep->nextDataItem();
            while ($item != NULL && $this->_shouldKeep($item, $type, $filterkey, $value, $negate)) {
              $lastfiltereditem = $item;
              $item = $prevstartfilterdatastep->nextDataItem();
            }
            // replace the items that should be kept (when the filter started) with the items from the end of the filter.
            while ($newitem !== NULL) {
              $newitem['item nr'] = $currentitemnr;
              $this->saveDataItem($newitem);
              $currentitemnr++;
              $newitem = $prevdatastep->nextDataItem();
              if ($newitem['_UID_'] === $lastfiltereditem['_UID_']) {
                break;
              }
            }
          }
          else {
            $item['item nr'] = $currentitemnr;
            $this->saveDataItem($item);
            $currentitemnr++;
            $item = $prevstartfilterdatastep->nextDataItem();
          }
        }
        while ($newitem !== NULL) {
          $newitem['item nr'] = $currentitemnr;
          $this->saveDataItem($newitem);
          $currentitemnr++;
          $newitem = $prevdatastep->nextDataItem();
        }
      }
      $prevstartfilterdatastep->clearDataIterator();
      $prevdatastep->clearDataIterator();
    }
    return FALSE;
  }

  private function _shouldKeep($item, $type, $filterkey, $value, $negate) {
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
    if ($negate) {
      $shouldkeep = !$shouldkeep;
    }
    return $shouldkeep;
  }

  private function _startFilterDataChangingStep() {
    $startfilterdatastep = $this->_previousDataChangingStep();
    $depth = 1;
    while ($startfilterdatastep !== NULL) {
      if (is_a($startfilterdatastep, 'EndFilterItemsStep')) {
        $depth++;
      }
      elseif (is_a($startfilterdatastep, 'FilterItemsStep')) {
        $depth--;
      }
      if ($depth === 0) {
        break;
      }
      $startfilterdatastep = $startfilterdatastep->_previousDataChangingStep();
    }
    return $startfilterdatastep;
  }

  public function verify($context) {
    // don't verify input keys or output keys, because some might be missing but that is okay.
    return array();
  }

  public function prepareIngest(&$context) {
    // no need to do anything
  }

}
