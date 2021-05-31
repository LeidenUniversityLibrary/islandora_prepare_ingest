<?php

/**
 * @file
 * steps/ReadCsvFileStep.inc
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


class ReadCsvFileStep extends PrepareIngestStep {
  
  public function label() {
    $title = t('Add items by reading @type file', array('@type' => $this->readableFileString()));
    if ($this->hasConfigKey('csv_filepath')) {
      $title .= ' ' . t('located at "@value"', array('@value' => $this->config['csv_filepath']));
    }
    if ($this->hasConfigKey('column_keys') && !($this->hasConfigKey('has_header') && $this->config['has_header'] === 'use')) {
      $title .= ' ' . t('and place the values in the keys "@value"', array('@value' => $this->config['column_keys']));
    }
    if ($this->hasConfigKey('has_header')) {
      if ($this->config['has_header'] === '1') {
        $title .= ' ' . t('but skip header');
      }
      elseif ($this->config['has_header'] === 'use') {
        $title .= ' ' . t('and place the values using the header as keys');
      }
    }
    $title .= '.';

    return $title;
  }

  private function readableFileString() {
    if (class_exists('PHPExcel_IOFactory')) {
      return 'CSV/Excel';
    }
    else {
      return 'CSV';
    }
  }

  public function arguments() {
    return array(
      'csv_filepath' => array(
        'label' => t('@type filepath', array('@type' => $this->readableFileString())),
        'description' => t('Specify the absolute path to the @type file.', array('@type' => $this->readableFileString())),
        'type' => 'filepath',
        'allow_constants' => TRUE,
      ),
      'column_keys' => array(
        'label' => t('Column keys'),
        'description' => t('Define the output keys of the @type file, separated by semicolons.', array('@type' => $this->readableFileString())),
        'type' => 'keys',
      ),
      'has_header' => array(
        'label' => t('Has header'),
        'description' => t('Does the @type file have a header.', array('@type' => $this->readableFileString())),
        'type' => 'select',
        'options' => array(
          '' => t('Choose value'),
          0 => t('No header'),
          1 => t('Has header but skip it'),
          'use' => t('Has header and use it as output keys'),
        ),
      ),
    );
  }

  protected function _inputKeyKeys() {
    return array();
  }

  public function outputKeys() {
    if ($this->hasConfigKey('column_keys')) {
      return array('column_keys' => explode(';', $this->config['column_keys']));
    }
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

  public function checkConfiguration($context) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

    $result = parent::checkConfiguration($context);

    $csvfilepath = $this->config['csv_filepath'];
    $columnkeys = $this->config['column_keys'];
    if (!pifs_is_file($csvfilepath, $this->workflowid, $this->stepid, $this->otherid)) {
      $result[] = array('text' => t("File not found at '@fp'.",
                array('@fp' => $csvfilepath)), 'type' => 'run_error');
    }
    if (preg_match('/^[a-zA-Z0-9_-]+((?:;|[\r\n]+)[a-zA-Z0-9_-]+)*$/', $columnkeys) === 0) {
      $result[] = array('text' => t("@key should contain keynames (containing letters, numbers and/or dashes) separated by semicolons, but is '@keys'.",
                array('@key' => $this->arguments()['column_keys']['label'], '@keys' => $columnkeys)), 'type' => 'run_error');
    }
    return $result;
  }

  public function dryRun(&$context) {
    if ($this->hasConfigKey('csv_filepath') && $this->hasConfigKey('column_keys')) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

      $prevdatastep = $this->_previousDataChangingStep();

      $csvfilepath = $this->config['csv_filepath'];
      $columnkeys = preg_split('/;|[\r\n]+/', $this->config['column_keys']);
      $hasheader = $this->config['has_header'];
      if (!pifs_is_file($csvfilepath, $this->workflowid, $this->stepid, $this->otherid)) {
        $result[] = array('text' => t("File not found at '@fp'.",
                  array('@fp' => $csvfilepath)), 'type' => 'run_error');
        return $result;
      }

      // first add all existing items
      $itemnr = 1;
      if (isset($prevdatastep)) {
        $prevdatastep->resetDataIterator();
        $item = $prevdatastep->nextDataItem();
        while ($item !== NULL) {
          $this->saveDataItem($item);
          $itemnr = $item['item nr'];
          $item = $prevdatastep->nextDataItem();
        }
        $itemnr++;
      }

      $realfilepath = pifs_get_filepath($csvfilepath, $this->workflowid, $this->stepid, $this->otherid);
      if (class_exists('PHPExcel_IOFactory')) {
        try {
          $identity = PHPExcel_IOFactory::identify($realfilepath);
        }
        catch(Exception $e) {
          $results[] = array('text' => t("Cannot read file @file with error: @error.",
                        array('@file' => $csvfilepath, '@error' => $e->getMessage())), 'type' => 'run_error');
          return $results;
        }
        if ($identity !== 'CSV') {
          return $this->readExcelFile($realfilepath, $itemnr, $columnkeys, $hasheader);
        }
      }
      return $this->readCsvFile($realfilepath, $itemnr, $columnkeys, $hasheader);
    }
  }

  private function readCsvFile($csvfilepath, &$itemnr, $columnkeys, $hasheader) {
    // then add items from csv
    ini_set("auto_detect_line_endings", "1");
    $linenr = 0;
    $delimit = "\t";
    if (($fh = fopen($csvfilepath, "r")) !== FALSE) {
      while (($csvdata = fgetcsv($fh, 0, $delimit)) !== FALSE) {
        $linenr++;
        if ($linenr == 1) {
          // Treat header special.
          if (count($csvdata) == 1) {
            $delimit = ",";
            $csvdata = str_getcsv($csvdata[0], $delimit);
          }
          if (count($csvdata) == 1) {
            $delimit = ";";
            $csvdata = str_getcsv($csvdata[0], $delimit);
          }
          if ($hasheader === '1') {
            // This is a header, so skip it.
            continue;
          }
          elseif ($hasheader === 'use') {
            $columnkeys = array_map(function ($v) { return preg_replace('/[^a-zA-Z0-9_-]+/', '', $v); }, $csvdata);
            continue;
          }
        }
        if (count($csvdata) === 1 && $csvdata[0] === NULL) {
          // Skip empty lines.
          continue;
        }
        $item = array();
        foreach ($columnkeys as $columnindex => $columnkey) {
          if (isset($csvdata[$columnindex])) {
            $item[$columnkey] = $csvdata[$columnindex];
          }
          else {
            $item[$columnkey] = '';
          }
        }
        $item['item nr'] = $itemnr;
        $this->saveDataItem($item);
        $itemnr++;
      }
    }
    return FALSE;
  }

  private function readExcelFile($csvfilepath, &$itemnr, $columnkeys, $hasheader) {
    $results = array();
    try {
      $identity = PHPExcel_IOFactory::identify($csvfilepath);

      $excelreader = PHPExcel_IOFactory::createReader($identity);

      $excelreader->setReadDataOnly(true);
      $excelobj = $excelreader->load($csvfilepath);
    }
    catch(Exception $e) {
      $results[] = array('text' => t("Cannot read file @file with error: @error.",
                    array('@file' => $csvfilepath, '@error' => $e->getMessage())), 'type' => 'run_error');
      return $results;
    }
    $usecolumnkeys = $columnkeys;
    $worksheet = $excelobj->getActiveSheet();
    foreach ($worksheet->getRowIterator() as $ri => $row) {
      $cellIterator = $row->getCellIterator();
      $cellIterator->setIterateOnlyExistingCells(false);

      $item = array();
      foreach ($cellIterator as $cs => $cell) {
        $ci = PHPExcel_Cell::columnIndexFromString($cs) - 1;
        if ($hasheader !== '0' && $ri === 1) {
          if ($hasheader === 'use') {
            $usecolumnkeys[$ci] = preg_replace('/[^a-zA-Z0-9_-]+/', '', $cell->getValue());
          }
        }
        else {
          if (isset($usecolumnkeys[$ci])) {
            $columnkey = $usecolumnkeys[$ci];
            $item[$columnkey] = $cell->getValue();
          }
        }
      }
      if (count($item) === 0) {
        continue;
      }
      $item['item nr'] = $itemnr;
      $this->saveDataItem($item);
      $itemnr++;
    }
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    // no need to do anything
  }

}
