<?php

class DefineConstantStep extends PrepareIngestStep {
  
  public function label() {
    $title = '';
    if ($this->hasConfigKey('name')) {
      $title .= t('Define constant "@name"', array('@name' => $this->config['name']));
      $value = '';
      if ($this->hasConfigKey('value')) {
        $value = $this->config['value'];
      }
      $title .= ' ' . t('with value "@value"', array('@value' => $value));
    }
    else {
      $title .= t('Define constant');
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'name' => array(
        'label' => t('Name'),
        'description' => t('The name of the constant.'),
        'type' => 'constantkey',
      ),
      'value' => array(
        'label' => t('Value'),
        'description' => t('The value of the constant, can be empty.'),
        'type' => 'string',
      ),
    );
  }

  public function promptValueOnPrepareIngest($key) {
    if ($key === 'value') {
       return t("Give value for constant '!name':", array('!name' => $this->valueForKey('name')));
     }
    return FALSE;
  }

  public function dryRun(&$context) {
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

  public function verify($context) {
    // no need to verify anything because there is nothing to verify.
    return array();
  }

  public function prepareIngest(&$context) {
    // no need to do anything
  }

}
