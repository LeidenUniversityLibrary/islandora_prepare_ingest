# Islandora prepare ingest: technical

## Introduction 

This module provides a way to make workflows that help with preparing data for ingest into an Islandora repository.
In this TECHNICAL file the technical details of the working of this module are described. Read the README for general information about this module or the HOWTO file to learn how to make a new workflow and use it to prepare data for ingest.

This module defines a new hook named hook_workflow_step_info. This new hook defines the steps a workflow can contain. The module itself defines about 22 different steps that can be used to change input data into output data that is suitable for ingest into an Islandora repository. Also there are steps defined to check the output of a workflow.

hook_workflow_step_info returns an array where the keys are the identifiers (textual) of possible steps, and the value is an array with 3 key-values:
 - name: a human readable name for the step;
 - type group: a human readable name for the group of the step, can be an array with multiple names. Current groups are: 'Constants', 'Add items', 'Add key', 'Modify key', 'Files and directories', 'Sorting, filtering and grouping', 'Handling XML', 'Visual' and 'Validation';
 - class name: the name of the class that implements this step.

The class that implements a step must implement the PrepareIngestStepInterface interface or should extend the PrepareIngestStep class (which implements the PrepareIngestStepInterface interface).
The following 4 methods must be implemented by the class, the other methods are optional and have default values:

## label()
This method must return a string that is simular to the name of the step, but include the configuration if this is defined.

## arguments()
This method must return an array containing the arguments for this step. The keys are the argument names. The values are an array with the following keys:
  * 'label': a human readable label for this key;
  * 'description': optional, further description of the key or its contents;
  * 'type': the type of the value of this argument. See 'Argument values' for more information;
  * 'default_value': optional, a default value of type 'type';
  * 'allow_constants': optional, this value allows constants.
If the step does not need arguments, return an empty array.

## dryRun()
This method must do a dryrun of the actions the step does. This method should manipulate the data, but only temporary, so don't make/change any files/directories (use pifs instead) or do any other "real" things.
The data_cache functions should be used to read the items of the previous step and store the manipulated items (in case the step only changes keys), or the old and new items (in case the step adds new items), or no items at all (e.g. step does validation only).
This method should return a value as described in 'Return values for ...'
The prepareIngest method will do the actual writing of files/directories and other "real" things based on the output of this step.
The method is run as part of an operation of a batch set. The context is the only argument to this method. If needed, the work done in this method can be divided in smaller pieces by using ```$context['finished']``` with a value less than 1 to call this method multiple times within the operation. See ```steps/TransformXmlStep.inc``` for an example.

## prepareIngest()
This method does the "real" things, usually based on the (temporary) output. If dryRun has file data saved as cache data, write the files to disk in this step. Do not do what dryRun does again, but reuse its output.
This method should return a value as described in 'Return values for ...'

## inputKeys()
Only overwrite when needed. Returns an array containing the keys whose values should be present as keys in the data for this step to work. The keys of this array indicate the argument name where the key(s) is coming from. The values are arrays with zero or more keys. E.g.: array('argument name' => array('key for data'))
In most cases, overwrite the protected method _inputKeyKeys() that simply returns an array of input keys, a subset of the keys returned from the arguments method.

## outputKeys()
Only overwrite when needed. Returns an array containing the keys whose values are present as keys in the data after this step. The keys of this array indicate the argument name where the key(s) is coming from. The values are arrays with zero or more keys. E.g.: array('argument name' => array('key for data'))
In most cases, overwrite the protected method _outputKeyKeys() that simply returns an array of output keys, a subset of the keys returned from the arguments method.

## changesCount()
Only overwrite when needed, Returns a boolean indicating if the step changes the count of the data array. So if items are added or removed from the data array, this should be TRUE. Returns FALSE by default.

## changesKeys()
Only overwrite when needed. Returns a boolean indicating if the step changes the keys of the items in the data array. So if the items get a new key, this shoud be TRUE. Returns FALSE by default.

## changesFiles()
Only overwrite when needed. Returns a boolean indication if the step adds and/or changes files on the file system. Returns FALSE by default.

## visual()
Only overwrite when needed. Returns an array indication how this step should visually by represented. Possible values are groupstart (start of visual grouping), groupremove (show remove button for visual group), ungroup (show ungroup button for visual group), groupend (end of visual grouping), nomove (do not show move buttons) and noremove (do not show remove button). Returns empty array by default.

## promptValueOnPrepareIngest($key)
Only overwrite when needed. Returns a string indicating the prompt for a specific key. Return FALSE when this is not needed. Returns FALSE by default. When a prepare ingest is run, this method is called (when available) on every argument of every step. This way, the user can change values of arguments before doing the prepare ingest.

## checkConfiguration($context)
Only overwrite when needed. Checks the configuration of this step. This method may be overwritten if you need to implement more checks of the configuration of this step, e.g. if the step does something with files you should check if directories and/or files exists if read from.
This method should return a value as described in 'Return values for ...'

## verify($context)
Only overrwrite when needed. Verifies the input (and possibly output) data of this step. This method may be overwritten of you need to implement more verifications of the input (and output) data of this step.
This method should return a value as described in 'Return values for ...'

## Return values for dryRun, prepareIngest, checkConfiguration and verify
The methods dryRun, prepareIngest, checkConfiguration and verify should return FALSE if they finish successfully, or return an array containing one or more entries if something went wrong. Each entry is an associated array with keys: text and type, and optionally key. The text key has a value (string) that explains what is wrong with the run. The type key indicates the type of error and can be run_error or validate_error. The optional key key holds the name of the key where the run went wrong.


## Argument values
The arguments of a step can have one of the following types:
 - number: a numeric value without fractions
 - string: a textual value. Can contain any value.
 - boolean: a true or false value.
 - template: a special kind of text. Can contain the names of keys, surrounded in curly brackets. These will be replaced. Visualized as a text area.
 - templatestring: same as the template, but visualized as a text field.
 - key: the name of a key, only the following characters are allowed: a-z A-Z 0-9 - _
 - filepath: the path to a directory of file, that exists or not.
 - regexp: a valid regular expression.
 - DOM: an XML DOM
 - xpath: an absolute xpath
 - collection: the identifier of an existing collection
 - namespace: a valid namespace for Islandora objects, without the ending colon

Before programming a new step, check out the existing steps. You can learn how to make a step and also see if the functionality of the step is not covered by an existing step. The functionality of a step should be as general and lean as possible.

