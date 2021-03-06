<?php

/**
 * @file
 * steps/PrepareIngestStep.inc
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


interface PrepareIngestStepInterface {

 /**
  * Returns the label of this step.
  **/
  public function label();

 /**
  * Returns an array containing the arguments for this step. The keys are the argument names. The value is an array with the following keys:
  * - 'label': a human readable label for this key;
  * - 'description': optional, further description of the key or its contents;
  * - 'type': the type of the value of this argument. See 'Argument values' for more information;
  * - 'default_value': optional, a default value of type 'type';
  * - 'allow_constants': optional, this value allows constants.
  **/
  public function arguments();

 /**
  * Dry runs this step of the workflow.
  *
  * This method must be overwritten with your own step logic. This method should manipulate the data, but only
  * in memory, so don't make/change any files/directories (use pifs instead) or do any other "real" things.
  * The prepareIngest method will do the actual writing of files/directories and other "real" things based on the output
  * of this step.
  *
  * @return FALSE or array
  *   Returns FALSE (run successful) or an array with one or more entries (something wrong with the run). Each entry is an associated
  *   array with keys: text and type, and optionally key.
  *   The text key has a value (string) that explains what is wrong with the run.
  *   The type key indicates the type of error and should be run_error.
  *   The optional key key holds the name of the key where the run went wrong.
  **/
  public function dryRun(&$context);

 /**
  * Prepares the actual data for ingest.
  *
  * This method must be overwritten with your own step logic. This method should prepare the actual data for ingest
  * based on the data provided by the run method. If you want to create directories/files or prepare the ingest in
  * another way, this is the place to do it.
  *
  **/
  public function prepareIngest(&$context);

}

class PrepareIngestStep implements PrepareIngestStepInterface {

  protected $name;
  protected $config;
  protected $workflowid;
  protected $stepid;
  protected $otherid;
  protected $weight;
  protected $previousstep;
  protected $previousdatachangingstep;
  private $di_dbstatement;
  private $di_lastdata;
  protected $di_currentitemnr;
  protected $di_type;

 /**
  * Initializes a PrepareIngestStep object with the appropriate values.
  * There should be no need to overwrite this function, it is automatically called when starting a step.
  *
  * @param array $stepconfig
  *   An associated array with keys the same as $stepinfo['arguments'] and values for those keys indicating
  *   the configuration of this specific step.
  * @param int $workflowid
  *   The identifier of the current workflow.
  * @param int $stepid
  *    The identifier of the current step.
  * @param string $otherid
  *    The identifier of the current run of this workflow.
  */
  public function __construct(array $workflowstep, array $stepconfig, $otherid, $prevstep = NULL) {
    $this->config = $stepconfig;
    $this->workflowid = (int)$workflowstep['workflow_id'];
    $this->stepid = (int)$workflowstep['id'];
    $this->otherid = $otherid;
    $this->weight = $workflowstep['weight'];
    $this->name = $workflowstep['step_name'];
    $this->previousstep = $prevstep;

    while ($prevstep !== NULL && !($prevstep->changesCount() || $prevstep->changesKeys())) {
      $prevstep = $prevstep->previousstep;
    }
    $this->previousdatachangingstep = $prevstep;
  }

 /**
  * Magic sleep function to keep from serializing the di_dbstatement and di_lastdata.
  **/
  public function __sleep() {
    return array('name', 'config', 'workflowid', 'stepid', 'otherid', 'weight', 'previousstep', 'previousdatachangingstep', 'di_currentitemnr', 'di_type');
  }

 /**
  * Magic wakeup function to refresh di_dbstatement and di_lastdata.
  **/
  public function __wakeup() {
    if (isset($this->di_currentitemnr, $this->di_type)) {
      module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

      $selectquery = islandora_prepare_ingest_data_cache_select_query($this->workflowid, $this->stepid, $this->otherid, $this->di_type, array());
      $selectquery = $selectquery->orderBy('item_nr');
      $selectquery->condition('item_nr', $this->di_currentitemnr, '>=');
      $this->di_dbstatement = $selectquery->execute();
      $this->di_lastdata = $this->di_dbstatement->fetchAssoc();
    }
  }

