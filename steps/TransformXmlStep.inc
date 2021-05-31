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


class TransformXmlStep extends PrepareIngestXmlStep {

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
        'description' => t('The input XSLT. The value of this key is either an absolute filepath to an XSLT file or contains XSLT itself. The keys of the items can be used as parameters within this XSLT.'),
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

  public function dryRun(&$context) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

    $error = NULL;
    error_reporting(E_ALL);
    $errfunc = function($errno, $errstr) use (&$error) {
      $prefix = 'XSLTProcessor::transformToXml(): ';
      if (substr($errstr, 0, strlen($prefix)) === $prefix) {
        $error = substr($errstr, strlen($prefix));
        set_error_handler(NULL);
      }
      else {
        return FALSE;
      }
    };
    set_error_handler($errfunc);

    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('target_filepath') && $this->hasConfigKey('xml_key_name') && isset($prevdatastep)) {
      if (!isset($context['sandbox']['total_items'])) {
        $context['sandbox']['total_items'] = islandora_prepare_ingest_data_cache_per_step_count($prevdatastep->workflowid, $prevdatastep->stepid, $prevdatastep->otherid, 1);

        $prevdatastep->resetDataIterator();
      }
      $xmlkey = $this->config['xml_key_name'];
      $xslkey = $this->config['xsl_key_name'];
      $targetfilepathtemplate = $this->config['target_filepath'];
      $results = array();

      $results = array();
      $item = $prevdatastep->nextDataItem();
      $limit = 5;
      while ($item !== NULL && $limit-- > 0) {
        $isValidXml = TRUE; // be positive about it!
        $xmlvalue = $item[$xmlkey];
        $xslvalue = $item[$xslkey];
        $xmldoc = new DOMDocument();
        $result = $this->_loadXmlFromValue($xmlvalue, $xmldoc, $item['item nr']);
        if ($result !== FALSE) {
          $results[] = $result;
          $isValidXml = FALSE;
        }
        $xsldoc = new DOMDocument();
        $result = $this->_loadXmlFromValue($xslvalue, $xsldoc, $item['item nr']);
        if ($result !== FALSE) {
          $results[] = $result;
          $isValidXml = FALSE;
        }
        if ($isValidXml) {
          $xsltproc = new XSLTProcessor;
          if ($xsltproc->importStyleSheet($xsldoc)) { // attach the xsl rules
            foreach ($item as $key => $value) {
              $xsltproc->setParameter('', $key, htmlspecialchars($value, ENT_XML1 | ENT_QUOTES, 'UTF-8'));
            }
            $content = $xsltproc->transformToXML($xmldoc);
            if ($content) {
              $targetfilepath = $this->fillOutTemplate($targetfilepathtemplate, $item);
              $targetdirectory = dirname($targetfilepath);
              if (!pifs_is_dir($targetdirectory, $this->workflowid, $this->stepid, $this->otherid)) {
                $results[] = array('text' => t("Directory '@directory' for file '@file' should exist, but does not.",
                          array('@directory' => $targetdirectory, '@file' => $targetfilepath)), 'type' => 'run_error');
              }
              if (pifs_entry_exists($targetfilepath, $this->workflowid, $this->stepid, $this->otherid)) {
                if (pifs_entry_is_dir($targetfilepath, $this->workflowid, $this->stepid, $this->otherid)) {
                  $results[] = array('text' => t("Cannot write file: a directory '@directory' already exists at that location.",
                            array('@directory' => $targetfilepath)), 'type' => 'run_error');
                }
              }
              pifs_entry_write_content($targetfilepath, $content, $this->workflowid, $this->stepid, $this->otherid);
            }
            else {
              if (!$error) {
                $error = 'Unknown error.';
              }
              $results[] =  array('text' => t("Transform failed for item at @nr: @error", array('@nr' => $item['item nr'], '@error' => $error)), 'type' => 'run_error');
            }
          }
          else {
            $results[] =  array('text' => t("Transform failed to import stylesheet for item at @nr", array('@nr' => $item['item nr'])), 'type' => 'run_error');
          }
        }
        if (count($results) > 0) {
          return $results;
        }
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
    }
    return FALSE;
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

    pifs_persist_entries($this->workflowid, $this->stepid, $this->otherid);
  }
}
