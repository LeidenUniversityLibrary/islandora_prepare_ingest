<?php

/**
 * @file
 * steps/AddKeyUsingMatchingItemStep.inc
 *
 *
 *  Copyright 2019 Leiden University Library
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


class AddKeyUsingMatchingItemStep extends PrepareIngestStep {

  public function label() {
    $title = '';
    if ($this->hasConfigKey('target_key_name')) {
      $title .= t('"@key"', array('@key' => $this->config['target_key_name']));
    }
    if ($this->hasConfigKey('template')) {
      $template = $this->config['template'];
      $title .= ' ' . t('with value "@template"', array('@template' => $template));
    }
    if ($this->hasConfigKey('matching_key_name')) {
      $title .= ' ' . t('using item where value of "@key"', array('@key' => $this->config['matching_key_name']));
    }
    if ($this->hasConfigKey('current_key_name')) {
      $title .= ' ' . t('matches current value of "@key"', array('@key' => $this->config['current_key_name']));
    }
    if (strlen($title) == 0) {
      $title = t('Add key using matching item');
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
        'description' => t('Template with keys from matching item.'),
        'type' => 'template',
        'allow_constants' => TRUE,
      ),
      'matching_key_name' => array(
        'label' => t('Key name matching item'),
        'description' => t('If there is an item where the value of this key matches the value of the key of the current item, then this matched item is used for the template.'),
        'type' => 'key',
      ),
      'current_key_name' => array(
        'label' => t('Key name current item'),
        'description' => t('If there is an item where the value of its key matches the value of this key of the current item, then that matched item is used for the template.'),
        'type' => 'key',
      ),
    );
  }

  public function inputKeys() {
    $result = $this->_getInputKeysFromTemplates(array('template'));
    if ($this->hasConfigKey('matching_key_name')) {
      $result['matching_key_name'] = array($this->config['matching_key_name']);
    }
    else {
      $result['matching_key_name'] = array();
    }
    if ($this->hasConfigKey('current_key_name')) {
      $result['current_key_name'] = array($this->config['current_key_name']);
    }
    else {
      $result['current_key_name'] = array();
    }
    return $result;
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

  public function dryRun(&$context) {
    $prevdatastep = $this->_previousDataChangingStep();
    if ($this->hasConfigKey('target_key_name') && $this->hasConfigKey('template')
        && $this->hasConfigKey('matching_key_name') && $this->hasConfigKey('current_key_name')
        && isset($prevdatastep)) {
      $targetkey = $this->config['target_key_name'];
      $template = $this->config['template'];
      $templatekeys = $this->_getKeysFromTemplates(array('template'));
      $matchingkey = $this->config['matching_key_name'];
      $currentkey = $this->config['current_key_name'];

      if (!isset($context['sandbox']['total_items'])) {
        $context['sandbox']['total_items'] = islandora_prepare_ingest_data_cache_per_step_count($prevdatastep->workflowid, $prevdatastep->stepid, $prevdatastep->otherid, 1);

        $prevdatastep->resetDataIterator();
      }

      $item = $prevdatastep->nextDataItem();
      $limit = 10000;
      while ($item !== NULL && $limit-- > 0) {
        if (isset($item[$currentkey])) {
          $matchvalue = $item[$currentkey];
          $matchingitem = $this->findMatchingItem($prevdatastep, $matchingkey, $matchvalue, $templatekeys);
          if ($matchingitem) {
            $newvalue = $this->fillOutTemplate($template, $matchingitem);
            $item[$targetkey] = $newvalue;
          }
        }
        $this->saveDataItem($item);
        if ($limit > 0) {
          $item = $prevdatastep->nextDataItem();
        }
      }
      if ($context['sandbox']['total_items'] > 0) {
        if ($item !== NULL) {
          $context['finished'] = $item['item nr'] / $context['sandbox']['total_items'];
        }
        else {
          $context['finished'] = 1;
        }
      }
    }
    return FALSE;
  }

  private function findMatchingItem($prevdatastep, $matchkey, $matchvalue, $neededkeys) {
    $cache = &drupal_static(__FUNCTION__);

    if (empty($cache[$matchkey])) {
      $value2id = islandora_prepare_ingest_data_cache_select_keyvalue_by_itemnr($prevdatastep->workflowid, $prevdatastep->stepid, $prevdatastep->otherid, 1, $matchkey);
      $itemdata = islandora_prepare_ingest_data_cache_select_keyvalues_by_itemnr($prevdatastep->workflowid, $prevdatastep->stepid, $prevdatastep->otherid, 1, $neededkeys);
      foreach ($value2id as $value => $itemid) {
        $cache[$matchkey][$value] = $itemdata[$itemid];
      }
    }
    if (isset($cache[$matchkey][$matchvalue])) {
      return $cache[$matchkey][$matchvalue];
    }
    return FALSE;
  }

  public function verify($context) {
    // custom verify method, because this step allows missing keys.
    $results = array();
    foreach ($this->inputKeys() as $ikey => $keys) {
      if (!isset($this->config[$ikey]) || (strlen($this->config[$ikey]) == 0)) {
        $results[] = array('text' => t("Missing argument '@key'.", array('@key' => $arguments[$ikey]['label'])),
                           'key' => $arguments[$ikey]['label'], 'type' => 'config_error');
        continue;
      }
    }
    return $results;
  }

  public function prepareIngest(&$context) {
    // no need to do anything
  }

}
