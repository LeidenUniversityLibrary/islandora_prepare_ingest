/**
 * @file
 * JavaScript for doing a form submit when reviewing.
 */

(function ($) {
  Drupal.behaviors.ublBatchCompound = {
    attach: function (context, settings) {
      $('.viewlinkclass', context).click(function (e) {
        var redirecturlfield = $('INPUT[name="redirecturl"]');
        var newvalue = this.href;
        if (newvalue) {
          e.preventDefault(); 
          redirecturlfield.val(newvalue);
          $('#ubl-batch-compound-import-confirm-form').submit();
        }
      });
    }
  };
})(jQuery);

