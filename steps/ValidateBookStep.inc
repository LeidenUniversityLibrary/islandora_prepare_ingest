<?php

/**
 * @file
 * steps/ValidateBookStep.inc
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


class ValidateBookStep extends PrepareIngestValidateStep {
  
  public function typeString() {
    return t('Book');
  }

  public function models() {
    return array('islandora:bookCModel');
  }

  public function dryRun(&$context) {
    if ($this->hasConfigKey('filepath')) {
      module_load_include('inc', 'islandora', 'includes/utilities');
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

      $importdirectory = $this->config['filepath'];
      $bookrequiredds2mimes = array();
      $bookoptionalds2mimes = array(
        'MODS' => array('application/xml', 'text/xml'),
        'TN'   => array('image/jpeg', 'image/png', 'image/gif'),
        'PDF'  => array('application/pdf'),
        'OCR'   => array('text/plain'),
        'HOCR'  => array('text/html', 'application/xml', 'text/xml', 'application/xhtml+xml'),
        'ALTO'  => array('application/xml', 'text/xml'),
      );
  
      /* this does not work (yet)
      $bookmodels = array("islandora:bookCModel");
      $bookdsreqs = islandora_get_datastreams_requirements_from_models($bookmodels);
      foreach ($bookdsreqs as $dsid => $dsreq) {
        $optional = $dsreq['optional'];
        $mimes = $dsreq['mime'];
        foreach ($mimes as $mime) {
          if (TRUE) { // nothing is required at book level
            $bookoptionalmime2ds[$mime] = $dsid;
          }
          else {
            $bookrequiredmime2ds[$mime] = $dsid;
          }
        }
      }
      */
      $pagerequiredds2mimes = array(
        'OBJ' => array('image/tiff', 'image/jpeg', 'image/jp2'),
      );
      $pageoptionalds2mimes = array(
        'MODS'  => array('application/xml', 'text/xml'),
        'JP2'   => array('image/jp2'),
        'JPG'   => array('image/jpeg'),
        'TN'    => array('image/png', 'image/gif', 'image/jpeg'),
        'OCR'   => array('text/plain'),
        'HOCR'  => array('text/html', 'application/xml', 'text/xml', 'application/xhtml+xml'),
        'ALTO'  => array('application/xml', 'text/xml'),
        'PDF'   => array('application/pdf'),
      );
      /* this does not work (yet)
      $pagemodels = array("islandora:pageCModel");
      $pagedsreqs = islandora_get_datastreams_requirements_from_models($pagemodels);
      foreach ($pagedsreqs as $dsid => $dsreq) {
        $optional = $dsreq['optional'];
        $mimes = $dsreq['mime'];
        foreach ($mimes as $mime) {
          if ($dsid === 'OBJ') { // only OBJ is required at page level
            $pagerequiredmime2ds[$mime] = $dsid;
          }
          else {
            $pageoptionalmime2ds[$mime] = $dsid;
          }
        }
      }
      */
      $mime_detect = new MimeDetect();
      $importdirectory = rtrim($importdirectory, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR;
      $filenames = pifs_full_filenames_from_directory($importdirectory, $this->workflowid, $this->stepid, $this->otherid, '', TRUE, TRUE);
      $booksarray = array();
      $results = array();
      foreach ($filenames as $filename) {
        $relpath = preg_replace('#^' . $importdirectory . '/*#', '', $filename);
        if (!$relpath) {
          $results[] = array('text' => t("Unexpected file at '@filename'.", array('@filename' => $filename)), 'type' => 'validate_error');
          continue;
        }
        $relpathparts = explode(DIRECTORY_SEPARATOR, $relpath);
        $bookname = $relpathparts[0];
        if (pifs_is_dir($filename, $this->workflowid, $this->stepid, $this->otherid)) {
          if (count($relpathparts) == 1) {
            $booksarray[$bookname] = array();
          }
          elseif (count($relpathparts) == 2) {
            $pagenr = $relpathparts[1];
            if (preg_match('/^[0-9]+$/', $pagenr) !== 1) {
              $results[] = array('text' => t("Pages should be numeric only, but '@directory' found.", array('@directory' => $filename)), 'type' => 'validate_error');
            }
            $booksarray[$bookname][$pagenr] = array();
          }
          else {
            $results[] = array('text' => t("Directory '@directory' found that should not be there", array('@directory' => $filename)), 'type' => 'validate_error');
          }
        }
        else {
          $mime = '';
          if (count($relpathparts) == 2) {
            // book
            $allowedfile = FALSE;
            $name = pathinfo($filename, PATHINFO_FILENAME);
            if (array_key_exists($name, $bookrequiredds2mimes)) {
              $mime = $mime_detect->getMimetype($filename);
              if (in_array($mime, $bookrequiredds2mimes[$name])) {
                $allowedfile = TRUE;
              }
            }
            if (array_key_exists($name, $bookoptionalds2mimes)) {
              $mime = $mime_detect->getMimetype($filename);
              if (in_array($mime, $bookoptionalds2mimes[$name])) {
                $allowedfile = TRUE;
              }
            }
            if (!$allowedfile) {
              $results[] = array('text' => t("File '@file' of mime '@mime' found that should not be there", array('@file' => $filename, '@mime' => $mime)), 'type' => 'validate_error');
            }
          }
          elseif (count($relpathparts) == 3) {
            // page
            $allowedfile = FALSE;
            $name = pathinfo($filename, PATHINFO_FILENAME);
            if (array_key_exists($name, $pagerequiredds2mimes)) {
              $mime = $mime_detect->getMimetype($filename);
              if (in_array($mime, $pagerequiredds2mimes[$name])) {
                $allowedfile = TRUE;
                $booksarray[$bookname][$pagenr][$name] = $filename;
              }
            }
            if (array_key_exists($name, $pageoptionalds2mimes)) {
              $mime = $mime_detect->getMimetype($filename);
              if (in_array($mime, $pageoptionalds2mimes[$name])) {
                $allowedfile = TRUE;
                $booksarray[$bookname][$pagenr][$name] = $filename;
              }
            }
            if (!$allowedfile) {
              if (!$mime) {
                $mime = $mime_detect->getMimetype($filename);
              }
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
      $isbookstructure = ((count($booksarray) > 0) && (count($results) === 0));
      $stats = array();
      $pagecount = 0;
      foreach ($booksarray as $book => $pages) {
        foreach ($pages as $page => $ds2file) {
          $pagecount++;
          $notfound = array_diff_key($pagerequiredds2mimes, $ds2file);
          if (count($notfound) > 0) {
            $results[] = array('text' => t("Missing required datastream @ds for page @page of book '@book'.", array('@ds' => key($notfound), '@page' => $page, '@book' => $book)), 'type' => 'validate_error');
            $isbookstructure = FALSE;
          }
          foreach ($ds2file as $ds => $file) {
            if (!array_key_exists($ds, $stats)) {
              $stats[$ds] = 0;
            }
            $stats[$ds]++;
          }
        }
      }
      $text = t("@bookcount books found", array('@bookcount' => count($booksarray)));
      if (count($booksarray) > 0) {
        if ($isbookstructure) {
          $results[] = array('text' => t("The structure appears to be valid!"), 'type' => 'validate_info');
        }
        $text .= ' ' . t('with @pagecount pages total', array('@pagecount' => $pagecount));
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
    return 'islandora_book_batch_preprocess'; 
  }
}
