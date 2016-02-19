
jQuery(document).ready(function() {
/**
 ** lvs: shorten overlay-title
 **/
  var overlayOrigTitleText;

  var overlayTitleShortner = function() {
    var overlayTitleElem = jQuery('#overlay-title');
    if (overlayTitleElem.length == 0) {
      return;
    }
    if ( ! overlayOrigTitleText) {
      overlayOrigTitleText = overlayTitleElem.text();
    }
    var margin = 50;
    var overlayTabs = jQuery('#overlay-tabs');
    if (overlayTabs.length == 0) {
      return;
    }
    var overlayTitleText = overlayOrigTitleText;
    overlayTitleElem.text(overlayTitleText);
    while (overlayTitleText.length > 10 && ((overlayTitleElem.offset().left + overlayTitleElem.width()) > (overlayTabs.offset().left - margin))) {
      overlayTitleText = overlayTitleText.slice(0,-10) + '...';
      overlayTitleElem.text(overlayTitleText);
    }
  };

  jQuery(window).resize(overlayTitleShortner);
  overlayTitleShortner();

/**
 ** lvs: make openseedragon window square
 **/
//  var osview = jQuery('#islandora-openseadragon');
//  var width = jQuery(osview).width();
//  jQuery(osview).height(width);
})
