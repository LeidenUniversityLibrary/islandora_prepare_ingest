# Prepare Ingest

## Introduction

Prepare Ingest helps you prepare data for ingest into Islandora. You can define a workflow that converts your input data into data that can be ingested into Islandora.
Prepare Ingest uses the concept of steps that manipulate the data in a list of items containing key-value pairs. These steps are grouped and form a part of a workflow or a whole workflow.
There are different steps available to for example read the filenames in a directory, parse a XML file or read a CSV file. This way data is obtained into the workflow. Other steps can manipulate that data, for example by adding data based on existing data. Then there are steps that use the data to write files. There are also steps to manipulate the list itself, for example by filtering unwanted items or grouping items together.

Every step can do any or none of the following with the list of items:
- manipulate the list itself, by adding or removing items in the list.
- manipulate the items in the list, by adding or changing a key-value pair of all items in the list.
- manipulate the filesystem based on the list of items

Most steps do only one of the actions above, but this is not mandated. There are steps that don’t do any of the actions above, but only validate the list of items and/or the file system.

For a complete list of available steps, see the documentation below named “The steps”.

# Using Prepare Ingest

## Making a workflow

When this module is installed, under Admin > Islandora there is an entry “Prepare Ingest”. Here you find two tabs: 
* “Generic workflows”: here you can define new (parts of) workflows, that can later be used as the base of an active workflow. These workflows are more general and can be reused for multiple data conversions and ingests.
* “Active workflows”: this contains the workflows you can actually use to prepare an ingest for Islandora. The workflows listed here are specific for one data conversion and ingest.

To make a new workflow, first go to the “Generic workflows” tab and click “Add New workflow”. A screen is presented with fields for the name and description of the workflow. Because we are going to make a general workflow that can be reused and is not specific for a single data set, let the name and description reflect this.
After saving this workflow, you can add steps to the workflow by choosing the step from the popup menu and clicking on “Add Step".
After adding a step to the workflow, the step must be configured.  This can be done by clicking the title of the step to open it, and filling out the fields.
Because this will be a general workflow, not all fields have to be filled in. All fields with the word “key” in their name define or use some data. These should all be filled out.
Make sure that for every key in every step all input items have a value for that key, otherwise use the filter items step (combined with the end filter step) to filter the items that don't have the key before the items in the list are given to the step that requires the key.
Add additional steps until the workflow will manipulate the input data in such a way that it is suitable for ingest into Islandora.
As the last step you should add a “validate” step to make sure your workflow generates a valid ingest directory.

If the order of your steps is incorrect, you can reorder the steps with the “move up” and “move down” buttons. If you have inserted the wrong step, you can delete the step with the “remove” button.

Save your workflow after any changes you make.

## Using constants

Th define constant step lets you define a constant. Because when you make a generic workflow some steps will have specific values, you can use constants instead of these specific values. Define all the constants you make at the top of your workflow, so it is easier to spot them.

## Checking a workflow

After you have completed your workflow and saved it, you can check it to make sure it is correct. Press the “check workflow” button to check if the workflow configuration is correct. If there is something wrong, you will be notified of what is wrong. Any steps you need to correct, will be opened. If the configuration of the workflow steps is correct, you see a message that the checks finished successful.

## Testing a workflow

After successfully checking a workflow, the workflow can be tested. If you did not fill out al the fields in the step configurations (which is good when you make a general workflow), you can fill in some test values in those fields. Preferably, these values are all constants that you should fill out with a specific value.
Remember that although it is a test, the values should be correct. For example if you have to fill out a directory path, you should provide a valid path that exists on the server and that contains files of the type the workflow step expects.
While testing the files will only be read. Any manipulation of files will only be done in memory, so no files will be changes, removed or added for real.
After filling out the missing values, you can test the workflow. If the test completes successfully, make sure to check the output of every step. If a step manipulates the key-value pairs of items, the items are shown for that step. If a step alters the file system, the files are listed.

## Making an active workflow

When you have made a full workflow and checked and tested it, you can make an active workflow based on this full workflow. Go to the active workflows tab, choose the workflow that will be the base of the active workflow in the popup menu in the “add workflow” section and click on “add new workflow”. Fill out a descriptive name and description for your new active workflow.
After this fill out the missing values in the step configuration with specific values. Again this should all be constants at the top of the workflow. Save the workflow when you are done.

## Dry run an active workflow

When you have made an active workflow and filled out all the missing step configuration, you can dry run the workflow. This means that you run the entire workflow with all the data, but only in memory, so without making any changes to the file system. This way you can check the workflow with the real data without polluting your file system.
If the workflow runs successfully, you will see how you can run the active workflow for real.

## Run an active workflow

When the dry run has finished, on the top of the page the drush command is shown that will run the active workflow for real.
The command looks like this: 	drush -v --user=user_name prepare_ingest --workflow=workflowid

This command should be run on the command line inside the drupal root directory as a user with adequate rights.

If you have include validate steps, these steps will output the drush command to do the actual ingest.


# The steps

Included in the prepare ingest module are 23 steps. These steps should be sufficient to make any workflow, but other steps can be added from within other modules. See the TECHNICAL documentation on how to do this.
Below are the included steps with a description of what they do:

## Define constant

## Add items by reading file names

