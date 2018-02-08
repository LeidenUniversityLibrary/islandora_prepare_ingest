<?php

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
