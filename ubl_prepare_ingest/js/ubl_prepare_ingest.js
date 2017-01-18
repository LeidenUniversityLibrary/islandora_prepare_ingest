/**
 * @file
 * js/ubl_show_persistent_url.js
 */

jQuery(document).ready(function() {
  // remove button
  jQuery('.remove_step_button').click(function(e) {
    e.preventDefault();
    var $thisstep = jQuery(this).parents('fieldset.workflow_step');
    if ($thisstep) { 
      $thisstep.find('.remove_step').val('1');
      $thisstep.hide(500);
    }
  });
  
  // move up button
  jQuery('.moveup_step_button').click(function(e) {
    e.preventDefault();
    var $thisstep = jQuery(this).parents('fieldset.workflow_step');
    if ($thisstep.size() == 1) { 
      var $prevstep = $thisstep.prev('.workflow_step');
      if ($prevstep.size() == 1) {
        var prefix = 'new_weight=';
        var thisstepweight = $thisstep.find('.weight_step').val();
        if (thisstepweight.lastIndexOf(prefix, 0) !== 0) { // does it have the prefix?
          thisstepweight = prefix + thisstepweight;
        }
        var prevstepweight = $prevstep.find('.weight_step').val();
        if (prevstepweight.lastIndexOf(prefix, 0) !== 0) { // does it have the prefix?
          prevstepweight = prefix + prevstepweight;
        }
        $thisstep.find('.weight_step').val(prevstepweight);
        $prevstep.find('.weight_step').val(thisstepweight);
        swapElements($prevstep, $thisstep);
      }
    }
  });

  // move down button
  jQuery('.movedown_step_button').click(function(e) {
    e.preventDefault();
    var $thisstep = jQuery(this).parents('fieldset.workflow_step');
    if ($thisstep.size() == 1) { 
      var $nextstep = $thisstep.next('.workflow_step');
      if ($nextstep.size() == 1) {
        var prefix = 'new_weight=';
        var thisstepweight = $thisstep.find('.weight_step').val();
        if (thisstepweight.lastIndexOf(prefix, 0) !== 0) { // does it have the prefix?
          thisstepweight = prefix + thisstepweight;
        }
        var nextstepweight = $nextstep.find('.weight_step').val();
        if (nextstepweight.lastIndexOf(prefix, 0) !== 0) { // does it have the prefix?
          nextstepweight = prefix + nextstepweight;
        }
        $thisstep.find('.weight_step').val(nextstepweight);
        $nextstep.find('.weight_step').val(thisstepweight);
        swapElements($thisstep, $nextstep);
      }
    } 
  });

  // autosuggest menu
  var autosuggestfunc = function(e) {
    var $textfield = jQuery(e.target);
    var $menu = jQuery('#autosuggestmenu');
    if (!($textfield.hasClass('input_key') || $textfield.hasClass('template') || $textfield.hasClass('regexp'))) {
      $menu.hide(500); 
      return;
    }
    if ($menu.size() == 0) {
      jQuery('BODY').append('<DIV id="autosuggestmenu"></DIV>');
      $menu = jQuery('#autosuggestmenu');
      $menu.css({
        'display': 'none',
        'position': 'absolute',
        'top': 0,
        'left': 0,
        'border': '1px solid gray',
        'min-width': '100px',
        'min-height': '2em',
        'background-color': 'white',
        'padding': '5px'
      });
    }
    else {
      $menu.html('');
    }
    var hasValue = [];
    if ($textfield.hasClass('input_key') || $textfield.hasClass('template')) {
      $menu.append('<DIV class="autosuggestmenuheader">Possible values:</DIV>');
      $textfield.parents('.workflow_step').prevAll('.workflow_step').find('.output_key, .keys').each(function(i, element) {
        var menuMaker = function(value) {
	  if (value.length > 0 && !hasValue[value]) {
	    $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="'+value+'">'+value+'</A></DIV>');
            hasValue[value] = 1;
	  }
        };
	var outputvalue = jQuery(element).val();
        if (jQuery(element).hasClass('keys')) {
          var keys = outputvalue.split(";");
          for (var i=0; i<keys.length; i++) {
            menuMaker(keys[i]);
          }
        }
        else {
          menuMaker(outputvalue);
        }
      }); 
    }
    else if ($textfield.hasClass('regexp')) {
      $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="/(.*\\/)([^\\/]+)$/">Select filepath ($1) and filename ($2)</A></DIV>');
      $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="/(?:.*\\/)?([^\\/]+)\\.([a-zA-Z0-9]+)$/">Select name ($1) and extension ($2) of a filename</A></DIV>');
      $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="/.*?(\\d+)\\.[a-zA-Z0-9]+$/">Select last numbers of a filename before the extension</A></DIV>');
      $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="/.*?(\\d+)\\D\\d+\\.[a-zA-Z0-9]+$/">Select second to last numbers of a filename before the extension</A></DIV>');
      $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="/^.*?(?:(\\d+)\\D*)+$/">Select numbers ($1, $2, ...) in value</A></DIV>');
      $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="/\\.([a-zA-Z0-9]+)$/">Replace extension ($1)</A></DIV>');
    }
    jQuery('.autosuggestmenuitem').click(function(e) {
      e.preventDefault();
      if ($textfield.hasClass('template')) {
        $textfield.val($textfield.val() + '{' + jQuery(e.target).data('value') + '}');
      }
      else {
        $textfield.val(jQuery(e.target).data('value')); 
      }
    });
    var offset = $textfield.offset();
    var height = $textfield.outerHeight();
    var newPos = {'top': ((offset.top + height + 5) + 'px'), 'left': (offset.left + 'px')}; 
    $menu.css(newPos);
    $menu.show(500);
  };
  jQuery('INPUT, TEXTAREA').change(autosuggestfunc).focus(autosuggestfunc);
  var hidefunc = function() { jQuery('#autosuggestmenu').hide(500) };
  jQuery('SELECT, INPUT[type="submit"], BUTTON').focus(hidefunc).click(hidefunc);

  // check value of key field
  var checkvaluefunc = function($textfield, regexp) {
    var value = $textfield.val();
    var result = value.match(regexp);
    var $fielderror = jQuery('#fielderror');
    if (result !== null) {
      if ($fielderror.size() == 0) {
        jQuery('BODY').append('<DIV id="fielderror"></DIV>');
        $fielderror = jQuery('#fielderror');
        $fielderror.css({
          'display': 'none',
          'position': 'absolute',
          'top': 0,
          'left': 0,
          'color': 'red'
        });
      }
      $fielderror.html("The following is not allowed: " + result.join(' '));
      var offset = $textfield.offset();
      var height = $textfield.outerHeight();
      var width = $textfield.outerWidth();
      var newPos = {'top': (offset.top + 'px'), 'left': ((offset.left + width + 5) + 'px'), 'height' : (height) + 'px'};
      $fielderror.css(newPos);
      $fielderror.show();
    }
    else {
      $fielderror.html('');
    }
  };
  jQuery('.number').keyup(function(e) { var $textfield = jQuery(e.target); checkvaluefunc($textfield, /[^0-9]+/g); });
  jQuery('.filepath').keyup(function(e) { var $textfield = jQuery(e.target); checkvaluefunc($textfield, /[^a-zA-Z0-9_.\/-]+/g); });
  jQuery('.key').keyup(function(e) { var $textfield = jQuery(e.target); checkvaluefunc($textfield, /[^a-zA-Z0-9_-]+/g); });
  jQuery('.keys').keyup(function(e) { var $textfield = jQuery(e.target); checkvaluefunc($textfield, /[^a-zA-Z0-9_;-]+/g); });
  jQuery('.number, .filepath, .key').blur(function(e) { jQuery('#fielderror').hide(); });

  var haschanges = false;
  jQuery('.workflow_step INPUT, .workflow_step SELECT').on('keydown change', function(e) {
     haschanges = true;
  });
  jQuery('#check_workflow_button').click(function (e) {
    if (haschanges) {
      if (!confirm('This workflow has changes. Are you sure you want to check it without saving the changes?')) {
        e.preventDefault();
      }
    }
  });

  // display full value
  jQuery('.upi_shortvalue').click(function(e) {
    e.stopPropagation();
    jQuery(this).prev('.upi_fullvalue').css('display', 'block');
  });
  jQuery('.upi_fullvalue').click(function(e) {
    jQuery(this).hide();
  });
});

