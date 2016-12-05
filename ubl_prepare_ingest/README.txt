ubl_prepare_ingest

This module provides a way to make workflows that help with preparing data for ingest into an Islandora repository.
In this README file the technical details are described of the working of this module. Read the HOWTO file to learn how to make a new workflow and use it to prepare data for ingest.

This module defines a new hook named hook_workflow_step_info. This new hook defines the steps a workflow can contain. The module itself defines 16 different steps that can be used to change input data into output data that is suitable for ingest into an Islandora repository. Also there are steps defined to check the output of a workflow.

hook_workflow_step_info returns an array where the keys are the names of possible steps. The name of a step is used as the bases of the 2 functions that every step must implement: label_<stepname> and work_<stepname>. The label function gets called with the steps info (described later) array and a configuration array as the second argument. The configuration array can contain the (zero or more) keys of the arguments array of the steps info array. The label function should return a string that is used as the step title in the user interface. It is recommended to contain the values of the configuration (if any) in the returned string.
The work function also gets called with the steps info array and a configuration array. Also a data array and a context array are passed by reference, so the workfunction can do the work and alter the data array. The context array can be used to keep a context between steps. For example, it is used with the included filter_items and end_filter steps.
The work function of the step should alter the data array in such a way as described by the step info array. The context array can be used and/or altered if needed by the step.
The last argument of the workfunction is the extent. This lets the work function know to what extent the work should be done. The following values of the extent should be handled by the work function:
 - WORK_EXTENT_ONLY_CHECK_INPUT: 


The hook_workflow_step_info returns an array that has a value of each key, this is an array containing the following keys:

 - 'arguments': an array containing the arguments for this step. The keys are the argument names. The value is an array with the following keys:
   - 'label': A human readable label for this key
   - 'description': Optional, further description of the key or its contents
   - 'type': The type of the value of this argument. See 'Argument values' for more information.
   - 'default_value': a default value of type 'type'
   -

 - 'input keys': an array containing the keys whose values should be present as keys in the data for this step to work.

 - 'input keys callback': the name of a function. This function returns an array containing the keys whose values should be present as keys in the data for this step to work.

 - 'output keys': an array containing the keys whose values are present as keys in the data after this step.

 - 'output keys callback': the name of a function. This function returns an array containing the keys whose values are present as keys in the data after this step.

 - 'changes count': boolean indicating if the step changes the count of the data array. So if items are added or removed from the data array, this should be TRUE.

 - 'changes keys': boolean indicating if the step changes the keys of the items in the data array. So if the items get a new key, this shoud be TRUE.


Argument values:
The arguments of a step can have one of the following values:
 - number: a numeric value without fractions
 - string: a textual value. Can contain any value.
 - boolean: a true or false value.
 - template: a special kind of string. Can contain the names of keys, surrounded in curly brackets. These will be replaced.
 - key: the name of a key, only the following characters are allowed: a-z A-Z 0-9 - _
 - filepath: the path to a directory of file, that exists or not.
 - regexp: a valid regular expression.
 - DOM: an XML DOM
 - xpath: an absolute xpath

Work extent:
The following levels of work are available:
 - WORK_EXTENT_CHECK_INPUT: only checks if the proper keys are given by the previous step.
 - WORK_EXTENT_CHECK_ARGUMENTS: checks if the proper kets are given and checks if the arguments are all filled out with the correct values.
 - WORK_EXTENT_DRY_RUN: does the above two levels and does a dry run: this means that all steps are done but writing of data is done to a sandbox (internal memory).
 - WORK_EXTENT_DO_ALL: does the actual work, preceded by the above levels.
