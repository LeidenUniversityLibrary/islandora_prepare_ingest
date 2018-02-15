<?php

/**
 * @file
 * steps/TransformXmlStep.inc
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


class TransformXmlStep extends PrepareIngestStep {
  
  public function label() {
    $title = t('Transform XML');
    if ($this->hasConfigKey('xml_key_name')) {
      $title .= ' ' . t('at "@key"', array('@key' => $this->config['xml_key_name']));
    }
    if ($this->hasConfigKey('xsl_key_name')) {
      $title .= ' ' . t('with XSL at "@key"', array('@key' => $this->config['xsl_key_name']));
    }
    if ($this->hasConfigKey('target_filepath')) {
      $title .= ' ' . t('and write to file at "@key"', array('@key' => $this->config['target_filepath']));
    }
    $title .= '.';
    return $title;
  }

  public function arguments() {
    return array(
      'xml_key_name' => array(
        'label' => t('XML key'),
        'description' => t('The input XML. The value of this key is either an absolute filepath to an XML file or contains XML itself.'),
        'type' => 'key',
      ),
      'xsl_key_name' => array(
        'label' => t('XSLT key'),
        'description' => t('The input XSLT. The value of this key is either an absolute filepath to an XSLT file or contains XSLT itself.'),
        'type' => 'key',
      ),
      'target_filepath' => array(
        'label' => t('Target filepath'),
        'description' => t('This should hold an absolute path to a not existing file where the output XML file will be written.'),
        'type' => 'templatestring',
      ),
    );
  }

  protected function _inputKeyKeys() {
    return array('xml_key_name', 'xsl_key_name');
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
    return TRUE;
  }

  public function checkConfiguration() {
    return parent::checkConfiguration();
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('filepath') && $this->hasConfigKey('xml_key_name') && isset($prevdatastep)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $xmlkey = $this->config['xml_key_name'];
      $xslkey = $this->config['xsl_key_name'];
      $targetfilepathtemplate = $this->config['target_filepath'];
      $results = array();
      $prevuseerr = libxml_use_internal_errors(true);

      $results = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      $itemnr = 1;
      while ($item !== NULL) {
        $isValidXml = TRUE; // be positive about it!
        $xmlvalue = $item[$xmlkey];
        $xslvalue = $item[$xslkey];
        $xml = new DOMDocument();
        $result = _load_xml_from_value($xmlvalue, $xml, $item['item nr']);
        if ($result !== FALSE) {
          $results[] = $result;
          $isValidXml = FALSE;
        }
        $xsl = new DOMDocument();
        $result = _load_xml_from_value($xslvalue, $xsl, $item['item nr']);
        if ($result !== FALSE) {
          $results[] = $result;
          $isValidXml = FALSE;
        }
        if ($isValidXml) {
          $xsltproc = new XSLTProcessor;
          if ($xsltproc->importStyleSheet($xsl)) { // attach the xsl rules
            $content = $xsltproc->transformToXML($xml);
            if ($content !== FALSE) {
              $targetfilepath = $this->fillOutTemplate($targetfilepathtemplate, $item);
              $targetdirectory = dirname($targetfilepath);
              if (!pifs_is_dir($targetdirectory, $this->workflowid, $this->otherid)) {
                $results[] = array('text' => t("Directory '@directory' for file '@file' should exist, but does not.",
                          array('@directory' => $targetdirectory, '@file' => $targetfilepath)), 'type' => 'run_error');
              }
              if (count($results) > 0) {
                return $results;
              }
              if (pifs_entry_exists($targetfilepath, $this->workflowid, $this->otherid)) {
                if (pifs_entry_is_file($targetfilepath, $this->workflowid, $this->otherid)) {
                  pifs_entry_append_content($targetfilepath, $content, $this->workflowid, $this->otherid);
                }
                else {
                  $results[] = array('text' => t("Cannot write file: a directory '@directory' already exists at that location.",
                            array('@directory' => $targetfilepath)), 'type' => 'run_error');
                }
              }
              else {
                pifs_entry_write_content($targetfilepath, $content, $this->workflowid, $this->stepid, $this->otherid, $itemnr);
                $itemnr++;
              }
            }
          }
          else {
            $results[] =  array('text' => t("Transform failed for item at @nr", array('@nr' => $current)), 'type' => 'run_error');
          }
        }
      }
      libxml_clear_errors();


        $item = $prevdatastep->nextDataItem();
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
  }

}
