# ubl_check_datastreams
===========================

## Check datastreams

With this module it is possible to check if all the necessary datastreams are available for an item. When you go to manage > datastreams of an item you see another table with the missing datastreams of that item, when appropriate with an generate link.
Books, compounds and collections also have a link to check the children, but this can take a long time.
When you check the children, you have the ability to download a CSV file with identifiers of objects with missing datastreams by datastream ID. This file can be used in the ubl_batch_ingest_datastreans module.

## drush

Because the checking of datastreams can take a long time, there is also a drush command.

The commands you can use are:

### check_datastreams

This checks tho datastreams of the requested object and prints the counts of missing datastreams.

This command needs two options:
 - objectid: The object id of a specific collection, book or compound, or the batch set id.
 - objecttype: The type of the object. Possible values are: collection, book, compound or batch.

Use this with a user with appropriate rights.


Examples:

 - drush --user=admin check_datastreams --objectid=islandora:root --objecttype=collection
 - drush --user=admin cds --objectid=2 --objecttype=batch


### missing_datastreams_report

This prints a CSV of the missing datastreams for the requested object, content model and datastream id. You can use this in the ubl_batch_ingest_datastreans module.

This command needs four options:
 - objectid: The object id of a specific collection, book or compound, or the batch set id.
 - objecttype: The type of the object. Possible values are: collection, book, compound or batch.
 - cmodel: The content model of the objects of the datastreams you are interested in.
 - dsid: The datastream id of the objects you want in the report.

Use this with a user with appropriate rights.


Examples:
 - drush --user=admin missing_datastreams_report --objectid=islandora:root --objecttype=collection --cmodel=islandora:pageCModel --dsid=JP2
 - drush --user=admin mdsr --objectid=2 --objecttype=batch --cmodel=islandora:sp_large_image_cmodel --dsid=TN




