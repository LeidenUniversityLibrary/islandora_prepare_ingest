<?php

/**
 * @file
 * Prepare Ingest File System.
 */

function pifs_file_exists($filepath, $context) {
  if (file_exists($filepath)) {
    return TRUE;
  }
  elseif  (array_key_exists('_PIFS', $context) && array_key_exists($filepath, $context['_PIFS'])) {
    return TRUE;
  }
  return FALSE;
}

function pifs_entry_exists($filepath, $context) {
  return (array_key_exists('_PIFS', $context) && array_key_exists($filepath, $context['_PIFS']));
}

function pifs_entry_is_file($filepath, $context) {
  return (array_key_exists('_PIFS', $context) && array_key_exists($filepath, $context['_PIFS']) 
          && array_key_exists('type', $context['_PIFS'][$filepath]) && $context['_PIFS'][$filepath]['type'] === 'file');
}

function pifs_is_file($filepath, $context) {
  if (is_file($filepath)) {
    return TRUE;
  }
  elseif  (array_key_exists('_PIFS', $context) && array_key_exists($filepath, $context['_PIFS'])) {
    if ($context['_PIFS'][$filepath]['type'] === 'file') {
      return TRUE;
    }
  }
  return FALSE;
}

function pifs_is_dir($filepath, $context) {
  if (is_dir($filepath)) {
    return TRUE;
  }
  elseif  (array_key_exists('_PIFS', $context) && array_key_exists($filepath, $context['_PIFS'])) {
    if ($context['_PIFS'][$filepath]['type'] === 'directory') {
      return TRUE;
    }
  }
  return FALSE;
}

function pifs_mkdir($filepath, &$context) {
  $context['_PIFS'][$filepath] = array('type' => 'directory');
}

function pifs_copy($sourcefile, $targetfile, &$context) {
  $context['_PIFS'][$targetfile] = array('type' => 'file', 'path' => $sourcefile);
}

function pifs_write_content($targetfile, $content, &$context) {
  $context['_PIFS'][$targetfile] = array('type' => 'file', 'content' => $content);
}

function pifs_append_content($targetfile, $content, &$context) {
  $context['_PIFS'][$targetfile]['content'] .= $content;
}

function pifs_read_content($filepath, $context) {
  if (pifs_entry_is_file($filepath, $context)) {
    if (array_key_exists('content', $context['_PIFS'][$filepath])) {
      return $context['_PIFS'][$filepath]['content'];
    }
    elseif (array_key_exists('path', $context['_PIFS'][$filepath])) {
      $origfile = $context['_PIFS'][$filepath]['path'];
      clearstatcache(TRUE, $filepath); // make sure we get the right size.
      $fh = @fopen($filepath, 'r');
      if ($fh !== FALSE) {
        $content = fread($fh, filesize($filepath));
        fclose($fh);

        return $content;
      }
    }
  }
  return NULL;
}

function pifs_full_filenames_from_directory($directorypath, $context, $filter = '', $recursive = FALSE, $includedirectories = FALSE) {
  $regexfilter = preg_replace('/\\*/', '[^\\/]*', str_replace('.', '\\.', $filter));
  $filenames = array();
  if ($dh = @opendir($directorypath)) {
    while (FALSE !== ($filename = readdir($dh))) {
      if ($filename !== '.' && $filename !== '..') {
        $filepath = $directorypath . DIRECTORY_SEPARATOR . $filename;
        if (is_file($filepath)) {
          if (strlen($filter) === 0) {
            $filenames[] = $filepath;
          }
          elseif (preg_match('/' . $regexfilter . '/', $filepath)) {
            $filenames[] = $filepath;
          }
        }
        elseif (is_dir($filepath)) {
          if ($includedirectories) {
            $filenames[] = $filepath;
          }
          if ($recursive) {
            $dirfiles = pifs_full_filenames_from_directory($filepath, $context, $filter, $recursive, $includedirectories);
            foreach ($dirfiles as $dirfile) {
              $filenames[] = $dirfile;
            }
          }
        }
      }
    }
  }
  if (array_key_exists('_PIFS', $context)) {
    foreach ($context['_PIFS'] as $filepath => $finfo) {
      if (substr($filepath, 0, strlen($directorypath)) === $directorypath) {
        if (strlen($filter) === 0) {
          $filenames[] = $filepath;
        } 
        elseif (preg_match('/' . $regexfilter . '/', $filepath)) {
          $filenames[] = $filepath;
        }
      }
    }
  }
  sort($filenames, SORT_STRING);
  return $filenames;
}

