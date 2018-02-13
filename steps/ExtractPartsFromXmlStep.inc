<?php

/**
 * @file
 * steps/ExtractPartsFromXmlStep.inc
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


class ExtractPartsFromXmlStep extends PrepareIngestStep {
  
  public function label() {
    $title = t('Add items by extracting XML parts from XML');
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
      $title .= ' ' . t('and insert item with key "@key"', array('@key' => $this->config['target_key_name']));
    }
    if ($this->hasConfigKey('related_key_name')) {
      $title .= ' ' . t('and related key "@key"', array('@key' => $this->config['related_key_name']));
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
        'description' => t('The key that should be used to hold the new XML parts.'),
        'type' => 'key',
      ),
      'xpath' => array(
        'label' => t('XPath'),
        'description' => t('An XPath to a repeating element.'),
        'type' => 'xpath',
        'allow_constants' => TRUE,
      ),
      'namespaces' => array(
        'label' => t('namespaces'),
        'description' => t('The namespaces; one namespace per line in the following format: prefix=namespace URI'),
        'type' => 'text',
        'optional' => TRUE,
        'allow_constants' => TRUE,
      ),
      'related_key_name' => array(
        'label' => t('Related key name'),
        'description' => t('Optionally, a key that will be included in the new items so they are related.'),
        'type' => 'key',
        'optional' => TRUE,
      ),
    );
  }

  public function _inputKeyKeys() {
    return array('source_xml_key_name', 'related_key_name');
  }

  public function _outputKeyKeys() {
    return array('target_key_name', 'related_key_name');
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

  public function checkConfiguration() {
    return parent::checkConfiguration();
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('source_xml_key_name') && $this->hasConfigKey('target_key_name') && $this->hasConfigKey('xpath') && isset($prevdatastep)) {
      $xmlkey = $this->config['source_xml_key_name'];
      $targetkey = $this->config['target_key_name'];
      $xpath = $this->config['xpath'];
      $namespaces = $this->_namespaces();
      $relatedkey = $this->hasConfigKey('related_key_name')?$this->config['related_key_name']:'';

      $results = array();
      $itemnr = 1;
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $xml = $item[$xmlkey];
        $domdoc = new DOMDocument();
        if ($domdoc->loadXML($xml) === FALSE) {
          $result[] = array('text' => t("Failed to parse xml at '@key' with '@value'.",
                    array('@key' => $this->arguments()['source_xml_key_name']['label'], '@value' => $this->config['source_xml_key_name'])), 'type' => 'argument_error');
          return $result;
        }
        $domxpath = new DOMXPath($domdoc);
        foreach ($namespaces as $prefix => $nsuri) {
          $domxpath->registerNamespace($prefix, $nsuri);
        }
        $domnodelist = $domxpath->query($xpath);
        if ($domnodelist === FALSE) {
          $results[] = array('text' => t("Malformed XPath at '@key' with '@value'.",
                    array('@key' => $this->arguments()['xpath']['label'], '@value' => $this->config['xpath'])), 'type' => 'argument_error');
          return $results;
        }
        $item['item nr'] = $itemnr++;
        $this->saveDataItem($item);
        if ($domnodelist->length > 0) {
          foreach ($domnodelist as $domnode) {
            $newdomdoc = new DOMDocument;
            $newnode = $newdomdoc->importNode($domnode, true);
            if ($newnode) {
              $newdomdoc->appendChild($newnode);
              $newditem= array();
              $newitem[$targetkey] = $newdomdoc->saveXML();
              if ($relatedkey) {
                $newitem[$relatedkey] = $item[$relatedkey];
              }
              $newitem['item nr'] = $itemnr++;
              $this->saveDataItem($newitem);
            }
          }
        }
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
