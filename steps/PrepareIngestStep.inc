<?php

interface PrepareIngestStepInterface {

  public function label();

  public function checkConfiguration();

  public function dryRun(&$context);

  public function verify($context);

  public function prepareIngest(&$context);

}

class PrepareIngestStep implements PrepareIngestStepInterface {

  protected $info;
  protected $config;
  protected $workflowid;
  protected $stepid;
  protected $otherid;
  private $dataiterator;

 /**
  * Initializes a PrepareIngestStep object with the appropriate values.
  * There should be no need to overwrite this function, it is automatically called when starting a step.
  *
  * @param array $stepinfo
  *   An associated array with the info from the step, as returned by hook_workflow_step_info.
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
  public function __construct(array $stepinfo, array $stepconfig, $workflowid, $stepid, $otherid) {
    $this->info = $stepinfo;
    $this->config = $stepconfig; 
    $this->workflowid = (int)$workflowid;
    $this->stepid = (int)$stepid;
    $this->otherid = $otherid;
  }
  
 /**
  * Returns the label of this step based on its current configuration.
  **/
  public function label() {
    return $this->info['name'];
  }

 /**
  * Checks the configuration of this step.
  *
  * This method may be overwritten of you need to implement more checks of the configuration of this step, e.g.
  * if the step does something with files you should check if directories and/or files exists if read from.
  *
  * @return array
  *   Returns an array with zero (check succesful) or more entries (something wrong with the configuration). Each entry is an associated
  *   array with 3 keys: key, text and type.
  *   The key key holds the name of the key where the configuration is wrong.
  *   The text key has a value (string) that explains what is wrong in the configuration.
  *   The type key indicates the type of error and should be argument_error.
  **/
  public function checkConfiguration() {
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

  protected function resetDataIterator() {
    $this->dataiterator = array();
    $this->dataiterator['selectquery'] = _islandora_prepare_ingest_data_cache_select_query($this->workflowid, $this->stepid, $this->otherid, 1);
    $this->dataiterator['lastdata'] = $this->dataiterator['selectquery']->fetchAssoc;
    $this->dataiterator['currentitemnr'] = 1;
  }

  protected function nextDataItem() {
    $dataitem = array();
    $currentitemnr = $this->dataiterator['currentitemnr'];
    $d = $this->dataiterator['lastdata'];
    $sq = $this->dataiterator['selectquery'];
    while ($d !== FALSE) {
      if ($d['item_nr'] > $currentitemnr) {
        $this->dataiterator['currentitemnr'] = $d['item_nr'];
        break;
      }
      $dataitem[$d['key']] = $d['value'];
      $d = $sq->fetchAssoc();
      $this->dataiterator['lastdata'] = $d;
    }
    return $dataitem;
  }

  protected function saveDataItem($dataitem) {
    if (!isset($dataitem['item nr'])) {
      throw new UnexpectedValueException("Data item is missing an item nr key.");
    }
    $itemnr = $dataitem['item nr'];
    islandora_prepare_ingest_data_cache_single_insert($workflow_id, $step_id, $other_id, 1, $itemnr, $dataitem);
  }

  protected function fillOutTemplate($template, $keyvalues) {
    if (strpos($template, '{') !== FALSE) {
      $keys = array();
      $values = array();
      foreach ($keyvalues as $k => $v) {
        if (strlen($k) > 0 && isset($v)) {
          $keys[] = '{' . $k . '}';
          $values[] = $v;
        }
      }
      $template = str_replace($keys, $values, $template);
    }
    return $template;
  }

  protected function _checkConfiguration() {
    $results = array();
    $outputkeys = array();
    if (isset($this->info['output keys callback'])) {
      $outputkeyscallback = $this->info['output keys callback'];
      if (method_exists($this, $outputkeyscallback) !== TRUE) {
        $classname = get_class($this);
        throw new BadMethodCallException("Method '$outputkeyscallback' not implemented for class '$classname'.");
      }
      $outputkeys = $this->$outputkeyscallback();
    }
    elseif (isset($this->info['output keys'])) {
      $outputkeys = $this->info['output keys'];
    }
    if (isset($this->info['arguments'])) {
      $args = $this->info['arguments'];
      foreach ($args as $argkey => $argconfig) {
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
              if (preg_match("/^[a-zA-Z0-9_;-]+$/", $value) === 0) {
                $results[] = array('text' => t("Argument '@key' should be one or more key values separated by a semicolon, but is '@value'",
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
          if (in_array($argkey, $outputkeys)) {
            $results[$argkey] = array('text' => t('Missing argument \'@argname\'', array('@argname' => $argconfig['label'])), 'key' => $argkey, 'type' => 'argument_error');
          }
        }
      }
    }
    else {
      throw new InvalidArgumentException(t("Missing arguments in info for step class '@stepclass.", array('@stepclass' => get_class($this))));
    }
    return $results;
  }

  protected function _checkInputKeysInData() {
    module_load_include('inc', 'islandora_prepare_ingest', 'includes/data_cache');
    $results = array();
    $inputkeys = array();
  
    if (array_key_exists('input keys callback', $this->info)) {
      $inputkeyscallback = $this->info['input keys callback'];
      if (method_exists($this, $inputkeyscallback) !== TRUE) {
        $classname = get_class($this);
        throw new BadMethodCallException("Method '$inputkeyscallback' not implemented for class '$classname'.");
      }
      $inputkeys = $this->$inputkeyscallback();
    }
    elseif (array_key_exists('input keys', $this->info)) {
      $inputkeys = $this->info['input keys'];
      foreach ($inputkeys as $ikey) {
        if (isset($this->info['arguments'][$ikey]['optional'])) {
          if (is_array($this->info['arguments'][$ikey]['optional'])) {
            foreach ($this->info['arguments'][$ikey]['optional'] as $optkey => $optvalues) {
              if (in_array($this->config[$optkey], $optvalues)) {
                continue 2;
              }
            }
          }
          else {
            if ($this->info['arguments'][$ikey]['optional']) {
              continue;
            }
          }
        }
        if (!isset($this->config[$ikey]) || (strlen($this->config[$ikey]) == 0)) {
          $results[] = array('text' => t("Missing argument '@key'.", array('@key' => $this->info['arguments'][$ikey]['label'])),
                             'key' => $this->info['arguments'][$ikey]['label'], 'type' => 'config_error');
          continue;
        }
        $key = $this->config[$ikey];
        $inputkeys[] = $key;
      }
    }
    $itemscount = islandora_prepare_ingest_data_cache_per_step_count($this->workflowid, $this->stepid, $this->otherid, 1);
    foreach ($inputkeys as $key) {
      $keycount = islandora_prepare_ingest_data_cache_per_step_key_count($this->workflowid, $this->stepid, $this->otherid, 1, $key);
      if ($keycount === 0) {
        $results[] = array('text' => t("Missing key '@key'. This key is not defined in any of the previous steps.", array('@key' => $key)),
                           'key' => $key, 'type' => 'key_error');
      }
      else if ($itemscount > $keycount) {
        $results[] = array('text' => t("Missing key '@key'. This key is defined in a previous step but not available for all items. Use a filter before this step to keep the items with the right keys.", array('@key' => $key)),
                           'key' => $key, 'type' => 'key_error');
      }
    }
    return $results;
  }
}
