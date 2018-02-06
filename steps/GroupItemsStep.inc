<?php

class GroupItemsStep extends PrepareIngestStep {
  
  public function label() {
    $title = t('Group items');
    if ($this->hasConfigKey('group_key_name']) {
      $title .= ' ' . t('by the value of key "@key"', array('@key' => $this->config['group_key_name']));
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'group_key_name' => array(
        'label' => t('Group key name'),
        'description' => t('Every item that has the same value for this key will be grouped to one item.'),
        'type' => 'key',
      ),
    );
  }

  public function _inputKeyKeys() {
    return array('group_key_name');
  }

  public function outputKeys() {
    return array();
  }

  public function changesCount() {
    return TRUE;
  }

  public function changesKeys() {
    return TRUE;
  }

  public function changesFiles() {
    return FALSE;
  }

  public function checkConfiguration() {
    $results = parent::checkConfiguration();
    return $results;
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('group_key_name') && isset($prevdatastep)) {
      $groupkey = $this->config['group_key_name'];

      $grouparray = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $group = $item[$groupkey];
        if (array_key_exists($group, $grouparray)) {
          $newarray = array_merge($grouparray[$group], $item);
          $grouparray[$group] = $newarray;
        }
        else {
          $grouparray[$group] = $item;
        }

        $item = $prevdatastep->nextDataItem();
      }
      foreach ($grouparray as $group => $item) {
        $this->saveDataItem($item);
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
