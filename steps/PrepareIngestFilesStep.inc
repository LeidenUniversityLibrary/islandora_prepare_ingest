<?php

/**
 * @file
 * steps/PrepareIngestFilesStep.inc
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

class PrepareIngestFilesStep extends PrepareIngestStep {

 /**
  * Returns a boolean indication if the step adds and/or changes files on the file system.
  **/
  public function changesFiles() {
    return TRUE;
  }

  protected function _previousFilesChangingStep() {
    $prevstep = $this->previousstep;
    while ($prevstep !== NULL && !$prevstep->changesFiles()) {
      $prevstep = $prevstep->previousstep;
    }
    return $prevstep;
  }

  protected function _previousStepFileEntries() {
    $fileentries = array();
    $prevfilestep = $this->_previousFilesChangingStep();
    if (isset($prevfilestep)) {
      $prevfilestep->resetDataIterator(2);
      $entry = $prevfilestep->nextDataItem();
      while ($entry !== NULL) {
        $fileentries[$entry['filepath']] = $entry;
        $entry = $prevfilestep->nextDataItem();
      }
    }
    return $fileentries;
  }

  protected function _saveFileEntries($fileentries) {
    ksort($fileentries, SORT_STRING);
    $currentitemnr = 1;
    foreach ($fileentries as $fileentry) {
      $fileentry['item nr'] = $currentitemnr;
      $this->saveFileEntry($fileentry);
      $currentitemnr++;
    }
  }

  protected function _prepareIngestFilesUsingFunction($funcname) {
    $this->resetDataIterator(2);
    $item = $this->nextDataItem();
    while ($item !== NULL) {
      $funcname($item);
      $item = $this->nextDataItem();
    }
  }

  protected function saveFileEntry($dataitem) {
    if (!isset($dataitem['item nr'])) {
      throw new UnexpectedValueException("Data item is missing an item nr key.");
    }
    $itemnr = $dataitem['item nr'];
    islandora_prepare_ingest_data_cache_single_insert($this->workflowid, $this->stepid, $this->otherid, 2, $itemnr, $dataitem);
  }

}
