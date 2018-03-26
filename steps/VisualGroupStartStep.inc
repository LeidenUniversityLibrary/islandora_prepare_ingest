<?php

/**
 * @file
 * steps/VisualGroupStartStep.inc
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


class VisualGroupStartStep extends PrepareIngestStep {
  
  public function label() {
    if ($this->hasConfigKey('description')) {
      $title = t('Visual group:') . ' ' . $this->config['description'];
    }
    else {
      $title = t('Visual group  start.'); // two spaces for sorting...
    }
    return $title;
  }

  public function arguments() {
    return array(
      'description' => array(
        'label' => t('Description'),
        'description' => t('A description of this visual grouping.'),
        'type' => 'template',
        'allow_constants' => TRUE,
      ),
    );
  }

  public function visual() {
    return array('groupstart', 'groupremove', 'ungroup');
  }

  public function dryRun(&$context) {
    // nothing to do
    return FALSE;
  }

  public function verify($context) {
    return parent::verify($context);
  }

  public function prepareIngest(&$context) {
    // no need to do anything
  }

}
