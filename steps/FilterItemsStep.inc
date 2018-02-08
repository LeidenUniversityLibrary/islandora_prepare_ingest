<?php

class FilterItemsStep extends PrepareIngestStep {
  
  public function label() {
    $title = t('Filter items');
    if ($this->hasConfigKey('filter_key_name')) {
      $title .= ' ' . t('if the value of key "@key"', array('@key' => $this->config['filter_key_name']));
    }
    if ($this->hasConfigKey('filter_type')) {
      if ($this->config['filter_type'] === 'exists') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('does not exist');
        }
        else {
          $title .= ' ' . t('exists');
        }
      }
      elseif ($this->config['filter_type'] === 'equals') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('does not equal');
        }
        else {
          $title .= ' ' . t('equals');
        }
        if ($this->hasConfigKey('filter_value')) {
          $title .= ' ' . '"' . $this->config['filter_value'] . '"';
        }
      }
      elseif ($this->config['filter_type'] === 'matches') {
        if ($this->hasConfigKey('negate') && $this->config['negate']) {
          $title .= ' ' . t('does not match');
        }
        else {
          $title .= ' ' . t('matches');
        }
        if ($this->hasConfigKey('filter_value')) {
          $title .= ' ' . $this->config['filter_value'];
        }
      }
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'filter_key_name' => array(
        'label' => t('Filter key name'),
        'description' => t('The key that should be used to filter.'),
        'type' => 'key',
      ),
      'filter_type' => array(
        'label' => t('Filter type'),
        'description' => t('Choose how you want to filter: "key exists" filters if the key has any value, "equals" filters if the value of the filter key name is exactly the same as the filter value, "matches" filters if the value of the filter key name matches the regular expression in the filter value.'),
        'type' => 'select',
        'options' => array('exists' => t('key exists'), 'equals' => t('equals'), 'matches' => t('matches')),
      ),
      'filter_value' => array(
        'label' => t('Filter value'),
        'description' => t('The value to filter by.'),
        'type' => 'string',
        'allow_constants' => TRUE,
        'optional' => array('filter_type' => array('exists')),
      ),
      'negate' => array(
        'label' => t('Negate'),
        'description' => t('If false, values of the filter_key_name key that are equal to filter_value, are included. If true, values of the filter_key_name key that are not equal to filter_value, are included.'),
        'type' => 'boolean',
      ),
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
    $results = parent::checkConfiguration();
    if ($this->hasConfigKey('filter_type')) {
      if ($this->config['filter_type'] === 'exists') {
        // no check needed, filter_value does not matter.
        unset($results['filter_value']);
      }
      elseif ($this->config['filter_type'] === 'equals') {
        if (!$this->hasConfigKey('filter_value')) {
          $results[] = array('text' => t("Filter with type 'equals' should have a value."), 'type' => 'argument_error');
        }
      }
      elseif ($this->config['filter_type'] === 'matches') {
        if (!($this->hasConfigKey('filter_value') && (@preg_match($this->config['filter_value'], "XXX") !== FALSE))) {
          $results[] = array('text' => t("Filter with type 'matches' should have a regular expression, but has '@value'.",
                             array('@value' => $this->config['filter_value'])), 'type' => 'argument_error');
        }
      }
    }
    return $results;
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('filter_key_name') && $this->hasConfigKey('filter_type') && isset($prevdatastep)) {
      $filterkey = $this->config['filter_key_name'];
      $type = $this->config['filter_type'];
      $value = $this->config['filter_value'];
      $negate = $this->config['negate'];

      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
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
        if ($shouldkeep) {
          $item['item nr'] = $currentitemnr;
          $this->saveDataItem($item);
          $currentitemnr++;
        }
        $item = $prevdatastep->nextDataItem();
      }
    }
    return FALSE;
  }

  public function verify($context) {
    // don't verify input keys or output keys, because some might be missing but that is okay.
    return array();
  }

  public function prepareIngest(&$context) {
    // no need to do anything
  }

}