This step reads the file names of the files in a specified directory and stores the full paths of the file names with the given key in new items that will be added to the list. A filter can be used to only include files with a specific name and/or extension. If wanted and needed, also files in subdirectories can be included.

## Add key with regular expression based value

This step adds a new key-value to all existing items in the list. The key is given. The values for this key are based on the values of an existing key. These values are modified by the given regular expression and replacement value to obtain the new values for the new key.
The regular expression can match part of the value or the whole value. The replacement can include backreferences ($1, $2, …) of the capturing groups in the regular expression.

## Add key with template based value

This step adds a new key-value to all existing items in the list. The key is given. The values for this key are based on the values of one or more existing keys. A template is used to make the new value. The template can include one or more existing keys placed between curly braces.

## Add key(s) using template

The template should reflect the value of the source key. The keys are in the place of the extracted value and between curly braces. Example: if the value is "test_1.tif" and you want to extract both the number and the extension, then you can use the following: test_{number}.{extension. Remember that the template values are greedy; this means that they want to contain the largest possible value that will match the template. In the example above the value "test_1.a.tif" will result in a key "number" with value "1.a" and a key "extension" with value "tif".

## Add key with counter value

This step adds a new key-value to all existing items in the list. The key is given. The values for this key is a number that starts at the start value, increments with a value of step and includes leading zero’s if the given width is greater than the actual width of the number. Optionally, you can specify a key for each unique value the counting starts from the start value.

## Parse XML file

This step adds a new key-value to all existing items in the list. The key is given. The value is the Document Object Model (DOM) of an existing XML file identified by a key in the existing items in the list. This step is used in combinations with one or more “add key from XML” or "add items by extracting XML parts from XML" steps.

## Add key from XML

This step adds a new key-value to all existing items in the list. The key is given. The value is retrieved from a DOM identified by a key of the existing items in the list by using a given XPath and namespaces. This step should be used after the “parse XML file” step. 

## Make directory

This step creates a directory in the file system based on the values of one or more keys in all existing items in the list. The value should be an absolute path to a not yet existing directory. The parent directory of this directory should exist.

## Copy file

This step copies a file indicated by a source filepath from values of one or more keys in all existing items in the list to a target indicated by a target filepath from values of one or more keys in all existing items in the list. The source filepath should be an absolute path to an existing file. The target filepath is also an absolute path; either to an existing directory, in which case the name of the file is retained. Or the target value is a absolute full path to a not existing file in an existing directory. The copy action is not actually always a copy of the data: first a hard link is tried to save disk space. If this fails, the file is copied.

## Write to file

This step writes to a file indicated by a filepath from values of one or more keys in all existing items in the list. The content that is written to the file is a the value of the content key in all existing items in the list. The target file should not exist when starting to write to the file, but it is possible to write the contents of a key of multiple existing items to the same file. No warning will be given if the file already exists. The directory containing the output file should exist.

## Group items

This step groups the existing items in the list, thus reducing the number of items in the list. The grouping is done by the value of the group key. If two items in the list have the same value of the group key, the key-values of both items will be merged into one item. This will be done to all items in the list. If items have the same key (other than the group key), the value of the latter item is kept.

## Filter items

This step filters the items in the list, thus reducing the list to only the items that match the filter configuration. The filtering is done on the value of a filter key of all existing items in the list. If the filter type is 'key exists', only items that include that key will remain in the list. If the filter type is 'equals', only items whose value of the filter key is equal to the filter value remain in the list. If the filter type is 'matches', only items whose value of the filter key matches the regular expression of the filter value remain in the list. A regular expression should be enclosed by delimiters, often forward slashes (/), hash signs (#) or percentage (%), but also brackets are allowed ({}).
Negate true reverses the logic, so any items that do not match the filter type and value will remain in the list.
Filtered items are not permanently gone; they can be recovered by a 'end filter' step.
Filters can be nested.

## End filter

This step ends the most recent filter and recovers the filtered items.

## Add items by reading CSV file

This step reads the given CSV file, parses it and stores the values of a row at the keys found in column keys. For every row a new item is added to the list. The CSV file should have at least the number of columns as the count of column keys, but can have more. Any additional columns in the CSV file will not be included in the items. If a column key is double in the column keys, only the last column will be stored in the item.
If the CSV file has a header, set has header to true to avoid making an item with the header values.
The CSV file columns can be separated by a tab, a comma or a semicolon. The rows should be ended by a line ending (Windows, Mac or Unix line endings).

## Add items by extracting XML parts from XML

This step adds items for every extracted XML part from a key in existing items that contains XML. The XML part is pointed to by an XPath.

## Visual group start

This step does not do anything but visually group other steps. This makes it easier to see in the workflow which steps belong together. Also possible to descibe the grouped steps.

## Visual group end

This step ends the most recent visual group.

## Sort items

This step sorts the items based on the values of one or two keys.

## Validate the basic images structure

This step validates a structure of files/directories identified by a path and checks that it is a valid basic images structure. The namespace and collection values are used to give a full drush command on how to ingest the data when the workflow is run.

## Validate the large images structure

This step validates a structure of files/directories identified by a path and checks that it is a valid large images structure. The namespace and collection values are used to give a full drush command on how to ingest the data when the workflow is run.

## Validate the books structure

This step validates a structure of files/directories identified by a path and checks that it is a valid books structure. The namespace and collection values are used to give a full drush command on how to ingest the data when the workflow is run.

