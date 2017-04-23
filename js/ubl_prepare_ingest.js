/**
 * @file
 * js/ubl_show_persistent_url.js
 */

jQuery(document).ready(function() {
  setUpButtonsAndFields(jQuery(document));
});

function setUpButtonsAndFields($context) {
  // remove button
  $context.find('.remove_step_button').click(function(e) {
    e.preventDefault();
    var $thisstep = jQuery(this).parents('fieldset.workflow_step').first();
    if ($thisstep) { 
      if (jQuery(this).hasClass('groupremove')) {
        var groupitemscount = $thisstep.find('.remove_step').size() - 1;
        if (confirm('Are you sure you want to remove this group containing '+groupitemscount+' item' + ((groupitemscount == 1)?'':'s') + '?')) {
          $thisstep.find('.remove_step').val('1');
          $thisstep.hide(500);
          var $nextstep = $thisstep.next();
          $nextstep.find('.remove_step').val('1');
          $nextstep.hide(500); 
        }
      }
      else {
        $thisstep.find('.remove_step').val('1');
        $thisstep.hide(500);
      }
    }
  });
 
  // ungroup button
  $context.find('.ungroup_step_button').click(function(e) {
    e.preventDefault();
    var $thisstep = jQuery(this).parents('fieldset.workflow_step').first();
    if ($thisstep) { 
      $thisstep.find('.remove_step').first().val('1');
      var $nextstep = $thisstep.next();
      $nextstep.find('.remove_step').val('1');
      $nextstep.hide(500); 
      var $groupitems = $thisstep.find('> div > div.grouped_steps > fieldset.workflow_step');
      swapElements($nextstep, $groupitems, function() { $nextstep.before($groupitems); $thisstep.hide(100); });
    }
  });
  
  var prefix = 'new_weight=';
  // move to button
  $context.find('.moveto_step_button').click(function(e) {
    e.preventDefault();
    e.stopPropagation();
    var $currentstep = jQuery(this).parents('FIELDSET.workflow_step').first();
    var $nextstep = $currentstep.next('FIELDSET.workflow_step');
    var $steps = jQuery('#edit-steps FIELDSET.workflow_step');
    $steps = $steps.not($currentstep).not($nextstep);
    $steps = $steps.not($currentstep.find('FIELDSET.workflow_step'));

    var htmlMoveherelink = '<DIV class="moveherelink"><A href="#">Move here</A></DIV>';
    $steps.prepend(htmlMoveherelink);
    $steps.last().next('#edit-add-step').prepend(htmlMoveherelink);
    jQuery('.moveherelink').not('FIELDSET.visual_group_start.collapsed + FIELDSET.visual_group_end > .moveherelink').hide().show(400);
    jQuery('BODY').one('click', function(e) {
      e.preventDefault();
      jQuery('.moveherelink').hide(400, function() {
        jQuery('.moveherelink').remove();
      });
    });
    jQuery('.moveherelink').one('click', function(e) {
      var $moveherestep = jQuery(this).closest('FIELDSET.workflow_step');
      var currentstepweight = Number($currentstep.find('.weight_step').first().val().replace(prefix,''));
      var moveherestepweight = 0;
      if ($moveherestep.size() == 0) {
        // move to end of list of steps; moveherestep is actually the FIELDSET containing the add step button.
        $moveherestep = jQuery(this).closest('FIELDSET');
        moveherestepweight = 2^31;
      }
      else {
        moveherestepweight = Number($moveherestep.find('.weight_step').first().val().replace(prefix,''));
      }
      if ($currentstep.hasClass('visual_group_start')) {
        $currentstep = $currentstep.add($currentstep.next('FIELDSET.workflow_step.visual_group_end'));
      }
      var animTime = 400;
      var currentStepOffset = $currentstep.offset();
      var currentStepWidth = $currentstep.outerWidth(true);
      var currentStepHeight = 0;
      var currentStepHeights = [];
      var currentStepOffsets = [];
      $currentstep.each(function(index) {
        currentStepHeights[index] = jQuery(this).outerHeight(true);
        currentStepHeight += currentStepHeights[index];
        currentStepOffsets[index] = jQuery(this).offset();
      });
      var movehereStepOffset = $moveherestep.offset();
      var $tmpFrom = jQuery('<DIV/>');
      $currentstep.last().after($tmpFrom);
      $tmpFrom.height(currentStepHeight);
      $tmpFrom.width(currentStepWidth);
      jQuery('BODY').prepend($currentstep);
      $currentstep.css({'position' : 'absolute', 'z-index' : 590});
      $currentstep.each(function(index) {
        jQuery(this).outerHeight(currentStepHeights[index]);
        jQuery(this).offset(currentStepOffsets[index]);
      });
      $currentstep.outerWidth(currentStepWidth);
      var $tmpTo = jQuery('<DIV/>');
      $moveherestep.before($tmpTo);
      $tmpTo.height(currentStepHeight);
      $tmpTo.width(currentStepWidth);
      $tmpTo.hide().show(animTime); 
      $tmpFrom.hide(animTime);
      var newTop;
      if (currentstepweight < moveherestepweight) {
        newTop = movehereStepOffset.top - currentStepHeight;
      }
      else {
        newTop = movehereStepOffset.top;
      }
      $currentstep.animate({'top' : newTop }, animTime, function() {
        $currentstep.removeAttr('style');
        $tmpTo.remove();
        $tmpFrom.remove(); 
        if ($moveherestep.prev('FIELDSET.workflow_step').hasClass('visual_group_start')) {
          $moveherestep.prev('FIELDSET.workflow_step').find('> DIV > DIV.grouped_steps').append($currentstep);
        }
        else {
          $moveherestep.before($currentstep);
        }
        reassignStepWeights();
      });
      e.preventDefault();
    });
  });

  function reassignStepWeights() {
    var currentWeight = 1;
    var $steps = jQuery('#edit-steps FIELDSET.workflow_step');
    $steps.each(function() {
      var newstepweight = currentWeight + '';
      var stepweight = jQuery(this).find('.weight_step').first().val();
      if (stepweight.lastIndexOf(prefix, 0) === 0) { // does it have the prefix?
        newstepweight = prefix + newstepweight;
      }
      if (newstepweight !== stepweight) {
        jQuery(this).find('.weight_step').first().val(prefix + currentWeight);
      }
      currentWeight += 1;
    }); 
  }

  function retrieveStepsFromTo($fromElement, $toElement, excluding) {
    var $allsteps = jQuery('#edit-steps').find('FIELDSET.workflow_step');
    var start = $allsteps.index($fromElement);
    var end = $allsteps.index($toElement);
    var $steps = jQuery();
    if (start >= 0) {
      if (end == -1) {
        $steps = $allsteps.slice(start + (excluding?1:0)); 
      }
      else {
        $steps = $allsteps.slice(start + (excluding?1:0), end); 
      }
    }
    else {
      alert('Error: fromStep ' + $fromElement.attr('id') + ' not found in all steps.');
    }
    return $steps;
  }

  // autosuggest menu
  var autosuggestfunc = function(e) {
    var $textfield = jQuery(e.target);
    var $menu = jQuery('#autosuggestmenu');
    //if (!($textfield.hasClass('input_key') || $textfield.hasClass('template') || $textfield.hasClass('templatestring') || $textfield.hasClass('regexp'))) {
    //  $menu.hide(500); 
    //  return;
   // }
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
    if (($textfield.hasClass('input_key') || $textfield.hasClass('template') || $textfield.hasClass('templatestring')) 
        || (!$textfield.hasClass('key') && !$textfield.hasClass('constantkey'))) {
      var menuMaker = function(value, type) {
        if (value.length > 0 && !hasValue[value]) {
          $menu.append('<DIV class="autosuggestmenuitem"><A href="#" class="'+type+'" data-value="'+value+'">'+value+'</A></DIV>');
          hasValue[value] = 1;
        }
      };
      if (!$textfield.hasClass('key') && !$textfield.hasClass('constantkey')) {
        var $prevFields = $textfield.parents('.workflow_step').prevAll('.workflow_step').find('.constantkey');
        if ($prevFields.size() > 0) {
          var hasMenuItems = false;
          $prevFields.each(function(i, element) {
	    var outputvalue = jQuery(element).val();
            if (outputvalue.length > 0) {
              menuMaker(outputvalue, 'constant');
              hasMenuItems = true;
            }
          }); 
          if (hasMenuItems) {
            $menu.prepend('<DIV class="autosuggestmenuheader">Possible constants:</DIV>');
          }
        }
      }
      if ($textfield.hasClass('input_key') || $textfield.hasClass('template') || $textfield.hasClass('templatestring') || $textfield.hasClass('regexp')) {
        var $prevFields = $textfield.parents('.workflow_step').prevAll('.workflow_step').find('.output_key, .keys, .keystemplate');
        if ($prevFields.size() > 0) {
          var hasMenuItems = false;
          $prevFields.each(function(i, element) {
	    var outputvalue = jQuery(element).val();
            if (jQuery(element).hasClass('keys')) {
              var keys = outputvalue.split(";");
              for (var i=0; i<keys.length; i++) {
                if (keys[i].length > 0) {
                  menuMaker(keys[i], 'key');
                  hasMenuItems = true;
                }
              }
            }
            else if (jQuery(element).hasClass('keystemplate')) {
              var re = /[^{]*{([^}]+)}/g;
              var match;
              while ((match = re.exec(outputvalue)) != null) {
                if (match[1].length > 0) {
                  menuMaker(match[1], 'key');
                  hasMenuItems = true;
                }
              }
            }
            else {
              if (outputvalue.length > 0) {
                menuMaker(outputvalue, 'key');
                hasMenuItems = true;
              }
            }
          }); 
          if (hasMenuItems) {
            $menu.prepend('<DIV class="autosuggestmenuheader">Possible keys:</DIV>');
          }
        }
      }
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
      var keyval = '{' + jQuery(e.target).data('value') + '}';
      if ($textfield.hasClass('template') || $textfield.hasClass('templatestring') || jQuery(e.target).hasClass('constant')) {
        if (document.selection) {
          $textfield.focus();
          var sel = document.selection.createRange();
          sel.text = keyval;
          $textfield.focus();
        }
        else if ($textfield[0].selectionStart || $textfield[0].selectionStart === 0)           {
          var startPos = $textfield[0].selectionStart;
          var endPos = $textfield[0].selectionEnd;
          var scrollTop = $textfield[0].scrollTop;
          var curvalue = $textfield.val();
          $textfield.val(curvalue.substring(0, startPos) + keyval + curvalue.substring(endPos, curvalue.length));
          $textfield.focus();
          $textfield[0].selectionStart = startPos + keyval.length;
          $textfield[0].selectionEnd = startPos + keyval.length;
          $textfield[0].scrollTop = scrollTop;
        } else {
          $textfield.val($textfield.val() + keyval);
          $textfield.focus();
        }
      }
      else {
        $textfield.val(jQuery(e.target).data('value')); 
      }
    });
    if ($menu.html().length > 0) {
      var offset = $textfield.offset();
      var height = $textfield.outerHeight();
      var newPos = {'top': ((offset.top + height + 5) + 'px'), 'left': (offset.left + 'px')}; 
      $menu.css(newPos);
      $menu.show(500);

      setTimeout(function() {
        var hidefunc = function(e) {
          setTimeout(function() {
            var $menu = jQuery('#autosuggestmenu');
            if (!jQuery(e.target).is(':focus')) {
              jQuery('#autosuggestmenu').hide(500);
            }
          }, 50);
        };
        $context.find('SELECT, INPUT[type="submit"], BUTTON').one('focus', hidefunc).one('click', hidefunc);
        jQuery('BODY').one('click', hidefunc);
      }, 500);
    }
    else {
      $menu.hide();
    }
  };
  $context.find("INPUT[type='text'], TEXTAREA").change(autosuggestfunc).focus(autosuggestfunc).click(autosuggestfunc);
  
  // check value of key field
  var checkvaluefunc = function($textfield, regexp) {
    var value = $textfield.val();
    value = value.replace(/{[a-zA-Z0-9_-]+}/g, ''); // allow constants in field
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
  $context.find('.number').keyup(function(e) { var $textfield = jQuery(e.target); checkvaluefunc($textfield, /[^0-9]+/g); });
  $context.find('.filepath').keyup(function(e) { var $textfield = jQuery(e.target); checkvaluefunc($textfield, /[^a-zA-Z0-9_.\/-]+/g); });
  $context.find('.key').keyup(function(e) { var $textfield = jQuery(e.target); checkvaluefunc($textfield, /[^a-zA-Z0-9_-]+/g); });
  $context.find('.keys').keyup(function(e) { var $textfield = jQuery(e.target); checkvaluefunc($textfield, /[^a-zA-Z0-9_;-]+/g); });
  $context.find('.number, .filepath, .key').blur(function(e) { jQuery('#fielderror').hide(); });

  var haschanges = false;
  $context.find(".workflow_step INPUT[type='text'], .workflow_step SELECT, .workflow_step TEXTAREA").on('keydown change', function(e) {
     haschanges = true;
  });
  $context.find('#check_workflow_button').click(function (e) {
    if (haschanges) {
      if (!confirm('This workflow has changes. Are you sure you want to check it without saving the changes?')) {
        e.preventDefault();
      }
    }
  });
  $context.find('#add_step_button').click(function(e) {
    var $whichStep = jQuery('#edit-which-step');
    var $addStepDiv = jQuery('#edit-add-step');
    if ($whichStep.size() == 1 && $addStepDiv.size() == 1) {
      var workflowid = jQuery('#ubl-prepare-ingest-edit-workflow-form > DIV > INPUT[name="workflowid"]').val();
      var stepname = $whichStep.val();
      if (workflowid && stepname) {
        var loc = window.location;
        var loadUrl = loc.protocol + '//' + loc.host + '/admin/islandora/ubl_prepare_ingest/ajax/addstep/' + workflowid + '/' + stepname; 
        loadContent(loadUrl, $addStepDiv);
        e.preventDefault();
      }
    }
  });
  $context.find('#add_workflow_steps_button').click(function(e) {
    var $whichWorkflow = jQuery('#edit-which-workflow-steps');
    var $addStepDiv = jQuery('#edit-add-step');
    if ($whichWorkflow.size() == 1 && $addStepDiv.size() == 1) {
      var workflowid = jQuery('#ubl-prepare-ingest-edit-workflow-form > DIV > INPUT[name="workflowid"]').val();
      var whichWorkflowId = $whichWorkflow.val();
      if (workflowid && whichWorkflowId) {
        var loc = window.location;
        var loadUrl = loc.protocol + '//' + loc.host + '/admin/islandora/ubl_prepare_ingest/ajax/addstepsgroup/' + workflowid + '/' + whichWorkflowId; 
        loadContent(loadUrl, $addStepDiv);
        e.preventDefault();
      }
    }
  });

function loadContent(loadUrl, $addStepDiv) {
  var $tmpAddStepContent = jQuery('#tmpaddstepcontent');
  if ($tmpAddStepContent.size() == 0) {
    jQuery('BODY').append('<DIV id="tmpaddstepcontent"></DIV>');
    $tmpAddStepContent = jQuery('#tmpaddstepcontent');
    $tmpAddStepContent.css({
      'display': 'none',
    });
  }
  $tmpAddStepContent.load(loadUrl, function() {
    var $newcontent = $tmpAddStepContent.find('> FORM > DIV > .workflow_step'); 
    $newcontent.each(function() {
      jQuery(this).addClass('collapsible collapse-processed');
      jQuery(this).hide();
      if (jQuery(this).hasClass('visual_group_start') && jQuery(this).find('> DIV > DIV.grouped_steps').size() == 0) {
        jQuery(this).find('> DIV').append('<div class="grouped_steps form-wrapper" id="edit-grouped"/>');
      }
    });
    setUpButtonsAndFields($newcontent);
    var $lastStep = $addStepDiv.prev('.workflow_step');
    var $lastGroup;
    var didAddContent = false;
    while ($lastStep.hasClass('visual_group_start')) {
      var $group = $lastStep.find('> DIV > DIV.grouped_steps');
      var $newLastStep = $group.children().last();
      if (!$newLastStep.hasClass('visual_group_start') && $group.size() > 0) {
        if ($newcontent.hasClass('visual_group_end')) {
          if ($lastGroup) {
            $lastGroup.append($newcontent);
            didAddContent = true; 
          }
        }
        else {
          $group.append($newcontent);
          didAddContent = true; 
        }
      }
      $lastStep = $newLastStep;
      $lastGroup = $group;
    }
    if (!didAddContent) {
      $addStepDiv.before($newcontent);
    }
    $newcontent.show(400, function() {
      $newcontent.removeAttr('style');
    });
  });
}

  // display full value
  $context.find('.upi_shortvalue').click(function(e) {
    e.stopPropagation();
    var fvdiv = jQuery(this).prev('.upi_fullvalue');
    fvdiv.css('display', 'block');
    var fsw = fvdiv.parents('.fieldset-wrapper').first();
    var fswoffset = fsw.offset();
    var fswh = fsw.height();
    var fsww = fsw.width();
    var fvdivoffset = fvdiv.offset();
    var fvdivh = fvdiv.height();
    var fvdivw = fvdiv.width();
    if ((fvdivoffset.left + fvdivw) > (fswoffset.left + fsww)) {
      var clipped = (fvdivoffset.left + fvdivw) - (fswoffset.left + fsww) + 100;
      if ((fvdivoffset.left - clipped) > (fswoffset.left + 10)) {
        fvdiv.offset({ top: fvdivoffset.top, left: (fvdivoffset.left - clipped) }); 
      }
      else {
        var toolittleleft = (fvdivoffset.left - clipped) - (fswoffset.left + 5);
        clipped = clipped + toolittleleft;
        fvdiv.offset({ top: fvdivoffset.top, left: (fvdivoffset.left - clipped) });
        fvdiv.width(fvdivw + toolittleleft);
      }
      fvdivoffset = fvdiv.offset();
    }
    if ((fvdivoffset.top + fvdivh) > (fswoffset.top + fswh)) {
      var clipped = (fvdivoffset.top + fvdivh) - (fswoffset.top + fswh);
      if ((fvdivoffset.top - clipped) > (fswoffset.top + 10)) {
        fvdiv.offset({ top: (fvdivoffset.top - clipped), left: fvdivoffset.left }); 
      }
      else {
        var toolittleleft = (fvdivoffset.top - clipped) - (fswoffset.top + 5);
        clipped = clipped + toolittleleft;
        fvdiv.offset({ top: (fvdivoffset.top - clipped), left: fvdivoffset.left });
        fvdiv.height(fvdivh + toolittleleft);
      }
    } 
  });
  $context.find('.upi_fullvalue').click(function(e) {
    jQuery(this).hide();
  });
};

{
var isSwapping = false;
function swapElements($elem1, $elem2, $endfunc, animtime) {
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
  if (typeof animtime === 'undefined') {
    animtime = 400;
  }
  var ending = function() {
    if ($endfunc) {
      $endfunc();
    }
    else {
      $elem2.after($elem1);
    }
    $elem1.css('top', elem1topcss);
    if ($elem2) {
      $elem2.css('top', elem2topcss);
    }
    isSwapping = false;
  };
  $elem1.animate({
    opacity: 0.8
  }, 100, function() {
    $elem1.animate({
      top: '+='+displacement1+'px',
    }, animtime, function() {
      $elem1.animate({
        opacity: "1"
      }, 100, function () {
        if (!$elem2) {
          ending();
        }
      });
    });
  });
  if ($elem2) {
    $elem2.animate({
      opacity: 0.8
    }, 100, function() {
      $elem2.animate({
        top: '+='+displacement2+'px',
      }, animtime, function() {
        $elem2.animate({
          opacity: "1"
        }, 100, ending);
      });
    });
  }
}
}
