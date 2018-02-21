<?php

/**
 * @file
 * steps/ParseXmlStep.inc
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


class ParseXmlStep extends PrepareIngestStep {
  
  public function label() {
    $title = t('Parse XML file');
    if ($this->hasConfigKey('filepath')) {
      $title .= ' ' . t('located at "@fp"', array('@fp' => $this->config['filepath']));
    }
    if ($this->hasConfigKey('xml_key_name')) {
      $title .= ' ' . t('and store the XML in key "@key"', array('@key' => $this->config['xml_key_name']));
    }
    $title .= '.';
    return $title;
  }

  public function arguments() {
    return array(
      'filepath' => array(
        'label' => t('Filepath'),
        'description' => t('This should hold an absolute path to an existing XML file.'),
        'type' => 'templatestring',
      ),
      'xml_key_name' => array(
        'label' => t('XML key name'),
        'description' => t('The key where the XML should be stored.'),
        'type' => 'key',
      ),
    );
  }

  public function inputKeys() {
    return $this->_getInputKeysFromTemplates(array('filepath'));
  }

  public function _outputKeyKeys() {
    return array('xml_key_name');
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
    if ($this->hasConfigKey('filepath') && $this->hasConfigKey('xml_key_name') && isset($prevdatastep)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $filepathtemplate = $this->config['filepath'];
      $xmlkey = $this->config['xml_key_name'];

      $results = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $filepath = $this->fillOutTemplate($filepathtemplate, $item);
        if (!pifs_is_file($filepath, $this->workflowid, $this->otherid)) {
          $results[] = array('text' => t("File '@file' does not exist.",
                  array('@file' => $filepath)), 'type' => 'run_error');
        }
        libxml_use_internal_errors(true);
        $domdoc = new DOMDocument();
        if (is_file($filepath)) {
          if (!$domdoc->load($filepath)) {
            $results[] = array('text' => t("File '@file' is not a valid XML document.",
                    array('@file' => $filepath)), 'type' => 'run_error');
          }
        }
        else {
          $itemnr = 0;
          $contentxml = pifs_entry_read_content($filepath, $this->workflowid, $this->otherid, $itemnr);
          if (!$domdoc->loadXML($contentxml)) {
            $results[] = array('text' => t("File '@file' is not a valid XML document.",
                      array('@file' => $filepath)), 'type' => 'run_error');
          }
        }
        $errors = libxml_get_errors();
        if (count($errors) > 0) {
          $message = '';
          foreach ($errors as $error) {
            if ($error->level !== LIBXML_ERR_WARNING) {
              $message .= ' ' . t("'@message' on line @line.", array('@message' => $error->message, '@line' => $error->line));
            }
          }
          $results[] = array('text' => t("Fatal error parsing file '@file':@message",
               array('@file' => $filepath, '@message' => $message)), 'type' => 'run_error');
        }

        if (count($results) > 0) {
          return $results;
        }
        $item[$xmlkey] = $domdoc->saveXML();

        $this->saveDataItem($item);
        $item = $prevdatastep->nextDataItem();
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
