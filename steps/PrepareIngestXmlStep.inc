<?php

/**
 * @file
 * steps/PrepareIngestXmlStep.inc
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


class PrepareIngestXmlStep extends PrepareIngestFilesStep {
  
  protected function _loadXmlFromValue($xmlvalue, $xmldom, $itemnr) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

    $isFilePath = ((substr($xmlvalue, 0, 1) === '/') && pifs_is_file($xmlvalue, $this->workflowid, $this->stepid, $this->otherid));
    if ($isFilePath) {
      if (is_file($xmlvalue)) {
        $isValidXml = @$xmldom->load($xmlvalue);
      }
      else {
        $content = pifs_entry_read_content($xmlvalue, $this->workflowid, $this->stepid, $this->otherid);
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

}
