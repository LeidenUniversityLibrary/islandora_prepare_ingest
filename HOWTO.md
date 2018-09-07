# Islandora Prepare Ingest

## Introduction

Islandora Prepare Ingest helps you prepare data for ingest into Islandora. You can define a workflow that converts your input data into data that can be ingested into Islandora.
Islandora Prepare Ingest uses the concept of steps that manipulate the data in a list of items containing key-value pairs. These steps are grouped and form a part of a workflow or a whole workflow.
There are different steps available to for example read the filenames in a directory, parse a XML file or read a CSV file. This way data is obtained into the workflow. Other steps can manipulate that data, for example by adding data based on existing data. Then there are steps that use the data to write files. There are also steps to manipulate the list itself, for example by filtering unwanted items or grouping items together.

Every step can do any or none of the following with the list of items:
- manipulate the list itself, by adding or removing items in the list.
- manipulate the items in the list, by adding or changing a key-value pair of all items in the list.
- manipulate the filesystem based on the list of items

Most steps do only one of the actions above, but this is not mandated. There are steps that don’t do any of the actions above, but only validate the list of items and/or the file system.

For a complete list of available steps, see the documentation below named “The steps”.

# Using Islandora Prepare Ingest

## Making a workflow

When this module is installed, under Admin > Islandora there is an entry “Prepare Ingest”. Here you will find a list of all workflows, a way to add a new workflow or import an existing, and delete, copy or export existing workflows.

To make a new workflow click “Add New workflow”. A screen is presented with fields for the name and description of the workflow. Because workflows can be reused and are not specific for a single data set, let the name and description reflect this.
After saving the workflow, you can add steps to the workflow by choosing the step from the popup menu and clicking on “Add Step”.
After adding a step to the workflow, the step must be configured.  This can be done by filling out the fields.
Use the “define constant” step for values that are not general in the workflow, so these values can be easily replaced. It is recommended (but not necessary) to define all constants at the beginning of your workflow. When the workflow is run for real output, the values of the constants can be changed.
Make sure that for every key in every step all input items have a value for that key, otherwise use the filter items step (combined with the end filter step) to filter the items that do not have the key before the items in the list are given to the step that requires the key.
Add additional steps until the workflow will manipulate the input data in such a way that it is suitable for ingest into Islandora.
As the last step you should add a “validate” step to make sure your workflow generates a valid ingest directory.

If the order of your steps is incorrect, you can reorder the steps with the “move to” button and then choose the location where you want to move it to. If you have inserted the wrong step, you can delete the step with the “remove” button.

Save your workflow after any changes you make.

## Using constants

The define constant step lets you define a constant. Because when you make a workflow some steps will have specific values, you can use constants instead of these specific values. Define all the constants you make at the top of your workflow, so it is easier to spot them.

## Dryrun a workflow

After you have completed your workflow and saved it, you can dryrun it to make sure it is correct. Press the “Dryrun workflow” button to check if the workflow configuration is correct. If there is something wrong, you will be notified of what is wrong. 
While dryrunning the files will only be read. Any manipulation of files will only be done in memory, so no files will be changes, removed or added for real.
If the dryrun completes successfully, make sure to check the output of every step. If a step manipulates the key-value pairs of items, the items are shown. If a step alters the file system, the files are listed.
Errors in the steps will be shown with the step. You can see the data the step generated. You can edit the step configuration by pressing the "edit" button.

## Run a workflow

To run a workflow, you must run it via drush in the following way:
```drush -v --user=user_name prepare_ingest```

This command should be run on the command line inside the drupal root directory as a user with adequate rights.

You can choose from all checked workflows (the workflows that have been dryrun successfully).

Depending on the workflow, you must answer some questions before the workflow is run. For example, if the workflow contains constants you get the possibility to change the value of that constant. Also, validation steps let you preprocess and/or ingest the data, but will ask before proceeding.


# The steps

Included in the prepare ingest module are many basic steps. These steps should be sufficient to make any workflow, but other steps can be added from within other modules. See the TECHNICAL documentation on how to do this.
Below are the included steps with a description of what they do:

## Define constant

This step defines a constant for use in subsequent steps.

## Add items by reading file names

This step reads the file names of the files in a specified directory and stores the full paths of the file names using the given key in new items that will be added to the list. A filter can be used to only include files with a specific name and/or extension. If wanted and needed, also files in subdirectories can be included.

## Add key with regular expression based value

This step adds a new key-value to all existing items in the list. The target key is given. The values for this key are based on the values of an existing key. These values are modified by the given regular expression and replacement value to obtain the new values for the new key.
The regular expression can match part of the value or the whole value. The replacement can include backreferences ($1, $2, …) of the capturing groups in the regular expression.

## Add key with template based value

This step adds a new key-value to all existing items in the list. The target key is given. The values for this key are based on the values of one or more existing keys. A template is used to make the new value. The template can include one or more existing keys placed between curly braces, like ```{an_existing_key}```. It is possible to have an alternative key when the existing key is empty or non-existent yet, like ```{an_existing_key|alternative_key}```. It is also possible to provide a constant alternative like so ```{an_existing_key|"""constant alternative"""}```. If you want a prefix and/or suffix, you can apply it like this: ```{"""prefix"""<an_existing_key>"""postfix"""}```. Also, constant alternatives, prefixes and postfixes can contain keys in curly braces.

