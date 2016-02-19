<?php

/**
 * Expose themes as context reactions.
 */
class ubl_context_reaction_theme_variables extends context_reaction {  
  /**
   * Allow admins to provide an alternative logo, site name and site slogan.
   */
  function options_form($context) {
    $form = '';
    $values = $this->fetch_from_context($context);
    
    $form['theme_variables'] = array(
      'logo' => array(
        '#title' => t('Logo'),
        '#description' => t('Provides the logo. The following file extensions are allowed: png, jpg, jpeg, gif.'),
        '#type' => 'textfield',
        '#maxlength' => 255,
        '#default_value' => isset($values['theme_variables']['logo']) ? $values['theme_variables']['logo'] : '',
      ),
      'site_name' => array(
        '#title' => t('Site name'),
        '#description' => t('Provides the site name.'),
        '#type' => 'textfield',
        '#maxlength' => 255,
        '#default_value' => isset($values['theme_variables']['site_name']) ? $values['theme_variables']['site_name'] : '',      
      ),
      'site_link' => array(
        '#title' => t('Site link'),
        '#description' => t('Provides the site link.'),
        '#type' => 'textfield',
        '#maxlength' => 255,
        '#default_value' => isset($values['theme_variables']['site_link']) ? $values['theme_variables']['site_link'] : '',      
      ),
      'site_slogan' => array(
        '#title' => t('Site slogan'),
        '#description' => t('Provides the site slogan.'),
        '#type' => 'textfield',
        '#maxlength' => 255,
        '#default_value' => isset($values['theme_variables']['site_slogan']) ? $values['theme_variables']['site_slogan'] : '',      
      ),
    );
    
    return $form;
  }

  /**
   * Overrides of options_form_submit().
   */
  function options_form_submit($values) {
    if(!empty($values['theme_variables']['logo'])) {
      $file = new stdClass;
      $file->filename = trim($values['theme_variables']['logo'], '/');
            
      if(is_file($file->filename)) {
        $errors = file_validate_extensions($file, 'png jpg jpeg gif');
        if(!empty($errors)) {
          form_set_error('reactions][plugins][theme_variables][theme_variables][logo', $errors[0]);
        }
      } else {
        form_set_error('reactions][plugins][theme_variables][theme_variables][logo', t('The path to logo needs to be a correct path to an image to be used.'));
      }
    }
    return $values;
  }

  /**
   * Sets theme variables.
   */
  function execute(&$vars) {
    global $base_url;
    global $base_path;

    foreach ($this->get_contexts() as $key => $value) {
      if (!empty($value->reactions[$this->plugin]['theme_variables']['logo'])) {
        $vars['logo'] = $base_url . $base_path . trim($value->reactions[$this->plugin]['theme_variables']['logo'], '/');
      }
      if (!empty($value->reactions[$this->plugin]['theme_variables']['site_name'])) {
        $vars['site_name'] = $value->reactions[$this->plugin]['theme_variables']['site_name'];
      }
      if (!empty($value->reactions[$this->plugin]['theme_variables']['site_link'])) {
        $vars['front_page'] = $value->reactions[$this->plugin]['theme_variables']['site_link'];
      }
      if (!empty($value->reactions[$this->plugin]['theme_variables']['site_slogan'])) {
        $vars['site_slogan'] = $value->reactions[$this->plugin]['theme_variables']['site_slogan'];
      }
    }
  }
}
