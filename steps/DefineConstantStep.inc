<?php

/**
 * @file
 * steps/DefineConstantStep.inc
 *
 *
 *  Copyright 2017-2018 Leiden University Library
 *
 *  This file is part of islandora_prepare_ingest.
 *
 *  islandora_prepare_ingest is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


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
    // do dry run again to make sure the constants get set.
    return $this->dryRun($context);
  }

}
