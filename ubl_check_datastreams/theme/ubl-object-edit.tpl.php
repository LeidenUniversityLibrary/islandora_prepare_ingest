<?php

/**
 * @file
 * The default manage datastreams view for objects.
 *
 * islandora_object is a fedora tuque Object
 *    $object->label
 *    $object->id
 * to get the contents of a datastream
 *    $object['dsid']->content
 *
 * $dublin_core is a DublinCore object
 * which is an array of elements, such as dc.title
 * and each element has an array of values.
 * dc.title can have none, one or many titles
 * this is the case for all dc elements.
 */
?>
<?php print (theme_table($variables['datastream_table'])); ?>
<?php
if (isset($variables['check_ds_table'])) { 
  print '<BR/><BR/><BR/>'; // lousy, can this be done in another way?
  print (theme_table($variables['check_ds_table']));
}
if (isset($variables['check_ds_children'])) {
  print '<BR/>'; // lousy, can this be done in another way?
  print $variables['check_ds_children']; 
  print '<BR/><BR/>'; // lousy, can this be done in another way?
}
?>

