<?php

/**
 * @file
 * ubl_prepare_ingest_prepare.form.inc
 */

/**
 * The form for setting up one prepare ingest workflow
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function ubl_prepare_ingest_prepare_form(array $form, array &$form_state) {

  $form['prepare'] = array(
    '#title' => t('Prepare'),
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  # source
  $form['prepare']['directory_path'] = array(
    '#title' => t('Input directory'),
    '#type' => 'textfield',
    '#default_value' => '/var/ingest/UBL/some_directory',
    '#size' => 60,
  );

  # target
  $form['prepare']['directory_path'] = array(
    '#title' => t('Output directory'),
    '#type' => 'textfield',
    '#default_value' => '/var/ingest/UBL/some_other_directory',
    '#size' => 60,
  );

  # which workflow
  $form['prepare']['which_workflow'] = array(
    '#type' => 'select',
    '#options' => array(
      'tifs_to_book' => t('Single TIFs to book hierarchy'),
      'digitool_mods_to_book'  => t('Digitool MODS to book hierarchy'),
    ),
  );

  # which collection
  $form['prepare']['which_collection'] = array(
    '#type' => 'select',
    '#options' => array(
      'col1' => t('Collection 1'),
      'col2'  => t('Collection 2'),
      'col3'  => t('Collection 3'),
    ),
  );

  # Test the Steps
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Prepare Ingest'),
  );
  return $form;
}

function ubl_prepare_ingest_prepare_form_submit(array $form, array &$form_state) {
  unset($_SESSION['workflowresults']);
  unset($_SESSION['workflowdata']);

 $form_state['redirect'] = 'admin/islandora/ubl_prepare_ingest/prepare/test';
}

/*
function ubl_prepare_ingest_prepare_test_form(array $form, array &$form_state) {

  # steps
  $form['steps'] = array(
    '#title' => t('Prepare Ingest Workflow'),
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  # Step read file names
  $form['steps']['read_file_names'] = array(
    '#title' => t('Read file names filtered by \'*.tif\' and insert as key \'filepath\'') . '&nbsp;<img src="/misc/watchdog-ok.png">&nbsp;',
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  # Make key regexp
  $form['steps']['make_key_regexp'] = array(
    '#title' => t('Make key named \'name\' from key \'filepath\' by replacing \'^.*D_I+_(.*)\.tif$\' with \'$1\'') . '&nbsp;<img src="/misc/watchdog-ok.png">&nbsp;',
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  # Insert counter
  $form['steps']['make_counter'] = array(
    '#title' => t('Make key named \'number\' starting at 1, width 5') . '&nbsp;<img src="/misc/watchdog-ok.png">&nbsp;',
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  # Make directory 
  $form['steps']['make_directory'] = array(
    '#title' => t('Make directory at location in key \'number\'') . '&nbsp;<img src="/misc/watchdog-ok.png">&nbsp;',
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  # Make key from template
  $form['steps']['make_key_template'] = array(
    '#title' => t('Make key named \'newobjfilepath\' from template \'{number}/OBJ.tif\'') . '&nbsp;<img src="/misc/watchdog-ok.png">&nbsp;',
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  # Copy file
  $form['steps']['copy_file'] = array(
    '#title' => t('Copy file in key \'filepath\' to location in key \'newobjfilepath\'') . '&nbsp;<img src="/misc/watchdog-ok.png">&nbsp;',
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  # Make key from template
  $form['steps']['make_key_template2'] = array(
    '#title' => t('Make key named \'modsxml\' from template \'&lt;mods xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="htt...\'') . '&nbsp;<img src="/misc/watchdog-ok.png">&nbsp;',
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  # Make key from template
  $form['steps']['make_key_template3'] = array(
    '#title' => t('Make key named \'modsfilepath\' from template \'{number}/MODS.xml\'') . '&nbsp;<img src="/misc/watchdog-ok.png">&nbsp;',
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  # Make file 
  $form['steps']['make_file'] = array(
    '#title' => t('Make file at location in key \'modsfilepath\' with content from key \'modsxml\'') . '&nbsp;<img src="/misc/watchdog-ok.png">&nbsp;',
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  # cont
  $form['cont'] = array(
    '#title' => t('Prepare Ingest Success'),
    '#description' => 'This workflow can be run on the command line by doing the following steps:<BR/><UL><LI>drush prepare_ingest --workflow=4<LI>drush -v --root=/var/www/drupal/drupal-7.36 --uri=http://islandora-o.leidenuniv.nl/ --user=user-name islandora_book_batch_preprocess --namespace=islandora --parent=urn-van-collectie --content_models=islandora:bookCModel --type=zip --target=/path/to/books.zip<LI>drush -v --root=/var/www/drupal/drupal-7.36 --uri=http://islandora-o.leidenuniv.nl/ --user=user-name islandora_batch_ingest',
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  return $form;
}
*/
