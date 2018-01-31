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

function pifs_file_exists($filepath, $workflow_id, $other_id) {
  if (file_exists($filepath)) {
    return TRUE;
  }
  return pifs_entry_exists($filepath, $workflow_id, $other_id);
}

function pifs_entry_exists($filepath, $workflow_id, $other_id) {
  $filepath = rtrim($filepath, DIRECTORY_SEPARATOR);
  $files = islandora_prepare_ingest_data_cache_select_key_value($workflow_id, $other_id, 2, 'filepath', $filepath);
  return (count($files) > 0);
}

function pifs_entry_is_file($filepath, $workflow_id, $other_id) {
  return pifs_entry_is_of_type($filepath, $workflow_id, $other_id, 'file');
}

function pifs_entry_is_directory($filepath, $workflow_id, $other_id) {
  return pifs_entry_is_of_type($filepath, $workflow_id, $other_id, 'directory');
}

function pifs_entry_is_of_type($filepath, $workflow_id, $other_id, $type) {
  $files = islandora_prepare_ingest_data_cache_select_key_value($workflow_id, $other_id, 2, 'filepath', db_like($filepath));
  if (count($files) > 0) {
    $lastfile = end($files);
    $items = islandora_prepare_ingest_data_cache_select($workflow_id, $lastfile['step_id'], $other_id, 2, $lastfile['item_nr'], $lastfile['item_nr']);
    if (count($items) === 1) {
      $item = end($items);
      if ($item['type'] === $type) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

function pifs_is_file($filepath, $workflow_id, $other_id) {
  if (is_file($filepath)) {
    return TRUE;
  }
  return pifs_entry_is_file($filepath, $workflow_id, $other_id);
}

function pifs_is_dir($filepath, $workflow_id, $other_id) {
  $filepath = rtrim($filepath, DIRECTORY_SEPARATOR);
  if (is_dir($filepath)) {
    return TRUE;
  }
  return pifs_entry_is_directory($filepath, $workflow_id, $other_id);
}

function pifs_entry_mkdir($filepath, $workflow_id, $step_id, $other_id, $item_nr) {
  $filepath = rtrim($filepath, DIRECTORY_SEPARATOR);
  $item = array('filepath' => $filepath, 'type' => 'directory');
  islandora_prepare_ingest_data_cache_single_insert($workflow_id, $step_id, $other_id, 2, $item_nr, $item);
}

function pifs_entry_copy($sourcefile, $targetfile, $workflow_id, $step_id, $other_id, $item_nr) {
  $files = islandora_prepare_ingest_data_cache_select_key_value($workflow_id, $other_id, 2, 'filepath', $sourcefile);
  if (count($files) > 0) {
    $lastfile = end($files);
    $items = islandora_prepare_ingest_data_cache_select($workflow_id, $lastfile['step_id'], $other_id, 2, $lastfile['item_nr'], $lastfile['item_nr']);
    if (count($items) === 1) {
      $item = array('filepath' => $targetfile, 'type' => $items[0]['type']);
      if (isset($items[0]['content'])) {
        $item['content'] = $items[0]['content'];
      }
      islandora_prepare_ingest_data_cache_single_insert($workflow_id, $step_id, $other_id, 2, $item_nr, $item);
    }
  }
  elseif (is_file($sourcefile)) {
    $item = array('filepath' => $targetfile, 'type' => 'file', 'realfilepath' => $sourcefile);
    islandora_prepare_ingest_data_cache_single_insert($workflow_id, $step_id, $other_id, 2, $item_nr, $item);
  }
}

function pifs_entry_write_content($targetfile, $content, $workflow_id, $step_id, $other_id, $item_nr) {
  $item = array('filepath' => $targetfile, 'type' => 'file', 'content' => $content);
  islandora_prepare_ingest_data_cache_single_insert($workflow_id, $step_id, $other_id, 2, $item_nr, $item);
}

function pifs_write_content_to_file($data) {
  if (isset($data['filepath'], $data['type']) && $data['type'] === 'file') {
    if (isset($data['content'])) {
      $fh = fopen($data['filepath'], "a"); // always append to file
      if ($fh) {
        $contentlen = mb_strlen($data['content'], '8bit');
        $byteswritten = fwrite($fh, $data['content'], $contentlen);
        if (fclose($fh) === TRUE) {
          if ($contentlen === $byteswritten) {
            return TRUE;
          }
        }
      }
    }
    elseif (isset($data['realfilepath'])) {
      $islinked = link($data['realfilepath'], $data['filepath']);
      if ($islinked) {
        return TRUE;
      }
      else {
        $iscopied = copy($data['realfilepath'], $data['filepath']);
        if ($iscopied) {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

function pifs_entry_append_content($targetfile, $content, $workflow_id, $step_id, $other_id, $item_nr) {
  $existitemnr = '';
  $oldcontent = pifs_entry_read_content($targetfile, $workflow_id, $other_id, $existitemnr);
  if (is_string($oldcontent) && strlen($oldcontent) > 0) {
    $content = $oldcontent . $content;
    $item = array('filepath' => $targetfile, 'type' => 'file', 'content' => $content);
    islandora_prepare_ingest_data_cache_single_update($workflow_id, $step_id, $other_id, 2, $existitemnr, $item);
  }
  else {
    $item = array('content' => $content);
    islandora_prepare_ingest_data_cache_single_insert($workflow_id, $step_id, $other_id, 2, $item_nr, $item);
  }
}

function pifs_entry_read_content($filepath, $workflow_id, $other_id, &$itemnr) {
  $content = FALSE;
  $files = islandora_prepare_ingest_data_cache_select_key_value($workflow_id, $other_id, 2, 'filepath', $filepath);
  if (count($files) > 0) {
    $lastfile = end($files);
    $itemnr = $lastfile['item_nr'];
    $items = islandora_prepare_ingest_data_cache_select($workflow_id, $lastfile['step_id'], $other_id, 2, $itemnr, $itemnr);
    if (count($items) === 1) {
      $lastitem = end($items);
      if (isset($lastitem['content'])) {
        $content = $lastitem['content'];
      }
      elseif (isset($lastitem['realfilepath'])) { 
        $origfile = $lastitem['realfilepath'];
        clearstatcache(TRUE, $origfile); // make sure we get the right size.
        $fh = @fopen($origfile, 'r');
        if ($fh !== FALSE) {
          $content = fread($fh, filesize($origfile));
          fclose($fh);
        }
      }
    }
  }
  if ($content !== FALSE) {
    return $content;
  }
  return NULL;
}

function pifs_full_filenames_from_directory($directorypath, $workflow_id, $other_id, $filter = '', $recursive = FALSE, $includedirectories = FALSE) {
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
            $dirfiles = pifs_full_filenames_from_directory($filepath, $context, $filter, $recursive, $includedirectories);
            foreach ($dirfiles as $dirfile) {
              $filenames[$filepath] = 1;
            }
          }
        }
      }
    }
  }
  $directorypath = rtrim($directorypath, DIRECTORY_SEPARATOR);
  $dcfilenames = islandora_prepare_ingest_data_cache_select_key_value($workflow_id, $other_id, 2, 'filepath', $directorypath . DIRECTORY_SEPARATOR, TRUE);
  if (count($dcfilenames) > 0) {
    $origlength = strlen($directorypath);
    foreach ($dcfilenames as $dcfilename) {
      $items = islandora_prepare_ingest_data_cache_select($workflow_id, $dcfilename['step_id'], $other_id, 2, $dcfilename['item_nr'], $dcfilename['item_nr']);
      if (count($items) === 1) {
        $newfilename = $items[0]['filepath'];
        if ($recursive || (strpos($newfilename, DIRECTORY_SEPARATOR, $origlength + 1) === FALSE)) {
          // either recursive, or should not have a directory separator after the length of the original directory path.
          if ($items[0]['type'] === 'directory') {
            if ($includedirectories) {
              $filenames[$newfilename] = 1;
            }
          }
          else {
            $filenames[$newfilename] = 1;
          }
        }
      }
    }
  }
  $filenames = array_keys($filenames);
  sort($filenames, SORT_STRING);
  return $filenames;
}

