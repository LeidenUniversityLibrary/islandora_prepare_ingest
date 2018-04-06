<?php

/**
 * @file
 * Constants handling.
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

function islandora_prepare_ingest_constants_set($name, $value, &$context) {
  if (!isset($context['results']['_constants'])) {
    $context['results']['_constants'] = array();
  }
  $key = '{' . $name . '}';
  if (isset($context['results']['_constants'][$key])) {
    $results[] = array('text' => t("Constant with name '@name' already exists, has value '@value'.",
                array('@name' => $name, '@value' => $context['results']['_constants'][$key])), 'type' => 'run_error');
    return $results;
  }
  $context['results']['_constants'][$key] = $value;
  return FALSE;
}

function islandora_prepare_ingest_constants_add_to_stepconfig(&$stepconfig, $context) {
  if (isset($context['results']['_constants'])) {
    $sckeys = array_keys($stepconfig);
    $ckeys = array_keys($context['results']['_constants']);
    $cvalues = array_values($context['results']['_constants']);
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
