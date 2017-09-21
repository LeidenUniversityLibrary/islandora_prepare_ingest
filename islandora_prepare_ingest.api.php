<?php

/**
 * @file
 * Hooks provided by islandora prepare ingest.
 *
 *
 *  Copyright 2017 Leiden University Library
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

/**
 * Hook to collect the steps that are possible in a workflow of prepare ingest.
 *
 * @return array
 *   Returns an array where the keys are the names of possible steps.
 *   The value of each key is an array that specifies the configuration of that step.
 *   This array can contain the following keys:
 *     - 'label callback': optional key, specifies the name of the function that
 *       returns the label of the step when configuring. This function is called with
 *       an array as argument. The keys of the array the are the keys as mentioned in
 *       the arguments key. The values of the array are the values provided in the
 *       configuration. If this key does not exist, the function named 'label_' + the
 *       name of the step is called.
 *
 *     - 'work callback' : optional key, specifies the name of the function that
 *       does the actual work in this step. This function is called with 5 arguments:
 *       the configuration of this hook_prepare_step_info value, an array with the
 *       arguments of this step as provided in the configuration, an array (passed by
 *       reference) that contains the data that was made by the previous steps, an
 *       array (passed by reference) that contains context information and a number
 *       indicating the extent of the work that must be done (see 'Work extent' for
 *       more information). This function can alter the data array by adding or
 *       deleting items only if 'changes count' is set to TRUE. If this function can
 *       add or delete the keys of the items of the array only if 'changes keys' is
 *       set to TRUE. The context array can be also be altered. If this key does not
 *       exist, the function named 'work_' + the name of the step is called.
 *
 *     - 'arguments': an array containing the arguments for this step. The keys are
 *       the argument names. The value is an array with the following keys:
 *        - 'label': A human readable label for this key
 *        - 'description': Optional, further description of the key or its contents
 *        - 'type': The type of the value of this argument. See 'Argument values'
 *           for more information.
 *        - 'default_value': a default value of type 'type'
 *
 *     - 'input keys': an array containing the keys whose values should be present
 *        as keys in the data for this step to work.
 *
 *     - 'input keys callback': the name of a function. This function returns an
 *        array containing the keys whose values should be present as keys in the
 *        data for this step to work.
 *
 *     - 'output keys': an array containing the keys whose values are present as keys
 *        in the data after this step.
 *
 *     - 'output keys callback': the name of a function. This function returns an
 *        array containing the keys whose values are present as keys in the data
 *        after this step.
 *
 *     - 'changes count': boolean indicating if the step changes the count of the
 *        data array. So if items are added or removed from the data array, this
 *        should be TRUE.
 *
 *     - 'changes keys': boolean indicating if the step changes the keys of the items
 *        in the data array. So if the items get a new key, this shoud be TRUE.
 *
 *     - 'type group': the (localized) string of the type group. Can be used to
 *        group the step with similar steps.
 */
function hook_workflow_step_info() {
}