{
var isSwapping = false;
function swapElements($elem1, $elem2) {
  // based on jquery-swapsies by Steve Marks
  if (isSwapping) {
    //setTimeout(function() { swapElements($elem1, $elem2) }, 100);
    return;
  }
  isSwapping = true;
  var elem1topcss = $elem1.css('top');
  var elem1offset = $elem1.offset();
  var elem1y = elem1offset.top;
  var elem1h = $elem1.height();
  var elem2topcss = $elem2.css('top');
  var elem2offset = $elem2.offset();
  var elem2y = elem2offset.top;
  var elem2h = $elem2.height();
  var betweenSize = elem2y - elem1y - elem1h;
  var displacement1 = elem2h + betweenSize;
  var displacement2 = elem1y - elem2y; 
  var animtime = 400;
  $elem1.animate({
    opacity: 0.8
  }, 100, function() {
    $elem1.animate({
      top: '+='+displacement1+'px',
    }, animtime, function() {
      $elem1.animate({
        opacity: "1"
      }, 100);
    });
  });
  $elem2.animate({
    opacity: 0.8
  }, 100, function() {
    $elem2.animate({
      top: '+='+displacement2+'px',
    }, animtime, function() {
      $elem2.animate({
        opacity: "1"
      }, 100, function() { 
        $elem2.after($elem1);
        $elem1.css('top', elem1topcss);
        $elem2.css('top', elem2topcss);
        isSwapping = false;
      });
    });
  });
}
}
