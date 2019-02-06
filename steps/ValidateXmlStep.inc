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
      if ($this->hasConfigKey('xml_key_contents_type') && $this->config['xml_key_contents_type'] !== 'filepath') {
        $title .= ' ' . t('at "@key"', array('@key' => $this->config['xml_key_name']));
      }
      else {
        $title .= ' ' . t('in file located at "@key"', array('@key' => $this->config['xml_key_name']));
      }
    }
    if ($this->hasConfigKey('validate_dtd') && $this->config['validate_dtd']) {
      $title .= ' ' . t('with included DTD');
    }
    if ($this->hasConfigKey('schema_key_name')) {
      $title .= ' ' . t('against schema at "@key"', array('@key' => $this->config['schema_key_name']));
    }
    if ($this->hasConfigKey('pretty_print') && $this->config['pretty_print']) {
      $title .= ' ' . t('and pretty print');
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
      'xml_key_contents_type' => array(
        'label' => t('XML key contains'),
        'description' => t('The key above contains XML or is a filepath.'),
        'type' => 'select',
        'options' => array(
          NULL => t('automatic'),
          'XML' => t('XML'),
          'filepath' => t('filepath'),
        ),
        'optional' => TRUE,
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
      'pretty_print' => array(
        'label' => t('Pretty print XML'),
        'description' => t('Format the XML nicely and save the result.'),
        'type' => 'boolean',
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
    $isFilePath = $this->hasConfigKey('xml_key_contents_type') && $this->config['xml_key_contents_type'] === 'filepath';
    return $this->hasConfigKey('xml_key_contents_type') && !$isFilePath && $this->hasConfigKey('pretty_print') && $this->config['pretty_print'];
  }

  public function changesFiles() {
    $isFilePath = $this->hasConfigKey('xml_key_contents_type') && $this->config['xml_key_contents_type'] === 'filepath';
    return $this->hasConfigKey('xml_key_contents_type') && $isFilePath &&  $this->hasConfigKey('pretty_print') && $this->config['pretty_print'];
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('xml_key_name') && isset($prevdatastep)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

      if (!isset($context['sandbox']['total_items'])) {
        $context['sandbox']['total_items'] = islandora_prepare_ingest_data_cache_per_step_count($prevdatastep->workflowid, $prevdatastep->stepid, $prevdatastep->otherid, 1);

        $prevdatastep->resetDataIterator();
      }
      $xmlkey = $this->config['xml_key_name'];
      $results = array();
      $prevuseerr = libxml_use_internal_errors(true);

      $results = array();
      $item = $prevdatastep->nextDataItem();
      $limit = 1;
      $isFilePath = NULL;
      if ($this->hasConfigKey('xml_key_contents_type')) {
        $isFilePath = $this->config['xml_key_contents_type'] === 'filepath';
      }
      while ($item !== NULL && $limit-- > 0) {
        $isValidXml = FALSE;
        $xmlvalue = $item[$xmlkey];
        $xmldoc = new DOMDocument();
        $result = $this->_loadXmlFromValue($xmlvalue, $xmldoc, $item['item nr'], $isFilePath);
        if ($result !== FALSE) {
          $results[] = $result;
        }
        else {
          $isValidXml = TRUE;
          if ($isValidXml && $this->config['validate_dtd']) {
            $isValidXml = $xmldoc->validate();
          }
          if ($isValidXml && $this->hasConfigKey('schema_key_name')) {
            $schemakey = $this->config['schema_key_name'];
            if (isset($item[$schemakey]) && strlen($item[$schemakey]) > 0) {
              $schemavalue = $item[$schemakey];
              if ((substr($schemavalue, 0, 1) === '/') && pifs_is_file($schemavalue, $this->workflowid, $this->stepid, $this->otherid)) {
                $isValidXml = $xmldoc->schemaValidate($schemavalue);
              }
              else {
                $isValidXml = $xmldoc->schemaValidateSource($schemavalue);
              }
            }
          }
          if (!$isValidXml) {
            $error = libxml_get_last_error();
            $results[] = array('text' => t("Key @key of item @itemnr does not contain valid XML: @error at line @line",
              array('@key' => $this->arguments()['xml_key_name']['label'], '@itemnr' => $item['item nr'], '@error' => $error->message, '@line' => $error->line)), 'type' => 'validate_error');
          }
          else {
            if ($this->hasConfigKey('pretty_print') && $this->config['pretty_print']) {
              $xmldoc->preserveWhiteSpace = FALSE;
              $xmldoc->formatOutput = TRUE;
              $prettyxml = $xmldoc->saveXML();
              $this->_saveXmlToKey($item, $prettyxml, $xmlkey, $isFilePath,  $item['item nr']);
            }
          }
        }
        libxml_clear_errors();
        if ($limit > 0) {
          $item = $prevdatastep->nextDataItem();
        }
      }
      libxml_use_internal_errors($prevuseerr);
      if ($context['sandbox']['total_items'] > 0) {
        if ($item !== NULL) {
          $context['finished'] = $item['item nr'] / $context['sandbox']['total_items'];
        }
        else {
          $context['finished'] = 1;
          $prevdatastep->clearDataIterator();
        }
      }
      if (count($results) > 0) {
        return $results;
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
