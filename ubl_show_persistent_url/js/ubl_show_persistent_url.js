
jQuery(document).ready(function() {
  var pagetitle = jQuery('#page-title');
  var button = jQuery(pagetitle).find('.ublpersistentbutton');
  if (pagetitle && button) {
    var title = button.data('title');
    if (title) {
      button.html(title);
    }
    button.click(function (e) {
      var url = jQuery(this).attr('href');
      prompt('Copy / paste the URL below',url);
      e.preventDefault();
    }); 
  }
})
