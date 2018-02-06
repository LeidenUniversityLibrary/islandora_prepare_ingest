<?php

class AddKeyCounterStep extends PrepareIngestStep {
  
  public function label() {
  $title = '';
  if ($this->hasConfigKey('target_key_name')) {
    $title .= ' ' . t('named "@key"', array('@key' => $this->config['target_key_name']));
  }
  if ($this->hasConfigKey('start')) {
    $title .= ' ' . t('from @start', array('@start' => $this->config['start']));
  }
  if ($this->hasConfigKey('step')) {
    $title .= ' ' . t('with increments of @step', array('@step' => $this->config['step']));
  }
  if ($this->hasConfigKey('width')) {
    $title .= ' ' . t('and width @width', array('@width' => $this->config['width']));
  }
  if ($this->hasConfigKey('foreach_value')) {
    $title .= ' ' . t('and start for each unique value of "@key"', array('@key' => $this->config['foreach_value']));
  }

  if (strlen($title) == 0) {
    $title = t('Add key with counter value');
  }
  else {
    $title = t('Add counter with key') . ' ' . $title;
  }
  $title .= '.';

  return $title;
  }

  public function arguments() {
    return array(
      'target_key_name' => array(
        'label' => t('Target key name'),
        'type' => 'key',
      ),
      'start' => array(
        'label' => t('Start'),
        'type' => 'number',
        'default_value' => 1,
        'allow_constants' => TRUE,
      ),
      'step' => array(
        'label' => t('Step'),
        'type' => 'number',
        'default_value' => 1,
        'allow_constants' => TRUE,
      ),
      'width' => array(
        'label' => t('Width'),
        'description' => t('The width of the number, it will be prepended by zero\'s to meet this width.'),
        'type' => 'number',
        'default_value' => 1,
        'allow_constants' => TRUE,
      ),
      'foreach_value' => array(
        'label' => t('Foreach value of key name'),
        'description' => t('Optional, start counting for each unique value of this key.'),
        'type' => 'key',
        'optional' => TRUE,
      ),
    );
  }

  public function _inputKeyKeys() {
    return array('foreach_value');
  }

  public function _outputKeyKeys() {
    return array('target_key_name');
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

  public function checkConfiguration() {
    $results = parent::checkConfiguration();
    foreach (array('step', 'width') as $key) {
      if ($this->hasConfigKey($key)) {
        if (intval($this->config[$key]) == 0) {
          $results[] = array('text' => t("Argument '@key' should be a number (not zero), but is '@value'.",
                    array('@key' => $this->arguments()[$key]['label'], '@value' => $this->config[$key])), 'type' => 'argument_error');
        }
      }
    }
    return $results;
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('target_key_name') && $this->hasConfigKey('start') && $this->hasConfigKey('step') && $this->hasConfigKey('width') && isset($prevdatastep)) {
      $targetkey = $this->config['target_key_name'];
      $start = intval($this->config['start']);
      $step = intval($this->config['step']);
      $width = intval($this->config['width']);
      $foreachkey = $this->config['foreach_value'];
      $format = '%0' . $width . 'd';
      $counter = array();

      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $foreachvalue = isset($item[$foreachkey])?$item[$foreachkey]:'_GLOB_COUNTER_';
        if (!array_key_exists($foreachvalue, $counter)) {
          $counter[$foreachvalue] = $start;
        }
        $newvalue = sprintf($format, $counter[$foreachvalue]);
        $counter[$foreachvalue] += $step;
        $item[$targetkey] = $newvalue;
        $this->saveDataItem($item);
        $item = $prevdatastep->nextDataItem();
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
