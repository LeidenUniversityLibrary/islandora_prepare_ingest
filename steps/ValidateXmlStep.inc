<?php

/**
 * @file
 * steps/ValidateXmlStep.inc
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


class ValidateXmlStep extends PrepareIngestXmlStep {
  
  public function label() {
    $title = t('Validate XML');
    if ($this->hasConfigKey('xml_key_name')) {
      $title .= ' ' . t('at "@key"', array('@key' => $this->config['xml_key_name']));
    }
    if ($this->hasConfigKey('validate_dtd') && $this->config['validate_dtd']) {
      $title .= ' ' . t('with included DTD');
    }
    if ($this->hasConfigKey('schema_key_name')) {
      $title .= ' ' . t('against schema at "@key"', array('@key' => $this->config['schema_key_name']));
    }
    $title .= '.';
    return $title;
  }

  public function arguments() {
    return array(
      'xml_key_name' => array(
        'label' => t('XML key'),
        'description' => t('Validate the XML. The value of this key is either an absolute filepath to an XML file or contains XML itself.'),
        'type' => 'key',
      ),
      'validate_dtd' => array(
        'label' => t('Validate based on DTD'),
        'description' => t('The XML should contain a DTD for this to work.'),
        'type' => 'boolean',
      ),
      'schema_key_name' => array(
        'label' => t('Schema key'),
        'description' => t('Optionally, validate against a schema. The value of this key is either an absolute filepath to a schema file or contains a schema itself.'),
        'type' => 'key',
        'optional' => TRUE,
      ),
    );
  }

  protected function _inputKeyKeys() {
    return array('xml_key_name', 'schema_key_name');
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
    return parent::checkConfiguration();
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('xml_key_name') && isset($prevdatastep)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $xmlkey = $this->config['xml_key_name'];
      $results = array();
      $prevuseerr = libxml_use_internal_errors(true);

      $results = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $isValidXml = FALSE;
        $xmlvalue = $item[$xmlkey];
        $xml = new DOMDocument();
        $result = $this->_loadXmlFromValue($xmlvalue, $xml, $item['item nr']);
        if ($result !== FALSE) {
          $results[] = $result;
        }
        else {
          $isValidXml = TRUE;
        }
        if ($isValidXml && $this->config['validate_dtd']) {
          $isValidXml = $xml->validate();
        }
        if ($isValidXml && $this->hasConfigKey('schema_key_name')) {
          $schemakey = $this->config['schema_key_name'];
          if (isset($item[$schemakey]) && strlen($item[$schemakey]) > 0) {
            $schemavalue = $item[$schemakey];
            if ((substr($schemavalue, 0, 1) === '/') && pifs_is_file($schemavalue, $this->workflowid, $this->otherid)) {
              $isValidXml = $xml->schemaValidate($schemavalue);
            }
            else {
              $isValidXml = $xml->schemaValidateSource($schemavalue);
            }
          }
        }
        if (!$isValidXml) {
          $error = libxml_get_last_error();
          $results[] = array('text' => t("Key @key of item @itemnr does not contain valid XML: @error at line @line",
            array('@key' => $this->arguments()['xml_key_name']['label'], '@itemnr' => $item['item nr'], '@error' => $error->message, '@line' => $error->line)), 'type' => 'validate_error');
        }
        libxml_clear_errors();
        $item = $prevdatastep->nextDataItem();
      }
      libxml_use_internal_errors($prevuseerr);
      if (count($results) > 0) {
        return $results;
      }
    }
    return FALSE;
  }

  private function _load_xml_from_value($xmlvalue, $xmldom, $itemnr) {
    $isFilePath = ((substr($xmlvalue, 0, 1) === '/') && pifs_is_file($xmlvalue, $this->workflowid, $this->otherid));
    if ($isFilePath) {
      if (is_file($xmlvalue)) {
        $isValidXml = @$xmldom->load($xmlvalue);
      }
      else {
        $content = pifs_read_content($xmlvalue, $this->workflowid, $this->otherid);
        $isValidXml = @$xmldom->loadXml($content);
        $xmldom->documentURI = $xmlvalue;
      }
    }
    else {
      $isValidXml = @$xmldom->loadXml($xmlvalue);
    }
    if (!$isValidXml) {
      $error = libxml_get_last_error();
      $message = $error?$error->message:t("Unknown error");
      $line = $error?$error->line:t("unknown");
      if ($isFilePath) {
        return array('text' => t("File @file does not contain valid XML: @error at line @line",
          array('@file' => $xmlvalue, '@error' => $message, '@line' => $line)), 'type' => 'validate_error');
      }
      else {
        return array('text' => t("Item @nr does not contain valid XML: @error at line @line",
          array('@nr' => $itemnr, '@error' => $message, '@line' => $line)), 'type' => 'validate_error');
      }
    }
    return FALSE;
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    // no need to do anything
    $this->resetDataIterator(2);
    $item = $this->nextDataItem();
    while ($item !== NULL) {
      pifs_write_content_to_file($item);
      $item = $this->nextDataItem();
    }
  }

}
