<?php

/**
 * @file
 * Prepare Ingest File System.
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

CONST PIFS_IS_FILE     = 0b00010000; // 16
CONST PIFS_IS_DIR      = 0b00100000; // 32
CONST PIFS_HAS_CONTENT = 0b00000010; // 2
CONST PIFS_HAS_REAL    = 0b00000001; // 1

function pifs_cleanup_filepath($filepath) {
  $filepath = preg_replace('#' . DIRECTORY_SEPARATOR . DIRECTORY_SEPARATOR . '+#', DIRECTORY_SEPARATOR, $filepath);
  $filepath = rtrim($filepath, DIRECTORY_SEPARATOR);
  return $filepath;
}

function _pifs_step_weight($workflowid, $stepid) {
  return db_select('islandora_prepare_ingest_step', 'step')
    ->fields('step', array('weight'))
    ->condition('step.id',$stepid)
    ->condition('step.workflow_id',$workflowid)
    ->execute()
    ->fetchField();
}

function pifs_file_exists($filepath, $workflow_id, $step_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  if (file_exists($filepath)) {
    return TRUE;
  }
  return pifs_entry_exists($filepath, $workflow_id, $step_id, $other_id);
}

function pifs_entry_exists($filepath, $workflow_id, $step_id, $other_id) {
  $stepweight = _pifs_step_weight($workflow_id, $step_id);
  $filepath = pifs_cleanup_filepath($filepath);
  $select = db_select('islandora_prepare_ingest_pifs', 'pifs');
  $select->fields('pifs', array('path'));
  $select->condition('pifs.workflow_id', $workflow_id)
    ->condition('pifs.other_id', $other_id)
    ->condition('path', $filepath);
  $select->join('islandora_prepare_ingest_step', 'step', 'step.id = pifs.step_id AND step.workflow_id = :workflowid AND step.weight <= :weight', array(':workflowid' => $workflow_id, ':weight' => $stepweight));
  $count = $select->countQuery()->execute()->fetchField();

  return ($count != "0");
}

function pifs_count_of_all_filepaths_for_entries($workflow_id, $step_id, $other_id) {
  $stepweight = _pifs_step_weight($workflow_id, $step_id);
  $select = db_select('islandora_prepare_ingest_pifs', 'pifs');
  $select->fields('pifs', array('path'));
  $select->condition('pifs.workflow_id', $workflow_id)
    ->condition('pifs.other_id', $other_id);
  $select->join('islandora_prepare_ingest_step', 'step', 'step.id = pifs.step_id AND step.workflow_id = :workflowid AND step.weight <= :weight', array(':workflowid' => $workflow_id, ':weight' => $stepweight));
  return $select->countQuery()->execute()->fetchField();
}

function pifs_filepaths_for_entries($workflow_id, $step_id, $other_id, $min, $max) {
  $stepweight = _pifs_step_weight($workflow_id, $step_id);
  $select = db_select('islandora_prepare_ingest_pifs', 'pifs');
  $select->fields('pifs', array('path'));
  $select->condition('pifs.workflow_id', $workflow_id)
    ->condition('pifs.other_id', $other_id);
  $select->join('islandora_prepare_ingest_step', 'step', 'step.id = pifs.step_id AND step.workflow_id = :workflowid AND step.weight <= :weight', array(':workflowid' => $workflow_id, ':weight' => $stepweight));
  $select->groupBy('path')
    ->orderBy('path')
    ->range($min-1, $max);
  $dcs = $select->execute();

  $results = array();
  $itemnr = $min;
  while ($dc = $dcs->fetchAssoc()) {
    $results[] = array('item nr' => $itemnr, 'filepath' => $dc['path']);
    $itemnr++;
  }
  return $results;
}

function pifs_entry_for_filepath($filepath, $workflow_id, $step_id, $other_id) {
  $stepweight = _pifs_step_weight($workflow_id, $step_id);
  $filepath = pifs_cleanup_filepath($filepath);
  $select = db_select('islandora_prepare_ingest_pifs', 'pifs');
  $select->fields('pifs', array('path', 'valuetype', 'filevalue'));
  $select->condition('pifs.workflow_id', $workflow_id)
    ->condition('pifs.other_id', $other_id)
    ->condition('path', $filepath);
  $select->join('islandora_prepare_ingest_step', 'step', 'step.id = pifs.step_id AND step.workflow_id = :workflowid AND step.weight <= :weight', array(':workflowid' => $workflow_id, ':weight' => $stepweight));
  $select->orderBy('step.weight');
  $dcs = $select->execute();
  $entry = NULL;
  while ($dc = $dcs->fetchAssoc()) {
    if ($entry === NULL) {
      $entry = array('path' => $dc['path'], 'valuetype' => $dc['valuetype'], 'filevalue' => $dc['filevalue']);
    }
    else {
      if ($dc['valuetype'] & PIFS_IS_DIR) {
        // The new value is a directory. TODO check if previous was a directory?
        $entry = array('path' => $dc['path'], 'valuetype' => $dc['valuetype'], 'filevalue' => $dc['filevalue']); 
      }
      else if ($entry['valuetype'] & PIFS_HAS_REAL) {
        // There is already a file entry with a relation to a real file from a previous step:
        if ($dc['valuetype'] & PIFS_HAS_REAL) {
          // The pifs file is overwritten by another pifs file with a relation to a real file 
          $entry['filevalue'] = $dc['filevalue'];
        }
        else if ($dc['valuetype'] & PIFS_HAS_CONTENT) {
          // Content was added to the pifs file, but the existing pifs file has a relation to a real file:
          // So first read this content and then add the other content. 
          $entry['valuetype'] = PIFS_IS_FILE & PIFS_HAS_CONTENT;
          clearstatcache(TRUE, $entry['filevalue']); // make sure we get the right size.
          $fh = @fopen($entry['filevalue'], 'r');
          if ($fh !== FALSE) {
            $content = fread($fh, filesize($entry['filevalue']));
            fclose($fh);
          }
          $entry['filevalue'] = $content . $dc['filevalue'];
        }
      }
      else if ($entry['valuetype'] & PIFS_HAS_CONTENT) {
        // There is already a file entry with content from a previous step:
        if ($dc['valuetype'] & PIFS_HAS_CONTENT) {
          // The new entry is also content, so add it to the existing content.
          $entry['filevalue'] .= $dc['filevalue'];
        }
        else if ($dc['valuetype'] & PIFS_HAS_REAL) {
          // The new entry is a pifs file with a relation to a real file. This overwrites the old entry.
          $entry['valuetype'] = PIFS_IS_FILE & PIFS_HAS_REAL;
          $entry['filevalue'] = $dc['filevalue'];
        }
      }
    }
  }
  return $entry;
}

function pifs_entry_is_file($filepath, $workflow_id, $step_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  return pifs_entry_is_of_type($filepath, $workflow_id, $step_id, $other_id, 'file');
}

function pifs_entry_is_dir($filepath, $workflow_id, $step_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  return pifs_entry_is_of_type($filepath, $workflow_id, $step_id, $other_id, 'directory');
}

function pifs_entry_is_of_type($filepath, $workflow_id, $step_id, $other_id, $type) {
  $stepweight = _pifs_step_weight($workflow_id, $step_id);
  $select = db_select('islandora_prepare_ingest_pifs', 'pifs');
  $select->fields('pifs', array('path', 'valuetype'));
  $select->condition('pifs.workflow_id', $workflow_id)
    ->condition('pifs.other_id', $other_id)
    ->condition('path', $filepath);
  $select->join('islandora_prepare_ingest_step', 'step', 'step.id = pifs.step_id AND step.workflow_id = :workflowid AND step.weight <= :weight', array(':workflowid' => $workflow_id, ':weight' => $stepweight));
  $select->orderBy('step.weight', 'DESC');
  $entry = $select->execute()->fetchAssoc();
  if ($type === 'file') {
    return ($entry['valuetype'] & PIFS_IS_FILE);
  }
  if ($type === 'directory') {
    return ($entry['valuetype'] & PIFS_IS_DIR);
  }
  return FALSE;
}

function pifs_is_file($filepath, $workflow_id, $step_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  if (is_file($filepath)) {
    return TRUE;
  }
  return pifs_entry_is_file($filepath, $workflow_id, $step_id, $other_id);
}

function pifs_is_dir($filepath, $workflow_id, $step_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  if (is_dir($filepath)) {
    return TRUE;
  }
  return pifs_entry_is_dir($filepath, $workflow_id, $step_id, $other_id);
}

function pifs_entry_mkdir($filepath, $workflow_id, $step_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);

  if (pifs_entry_exists($filepath, $workflow_id, $step_id, $other_id)) {
    // TODO should raise error when this is not a directory?
    return;
  }
  $entry = array('valuetype' => PIFS_IS_DIR, 'path' => $filepath);

  pifs_store_entry($entry, $workflow_id, $step_id, $other_id);
}

function pifs_store_entry($entry, $workflow_id, $step_id, $other_id) {
  $fields = array('workflow_id', 'step_id', 'other_id', 'dctimestamp', 'valuetype', 'path', 'filevalue');
  $insert = db_insert('islandora_prepare_ingest_pifs')->fields($fields);
  $timestamp = REQUEST_TIME;
  $record = array(
    'workflow_id' => $workflow_id,
    'step_id'     => $step_id,
    'other_id'    => $other_id,
    'dctimestamp' => $timestamp,
    'valuetype'   => $entry['valuetype'],
    'path'        => $entry['path'],
    'filevalue'   => isset($entry['filevalue'])?$entry['filevalue']:NULL,
  );
  $insert->values($record);
  $insert->execute();
}

function pifs_entry_copy($sourcefile, $targetfile, $workflow_id, $step_id, $other_id) {
  $sourcefile = pifs_cleanup_filepath($sourcefile);
  $targetfile = pifs_cleanup_filepath($targetfile);
  $entry = NULL;
  if (pifs_is_dir($targetfile, $workflow_id, $step_id, $other_id)) {
    $targetfile = rtrim($targetfile, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR . basename($sourcefile);
  }
  $sourceentry = pifs_entry_for_filepath($sourcefile, $workflow_id, $step_id, $other_id);
  if ($sourceentry !== NULL) {
    $entry = $sourceentry;
    $entry['path'] = $targetfile;
    pifs_store_entry($entry, $workflow_id, $step_id, $other_id); 
  }
  elseif (is_file($sourcefile)) {
    $entry = array(
      'path'      => $targetfile,
      'valuetype' => PIFS_IS_FILE | PIFS_HAS_REAL,
      'filevalue' => $sourcefile
    );
    pifs_store_entry($entry, $workflow_id, $step_id, $other_id); 
  }
}

function pifs_entry_write_content($targetfile, $content, $workflow_id, $step_id, $other_id) {
  $targetfile = pifs_cleanup_filepath($targetfile);
  $entry = array(
    'path'      => $targetfile,
    'valuetype' => PIFS_IS_FILE | PIFS_HAS_CONTENT,
    'filevalue' => $content
  );
  pifs_store_entry($entry, $workflow_id, $step_id, $other_id); 
}

function pifs_persist_entries($workflow_id, $step_id, $other_id) {
  $results = array();
  $select = db_select('islandora_prepare_ingest_pifs', 'pifs');
  $select->fields('pifs', array('path', 'valuetype', 'filevalue'));
  $select->condition('workflow_id', $workflow_id)
    ->condition('other_id', $other_id)
    ->condition('step_id', $step_id);
  $dcs = $select->execute();
  while ($dc = $dcs->fetchAssoc()) {
    $success = FALSE;
    if ($dc['valuetype'] & PIFS_IS_DIR) {
      if (!is_dir($dc['path'])) {
        $success = mkdir($dc['path']);
      }
      else {
        $success = TRUE;
      }
    }
    else if ($dc['valuetype'] & PIFS_IS_FILE) {
      if ($dc['valuetype'] & PIFS_HAS_REAL) {
        $islinked = @link($dc['filevalue'], $dc['path']);
        if ($islinked) {
          $success = TRUE;
        }
        else {
          $iscopied = @copy($dc['filevalue'], $dc['path']);
          if ($iscopied) {
            $success = TRUE;
          }
        }
      }
      else if ($dc['valuetype'] & PIFS_HAS_CONTENT) {
        $fh = fopen($dc['path'], "a"); // always append to file
        if ($fh) {
          $contentlen = mb_strlen($dc['filevalue'], '8bit');
          $byteswritten = fwrite($fh, $dc['filevalue'], $contentlen);
          if (fclose($fh) === TRUE) {
            if ($contentlen === $byteswritten) {
              $success = TRUE;
            }
          }
        }
      }
    }
    if (!$success) {
      $results[] = array('text' => t("Failed to persist '@file'.", array('@file' => $dc['path'])), 'type' => 'run_error');
    }
  }
  return $results;
}

function pifs_entry_read_content($filepath, $workflow_id, $step_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  $entry = pifs_entry_for_filepath($filepath, $workflow_id, $step_id, $other_id);
  if ($entry !== NULL) {
    if (($entry['valuetype'] & PIFS_IS_FILE) && ($entry['valuetype'] & PIFS_HAS_CONTENT)) {
      return $entry['filevalue']; 
    }
    if (($entry['valuetype'] & PIFS_IS_FILE) && ($entry['valuetype'] & PIFS_HAS_REAL)) {
      clearstatcache(TRUE, $entry['filevalue']); // make sure we get the right size.
      $fh = @fopen($entry['filevalue'], 'r');
      if ($fh !== FALSE) {
         $content = fread($fh, filesize($entry['filevalue']));
         fclose($fh);
      }
      return $content;
    }
  }
  return NULL;
}

function pifs_full_filenames_from_directory($directorypath, $workflow_id, $step_id, $other_id, $filter = '', $recursive = FALSE, $includedirectories = FALSE) {
  $directorypath = pifs_cleanup_filepath($directorypath);
  $regexfilter = preg_replace('/\\*/', '[^\\/]*', str_replace('.', '\\.', $filter));
  $filenames = array();
  if ($dh = @opendir($directorypath)) {
    while (FALSE !== ($filename = readdir($dh))) {
      if ($filename !== '.' && $filename !== '..') {
        $filepath = $directorypath . DIRECTORY_SEPARATOR . $filename;
        if (is_file($filepath)) {
          if (strlen($filter) === 0) {
            $filenames[$filepath] = 1;
          }
          elseif (preg_match('/' . $regexfilter . '$/', $filepath)) {
            $filenames[$filepath] = 1;
          }
        }
        elseif (is_dir($filepath)) {
          if ($includedirectories) {
            $filenames[$filepath] = 1;
          }
          if ($recursive) {
            $dirfiles = pifs_full_filenames_from_directory($filepath, $workflow_id, $step_id, $other_id, $filter, $recursive, $includedirectories);
            foreach ($dirfiles as $dirfile) {
              $filenames[$dirfile] = 1;
            }
          }
        }
      }
    }
  }
  $origlength = strlen($directorypath);
  $stepweight = _pifs_step_weight($workflow_id, $step_id);
  $select = db_select('islandora_prepare_ingest_pifs', 'pifs');
  $select->fields('pifs', array('path', 'valuetype'));
  $select->condition('pifs.workflow_id', $workflow_id)
    ->condition('pifs.other_id', $other_id)
    ->condition('path', db_like($directorypath . DIRECTORY_SEPARATOR) . '%', 'LIKE');
  $select->join('islandora_prepare_ingest_step', 'step', 'step.id = pifs.step_id AND step.workflow_id = :workflowid AND step.weight <= :weight', array(':workflowid' => $workflow_id, ':weight' => $stepweight));
  $select->orderBy('step.weight');
  $dcs = $select->execute();
  while ($dc = $dcs->fetchAssoc()) {
    $newfilename = $dc['path'];
    if ($recursive || (strpos($newfilename, DIRECTORY_SEPARATOR, $origlength + 1) === FALSE)) {
      // either recursive, or should not have a directory separator after the length of the original directory path.
      if ($dc['valuetype'] & PIFS_IS_DIR) {
        if ($includedirectories) {
          $filenames[$newfilename] = 1;
        }
      }
      else {
        $filenames[$newfilename] = 1;
      }
    }
  }
  $filenames = array_keys($filenames);
  sort($filenames, SORT_STRING);
  return $filenames;
}

/**
 * Delete all old pifs values with a specific other id or that are more than 1 day old.
 */
function islandora_prepare_ingest_pifs_delete_old($other_id = NULL) {
  if ($other_id !== NULL) {
    db_delete('islandora_prepare_ingest_pifs')
      ->condition('other_id', $other_id)
      ->execute();
  }
  $timestamp = REQUEST_TIME - (24*60*60);
  db_delete('islandora_prepare_ingest_pifs')
    ->condition('dctimestamp', $timestamp, '<')
    ->execute();
}

