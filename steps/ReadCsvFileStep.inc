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
    $title = t('Add items by reading CSV file');
    if ($this->hasConfigKey('csv_filepath')) {
      $title .= ' ' . t('located at "@value"', array('@value' => $this->config['csv_filepath']));
    }
    if ($this->hasConfigKey('column_keys')) {
      $title .= ' ' . t('and place the values in the keys "@value"', array('@value' => $this->config['column_keys']));
    }
    if ($this->hasConfigKey('has_header') && $this->config['has_header']) {
      $title .= ' ' . t('but skip header');
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'csv_filepath' => array(
        'label' => t('CSV filepath'),
        'description' => t('Specify the absolute path to the CSV file.'),
        'type' => 'filepath',
        'allow_constants' => TRUE,
      ),
      'column_keys' => array(
        'label' => t('Column keys'),
        'description' => t('Define the output keys of the CSV file, separated by semicolons.'),
        'type' => 'keys',
      ),
      'has_header' => array(
        'label' => t('Has header'),
        'description' => t('Does the CSV file have a header.'),
        'type' => 'boolean',
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
    $result = parent::checkConfiguration($context);

    $csvfilepath = $this->config['csv_filepath'];
    $columnkeys = $this->config['column_keys'];
    if (!is_file($csvfilepath)) {
      $result[] = array('text' => t("File not found at '@fp'.",
                array('@fp' => $csvfilepath)), 'type' => 'run_error');
    }
    if (preg_match('/^[a-zA-Z0-9_-]+(;[a-zA-Z0-9_-]+)*$/', $columnkeys) === 0) {
      $result[] = array('text' => t("@key should contain keynames (containing letters, numbers and/or dashes) separated by semicolons, but is '@keys'.",
                array('@key' => $this->arguments()['column_keys']['label'], '@keys' => $columnkeys)), 'type' => 'run_error');
    }
    return $result;
  }

  public function dryRun(&$context) {
    if ($this->hasConfigKey('csv_filepath') && $this->hasConfigKey('column_keys')) {
      $prevdatastep = $this->_previousDataChangingStep();

      $csvfilepath = $this->config['csv_filepath'];
      $columnkeys = explode(';', $this->config['column_keys']);
      $hasheader = $this->config['has_header'];
      if (!is_file($csvfilepath)) {
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
	    if ($hasheader) {
	      // This is a header, so skip it.
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
