<?php

/**
 * @file
 * Prepare Ingest File System.
 *
 *
 *  Copyright 2017-2019 Leiden University Library
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

CONST PIFS_IS_FILE         = 0b00010000; // 16
CONST PIFS_IS_DIR          = 0b00100000; // 32
CONST PIFS_FROM_ARCHIVE    = 0b10000000; // 128
CONST PIFS_IS_REMOVED      = 0b00001000; // 8
CONST PIFS_OVERWRITTEN     = 0b00000100; // 4
CONST PIFS_HAS_CONTENT     = 0b00000010; // 2
CONST PIFS_HAS_REAL        = 0b00000001; // 1

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
  $select->distinct();
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
    ->range(($min - 1), ($max - $min + 1));
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
  $select->fields('pifs', array('path', 'valuetype', 'filevalue', 'dctimestamp'));
  $select->condition('pifs.workflow_id', $workflow_id)
    ->condition('pifs.other_id', $other_id)
    ->condition('path', $filepath);
  $select->join('islandora_prepare_ingest_step', 'step', 'step.id = pifs.step_id AND step.workflow_id = :workflowid AND step.weight <= :weight', array(':workflowid' => $workflow_id, ':weight' => $stepweight));
  $select->orderBy('step.weight');
  $dcs = $select->execute();
  $entry = NULL;
  while ($dc = $dcs->fetchAssoc()) {
    if ($entry === NULL) {
      $entry = array('path' => $dc['path'], 'valuetype' => $dc['valuetype'], 'filevalue' => $dc['filevalue'], 'timestamp' => $dc['dctimestamp']);
    }
    else {
      if ($dc['valuetype'] & PIFS_IS_DIR) {
        // The new value is a directory. TODO check if previous was a directory?
        $entry = array('path' => $dc['path'], 'valuetype' => $dc['valuetype'], 'filevalue' => $dc['filevalue'], 'timestamp' => $dc['dctimestamp']);
      }
      else if ($entry['valuetype'] & PIFS_HAS_REAL) {
        // There is already a file entry with a relation to a real file from a previous step:
        if ($dc['valuetype'] & PIFS_IS_REMOVED) {
          // The file is removed, this also removes an existing file.
          $entry['valuetype'] = $dc['valuetype'];
          $entry['filevalue'] = $dc['filevalue'];
          $entry['timestamp'] = $dc['dctimestamp'];
        }
        else if ($dc['valuetype'] & PIFS_FROM_ARCHIVE) {
          // An archive file always overwrites an existing file.
          $entry['valuetype'] = $dc['valuetype'];
          $entry['filevalue'] = $dc['filevalue'];
          $entry['timestamp'] = $dc['dctimestamp'];
        }
        else if ($dc['valuetype'] & PIFS_HAS_REAL) {
          // The pifs file is overwritten by another pifs file with a relation to a real file
          $entry['valuetype'] = $dc['valuetype'];
          $entry['filevalue'] = $dc['filevalue'];
          $entry['timestamp'] = $dc['dctimestamp'];
        }
        else if ($dc['valuetype'] & PIFS_HAS_CONTENT) {
          // Content was added to the pifs file, but the existing pifs file has a relation to a real file:
          // should the added content overwrite the real file?
          if ($dc['valuetype'] & PIFS_OVERWRITTEN) {
            // Yes, the added content overwrites the existing content.
            $entry['valuetype'] = $dc['valuetype'];
            $entry['filevalue'] = $dc['filevalue'];
            $entry['timestamp'] = $dc['dctimestamp'];
          }
          else {
            // no, so first read this content and then add the other content.
            $content = '';
            if ($entry['valuetype'] & PIFS_IS_REMOVED) {
              // previous file entry is removed, so content stays empty.
            }
            else if ($entry['valuetype'] & PIFS_FROM_ARCHIVE) {
              $content = pifs_archive_read_content_from_file($entry, $workflow_id, $step_id, $other_id);
            }
            else if ($entry['valuetype'] & PIFS_IS_FILE) {
              clearstatcache(TRUE, $entry['filevalue']); // make sure we get the right size.
              $fh = @fopen($entry['filevalue'], 'r');
              if ($fh !== FALSE) {
                $content = fread($fh, filesize($entry['filevalue']));
                fclose($fh);
              }
            }
            $entry['valuetype'] = PIFS_IS_FILE | PIFS_HAS_CONTENT;
            $entry['filevalue'] = $content . $dc['filevalue'];
            $entry['timestamp'] = $dc['dctimestamp'];
          }
        }
      }
      else if ($entry['valuetype'] & PIFS_HAS_CONTENT) {
        // There is already a file entry with content from a previous step:
        if ($dc['valuetype'] & PIFS_IS_REMOVED) {
          // The new entry is removed, so remove filevalue.
          $entry['valuetype'] = $dc['valuetype'];
          $entry['filevalue'] = '';
          $entry['timestamp'] = $dc['dctimestamp'];
        }
        else if ($dc['valuetype'] & PIFS_HAS_CONTENT) {
          if ($dc['valuetype'] & PIFS_OVERWRITTEN) {
            // The new entry is also content, but overwritten, so replace.
            $entry['filevalue'] = $dc['filevalue'];
            $entry['timestamp'] = $dc['dctimestamp'];
          }
          else {
            // The new entry is also content, so add it to the existing content.
            $entry['filevalue'] .= $dc['filevalue'];
            $entry['timestamp'] = $dc['dctimestamp'];
          }
        }
        else if ($dc['valuetype'] & PIFS_HAS_REAL) {
          // The new entry is a pifs file with a relation to a real file. This overwrites the old entry.
          $entry['valuetype'] = $dc['valuetype'];
          $entry['filevalue'] = $dc['filevalue'];
          $entry['timestamp'] = $dc['dctimestamp'];
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
  if ($entry === FALSE) {
    return NULL;
  }
  if ($entry['valuetype'] & PIFS_IS_REMOVED) {
    // Removed files or directories are neither file or directory.
    return FALSE;
  }
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
  $result = pifs_entry_is_file($filepath, $workflow_id, $step_id, $other_id);
  if ($result === NULL) {
    if (is_file($filepath)) {
      return TRUE;
    }
  }
  return $result;
}

function pifs_is_dir($filepath, $workflow_id, $step_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  $rsult = pifs_entry_is_dir($filepath, $workflow_id, $step_id, $other_id);
  if ($result === NULL) {
    if (is_dir($filepath)) {
      return TRUE;
    }
  }
  return $result;
}

function pifs_filesize($filepath, $workflow_id, $step_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  $entry = pifs_entry_for_filepath($filepath, $workflow_id, $step_id, $other_id);
  if ($entry === NULL) {
    if (is_file($filepath)) {
      clearstatcache(TRUE, $filepath); // make sure we get the right size.
      return filesize($filepath);
    }
    return FALSE;
  }
  if ($entry['valuetype'] & PIFS_IS_REMOVED) {
    return FALSE;
  }
  else if ($entry['valuetype'] & PIFS_FROM_ARCHIVE) {
    pifs_archive_extract_if_needed($entry, $workflow_id, $step_id, $other_id);
    $sourcefile = pifs_archive_get_temporary_filepath($entry);
    if ($sourcefile) {
      clearstatcache(TRUE, $sourcefile); // make sure we get the right size.
      return filesize($sourcefile);
    }
  }
  else if ($entry['valuetype'] & PIFS_IS_FILE) {
    if ($entry['valuetype'] & PIFS_HAS_REAL) {
      clearstatcache(TRUE, $entry['filevalue']); // make sure we get the right size.
      return filesize($entry['filevalue']);
    }
    elseif ($entry['valuetype'] & PIFS_HAS_CONTENT) {
      return strlen($entry['filevalue']);
    }
  }
  return FALSE;
}

function pifs_filemtime($filepath, $workflow_id, $step_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  $entry = pifs_entry_for_filepath($filepath, $workflow_id, $step_id, $other_id);
  if ($entry === NULL) {
    if (is_file($filepath)) {
      clearstatcache(TRUE, $filepath); // make sure we get the right modification date.
      return filemtime($filepath);
    }
    return FALSE;
  }
  if ($entry['valuetype'] & PIFS_IS_REMOVED) {
    return FALSE;
  }
  else if ($entry['valuetype'] & PIFS_FROM_ARCHIVE) {
    pifs_archive_extract_if_needed($entry, $workflow_id, $step_id, $other_id);
    $sourcefile = pifs_archive_get_temporary_filepath($entry);
    if ($sourcefile) {
      clearstatcache(TRUE, $sourcefile); // make sure we get the right modification date.
      return filemtime($sourcefile);
    }
  }
  else if ($entry['valuetype'] & PIFS_IS_FILE) {
    if ($entry['valuetype'] & PIFS_HAS_REAL) {
      clearstatcache(TRUE, $entry['filevalue']); // make sure we get the right modification date.
      return filemtime($entry['filevalue']);
    }
    elseif ($entry['valuetype'] & PIFS_HAS_CONTENT) {
      return $entry['timestamp'];
    }
  }
  return FALSE;
}

function pifs_entry_mkdir($filepath, $workflow_id, $step_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);

  $entry = pifs_entry_for_filepath($filepath, $workflow_id, $step_id, $other_id);
  if (($entry !== NULL) && (!($entry['valuetype'] & PIFS_IS_REMOVED))) {
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
    if (!($entry['valuetype'] & PIFS_IS_REMOVED)) {
      $entry = $sourceentry;
      $entry['path'] = $targetfile;
      pifs_store_entry($entry, $workflow_id, $step_id, $other_id);
    }
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

function pifs_entry_recursive_copy($sourcefile, $targetfile, $workflow_id, $step_id, $other_id) {
  $sourcefile = pifs_cleanup_filepath($sourcefile);
  $targetfile = pifs_cleanup_filepath($targetfile);
  if (pifs_is_dir($sourcefile, $workflow_id, $step_id, $other_id)) {
    if (!pifs_is_dir($targetfile, $workflow_id, $step_id, $other_id)) {
      pifs_entry_mkdir($targetfile, $workflow_id, $step_id, $other_id);
    }
    $inners = pifs_full_filenames_from_directory($sourcefile, $workflow_id, $step_id, $other_id, '', FALSE, TRUE);
    foreach ($inners as $inner) {
      $newto = $targetfile . DIRECTORY_SEPARATOR . basename($inner);
      pifs_entry_recursive_copy($inner, $newto, $workflow_id, $step_id, $other_id);
    }
  }
  else {
    pifs_entry_copy($sourcefile, $targetfile, $workflow_id, $step_id, $other_id);
  }
}

function pifs_entry_remove($sourcefile, $workflow_id, $step_id, $other_id) {
  $sourcefile = pifs_cleanup_filepath($sourcefile);
  $sourceentry = pifs_entry_for_filepath($sourcefile, $workflow_id, $step_id, $other_id);
  if ($sourceentry !== NULL) {
    $sourceentry['valuetype'] = $sourceentry['valuetype'] | PIFS_IS_REMOVED;
  }
  else {
    $sourceentry = array(
      'path'      => $sourcefile,
      'valuetype' => (is_file($sourcefile)?PIFS_IS_FILE:PIFS_IS_DIR) | PIFS_HAS_REAL | PIFS_IS_REMOVED,
      'filevalue' => $sourcefile
    );
  }
  pifs_store_entry($sourceentry, $workflow_id, $step_id, $other_id);
}

function pifs_entry_recursive_remove($sourcefile, $workflow_id, $step_id, $other_id) {
  $sourcefile = pifs_cleanup_filepath($sourcefile);
  if (pifs_is_dir($sourcefile, $workflow_id, $step_id, $other_id)) {
    $inners = pifs_full_filenames_from_directory($sourcefile, $workflow_id, $step_id, $other_id, '', FALSE, TRUE);
    foreach ($inners as $inner) {
      pifs_entry_recursive_remove($inner, $workflow_id, $step_id, $other_id);
    }
  }
  pifs_entry_remove($sourcefile, $workflow_id, $step_id, $other_id);
}

function pifs_entry_write_content($targetfile, $content, $workflow_id, $step_id, $other_id, $do_append = true) {
  $targetfile = pifs_cleanup_filepath($targetfile);
  $entry = array(
    'path'      => $targetfile,
    'valuetype' => PIFS_IS_FILE | ($do_append?PIFS_HAS_CONTENT:(PIFS_HAS_CONTENT | PIFS_OVERWRITTEN)),
    'filevalue' => $content
  );
  pifs_store_entry($entry, $workflow_id, $step_id, $other_id);
}

function pifs_persist_entries($workflow_id, $step_id, $other_id, $exists_action = 'append') {
  $results = array();
  $select = db_select('islandora_prepare_ingest_pifs', 'pifs');
  $select->fields('pifs', array('path', 'valuetype', 'filevalue'));
  $select->condition('workflow_id', $workflow_id)
    ->condition('other_id', $other_id)
    ->condition('step_id', $step_id);
  $dcs = $select->execute();
  $removeddirs = array();
  while ($dc = $dcs->fetchAssoc()) {
    $success = FALSE;
    if ($dc['valuetype'] & PIFS_IS_REMOVED) {
      if ($dc['valuetype'] & PIFS_IS_FILE) {
        if (is_file($dc['path'])) {
          $success = @unlink($dc['path']);
        }
        else {
          if (!is_dir($dc['path'])) {
            $success = TRUE;
          }
        }
      }
      else if ($dc['valuetype'] & PIFS_IS_DIR) {
        if (is_dir($dc['path'])) {
          $removeddirs[$dc['path']] = 1;
        }
        else {
          if (!is_file($dc['path'])) {
            $success = TRUE;
          }
        }
      }
    }
    else if ($dc['valuetype'] & PIFS_IS_DIR) {
      if (isset($removeddirs[$dc['path']])) {
        // If the dir was previously marked as removed, unmark it.
        unset($removeddirs[$dc['path']]);
      }
      if (!is_dir($dc['path'])) {
        $success = mkdir($dc['path']);
      }
      else {
        $success = TRUE;
      }
    }
    else if ($dc['valuetype'] & PIFS_IS_FILE) {
      if ($dc['valuetype'] & PIFS_FROM_ARCHIVE) {
        $success = pifs_archive_persist_entry($dc, $workflow_id, $step_id, $other_id);
      }
      else if ($dc['valuetype'] & PIFS_HAS_REAL) {
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
        if ($exists_action === 'append') {
          $open_mode = 'a';
        }
        elseif ($exists_action === 'overwrite') {
          $open_mode = 'w';
        }
        elseif ($exists_action === 'error') {
          $open_mode = 'x';
        }
        $fh = fopen($dc['path'], $open_mode);
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
  // remove the directories last...
  foreach ($removeddirs as $removeddir) {
    if (!rmdir($removeddir)) {
      $results[] = array('text' => t("Failed to remove '@file'.", array('@file' => $removeddir)), 'type' => 'run_error');
    }
  }
  return $results;
}

function pifs_entry_read_content($filepath, $workflow_id, $step_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  $entry = pifs_entry_for_filepath($filepath, $workflow_id, $step_id, $other_id);
  if ($entry !== NULL) {
    if ($entry['valuetype'] & PIFS_IS_REMOVED) {
      return NULL;
    }
    else if (($entry['valuetype'] & PIFS_IS_FILE) && ($entry['valuetype'] & PIFS_HAS_CONTENT)) {
      return $entry['filevalue'];
    }
    if (($entry['valuetype'] & PIFS_IS_FILE) && ($entry['valuetype'] & PIFS_HAS_REAL)) {
      if ($entry['valuetype'] & PIFS_FROM_ARCHIVE) {
        $content = pifs_archive_read_content_from_file($entry, $workflow_id, $step_id, $other_id);
      }
      else {
        clearstatcache(TRUE, $entry['filevalue']); // make sure we get the right size.
        $fh = @fopen($entry['filevalue'], 'r');
        if ($fh !== FALSE) {
           $content = fread($fh, filesize($entry['filevalue']));
           fclose($fh);
        }
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
          elseif (preg_match('/' . $regexfilter . '$/i', $filepath)) {
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
      if ($dc['valuetype'] & PIFS_IS_REMOVED) {
        unset($filenames[$newfilename]);
      }
      else if ($dc['valuetype'] & PIFS_IS_DIR) {
        if ($includedirectories) {
          $filenames[$newfilename] = 1;
        }
      }
      else {
        if (strlen($filter) === 0) {
          $filenames[$newfilename] = 1;
        }
        elseif (preg_match('/' . $regexfilter . '$/i', $newfilename)) {
          $filenames[$newfilename] = 1;
        }
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

/**
 * Read the contents from a single file within an archive.
 */
