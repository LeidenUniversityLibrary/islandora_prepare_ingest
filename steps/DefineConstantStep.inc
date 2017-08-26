<?php

class DefineConstantStep extends PrepareIngestStep {
  
  public function label() {
    $title = '';
    if ($this->hasConfigKey('name')) {
      $title .= t('Defined constant with name "@name"', array('@name' => $this->config['name']));
      $value = '';
      if ($this->hasConfigKey('value')) {
        $value = $this->config['value'];
      }
      $title .= ' ' . t('and value "@value"', array('@value' => $value));
    }
    else {
      $title .= t('Define constant');
    }
    $title .= '. XXXX';

    return $title;
  }

  public function checkConfiguration() {
    return parent::checkConfiguration();
  }

  public function dryRun() {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');
    if (isset($this->config['name'])) {
      $name = $this->config['name'];
      $value = isset($this->config['value'])?$this->config['value']:'';
      $results = islandora_prepare_ingest_constants_set($name, $value, $context);
      if ($results) {
        return $results;
      }
    }
    return FALSE;
  }

  public function verify() {
    // no need to verify anything because there is nothing to verify.
    return array();
  }

  public function prepareIngest() {
    // no need to do anything
  }

}