 /**
  * Returns the identifier of this step.
  **/
  public function id() {
    return $this->stepid;
  }

 /**
  * Returns the identifier of the workflow this step belongs to.
  **/
  public function workflowId() {
    return $this->workflowid;
  }

 /**
  * Returns the label of this step.
  **/
  public function label() {
    $classname = get_class($this);
    $methodname = __METHOD__;
    throw new BadMethodCallException("Method '$methodname' not implemented for class '$classname'.");
  }

 /**
  * Returns the name of this step.
  **/
  public function name() {
    return $this->name;
  }

 /**
  * Returns the weight of this step.
  **/
  public function weight() {
    return $this->weight;
  }

 /**
  * Returns the filled in value of the configuration, or the default value from the arguments.
  **/
  public function valueForKey($key) {
    if (isset($this->config[$key])) {
      return $this->config[$key];
    }
    elseif (isset($this->arguments()[$key]['default_value'])) {
      return $this->arguments()[$key]['default_value'];
    }
    return '';
  }

 /**
  * Returns an array containing the arguments for this step. The keys are the argument names. The value is an array with the following keys:
  * - 'label': A human readable label for this key
  * - 'description': Optional, further description of the key or its contents
  * - 'type': The type of the value of this argument. See 'Argument values' for more information.
  * - 'default_value': a default value of type 'type'
  * - 'allow_constants': this value allows constants
  **/
  public function arguments() {
    return array();
  }

 /**
  * Returns an array containing the keys whose values should be present as keys in the data for this step to work.
  * The keys of this array indicate the argument name where the key(s) is coming from. The values are arrays with zero or more keys.
  * E.g.: array('argument name' => array('key for data'))
  **/
  public function inputKeys() {
    $inputKeys = array();
    foreach ($this->_inputKeyKeys() as $keyKey) {
      if ($this->hasConfigKey($keyKey)) {
        $inputKeys[$keyKey] = array($this->config[$keyKey]);
      }
      else {
        $inputKeys[$keyKey] = array();
      }
    }
    return $inputKeys;
  }

  protected function _inputKeyKeys() {
    return array();
  }

 /**
  * Returns an array containing the keys whose values are present as keys in the data after this step.
  * The keys of this array indicate the argument name where the key(s) is coming from. The values are arrays with zero or more keys.
  * E.g.: array('argument name' => array('key for data'))
  **/
  public function outputKeys() {
    $outputKeys = array();
    foreach ($this->_outputKeyKeys() as $keyKey) {
      if ($this->hasConfigKey($keyKey)) {
        $outputKeys[$keyKey] = array($this->config[$keyKey]);
      }
      else {
        $outputKeys[$keyKey] = array();
      }
    }
    return $outputKeys;
  }

  protected function _outputKeyKeys() {
    return array();
  }

 /**
  * Returns a boolean indicating if the step changes the count of the data array. So if items are added or removed from the data array, this should be TRUE.
  **/
  public function changesCount() {
    return FALSE;
  }

 /**
  * Returns a boolean indicating if the step changes the keys of the items in the data array. So if the items get a new key, this shoud be TRUE.
  **/
  public function changesKeys() {
    return FALSE;
  }

 /**
  * Returns a boolean indication if the step adds and/or changes files on the file system.
  **/
  public function changesFiles() {
    return FALSE;
  }

 /**
  * Returns an array indication how this step should visually by represented.
  **/
  public function visual() {
    return array();
  }

 /**
  * Returns a string indicating the prompt for a specific key. Return FALSE when this is not needed.
  */
  public function promptValueOnPrepareIngest($key) {
    // if ($key === 'somekey') {
    //   return "Give the value for somekey?";
    // }
    return FALSE;
  }

