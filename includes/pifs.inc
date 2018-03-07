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

function pifs_cleanup_filepath($filepath) {
  $filepath = preg_replace('#' . DIRECTORY_SEPARATOR . DIRECTORY_SEPARATOR . '+#', DIRECTORY_SEPARATOR, $filepath);
  $filepath = rtrim($filepath, DIRECTORY_SEPARATOR);
  return $filepath;
}

function pifs_file_exists($filepath, $workflow_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  if (file_exists($filepath)) {
    return TRUE;
  }
  return pifs_entry_exists($filepath, $workflow_id, $other_id);
}

function pifs_entry_exists($filepath, $workflow_id, $other_id) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

  $filepath = pifs_cleanup_filepath($filepath);
  $files = islandora_prepare_ingest_data_cache_select_key_value($workflow_id, $other_id, 2, 'filepath', $filepath);
  return (count($files) > 0);
}

function pifs_entry_is_file($filepath, $workflow_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  return pifs_entry_is_of_type($filepath, $workflow_id, $other_id, 'file');
}

function pifs_entry_is_dir($filepath, $workflow_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  return pifs_entry_is_of_type($filepath, $workflow_id, $other_id, 'directory');
}

function pifs_entry_is_of_type($filepath, $workflow_id, $other_id, $type) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

  $files = islandora_prepare_ingest_data_cache_select_key_value($workflow_id, $other_id, 2, 'filepath', $filepath);
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
  $filepath = pifs_cleanup_filepath($filepath);
  if (is_file($filepath)) {
    return TRUE;
  }
  return pifs_entry_is_file($filepath, $workflow_id, $other_id);
}

function pifs_is_dir($filepath, $workflow_id, $other_id) {
  $filepath = pifs_cleanup_filepath($filepath);
  if (is_dir($filepath)) {
    return TRUE;
  }
  return pifs_entry_is_dir($filepath, $workflow_id, $other_id);
}

function pifs_entry_mkdir($filepath, $workflow_id, $step_id, $other_id, $item_nr) {
  $filepath = pifs_cleanup_filepath($filepath);
  $entry = NULL;
  if (!pifs_entry_is_dir($filepath, $workflow_id, $other_id)) {
    $filepath = rtrim($filepath, DIRECTORY_SEPARATOR);
    $entry = array('filepath' => $filepath, 'type' => 'directory');
  }
  return $entry;
}

function pifs_entry_copy($sourcefile, $targetfile, $workflow_id, $step_id, $other_id, $item_nr) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

  $sourcefile = pifs_cleanup_filepath($sourcefile);
  $targetfile = pifs_cleanup_filepath($targetfile);
  $entry = NULL;
  if (pifs_is_dir($targetfile, $workflow_id, $other_id)) {
    $targetfile = rtrim($targetfile, DIRECTORY_SEPARATOR) . DIRECTORY_SEPARATOR . basename($sourcefile);
  }
  $files = islandora_prepare_ingest_data_cache_select_key_value($workflow_id, $other_id, 2, 'filepath', $sourcefile);
  if (count($files) > 0) {
    $lastfile = end($files);
    $items = islandora_prepare_ingest_data_cache_select($workflow_id, $lastfile['step_id'], $other_id, 2, $lastfile['item_nr'], $lastfile['item_nr']);
    if (count($items) === 1) {
      $entry = array('filepath' => $targetfile, 'type' => $items[0]['type']);
      if (isset($items[0]['content'])) {
        $entry['content'] = $items[0]['content'];
      }
      elseif (isset($items[0]['realfilepath'])) {
        $entry['realfilepath'] = $items[0]['realfilepath'];
      }
    }
  }
  elseif (is_file($sourcefile)) {
    $entry = array('filepath' => $targetfile, 'type' => 'file', 'realfilepath' => $sourcefile);
  }
  return $entry;
}

function pifs_entry_write_content($targetfile, $content, $workflow_id, $step_id, $other_id, $item_nr) {
  $targetfile = pifs_cleanup_filepath($targetfile);
  return array('filepath' => $targetfile, 'type' => 'file', 'content' => $content);
}

function pifs_write_content_to_file($entry) {
  if (isset($entry['filepath'], $entry['type']) && $entry['type'] === 'file') {
    if (isset($entry['content'])) {
      $fh = fopen($entry['filepath'], "a"); // always append to file
      if ($fh) {
        $contentlen = mb_strlen($entry['content'], '8bit');
        $byteswritten = fwrite($fh, $entry['content'], $contentlen);
        if (fclose($fh) === TRUE) {
          if ($contentlen === $byteswritten) {
            return TRUE;
          }
        }
      }
    }
    elseif (isset($entry['realfilepath'])) {
      $islinked = link($entry['realfilepath'], $entry['filepath']);
      if ($islinked) {
        return TRUE;
      }
      else {
        $iscopied = copy($entry['realfilepath'], $entry['filepath']);
        if ($iscopied) {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

function pifs_entry_append_content($targetfile, $content, $workflow_id, $step_id, $other_id, $item_nr) {
  $targetfile = pifs_cleanup_filepath($targetfile);
  $existitemnr = '';
  $oldcontent = pifs_entry_read_content($targetfile, $workflow_id, $other_id, $existitemnr);
  if (is_string($oldcontent) && strlen($oldcontent) > 0) {
    $content = $oldcontent . $content;
  }
  $entry = array('filepath' => $targetfile, 'type' => 'file', 'content' => $content);
  return $entry;
}

function pifs_entry_read_content($filepath, $workflow_id, $other_id, &$itemnr) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

  $filepath = pifs_cleanup_filepath($filepath);
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

function pifs_mkdir($entry) {
  if (isset($entry['type'], $entry['filepath'])) {
    if ($entry['type'] === 'directory') {
      if (file_exists($entry['filepath'])) {
        return is_dir($entry['filepath']);
      }
      return mkdir($entry['filepath']);
    }
  }
  return FALSE;
}

function pifs_copy($entry) {
  if (isset($entry['type'], $entry['filepath'])) {
    if ($entry['type'] === 'file') {
      return pifs_write_content_to_file($entry);
    }
  }
  return FALSE;
}

function pifs_full_filenames_from_directory($directorypath, $workflow_id, $other_id, $filter = '', $recursive = FALSE, $includedirectories = FALSE) {
  module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

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
            $dirfiles = pifs_full_filenames_from_directory($filepath, $workflow_id, $other_id, $filter, $recursive, $includedirectories);
            foreach ($dirfiles as $dirfile) {
              $filenames[$dirfile] = 1;
            }
          }
        }
      }
    }
  }
  $directorypath = pifs_cleanup_filepath($directorypath);
  $dcfilenames = islandora_prepare_ingest_data_cache_select_key_value($workflow_id, $other_id, 2, 'filepath', $directorypath . DIRECTORY_SEPARATOR, TRUE);
  if (count($dcfilenames) > 0) {
    $origlength = strlen($directorypath);
    foreach ($dcfilenames as $dcfilename) {
      $newfilename = $dcfilename['value'];
      if ($recursive || (strpos($newfilename, DIRECTORY_SEPARATOR, $origlength + 1) === FALSE)) {
        // either recursive, or should not have a directory separator after the length of the original directory path.
        if (pifs_is_dir($newfilename, $workflow_id, $other_id)) {
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
  $filenames = array_keys($filenames);
  sort($filenames, SORT_STRING);
  return $filenames;
}

