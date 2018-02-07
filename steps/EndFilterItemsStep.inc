<?php

class EndFilterItemsStep extends PrepareIngestStep {
  
  public function label() {
    return t('Filter end');
  }

  public function arguments() {
    return array(
    );
  }

  public function _inputKeyKeys() {
    return array('filter_key_name');
  }

  public function outputKeys() {
    return array();
  }

  public function changesCount() {
    return TRUE;
  }

  public function changesKeys() {
    return FALSE;
  }

  public function changesFiles() {
    return FALSE;
  }

  public function checkConfiguration() {
    return parent::checkConfiguration();
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    // First search for the acompanying FilterItemStep:
    $startfilterdatastep = $this->_previousDataChangingStep();
    $depth = 1;
    while (isset($startfilterdatastep)) {
      if (is_a($startfilterdatastep, 'EndFilterItemsStep')) {
        $depth++;
      }
      elseif (is_a($startfilterdatastep, 'FilterItemsStep')) {
        $depth--;
      }
      if ($depth === 0) {
        break;
      }
      $startfilterdatastep = $startfilterdatastep->_previousDataChangingStep();
    }
    // If found, do the filtering again on it's children but replace the filtered items with the items of the previous step:
    if (isset($startfilterdatastep) && $startfilterdatastep->hasConfigKey('filter_key_name') && $startfilterdatastep->hasConfigKey('filter_type') && isset($prevdatastep)) {
      $filterkey = $startfilterdatastep->config['filter_key_name'];
      $type = $startfilterdatastep->config['filter_type'];
      $value = $startfilterdatastep->config['filter_value'];
      $negate = $startfilterdatastep->config['negate'];

      $startfilterdatastep->resetDataIterator(); ?
      $item = $startfilterdatastep->nextDataItem();
      $currentitemnr = 1;
      while ($item !== NULL) {
        $shouldkeep = FALSE;
        if ($type === 'exists') {
          $shouldkeep = isset($item[$filterkey]);
        }
        elseif ($type === 'equals') {
          $shouldkeep = (isset($item[$filterkey]) && ($item[$filterkey] === $value));
        }
        elseif ($type === 'matches') {
          $shouldkeep = (isset($item[$filterkey]) && preg_match($value, $item[$filterkey]));
        }
        if ($negate) {
          $shouldkeep = !$shouldkeep;
        }
        $item['item nr'] = $currentitemnr;
        $currentitemnr++;
        if ($shouldkeep) {
juiste item bewaren
let op: wat te doen als startfilterdatastep === prevdatastep ?
          $this->saveDataItem($item);
        }
        $item = $startfilterdatastep->nextDataItem();
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