 /**
  * Checks the configuration of this step.
  *
  * This method may be overwritten if you need to implement more checks of the configuration of this step, e.g.
  * if the step does something with files you should check if directories and/or files exists if read from.
  *
  * @return array
  *   Returns an array with zero (check succesful) or more entries (something wrong with the configuration). Each entry is an associated
  *   array with 3 keys: key, text and type.
  *   The key key holds the name of the key where the configuration is wrong.
  *   The text key has a value (string) that explains what is wrong in the configuration.
  *   The type key indicates the type of error and should be argument_error.
  **/
  public function checkConfiguration($context) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/constants');

    islandora_prepare_ingest_constants_add_to_stepconfig($this->config, $context);

    return $this->_checkConfiguration();
  }

 /**
  * Dry runs this step of the workflow.
  *
  * This method must be overwritten with your own step logic. This method should manipulate the data, but only
  * in memory, so don't make/change any files/directories (use pifs instead) or do any other "real" things.
  * The prepareIngest method will do the actual writing of files/directories and other "real" things based on the output
  * of this step.
  *
  * @return FALSE or array
  *   Returns FALSE (run succesful) or an array with one or more entries (something wrong with the run). Each entry is an associated
  *   array with keys: text and type, and optionally key.
  *   The text key has a value (string) that explains what is wrong with the run.
  *   The type key indicates the type of error and should be run_error.
  *   The optional key key holds the name of the key where the run went wrong.
  **/
  public function dryRun(&$context) {
    $classname = get_class($this);
    $methodname = __METHOD__;
    throw new BadMethodCallException("Method '$methodname' not implemented for class '$classname'.");
  }

 /**
  * Verifies the input (and possibly output) data of this step.
  *
  * This method may be overwritten of you need to implement more verifications of the input (and output) data of this step.
  *
  * @return array
  *   Returns an array with zero (verify succesful) or more entries (something wrong). Each entry is an associated
  *   array with 3 keys: key, text and type.
  *   The key key holds the name of the key where the verification failed.
  *   The text key has a value (string) that explains what is wrong.
  *   The type key indicates the type of error and should be config_error or key_error.
  **/
  public function verify($context) {
    return $this->_checkInputKeysInData();
  }

 /**
  * Prepares the actual data for ingest.
  *
  * This method must be overwritten with your own step logic. This method should prepare the actual data for ingest
  * based on the data provided by the run method. If you want to create directories/files or prepare the ingest in
  * another way, this is the place to do it.
  *
  **/
  public function prepareIngest(&$context) {
    $classname = get_class($this);
    $methodname = __METHOD__;
    throw new BadMethodCallException("Method '$methodname' not implemented for class '$classname'.");
  }

  protected function hasConfigKey($keyname) {
    return (isset($this->config[$keyname]) && (strlen($this->config[$keyname]) > 0));
  }

  protected function resetDataIterator($type = 1) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

    $selectquery = islandora_prepare_ingest_data_cache_select_query($this->workflowid, $this->stepid, $this->otherid, $type, array());
    $selectquery = $selectquery->orderBy('item_nr');
    $this->di_dbstatement = $selectquery->execute();
    $this->di_lastdata = $this->di_dbstatement->fetchAssoc();
    $this->di_currentitemnr = $this->di_lastdata['item_nr'];
    $this->di_type = $type;
  }

  protected function nextDataItem() {
    $d = $this->di_lastdata;
    $dbs = $this->di_dbstatement;
    do {
      $dataitem = NULL;
      $currentitemnr = $this->di_currentitemnr;
      while ($d !== FALSE) {
        if ($d['item_nr'] > $currentitemnr) {
          $this->di_currentitemnr = $d['item_nr'];
          break;
        }
        if ($dataitem === NULL) {
          $dataitem = array();
        }
        $dataitem[$d['dckey']] = unserialize($d['dcvalue']);
        $d = $dbs->fetchAssoc();
        $this->di_lastdata = $d;
      }
    } while ($dataitem !== NULL && isset($dataitem['_SKIP_']) && $dataitem['_SKIP_']);
    return $dataitem;
  }

  protected function saveDataItem($dataitem) {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

    if (!isset($dataitem['item nr'])) {
      throw new UnexpectedValueException("Data item is missing an item nr key.");
    }
    if (!isset($dataitem['_UID_'])) {
      $dataitem['_UID_'] = uniqid($dataitem['item nr'] . '_');
    }
    if (!isset($dataitem['_SKIP_'])) {
      $dataitem['_SKIP_'] = 0;
    }
    $itemnr = $dataitem['item nr'];
    islandora_prepare_ingest_data_cache_single_insert($this->workflowid, $this->stepid, $this->otherid, 1, $itemnr, $dataitem);
  }

  protected function skipDataItemInNextSteps($dataitem) {
    $dataitem['_SKIP_'] = 1;
    $itemnr = $dataitem['item nr'];
    islandora_prepare_ingest_data_cache_single_update($this->workflowid, $this->stepid, $this->otherid, 1, $itemnr, array('_SKIP_' => 1));
  }

  protected function fillOutTemplate($template, $keyvalues) {
    if (strpos($template, '{') !== FALSE) {
      $template = preg_replace_callback(
         '/(?<!{){(?:"""(.*?)"""<)?([a-zA-Z0-9_-]+)(?:\|([a-zA-Z0-9_-]+)|\|("""(.*?)"""))?(?:>"""(.*?)""")?}/',
         function($matches) use ($keyvalues) {
           $prefix = isset($matches[1])?$matches[1]:'';
           $postfix = isset($matches[6])?$matches[6]:'';
           $prefix = $this->fillOutTemplate($prefix, $keyvalues);
           $postfix = $this->fillOutTemplate($postfix, $keyvalues);
           if (isset($matches[2], $keyvalues[$matches[2]]) && strlen($keyvalues[$matches[2]]) > 0) {
             return $prefix . $keyvalues[$matches[2]] . $postfix;
           }
           elseif (isset($matches[3], $keyvalues[$matches[3]]) && strlen($keyvalues[$matches[3]]) > 0) {
             return $prefix . $keyvalues[$matches[3]] . $postfix;
           }
           elseif (isset($matches[4])) {
             if (strlen($matches[4]) > 0) {
               $value = $this->fillOutTemplate($matches[5], $keyvalues);
               return $prefix . $value . $postfix;
             }
             else {
               return '';
             }
           }
           return $matches[0];
         },
         $template
      );
    }
    return $template;
  }

  protected function _checkConfiguration() {
    $results = array();
    foreach ($this->arguments() as $argkey => $argconfig) {
      $argkeytext = $argconfig['label'];
      if (isset($this->config[$argkey]) && strlen($this->config[$argkey]) > 0) {
        $value = $this->config[$argkey];
        switch($argconfig['type']) {
          case "number":
            if (!is_numeric($value)) {
              $results[] = array('text' => t("Argument '@key' should be a numeric value, but is '@value'",
                array('@key' => $argkeytext, '@value' => $value)), 'key' => $argkey, 'type' => 'argument_error');
            }
            break;
          case "string":
            // no need to check
            break;
          case "boolean":
            if ($value !== "0" && $value !== "1") {
              $results[] = array('text' => t("Argument '@key' should be a boolean value, but is '@value'",
                array('@key' => $argkeytext, '@value' => $value)), 'key' => $argkey, 'type' => 'argument_error');
            }
            break;
          case "template":
            // no need to check
            break;
          case "key":
          case "constantkey":
            if (preg_match("/^[a-zA-Z0-9_-]+$/", $value) === 0) {
              $results[] = array('text' => t("Argument '@key' should be a key value, but is '@value'",
                array('@key' => $argkeytext, '@value' => $value)), 'key' => $argkey, 'type' => 'argument_error');
            }
            break;
          case "keys":
            if (preg_match("/^[a-zA-Z0-9_;\-\n\r]+$/", $value) === 0) {
              $results[] = array('text' => t("Argument '@key' should be one or more key values separated by a semicolon and/or linebreak, but is '@value'",
                array('@key' => $argkeytext, '@value' => $value)), 'key' => $argkey, 'type' => 'argument_error');
            }
            break;
          case "filepath":
            if (preg_match("/^(?:\/[a-zA-Z0-9_.-]+)+\/?$/", $value) === 0) {
              $results[] = array('text' => t("Argument '@key' should be a filepath, but is '@value'",
                array('@key' => $argkeytext, '@value' => $value)), 'key' => $argkey, 'type' => 'argument_error');
            }
            break;
          case "regexp":
            if (@preg_match($value, "XXX") === false) {
              $results[] = array('text' => t("Argument '@key' should be a regular expression, but is '@value'",
                array('@key' => $argkeytext, '@value' => $value)), 'key' => $argkey, 'type' => 'argument_error');
            }
            break;
          case "xpath":
            // is there a better way to validate xpath's, without removing the namespaces?
            $doc = new DOMDocument;
            $xpath = new DOMXPath($doc);
            $valuewons = preg_replace('#[^/:\\]\\[]+:#', '', $value);

            if ($xpath->query($valuewons) === false) {
              $results[] = array('text' => t("Argument '@key' should be a valid XPath, but is '@value'.",
                array('@key' => $argkeytext, '@value' => $value, '@type' => gettype($value))), 'key' => $argkey, 'type' => 'argument_error');
            }
            break;
          case "collection":
            module_load_include('inc', 'islandora', 'includes/utilities');
            $is_collection = FALSE;
            if (islandora_is_valid_pid($value)) {
              $obj = islandora_object_load($value);
              if ($obj && in_array('islandora:collectionCModel', $obj->models)) {
                $is_collection = TRUE;
              }
            }
            if (!$is_collection) {
              $results[] = array('text' => t("Argument '@key' should be a valid PID of a collection, but is '@value'.",
                array('@key' => $argkeytext, '@value' => $value, '@type' => gettype($value))), 'key' => $argkey, 'type' => 'argument_error');
            }
            break;
          case "namespace":
            module_load_include('inc', 'islandora', 'includes/utilities');
            if (preg_match('/^([A-Za-z0-9-\.]+)$/', $value) === 0 || !islandora_namespace_accessible($value)) {
              $results[] = array('text' => t("Argument '@key' should be a valid namespace, but is '@value'.",
                array('@key' => $argkeytext, '@value' => $value, '@type' => gettype($value))), 'key' => $argkey, 'type' => 'argument_error');
            }
            break;
        }
      }
      else {
        if (isset($argconfig['optional'])) {
          if (is_array($argconfig['optional'])) {
            foreach ($argconfig['optional'] as $optkey => $optvalues) {
              if (in_array($this->config[$optkey], $optvalues)) {
                continue 2;
              }
            }
          }
          else {
            if ($argconfig['optional']) {
              continue;
            }
          }
        }
        $results[$argkey] = array('text' => t('Missing argument \'@argname\'', array('@argname' => $argconfig['label'])), 'key' => $argkey, 'type' => 'argument_error');
      }
    }
    return $results;
  }

  protected function _checkInputKeysInData() {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');

    $prevdatastep = $this->_previousDataChangingStep();

    $results = array();

    $inputkeys = array();
    $arguments = $this->arguments();
    foreach ($this->inputKeys() as $ikey => $keys) {
      if (isset($arguments[$ikey]['optional'])) {
        if (is_array($arguments[$ikey]['optional'])) {
          foreach ($arguments[$ikey]['optional'] as $optkey => $optvalues) {
            if (in_array($this->config[$optkey], $optvalues)) {
              continue 2;
            }
          }
        }
        else {
          if ($arguments[$ikey]['optional']) {
            continue;
          }
        }
      }
      if (!isset($this->config[$ikey]) || (strlen($this->config[$ikey]) == 0)) {
        $results[] = array('text' => t("Missing argument '@key'.", array('@key' => $arguments[$ikey]['label'])),
                           'key' => $arguments[$ikey]['label'], 'type' => 'config_error');
        continue;
      }
      $inputkeys = array_merge($inputkeys, $keys);
    }

    if (isset($prevdatastep)) {
      $itemscount = islandora_prepare_ingest_data_cache_per_step_count($this->workflowid, $prevdatastep->stepid, $this->otherid, 1);
      foreach ($inputkeys as $key) {
        $keycount = islandora_prepare_ingest_data_cache_per_step_key_count($this->workflowid, $prevdatastep->stepid, $this->otherid, 1, $key);
        if (($keycount === 0) && ($itemscount > 0)) {
          $results[] = array('text' => t("Missing key '@key'. This key is not defined in any of the previous steps.", array('@key' => $key)),
                             'key' => $key, 'type' => 'key_error');
        }
        else if (($itemscount > $keycount)  && ($keycount > 0)) {
        $results[] = array('text' => t("Missing key '@key'. This key is defined in a previous step but not available for all items. Use a filter before this step to keep the items with the right keys.", array('@key' => $key)),
                             'key' => $key, 'type' => 'key_error');
        }
      }
    }
    else if (count($inputkeys) > 0) {
      $results[] = array('text' => t("No previous steps available, but input keys needed."), 'key' => $inputkeys[0], 'type' => 'key_error');
    }
    return $results;
  }

  protected function _getInputKeysFromTemplates($templatekeynames, $includeoptionalkeys = TRUE) {
    return $this->_getKeysFromTemplates($templatekeynames, $includeoptionalkeys);
  }

  protected function _getKeysFromTemplates($templatekeynames, $includeoptionalkeys = TRUE) {
    $results = array();
    foreach ($templatekeynames as $templatekeyname) {
      if ($this->hasConfigKey($templatekeyname)) {
        $template = $this->config[$templatekeyname];
        $keys = array();
        $this->_findKeysInTemplate($template, $keys, $includeoptionalkeys);
        $results[$templatekeyname] = $keys;
      }
    }
    return $results;
  }

  protected function _findKeysInTemplate($template, &$keys, $includeoptionalkeys = TRUE) {
    if (strpos($template, '{') !== FALSE) {
      //$value = $template;
      $value = preg_replace_callback(
         '/(?<!{){(?:"""(.*?)"""<)?([a-zA-Z0-9_-]+)(?:\|([a-zA-Z0-9_-]+)|\|("""(.*?)"""))?(?:>"""(.*?)""")?}/',
         function($matches) use (&$keys, $includeoptionalkeys) {
           if (isset($matches[2]) && strlen($matches[2]) > 0) {
             if ($includeoptionalkeys || (!(isset($matches[3]) && strlen($matches[3]) > 0) && !(isset($matches[4]) && strlen($matches[4]) > 0))) {
               $keys[] = $matches[2];
             }
           }
           if (isset($matches[3]) && strlen($matches[3]) > 0) {
             if ($includeoptionalkeys) {
               $keys[] = $matches[3];
             }
           }
           if (isset($matches[1]) && strlen($matches[1]) > 0) {
             $this->_findKeysInTemplate($matches[1], $keys, $includeoptionalkeys);
           }
           if (isset($matches[6]) && strlen($matches[6]) > 0) {
             $this->_findKeysInTemplate($matches[6], $keys, $includeoptionalkeys);
           }
           if (isset($matches[4]) && strlen($matches[4]) > 0) {
             if ($includeoptionalkeys) {
               $this->_findKeysInTemplate($matches[5], $keys, $includeoptionalkeys);
             }
           }
           return '';
         },
         $template
      );
    }
  }

  protected function _previousDataChangingStep() {
    return $this->previousdatachangingstep;
  }

}
