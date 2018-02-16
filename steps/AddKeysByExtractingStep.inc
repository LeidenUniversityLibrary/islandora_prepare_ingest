<?php

class AddKeysByExtractingStep extends PrepareIngestStep {
  
  public function label() {
    $title = t('Add key(s) by extracting');
    if ($this->hasConfigKey('source_key_name')) {
      $title .= ' ' . t('from value of key "@key"', array('@key' => $this->config['source_key_name']));
    }
    if ($this->hasConfigKey('target_keys_format')) {
      $title .= ' ' . t('with format "@format"', array('@format' => $this->config['target_keys_format']));
    }
    return $title;
  }

  public function arguments() {
    return array(
      'source_key_name' => array(
        'label' => t('Source key name'),
        'type' => 'key',
      ),
      'target_keys_format' => array(
        'label' => t('Format'),
        'description' => t('The format should reflect the value of the source key. The keys are in the place of the extracted value and between curly braces. Example: if the value is "test_1.tif" and you want to extract both the number and the extension, then you can use the following: test_{number}.{extension}. If you know that a value only contains specific characters, you can supply this by using a range between brackets. For example for the number you can use {number[0-9]}.'),
        'type' => 'keystemplate',
        'allow_constants' => TRUE,
      ),
    );
  }

  public function _inputKeyKeys() {
    return array('source_key_name');
  }

  public function outputKeys() {
    $extrKeys = $this->_extractKeysFromFormat();
    return array('target_keys_format' => array_keys($extrKeys));
  }

  private function _extractKeysFromFormat() {
    $outputkeys = array();
    if ($this->hasConfigKey('target_keys_format')) {
      $format = $this->config['target_keys_format'];
      $matches = array();
      if (preg_match_all('/{([a-zA-Z0-9_-]+)(\\[([^\\[\\]]+)\\])?}/', $format, $matches, PREG_SET_ORDER)) {
        foreach ($matches as $match) {
          $key = $match[1];
          $range = isset($match[3])?$match[3]:NULL;
          $outputkeys[$key] = $range;
        }
      }
    }
    return $outputkeys;
  }

  public function changesCount() {
    return FALSE;
  }

  public function changesKeys() {
    return TRUE;
  }

  public function changesFiles() {
    return FALSE;
  }

  private function _formatRegex($outputkeys) {
    $format = preg_quote($this->config['target_keys_format'], '/');
    foreach ($outputkeys as $keyname => $range) {
      if (isset($range)) {
        $norm = array('-', '^');
        $trans = array('HHyPHeNN', 'CiRCuMFLeX');
        $rangeregex = str_replace($trans, $norm, preg_quote(str_replace($norm, $trans, $range), '/'));
        $format = str_replace('\\{' . $keyname . '\\[' . preg_quote($range, '/') . '\\]\\}', "([$rangeregex]+)", $format);
      }
      else {
        $format = str_replace('\\{' . $keyname . '\\}', "(.+)", $format);
      }
    }
    return $format;
  }

  public function checkConfiguration() {
    $result = parent::checkConfiguration();
    $outputkeys = $this->_extractKeysFromFormat();
    $format = $this->_formatRegex($outputkeys);
    if (@preg_match('/' . $format . '/', 'test string') === FALSE) {
      $result[] = array('text' => t("Format '@value' is not correct, use for example [a-z] for a range.",
                   array('@value' => $this->config['target_keys_format'])), 'type' => 'argument_error');
    }
    return $result;
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('source_key_name') && $this->hasConfigKey('target_keys_format') && isset($prevdatastep)) {
      $sourcekey = $this->config['source_key_name'];
      $outputkeys = $this->_extractKeysFromFormat();
      $format = $this->_formatRegex($outputkeys);

      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        if (array_key_exists($sourcekey, $item)) {
          $outputvalues = array();
          if (preg_match('/' . $format . '/', $item[$sourcekey], $outputvalues)) {
            array_shift($outputvalues);
            $output = array_combine(array_keys($outputkeys), $outputvalues);
            foreach ($output as $key => $value) {
              $item[$key] = $value;
            }
          }
          $this->saveDataItem($item);
          $item = $prevdatastep->nextDataItem();
        }
      }
    }
    return FALSE;
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    // no need to do anything
  }

}
