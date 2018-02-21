<?php

/**
 * @file
 * steps/PrepareIngestValidateStep.inc
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

interface PrepareIngestValidateStepInterface {

  function typeString();

  function models();

  function preprocessDrushCommand();

  function ingestDrushCommand();

  function modifyPreprocessOptions(&$options);

  function modifyIngestOptions(&$options);

}

class PrepareIngestValidateStep extends PrepareIngestStep implements PrepareIngestValidateStepInterface {
  
  public function label() {
    $type = $this->typeString();
    $title = t('Validate the @type structure', array('@type' => $type));
    if ($this->hasConfigKey('filepath')) {
      $title .= ' ' . t('at "@value"', array('@value' => $this->config['filepath']));
    }    
    $title .= '.';
         
    return $title;
  }
             
  public function arguments() { 
    return array(
      'filepath' => array(
        'label' => t('Filepath'),
        'description' => t('The absolute path to the directory where the import files are placed.'),
        'type' => 'filepath',
        'allow_constants' => TRUE,
      ),
    );
  }

  public function typeString() {
    $classname = get_class($this); 
    $methodname = __METHOD__;
    throw new BadMethodCallException("Method '$methodname' not implemented for class '$classname'.");
  } 

  public function models() {
    $classname = get_class($this);
    $methodname = __METHOD__;
    throw new BadMethodCallException("Method '$methodname' not implemented for class '$classname'.");
  }

  protected function _inputKeyKeys() {
    return array();
  }

  protected function _outputKeyKeys() {
    return array();
  }

  public function changesCount() {
    return FALSE;
  }

  public function changesKeys() {
    return FALSE;
  }

  public function changesFiles() {
    return FALSE;
  }

  public function checkConfiguration($context) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');
    $results = parent::checkConfiguration($context);
    if (!pifs_is_dir($this->config['filepath'], $this->workflowid, $this->otherid)) {
      $results[] = array('text' => t("Argument '@key' should be a valid directory.",
                    array('@key' => $this->arguments()['filepath']['label'])), 'type' => 'argument_error');
    }
    return $results;
  }

  public function preprocessDrushCommand() {
    $classname = get_class($this);
    $methodname = __METHOD__;
    throw new BadMethodCallException("Method '$methodname' not implemented for class '$classname'.");
  }

  public function ingestDrushCommand() {
    return 'islandora_batch_ingest';
  }

  public function modifyPreprocessOptions(&$options) {
    return;
  }

  public function modifyIngestOptions(&$options) {
    return;
  }

  public function prepareIngest(&$context) {
    if (isset($context['drush']) && $context['drush']) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/pifs');

      $filepath = $this->config['filepath'];
      $type = $this->typeString();

      $namespace = FALSE; 
      if (module_load_include('inc', 'islandora', 'includes/utilities')) {
        $namespaces = islandora_get_allowed_namespaces();
        $options = array_combine($namespaces, $namespaces);
        $namespace = drush_choice($options, dt('Choose a namespace to use for ingest:'));
      }
      if (!isset($namespace)) {
        $namespace = drush_prompt(dt('Give namespace'));
        if (preg_match('/^([A-Za-z0-9-\.]+)$/', $namespace) === 0 || !islandora_namespace_accessible($namespace)) {
          drush_log(dt('Invalid namespace given.'), 'warning');
          $namespace = FALSE;
        }
      }
      if ($namespace) {
        $collection = drush_prompt(dt('Give a collection to ingest into'));
        $obj = NULL;
        if (islandora_is_valid_pid($collection)) {
          $obj = islandora_object_load($collection);
        }
        if (!($obj && in_array('islandora:collectionCModel', $obj->models))) {
           drush_log(dt('Invalid collection given.'), 'warning');
           $collection = FALSE;
        }
      }
      global $base_url;
      global $user;

      $models = implode(',', $this->models());
      $preprocesscommand = $this->preprocessDrushCommand();
      $args = array();
      $preprocessoptions = array(
        'root' => DRUPAL_ROOT,
        'uri' => $base_url . '/',
        'user' => $user->name,
        'namespace' => $namespace,
        'parent' => $collection,
        'content_models' => $models,
        'type' => 'directory',
        'target' => $filepath,
      );
      $this->modifyPreprocessOptions($preprocessoptions);

      $ingestcommand = $this->ingestDrushCommand();
      $ingestoptions = array(
        'root' => DRUPAL_ROOT,
        'uri' => $base_url . '/',
        'user' => $user->name,
      );
      $this->modifyIngestOptions($preprocessoptions);

      $didpreprocess = FALSE;
      if ($collection && $namespace) {
        if (drush_confirm(dt('Preprocess the files at "@filepath" as @type?', array('@filepath' => $filepath, '@type' => $type)))) {
          drush_log(dt('Starting preprocessing...'), 'ok');
          $results = drush_invoke_process('@self', $preprocesscommand, $args, $preprocessoptions);
          if (($results !== FALSE) && ($results['error_status'] == 0)) {
            $didpreprocess = TRUE;
            drush_log(dt('Preprocessing successful!'), 'ok');
            if (drush_confirm(dt('Start ingest of all preprocessed items (could be more than the just preprocessed items)?'))) {
              drush_log(dt('Starting ingest...'), 'ok');   
              $results = drush_invoke_process('@self', $ingestcommand, $args, $ingestoptions);
              if ($results !== FALSE) {
                drush_log(dt('Ingest successful!'), 'ok');
                return;
              }
              else {
                drush_log(dt('Error when ingesting'), 'error');
              }
            }
          }
          else {
            drush_log(dt('Error when preprocessing'), 'error');
          }
        }
      }
      else {
        if (!$collection) {
          $preprocessoptions['parent'] = 'COLLECTIONID';
        }
        if (!$namespace) {
          $preprocessoptions['namespace'] = 'NAMESPACE';
        }
      }
      drush_log(dt("To ingest do the following:"), 'ok');
      if (!$didpreprocess) {
        $drushcmd = "drush -v $preprocesscommand";
        foreach ($preprocessoptions as $key => $value) {
          $drushcmd .= " --$key=$value";
        }
        drush_log($drushcmd, 'ok'); 
      }
      $drushcmd = "drush -v $ingestcommand";
      foreach ($ingestoptions as $key => $value) {
        $drushcmd .= " --$key=$value";
      }
      drush_log($drushcmd, 'ok'); 
    }
  }

}
