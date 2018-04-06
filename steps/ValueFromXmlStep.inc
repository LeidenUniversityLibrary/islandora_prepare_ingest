<?php

/**
 * @file
 * steps/ValueFromXmlStep.inc
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


class ValueFromXmlStep extends PrepareIngestStep {
  
  public function label() {
    $title = t('Add key from XML');
    if ($this->hasConfigKey('source_xml_key_name')) {
      $title .= ' ' . t('at key "@key"', array('@key' => $this->config['source_xml_key_name']));
    }
    if ($this->hasConfigKey('xpath')) {
      $title .= ' ' . t('located by XPath "@value"', array('@value' => $this->config['xpath']));
    }
    if ($this->hasConfigKey('namespaces')) {
      $namespaces = $this->_namespaces();
      if (count($namespaces) > 0) {
        $title .= ' ' . format_plural(count($namespaces), 'with namespace prefix:', 'with namespace prefixes:');
        foreach ($namespaces as $prefix => $nsuri) {
          $title .= ' ' . $prefix;
        }
      }
    }
    if ($this->hasConfigKey('target_key_name')) {
      $title .= ' ' . t('and store it in key "@key"', array('@key' => $this->config['target_key_name']));
    }
    $title .= '.';
    return $title;
  }

  private function _namespaces() {
    $namespaces = array();
    if ($this->hasConfigKey('namespaces')) {
      $nslines = explode("\n", $this->config['namespaces']);
      foreach ($nslines as $nsline) {
        $nsline = trim($nsline);
        if (strlen($nsline) > 0) {
          $parts = explode("=", $nsline);
          if (count($parts) == 2) {
	    $prefix = trim($parts[0]);
	    $nsuri = trim($parts[1]);
	    if (strlen($prefix) > 0 && strlen($nsuri) > 0) {
	      $namespaces[$prefix] = $nsuri;
	    }
          }
        }
      }
    }
    return $namespaces;
  }

  public function arguments() {
    return array(
      'source_xml_key_name' => array(
        'label' => t('XML key name'),
        'description' => t('The key name where the XML is stored.'),
        'type' => 'key',
      ),
      'target_key_name' => array(
        'label' => t('Target key name'),
        'description' => t('The key that should be used to hold the value.'),
        'type' => 'key',
      ),
      'xpath' => array(
        'label' => t('XPath'),
        'description' => t('An XPath to the needed value.'),
        'type' => 'xpath',
        'allow_constants' => TRUE,
      ),
      'namespaces' => array(
        'label' => t('namespaces'),
        'description' => t('The namespaces; one namespace per line in the following format: prefix=namespace URI'),
        'type' => 'text',
        'allow_constants' => TRUE,
        'optional' => TRUE,
      ),
    );
  }

  public function _inputKeyKeys() {
    return array('source_xml_key_name');
  }

  public function _outputKeyKeys() {
    return array('target_key_name');
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
    if ($this->hasConfigKey('source_xml_key_name') && $this->hasConfigKey('target_key_name') && $this->hasConfigKey('xpath') && isset($prevdatastep)) {
      if (!isset($context['sandbox']['total_items'])) {
        $context['sandbox']['total_items'] = islandora_prepare_ingest_data_cache_per_step_count($prevdatastep->workflowid, $prevdatastep->stepid, $prevdatastep->otherid, 1);

        $prevdatastep->resetDataIterator();
      }
      $xmlkey = $this->config['source_xml_key_name'];
      $targetkey = $this->config['target_key_name'];
      $xpath = $this->config['xpath'];
      $namespaces = $this->_namespaces();

      $results = array();
      $item = $prevdatastep->nextDataItem();
      $limit = 10;
      while ($item !== NULL && $limit-- > 0) {
        $xml = $item[$xmlkey];
        $domdoc = new DOMDocument();
        if ($domdoc->loadXML($xml) === FALSE) {
          $results[] = array('text' => t("Failed to parse xml at '@key' with '@value'.",
                    array('@key' => $this->arguments()['source_xml_key_name']['label'], '@value' => $this->config['source_xml_key_name'])), 'type' => 'argument_error');
        }
        $domxpath = new DOMXPath($domdoc);
        foreach ($namespaces as $prefix => $nsuri) {
          $domxpath->registerNamespace($prefix, $nsuri);
        }
        $domnodelist = $domxpath->query($xpath);
        if ($domnodelist === FALSE) {
          $results[] = array('text' => t("Malformed XPath at '@key' with '@value'.",
                    array('@key' => $this->arguments()['xpath']['label'], '@value' => $this->config['xpath'])), 'type' => 'argument_error');
        }
        if ($domnodelist->length > 0) {
          foreach ($domnodelist as $domnode) {
            $text = $domnode->textContent;
            if (isset($text) && strlen($text) > 0) {
              $item[$targetkey] = $text;
              break;
            }
          }
        }
        $this->saveDataItem($item);
        if ($limit > 0) {
          $item = $prevdatastep->nextDataItem();
        }
      }
      if ($context['sandbox']['total_items'] > 0) {
        if ($item !== NULL) {
          $context['finished'] = $item['item nr'] / $context['sandbox']['total_items'];
        }
        else {
          $context['finished'] = 1;
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
