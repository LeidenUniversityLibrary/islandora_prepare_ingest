<?php

/**
 * @file
 * Constants handling.
 *
 *
 *  Copyright 2017 Leiden University Library
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

function islandora_prepare_ingest_constants_set($name, $value, &$context) {
  if (!array_key_exists('_constants', $context)) {
    $context['_constants'] = array();
  }
  if (array_key_exists($name, $context['_constants'])) {
    $results[] = array('text' => t("Constant with name '@name' already exists, has value '@value'.",
                array('@name' => $name, '@value' => $value)), 'type' => 'run_error');
    return $results;
  }
  $context['_constants']['{' . $name . '}'] = $value;
  return FALSE;
}

function islandora_prepare_ingest_constants_add_to_stepconfig(&$stepconfig, $context) {
  if (array_key_exists('_constants', $context)) {
    $sckeys = array_keys($stepconfig);
    $ckeys = array_keys($context['_constants']);
    $cvalues = array_values($context['_constants']);
    foreach ($sckeys as $key) {
      $value = $stepconfig[$key];
      $newvalue = str_replace($ckeys, $cvalues, $value);
      if ($value !== $newvalue) {
        $stepconfig[$key] = $newvalue;
      }
    }
  }
  return $stepconfig;
}
