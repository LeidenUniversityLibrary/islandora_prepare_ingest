<?php

/**
 * @file
 * steps/ValidateCompoundStep.inc
 *
 *
 *  Copyright 2018 Leiden University Library
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


class ValidateCompoundStep extends PrepareIngestValidateStep {
  
  public function typeString() {
    return t('Compound');
  }

  public function models() {
    return array('islandora:compoundCModel');
  }

  public function dryRun(&$context) {
    if ($this->hasConfigKey('filepath')) {
      module_load_include('inc', 'islandora', 'includes/utilities');
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      if (module_load_include('inc', 'islandora_compound_batch', 'includes/utilities')) {
        $cb_utilities = new Utilities();
        $ext2contentmodel = $cb_utilities->extensionToContentModelMap;
      }
      else {
        $ext2contentmodel = array('tif' => 'islandora:sp_large_image_cmodel', 'pdf' => 'islandora:sp_pdf');
      }
      $childrequiredds2mimes = array(
        'OBJ' => array('application/pdf', 'image/tiff', 'image/jpeg', 'image/jp2'),
      );

      $mime_detect = new MimeDetect();
      $importdirectory = $this->config['filepath'];
      $importdirectory = rtrim($importdirectory, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
      $filenames = pifs_full_filenames_from_directory($importdirectory, $this->workflowid, $this->stepid, $this->otherid, '', TRUE, TRUE);
      $compoundsarray = array();
      $results = array();
      foreach ($filenames as $filename) {
        $relpath = preg_replace('#^' . $importdirectory . '/*#', '', $filename);
        if (!$relpath) {
          $results[] = array('text' => t("Unexpected file at '@filename'.", array('@filename' => $filename)), 'type' => 'validate_error');
          continue;
        }
        $relpathparts = explode(DIRECTORY_SEPARATOR, $relpath);
        $compoundname = $relpathparts[0];
        if (pifs_is_dir($filename, $this->workflowid, $this->stepid, $this->otherid)) {
          if (count($relpathparts) == 1) {
            $compoundsarray[$compoundname] = array();
          }
          elseif (count($relpathparts) == 2) {
            $childname = $relpathparts[1];
            $compoundsarray[$compoundname][$childname] = array();
          }
          else {
            $results[] = array('text' => t("Directory '@directory' found that should not be there", array('@directory' => $filename)), 'type' => 'validate_error');
          }
        }
        else {
          $mime = $mime_detect->getMimetype($filename);
          $fileinfo = pathinfo($filename);
          if (count($relpathparts) == 2) {
            // compound parent
            $allowedfile = FALSE;
            $ext = $fileinfo['extension'];
            if ($fileinfo['filename'] === 'OBJ') {
              if (isset($ext2contentmodel[$ext])) {
                $allowedfile = TRUE;
              }
            }
            if (in_array($fileinfo['basename'], array('structure.xml', 'MODS.xml', 'MARCXML.xml', 'LICENSE.txt'))) {
              $allowedfile = TRUE;
            }
            if (!$allowedfile) {
              $results[] = array('text' => t("File '@file' of mime '@mime' found that should not be there", array('@file' => $filename, '@mime' => $mime)), 'type' => 'validate_error');
            }
          }
          elseif (count($relpathparts) == 3) {
            // compound child
            $allowedfile = FALSE;
            $ext = $fileinfo['extension'];
            if ($fileinfo['filename'] === 'OBJ') {
              if (isset($ext2contentmodel[$ext])) {
                $allowedfile = TRUE;
                $compoundsarray[$compoundname][$childname][$fileinfo['filename']] = $filename;
              }
            }
            if (in_array($fileinfo['basename'], array('MODS.xml', 'MARCXML.xml'))) {
              $allowedfile = TRUE;
              $compoundsarray[$compoundname][$childname][$fileinfo['filename']] = $filename;
            }
            if (!$allowedfile) {
              $results[] = array('text' => t("File '@file' of mime '@mime' found that should not be there", array('@file' => $filename, '@mime' => $mime)), 'type' => 'validate_error');
            }
          }
          else {
            $results[] = array('text' => t("File '@file' found that should not be there", array('@file' => $filename)), 'type' => 'validate_error');
          }
          if ($mime === 'application/xml') {
            $isValidXml = FALSE;
            $xmldom = new DOMDocument();
            if (is_file($filename)) {
              $isValidXml = @$xmldom->load($filename);
            }
            else {
              $content = pifs_entry_read_content($filename, $this->workflowid, $this->stepid, $this->otherid);
              $isValidXml = @$xmldom->loadXml($content);
              $xmldom->documentURI = $filename;
            }
            if (!$isValidXml) {
              $results[] = array('text' => t("File '@file' is not a valid XML file", array('@type' => $type, '@file' => $filename)), 'type' => 'run_error');
            }
          }
        }
      }
      $iscompoundstructure = ((count($compoundsarray) > 0) && (count($results) === 0));
      $stats = array();
      $childcount = 0;
      foreach ($compoundsarray as $parent => $children) {
        foreach ($children as $child => $ds2file) {
          $childcount++;
          $notfound = array_diff_key($childrequiredds2mimes, $ds2file);
          if (count($notfound) > 0) {
            $results[] = array('text' => t("Missing required datastream @ds for child @child of compound '@compound'.", array('@ds' => key($notfound), '@child' => $child, '@compound' => $parent)), 'type' => 'validate_error');
            $iscompoundstructure = FALSE;
          }
          foreach ($ds2file as $ds => $file) {
            if (!array_key_exists($ds, $stats)) {
              $stats[$ds] = 0;
            }
            $stats[$ds]++;
          }
        }
      }
      $text = t("@compoundcount compounds found", array('@compoundcount' => count($compoundsarray)));
      if (count($compoundsarray) > 0) {
        if ($iscompoundstructure) {
          $results[] = array('text' => t("The structure appears to be valid!"), 'type' => 'validate_info');
        }
        $text .= ' ' . t('with @childcount children total', array('@childcount' => $childcount));
        $i = 1;
        if (count($stats) > 0) {
          $text .= ' ' . t('and');
          foreach ($stats as $mime => $count) {
            $text .= (($i === 1)?' ':(($i === count($stats))?' and ':', ')) . t('@count @mime datastreams', array('@count' => $count, '@mime' => $mime));
            $i++;
          }
        }
        else {
          $text .= ' ' . t('but without any datastreams');
        }
      }
      $text .= '.';
      $results[] = array('text' => $text, 'type' => 'validate_info');

      return $results;
    }
    return FALSE;
  }

  public function preprocessDrushCommand() {
    return 'islandora_compound_batch_preprocess'; 
  }

  public function modifyPreprocessOptions(&$options) {
    unset($options['content_models']);
    unset($options['type']);
  }

  public function prepareIngest(&$context) {
    if (isset($context['sandbox']['drush']) && $context['sandbox']['drush']) {
      $filepath = $this->config['filepath'];
      $type = $this->typeString();
      $shouldcreatestructure = FALSE;
      $directorypath = $this->config['filepath'];
      if ($dh = @opendir($directorypath)) {
        while (FALSE !== ($filename = readdir($dh))) {
          if ($filename !== '.' && $filename !== '..') {
            $curfilepath = $directorypath . DIRECTORY_SEPARATOR . $filename;
            if (is_dir($curfilepath)) {
              if (!is_file("$curfilepath/structure.xml")) {
                $shouldcreatestructure = TRUE;
              }
            }
          }
        }
      }
      if ($shouldcreatestructure) {
        if (drush_confirm(dt('Not all of the subdirectories of "@filepath" contain a structure.xml file. This is necessary to ingest compounds properly. Create missing structure.xml files?', array('@filepath' => $filepath)))) {
          $cbdir = '';
          foreach (array($_SERVER['DOCUMENT_ROOT'], $_SERVER['PWD'], $_SERVER['OLDPWD']) as $posdir) {
            if (preg_match('#^(/.*/)sites/[^/]+/.*#', $posdir, $matches) === 1) {
              $sitedir = $matches[1];
              $moddir = drupal_get_path('module', 'islandora_compound_batch');
              if (is_dir("$sitedir/$moddir/extras/scripts")) {
                $cbdir = "$sitedir/$moddir/extras/scripts";
                break;
              }
            }
          }
          if ($cbdir) {
            exec("cd $cbdir; php create_structure_files.php $filepath 2>&1", $output, $retvalue); 
            if ($retvalue === 0 && count($output) > 0) {
              drush_log(dt("Made @count structure.xml files", array('@count' => count($output))));
            }
            else {
              drush_log(dt("Error while making structure.xml files: @error", array('@error' => end($output))), 'error');
              return;
            }
          }
          else {
            drush_log(dt("Cannot find the create_structure_files script. Aborting."), 'error');
            return;
          }
        }
      }
    }
    parent::prepareIngest($context);
  }
}
