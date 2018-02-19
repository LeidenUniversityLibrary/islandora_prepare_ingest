<?php

/**
 * @file
 * steps/ValidateFlatStructureStep.inc
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


class ValidateFlatStructureStep extends PrepareIngestStep {
  
  protected function _flatStructureLabel($type) {
    $title = t('Validate the @type structure', array('@type' => $type));
    if ($this->hasConfigKey('filepath')) {
      $title .= ' ' . t('at "@value"', array('@value' => $this->config['filepath']));
    }
    $title .= '.';
  
    return $title;
  }

  public function arguments() {
    return array(
      'filepath' => array(
        'label' => t('Filepath'),
        'description' => t('The absolute path to the directory where the import files are placed.'),
        'type' => 'filepath',
        'allow_constants' => TRUE,
      ),
    );
  }

  protected function _inputKeyKeys() {
    return array();
  }

  public function _outputKeyKeys() {
    return array();
  }

  public function changesCount() {
    return FALSE;
  }

  public function changesKeys() {
    return FALSE;
  }

  public function changesFiles() {
    return FALSE;
  }

  public function checkConfiguration() {
    $results = parent::checkConfiguration();
    if (!pifs_is_dir($this->config['filepath'], $this->workflowid, $this->otherid)) {
      $results[] = array('text' => t("Argument '@key' should be a valid directory.",
                    array('@key' => $this->arguments()['filepath']['label'])), 'type' => 'argument_error');
    }
    return $results;
  }

  protected function _flatStructureDryRun(array $models, &$context) {
    if ($this->hasConfigKey('filepath')) {
      module_load_include('inc', 'islandora', 'includes/utilities');
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $results = array(); 
      $importdirectory = rtrim($this->config['filepath'], DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
      $dsreqs = islandora_get_datastreams_requirements_from_models($models);
      $requiredmime2ds = array();
      $optionalmime2ds = array();
      foreach ($dsreqs as $dsid => $dsreq) {
        $optional = $dsreq['optional'];
        $mimes = $dsreq['mime'];
        foreach ($mimes as $mime) {
          if ($dsid === 'OBJ') {
            $requiredmime2ds[$mime] = $dsid;
          }
          else {
            $optionalmime2ds[$mime] = $dsid;
          }
        }
      }
      $mime_detect = new MimeDetect();
      $importpathdepth = count(explode(DIRECTORY_SEPARATOR, $importdirectory));
      $rightdepth = $importpathdepth + 1;
      $filenames = pifs_full_filenames_from_directory($importdirectory, $this->workflowid, $this->otherid, '', TRUE, TRUE);
      $isgood = (count($filenames) > 0);
      $groupedfilenames = array();
      foreach ($filenames as $filename) {
        $filepathdepth = count(explode(DIRECTORY_SEPARATOR, $filename));
        if (($filepathdepth > $rightdepth) || pifs_is_dir($filename, $this->workflowid, $this->otherid)) {
          $type = pifs_is_dir($filename, $this->workflowid, $this->otherid)?t('Directory'):t('File');
          $results[] = array('text' => t("@type '@file' found that should not be there", array('@type' => $type, '@file' => $filename)), 'type' => 'run_error');
          $isgood = FALSE;
        }
        $info = pathinfo($filename);
        $mime = $mime_detect->getMimetype($filename);
        $groupedfilenames[$info['filename']][$mime] = $filename;
        if ($mime === 'application/xml') {
           $isValidXml = FALSE;
           $xmldom = new DOMDocument();
           if (is_file($filename)) {
             $isValidXml = @$xmldom->load($filename);
           }
           else {
             $content = pifs_read_content($filename, $this->workflowid, $this->otherid);
             $isValidXml = @$xmldom->loadXml($content);
             $xmldom->documentURI = $filename;
           }
           if (!$isValidXml) {
             $results[] = array('text' => t("File '@file' is not a valid XML file", array('@file' => $filename)), 'type' => 'run_error');
           }
        }
      }
      $stats = array();
      foreach ($groupedfilenames as $group => $mime2filename) {
        $requiredfound = $requiredmime2ds;
        $optionalfound = array();
        $otherfound = array();
        foreach ($mime2filename as $mime => $filename) {
          if (array_key_exists($mime, $requiredmime2ds)) {
            $founddsid = $requiredmime2ds[$mime];
            $requiredfound = array_diff($requiredfound, array($founddsid));
          }
          elseif (array_key_exists($mime, $optionalmime2ds)) {
            $optionalfound[] = $mime;
          }
          else {
            $otherfound[] = $mime;
          }
          if (!array_key_exists($mime, $stats)) {
            $stats[$mime] = 0;
          }
          $stats[$mime]++;
        }
        if (count($requiredfound) > 0) {
          $missingmime = implode(' ' . t('or') . ' ', array_keys($requiredfound));
          $results[] = array('text' => t("Missing required datastream with mime @mime for object named '@name'.", array('@mime' => $missingmime, '@name' => $group)), 'type' => 'run_error');
        }
        if (count($otherfound) > 0) {
          $results[] = array('text' => t("Other datastream found with mime type '@mime' for object named '@name'.", array('@mime' => $otherfound[0], '@name' => $group)), 'type' => 'run_error');
        }
      }
      if (count($results) > 0) {
        return $results;
      }
      $text = t("@objectcount objects found", array('@objectcount' => count($groupedfilenames)));
      if (count($stats) > 0) {
        $results[] = array('text' => t("The structure appears to be valid!"), 'type' => 'validate_info');
        $text .= ' ' . t('with');
        $i = 1;
        foreach ($stats as $mime => $count) {
          $text .= (($i === 1)?' ':(($i === count($stats))?' and ':', ')) . t('@count @mime datastreams', array('@count' => $count, '@mime' => $mime));
          $i++;
        }
      }
      $text .= '.';
      $results[] = array('text' => $text, 'type' => 'validate_info');
 
      return $results;
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
