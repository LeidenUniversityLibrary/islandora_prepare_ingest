<?php

class AddKeyTemplateStep extends PrepareIngestStep {
  
  public function label() {
    $title = '';
    if ($this->hasConfigKey('target_key_name')) {
      $title .= t('named "@key"', array('@key' => $this->config['target_key_name']));
    }
    if ($this->hasConfigKey('template')) {
      $template = $this->config['template'];
      $title .= t('with value from template "@template"', array('@template' => $template));
    }

    if (strlen($title) == 0) {
      $title = t('Add key with template based value');
    }
    else {
      $title = t('Add key') . ' ' . $title;
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
      'template' => array(
        'label' => t('Template'),
        'description' => t('Text with keys that will be replaced between curly braces.'),
        'type' => 'template',
        'allow_constants' => TRUE,
      ),
    );
  }

  public function inputKeys() {
    return $this->_get_input_keys_from_templates(array('template'));
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
    return parent::checkConfiguration();
  }

  public function dryRun(&$context) {
    if ($this->hasConfigKey('target_key_name') && $this->hasConfigKey('template') && isset($this->previousstep)) {
      $targetkey = $this->config['target_key_name'];
      $template = $this->config['template'];

      $this->previousstep->resetDataIterator();
      $item = $this->previousstep->nextDataItem();
      while ($item !== NULL) {
        $newvalue = $this->fillOutTemplate($template, $item);
        $item[$targetkey] = $newvalue;
        $this->saveDataItem($item);
        $item = $this->previousstep->nextDataItem();
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
