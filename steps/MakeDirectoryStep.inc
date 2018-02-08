<?php

class MakeDirectoryStep extends PrepareIngestFilesStep {
  
  public function label() {
    $title = t('Make directory');
    if ($this->hasConfigKey('directory_name')) {
      $title .= ' ' . t('based on the value of "@name"', array('@name' => $this->config['directory_name']));
    }
    $title .= '.';

    return $title;
  }

  public function arguments() {
    return array(
      'directory_name' => array(
        'label' => t('Directory name'),
        'description' => t('The absolute path to a directory that will be created. The parent directory should exist already.'),
        'type' => 'templatestring',
      ),
    );
  }

  public function inputKeys() {
    return $this->_getInputKeysFromTemplates(array('directory_name'));
  }

  public function _outputKeyKeys() {
    return array();
  }

  public function changesCount() {
    return FALSE;
  }

  public function changesKeys() {
    return FALSE;
  }

  public function changesFiles() {
    return TRUE;
  }

  public function checkConfiguration() {
    return parent::checkConfiguration();
  }

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('directory_name') && isset($prevdatastep)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
      $dirnametemplate = $this->config['directory_name'];
  
      $fileentries = $this->_previousStepFileEntries();
      $results = array();
      $prevdatastep->resetDataIterator();
      $item = $prevdatastep->nextDataItem();
      while ($item !== NULL) {
        $dirname = $this->fillOutTemplate($dirnametemplate, $item);
        $parentdir = dirname($dirname);
        if (pifs_file_exists($dirname, $this->workflowid, $this->otherid) && !pifs_is_dir($dirname, $this->workflowid, $this->otherid)) {
          $results[] = array('text' => t("'@directory' already exists but is not a directory.",
                    array('@directory' => $dirname)), 'type' => 'run_error');
        }
        if (!pifs_is_dir($parentdir, $this->workflowid, $this->otherid)) {
          $results[] = array('text' => t("Parent directory '@parentdir' of directory '@dir' should exist, but does not.",
                    array('@parentdir' => $parentdir, '@dir' => $dirname)), 'type' => 'run_error');
        }
        if (count($results) > 0) {
          return $results;
        }
        $entry = pifs_entry_mkdir($dirname, $this->workflowid, $this->stepid, $this->otherid, $item['item nr']);
        $fileentries[$entry['filepath']] = $entry;
        $this->saveDataItem($item);

        $item = $prevdatastep->nextDataItem();
      }
      $this->_saveFileEntries($fileentries);
    }
    return FALSE;
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    $this->_prepareIngestFilesUsingFunction('pifs_mkdir');
  }
}
