<?php

/**
 * @file
 * Constants handling.
 */

function ubl_prepare_ingest_constants_set($name, $value, &$context) {
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

function ubl_prepare_ingest_constants_add_to_stepconfig(&$stepconfig, $context) {
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