## Add key(s) using template

The template should reflect the value of the source key. The keys are in the place of the extracted value and between curly braces. Example: if the value is “test_1.tif” and you want to extract both the number and the extension, then you can use the following: test_{number}.{extension}. Remember that the template values are greedy; this means that they want to contain the largest possible value that will match the template. In the example above the value “test_1.a.tif” will result in a key “number” with value “1.a” and a key “extension” with value “tif”.
In this situation, you can use a range to specify what type of characters the value of that key can contain. In the above example, if you use test_{number[0-9]}.{extension} the key "number" will contain "1" (but the key "extension" contains "a.tif" which might not be what you expect).
Use the step “Add key with regular expression based value” if you need more control.

## Add key with counter value

This step adds a new key-value to all existing items in the list. The target key is given. The values for this key is a number that starts at the start value, increments with a value of step and includes leading zero’s if the given width is greater than the actual width of the number. Optionally, you can specify a key for each unique value the counting starts from the start value.

## Change the value of a key

This step changes the value of a specific source key in one of the following ways: to uppercase, to lowercase, remove -, remove - or _, remove spaces, remove spaces from front and end of string, replace - with space, replace - or _ with space, pad zeros to a number to get 5 numbers, strip tags, to SHA1 hash, XML encode or XML decode.

## Parse XML file

This step adds a new key-value to all existing items in the list. The target key is given. The value is the Document Object Model (DOM) of an existing XML file identified by a key in the existing items in the list. This step can be used in combination with one or more “add key from XML” or “add items by extracting XML parts from XML” steps.

## Add key from XML

This step adds a new key-value to all existing items in the list. The target key is given. The value is retrieved from a DOM identified by a key of the existing items in the list by using a given XPath and namespaces. This step should be used after the “parse XML file” step. 

## Make directory

This step creates a directory in the file system based on the values of one or more keys in all existing items in the list. The value should be an absolute path to a not yet existing directory, but if the directory exists already no action is taken. The parent directory of this directory should exist.

## Copy file

This step copies a file indicated by a source filepath from values of one or more keys in all existing items in the list to a target indicated by a target filepath from values of one or more keys in all existing items in the list. The source filepath should be an absolute path to an existing file. The target filepath is also an absolute path; either to an existing directory, in which case the name of the file is retained. Or the target value is a absolute full path to a not existing file in an existing directory. The copy action is not actually always a copy of the data: first a hard link is tried to save disk space. If this fails, the file is copied.

## Write to file

This step writes to a file indicated by a filepath from values of one or more keys in all existing items in the list. The content that is written to the file is a the value of the content key in all existing items in the list. The target file should not exist when starting to write to the file, but it is possible to write the contents of a key of multiple existing items to the same file. No warning will be given if the file already exists. The directory containing the output file should exist.

## Group items

This step groups the existing items in the list, thus reducing the number of items in the list. The grouping is done by the value of the group key. If two items in the list have the same value of the group key, the key-values of both items will be merged into one item. This will be done to all items in the list. If items have the same key (other than the group key), the value of the latter item is kept.

## Filter items

This step filters the items in the list, thus reducing the list to only the items that match the filter configuration. The filtering is done on the value of a filter key of all existing items in the list. If the filter type is “key exists”, only items that include that key will remain in the list. If the filter type is “equals”, only items whose value of the filter key is equal to the filter value remain in the list. If the filter type is “matches”, only items whose value of the filter key matches the regular expression of the filter value remain in the list. A regular expression should be enclosed by delimiters, often forward slashes (/), hash signs (#) or percentage (%), but also brackets are allowed ({}).
Negate true reverses the logic, so any items that do not match the filter type and value will remain in the list.
Filtered items are not permanently gone; they can be recovered by a “end filter” step.
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

This step does not do anything but visually group other steps. This makes it easier to see in the workflow which steps belong together. Also possible to describe the grouped steps.

## Visual group end

This step ends the most recent visual group.

## Sort items

This step sorts the items based on the values of one or two keys.

## Transform XML

This step transforms XML by using XSLT to a target filepath. The XML key can include a filepath to an XML file or XML data. The XSLT key can include a filepath to an XSLT file or the XSLT data itself. The keys of the items are available as parameters within the XSLT.

## Validate XML

This step validates XML, optionally using a DTD and/or a schema. The XML key can include a filepath to an XML file or XML data.

## Validate the basic images structure

This step validates a structure of files/directories identified by a path and checks that it is a valid basic images structure.
When the workflow is run, the namespace and collection are asked, so the data can be ingested immediatedly.

## Validate the large images structure

This step validates a structure of files/directories identified by a path and checks that it is a valid large images structure.
When the workflow is run, the namespace and collection are asked, so the data can be ingested immediatedly.

## Validate the books structure

This step validates a structure of files/directories identified by a path and checks that it is a valid books structure.
When the workflow is run, the namespace and collection are asked, so the data can be ingested immediatedly.