function pifs_archive_read_content_from_file($entry, $workflow_id, $step_id, $other_id, $maxreadlength = PHP_INT_MAX) {
  $content = '';
  if ($entry['valuetype'] & PIFS_FROM_ARCHIVE) {
    pifs_archive_extract_if_needed($entry, $workflow_id, $step_id, $other_id);
    $sourcepath = pifs_archive_get_temporary_filepath($entry);
    if ($sourcepath) {
      clearstatcache(TRUE, $sourcepath); // make sure we get the right size.
      $fh = @fopen($sourcepath, 'r');
      if ($fh !== FALSE) {
        $filesize = min(filesize($sourcepath), $maxreadlength);
        $content = fread($fh, filesize($sourcepath));
        fclose($fh);
      }
    }
  }
  return $content;
}

/**
 * Write the contents of a single file within an archive to a path.
 */
function pifs_archive_persist_entry($entry, $workflow_id, $step_id, $other_id) {
  if ($entry['valuetype'] & PIFS_FROM_ARCHIVE) {
    pifs_archive_extract_if_needed($entry, $workflow_id, $step_id, $other_id);
    $sourcepath = pifs_archive_get_temporary_filepath($entry);
    if ($sourcepath) {
      $islinked = @link($sourcepath, $dc['path']);
      if ($islinked) {
        return TRUE;
      }
      else {
        $iscopied = @copy($sourcepath, $dc['path']);
        if ($iscopied) {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

/**
 * Extract all the contents from an archive into a temporary location so a single entry can be retrieved but only if needed.
 */
function pifs_archive_extract_if_needed($entry, $workflow_id, $step_id, $other_id) {
  if ($entry['valuetype'] & PIFS_FROM_ARCHIVE) {
    $sourcepath = pifs_archive_get_temporary_filepath($entry);
    if ($sourcepath && !is_file($sourcepath) && !is_dir($sourcepath)) {
      $archivepath = pifs_archive_get_archivepath($entry);
      if ($archivepath) {
        pifs_archive_extract_files($archivepath, $outputdirectory, $workflow_id, $step_id, $other_id);
      }
    }
  }
}

/**
 * Extract all the contents from an archive into a temporary location.
 */
function pifs_archive_extract_files($archivepath, $outputdirectory, $workflow_id, $step_id, $other_id) {
  $archiveinfo = pathinfo($archivepath);
  if ($archiveinfo['extension'] === 'zip') {
    $filepaths = pifs_archive_unzip_to_temporary_directory($archivepath, $workflow_id, $step_id, $other_id);
  }
  else if ($archiveinfo['extension'] === 'gz') {
    $filepaths = pifs_archive_ungz_to_temporary_directory($archivepath, $workflow_id, $step_id, $other_id);
  }
  else if ($archiveinfo['extension'] === 'rar') {
    $filepaths = pifs_archive_unrar_to_temporary_directory($archivepath, $workflow_id, $step_id, $other_id);
  }
  else if ($archiveinfo['extension'] === 'tar') {
    $filepaths = pifs_archive_untar_to_temporary_directory($archivepath, $workflow_id, $step_id, $other_id);
  }
  else {
    $filepaths = FALSE;
  }
  if ($filepaths && count($filepaths) > 0) {
    $onlytarfiles = TRUE;
    $firsttarfile = NULL;
    foreach ($filepaths as $filepath => $temporary) {
      if ($filepath === '.') {
        continue;
      }
      $archiveinfo = pathinfo($temporary);
      if ($archiveinfo['extension'] === 'tar') {
        $firsttarfile = $temporary;
      }
      else {
        $onlytarfiles = FALSE;
      }
    }
    if ($onlytarfiles) {
      $filepaths = pifs_archive_untar_to_temporary_directory($firsttarfile, $workflow_id, $step_id, $other_id);
    }
  }
  return $filepaths;
}

/**
 * Extract all the contents from an archive into a temporary location and make entries.
 */
function pifs_archive_extract_make_entries($archivepath, $outputdirectory, $workflow_id, $step_id, $other_id) {
  $filepaths = pifs_archive_extract_files($archivepath, $outputdirectory, $workflow_id, $step_id, $other_id);
  if ($filepaths === FALSE) {
    return FALSE;
  }
  foreach ($filepaths as $filepath => $temporary) {
    if ($filepath === '.') {
      continue;
    }
    $filevalue = array(
      'archivepath' => $archivepath,
      'temporary'   => $temporary,
    );
    $targetfile = pifs_cleanup_filepath($outputdirectory . DIRECTORY_SEPARATOR . $filepath);
    $entry = array(
      'path'      => $targetfile,
      'filevalue' => serialize($filevalue),
    );
    if (substr($filepath, -1) === DIRECTORY_SEPARATOR) {
      $entry['valuetype'] = PIFS_IS_DIR | PIFS_HAS_REAL | PIFS_FROM_ARCHIVE;
    }
    else {
      $entry['valuetype'] = PIFS_IS_FILE | PIFS_HAS_REAL | PIFS_FROM_ARCHIVE;
    }
    pifs_store_entry($entry, $workflow_id, $step_id, $other_id);
  }
  return TRUE;
}

/**
 * Extract all the contents from a zip archive into a temporary location.
 */
function pifs_archive_unzip_to_temporary_directory($archivepath, $workflow_id, $step_id, $other_id) {
  if (!class_exists('ZipArchive')) {
    return FALSE;
  }
  $tmpdir = pifs_cleanup_filepath( file_directory_temp() . DIRECTORY_SEPARATOR . 'pifs_unzipped_' . $workflow_id . '_' . $step_id . '_' . $other_id );
  $zip = new ZipArchive();
  if ($zip->open($archivepath) !== TRUE) {
    return FALSE;
  }
  $filepaths = array('.' => $tmpdir);
  if (!is_dir($tmpdir)) {
    mkdir($tmpdir);
  }
  if (!$zip->extractTo($tmpdir)) {
    return FALSE;
  }
  $count = $zip->numFiles;
  for ($i = 0; $i < $count; $i++) {
    $name = $zip->getNameIndex($i);
    $realpath = pifs_cleanup_filepath($tmpdir . DIRECTORY_SEPARATOR . $name);
    $filepaths[$name] = $realpath;
  }
  return $filepaths;
}

/**
 * Extract all the contents from a rar archive into a temporary location.
 */
function pifs_archive_unrar_to_temporary_directory($archivepath, $workflow_id, $step_id, $other_id) {
  if (!class_exists('RarArchive')) {
    return FALSE;
  }
  $tmpdir = pifs_cleanup_filepath( file_directory_temp() . DIRECTORY_SEPARATOR . 'pifs_unrarred_' . $workflow_id . '_' . $step_id . '_' . $other_id );
  $rar_file = RarArchive::open($archivepath);
  if ($rar_file === FALSE) {
    return FALSE;
  }
  $filepaths = array('.' => $tmpdir);
  if (!is_dir($tmpdir)) {
    mkdir($tmpdir);
  }
  $entries = $rar_file->getEntries();
  if ($entries === FALSE) {
    return FALSE;
  }
  foreach ($entries as $entry) {
    $name = $entry->getName();
    $filepaths[$name] = pifs_cleanup_filepath($tmpdir . DIRECTORY_SEPARATOR . $name);
    $entry->extract($tmpdir);
  }
  $rar_file->close();

  return $filepaths;
}

/**
 * Extract all the contents from a gz archive into a temporary location.
 */
function pifs_archive_ungz_to_temporary_directory($archivepath, $workflow_id, $step_id, $other_id) {
  return pifs_archive_unphar_to_temporary_directory($archivepath, 'pifs_ungzed_', 'decompress', $workflow_id, $step_id, $other_id);
}

/**
 * Extract all the contents from a tar archive into a temporary location.
 */
function pifs_archive_untar_to_temporary_directory($archivepath, $workflow_id, $step_id, $other_id) {
  return pifs_archive_unphar_to_temporary_directory($archivepath, 'pifs_untarred_', 'extractTo', $workflow_id, $step_id, $other_id);
}

/**
 * Extract all the contents from a phar archive into a temporary location.
 */
function pifs_archive_unphar_to_temporary_directory($archivepath, $tmpprefix, $unarchivefunc, $workflow_id, $step_id, $other_id) {
  if (!class_exists('PharData')) {
    return FALSE;
  }
  $basetmpdir = pifs_cleanup_filepath( file_directory_temp() . DIRECTORY_SEPARATOR . $tmpprefix . $workflow_id . '_' . $step_id . '_' . $other_id );
  $i = 0;
  $tmpdir = $basetmpdir . "_$i";
  do {
    $i++;
    $tmpdir = $basetmpdir . "_$i";
  } while (!@mkdir($tmpdir) && $i < 1000);
  if (!($i < 1000)) {
    return FALSE;
  }
  $filepaths = array('.' => $tmpdir);
  $tmparchivepath = $tmpdir . DIRECTORY_SEPARATOR . basename($archivepath);
  try {
    if (!copy($archivepath, $tmparchivepath)) {
      return FALSE;
    }
    $phar = new PharData($tmparchivepath);
    if ($unarchivefunc === 'decompress') {
      $phar->decompress();
    }
    else if ($unarchivefunc === 'extractTo') {
      $phar->extractTo($tmpdir);
    }
    if (!unlink($tmparchivepath)) {
      return FALSE;
    }
  }
  catch (Exception $e) {
    return FALSE;
  }
  $fullpaths = pifs_full_filenames_from_directory($tmpdir, 0, 0, 0, '', TRUE, TRUE);
  foreach ($fullpaths as $fullpath) {
    $name = substr($fullpath, strlen($tmpdir) + 1);
    $filepaths[$name] = $fullpath;
  }
  return $filepaths;
}

/**
 * Get temporary filepath within extracted archive.
 */
function pifs_archive_get_temporary_filepath($entry) {
  if ($entry['valuetype'] & PIFS_FROM_ARCHIVE) {
    $filevalue = unserialize($entry['filevalue']);
    return isset($filevalue['temporary'])?$filevalue['temporary']:FALSE;
  }
  return FALSE;
}

/**
 * Get the original archive path.
 */
function pifs_archive_get_archivepath($entry) {
  if ($entry['valuetype'] & PIFS_FROM_ARCHIVE) {
    $filevalue = unserialize($entry['filevalue']);
    return isset($filevalue['archivepath'])?$filevalue['archivepath']:FALSE;
  }
  return FALSE;
}
