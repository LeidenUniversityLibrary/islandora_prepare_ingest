<?php

class AddKeyTemplate extends PrepareIngestStep {
  
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

  public function checkConfiguration() {
    return parent::checkConfiguration();
  }

  public function dryRun() {
    if ($this->hasConfigKey('target_key_name') && $this->hasConfigKey(['template')) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $targetkey = $this->config['target_key_name'];
      $template = $this->config['template'];

      $this->resetDataIterator();
      while ($item = $this->nextDataItem()) {
        $newvalue = $this->fillOutTemplate($template, $item);
        $item[$targetkey] = $newvalue;
        $this->saveDataItem($item);
      }
    }
  }

  public function verify() {
    return parent::verify();
  }

  public function prepareIngest() {
    // no need to do anything
  }

}
