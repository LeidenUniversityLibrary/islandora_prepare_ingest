/**
 * @file
 * js/islandora_prepare_ingest.js
 *
 *
 *  Copyright 2017-2018 Leiden University Library
 *
 *  This file is part of islandora_prepare_ingest.
 *
 *  islandora_prepare_ingest is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

var hasUnsavedChanges = false;

jQuery(document).ready(function() {
  setUpButtonsAndFields(jQuery(document));

  var allkeysobj = {};
  var prevElement1 = undefined;
  var prevElement2 = undefined;
  jQuery('.datacache').each(function() {
    var type = jQuery(this).data('type');
    if (type == 1) {
      var inputkeysstring = jQuery(this).data('inputkeys')
      var outputkeysstring = jQuery(this).data('outputkeys')
      var inputkeys = (typeof inputkeysstring == 'string' && inputkeysstring.length)?inputkeysstring.split(','):[];
      var outputkeys = (typeof outputkeysstring == 'string' && outputkeysstring.length)?outputkeysstring.split(','):[];
      jQuery(this).data('inputkeys', inputkeys);
      jQuery(this).data('outputkeys', outputkeys);
      for (var i=0; i<inputkeys.length; i++) {
        allkeysobj[inputkeys[i]] = 1;
      }
      for (var i=0; i<outputkeys.length; i++) {
        allkeysobj[outputkeys[i]] = 1;
      }
      var allkeys = [];
      for (var key in allkeysobj) {
        allkeys.push(key);
      }
      jQuery(this).data('allkeys', allkeys);
      jQuery(this).data('prevelement', prevElement1);
      prevElement1 = this;
    }
    else if (type == 2) {
      jQuery(this).data('prevelement', prevElement2);
      prevElement2 = this;
    }
    jQuery(this).data('show', 1);
    var element = this;
    var $step = jQuery(element).parent('DIV').parent('.workflow_step');
    if ($step.hasClass('collapsed')) {
      $step.find('> LEGEND > SPAN > A').click(function() {
        filloutDataCacheElement(element, true);
      });
    }
    else {
      filloutDataCacheElement(element, true);
    }
  });
  jQuery('.workflow_step .tabs').each(function() {
    var $tabs = jQuery(this);
    var $showDataButton = $tabs.find('.show_data_button');
    var $editButton = $tabs.find('.edit_button');
    if ($showDataButton.size() == 1 && $editButton.size() == 1) {
      $showDataButton.click(function(e) {
        var $thisstep = $tabs.parents('.workflow_step').first();
        $thisstep.find('.datacache').show();
        $thisstep.find('.fields').hide();
        $thisstep.find('.buttons').hide();
        $showDataButton.hide();
        $editButton.show();
        e.preventDefault();
      });
      $editButton.click(function(e) {
        var $thisstep = $tabs.parents('.workflow_step').first();
        $thisstep.find('.datacache').hide();
        $thisstep.find('.fields').show();
        $thisstep.find('.buttons').show();
        $showDataButton.show();
        $editButton.hide();
        e.preventDefault();
      });
      if ($tabs.prev().size() == 0) {
        $showDataButton.click();
      }
      else {
        $editButton.click();
      }
    }
  });
});

function setUnsavedChanges() {
  if (!hasUnsavedChanges) {
    hasUnsavedChanges = true;
    jQuery('#save_workflow_button').show();
    jQuery('#dryrun_workflow_button').hide();
  }
}

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
          setUnsavedChanges();
        }
      }
      else {
        $thisstep.find('.remove_step').val('1');
        $thisstep.hide(500);
        setUnsavedChanges();
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
      setUnsavedChanges();
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
        setUnsavedChanges();
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

  // autosuggest menu
  {
  var $currentElement;
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
    if ($textfield.hasClass('regexp')) {
      $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="/(.*\\/)([^\\/]+)$/">Select filepath ($1) and filename ($2)</A></DIV>');
      $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="/(?:.*\\/)?([^\\/]+)\\.([a-zA-Z0-9]+)$/">Select name ($1) and extension ($2) of a filename</A></DIV>');
      $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="/.*?(\\d+)\\.[a-zA-Z0-9]+$/">Select last numbers of a filename before the extension</A></DIV>');
      $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="/.*?(\\d+)\\D\\d+\\.[a-zA-Z0-9]+$/">Select second to last numbers of a filename before the extension</A></DIV>');
      $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="/^.*?(?:(\\d+)\\D*)+$/">Select numbers ($1, $2, ...) in value</A></DIV>');
      $menu.append('<DIV class="autosuggestmenuitem"><A href="#" data-value="/\\.([a-zA-Z0-9]+)$/">Replace extension ($1)</A></DIV>');
    }
    else if (($textfield.hasClass('input_key') || $textfield.hasClass('template') || $textfield.hasClass('templatestring'))
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
        var $prevLastItem = $menu.find('> DIV').last();
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
              var re = /[^{]*{([a-zA-Z0-9_-]+)(?:\[([^\[\]]+)\])?}/g;
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
            var headHtml = '<DIV class="autosuggestmenuheader">Possible keys:</DIV>';
            if ($prevLastItem.size() > 0) {
              $prevLastItem.after(headHtml);
            }
            else {
              $menu.prepend(headHtml);
            }
          }
        }
      }
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
        $textfield.focus();
      }
      setUnsavedChanges();
    });
    if ($menu.html().length > 0) {
      var offset = $textfield.offset();
      var height = $textfield.outerHeight();
      var newPos = {'top': ((offset.top + height + 5) + 'px'), 'left': (offset.left + 'px')};
      $menu.css(newPos);
      $menu.show(500);
      $currentElement = $textfield;
    }
    else {
      $menu.hide();
      $currentElement = null;
    }
    $textfield.focusout(function() {
      setTimeout(function() {
        if (($currentElement == null) || !$currentElement.is(':focus')) {
          jQuery('#autosuggestmenu').hide(500);
        }
      }, 500);
    });
  };
  $context.find("INPUT[type='text'], TEXTAREA").focusin(autosuggestfunc);
  }

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

  $context.find("#edit-workflowname, #edit-workflowdescription, #edit-workflowtype, .workflow_step INPUT[type='text'], .workflow_step SELECT, .workflow_step TEXTAREA").on('keydown change', function(e) {
     setUnsavedChanges();
  });
  $context.find('#back_button').click(function (e) {
    if (hasUnsavedChanges) {
      if (!confirm('This workflow has changes. Are you sure you want to leave without saving the changes?')) {
        e.preventDefault();
      }
    }
  });
  $context.find('#add_step_button').click(function(e) {
    var $whichStep = jQuery('#edit-which-step');
    var $addStepDiv = jQuery('#edit-add-step');
    if ($whichStep.size() == 1 && $addStepDiv.size() == 1) {
      var workflowid = jQuery('#islandora-prepare-ingest-edit-workflow-form > DIV > INPUT[name="workflowid"]').val();
      var stepname = $whichStep.val();
      if (workflowid && stepname) {
        var loc = window.location;
        var loadUrl = loc.protocol + '//' + loc.host + '/admin/islandora/prepare_ingest/ajax/addstep/' + workflowid + '/' + stepname;
        loadContent(loadUrl, $addStepDiv);
        setUnsavedChanges();
        e.preventDefault();
      }
    }
  });
  $context.find('#add_workflow_steps_button').click(function(e) {
    var $whichWorkflow = jQuery('#edit-which-workflow-steps');
    var $addStepDiv = jQuery('#edit-add-step');
    if ($whichWorkflow.size() == 1 && $addStepDiv.size() == 1) {
      var workflowid = jQuery('#islandora-prepare-ingest-edit-workflow-form > DIV > INPUT[name="workflowid"]').val();
      var whichWorkflowId = $whichWorkflow.val();
      if (workflowid && whichWorkflowId) {
        var loc = window.location;
        var loadUrl = loc.protocol + '//' + loc.host + '/admin/islandora/prepare_ingest/ajax/addstepsgroup/' + workflowid + '/' + whichWorkflowId;
        loadContent(loadUrl, $addStepDiv);
        setUnsavedChanges();
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
};

{
var batchsize = 10;

function filloutDataCacheElement(element, isStarting, prevcount, endFunction, filter) {
  var workflowid = jQuery('#islandora-prepare-ingest-edit-workflow-form > DIV > INPUT[name="workflowid"]').val();
  var otherid = jQuery('#islandora-prepare-ingest-edit-workflow-form > DIV > INPUT[name="otherid"]').val();
  var stepid = jQuery(element).data('stepid');
  var type = jQuery(element).data('type');
  var startitemnr = jQuery(element).data('startitemnr');
  var enditemnr = jQuery(element).data('enditemnr');
  var show = jQuery(element).data('show');

  if (prevcount === undefined) {
    prevcount = 0;
  }
  if (workflowid && otherid && stepid && type && (startitemnr < enditemnr)) {
    if (isStarting && (show === 2)) {
      var prevElement = jQuery(element).data('prevelement');
      if (prevElement) {
        prevcount = jQuery(prevElement).data('count');

        if (prevcount === undefined) {
          filloutDataCacheElement(prevElement, true, 0, function() {
            filloutDataCacheElement(element, isStarting);
          });
          return;
        }
        if (prevcount < jQuery(element).data('count')) {
          // The previous step count is smaller than the current, so there are items added:
          if (enditemnr <= prevcount) {
            // the added items are not visible yet, so go to the batch where the new items are visible:
            enditemnr = Math.floor((prevcount + batchsize - 1) / batchsize) * batchsize;
            startitemnr = enditemnr - batchsize + 1;
          }
        }
        else {
          prevcount = 0;
        }
      }
    }

    var query = {
       'workflowid'   : workflowid,
       'otherid'      : otherid,
       'stepid'       : stepid,
       'type'         : type,
       'startitemnr'  : startitemnr,
       'enditemnr'    : enditemnr
    };
    var url = '/admin/islandora/prepare_ingest/ajax/' + ((type === 1)?'datacache':'files');

    jQuery(element).find('> DIV > DIV.datacache > TABLE').animate({opacity: 0.25}, 500, function() {
      jQuery(this).before('<DIV id="loadingdata">Loading...</DIV>');
    });
    if (filter === undefined) {
      jQuery.getJSON(url, query, function(data) {
        filloutDataCacheElementWithData(element, data, workflowid, otherid, stepid, type, show, startitemnr, enditemnr, prevcount);
        if (endFunction) {
          endFunction();
        }
      });
    }
    else {
      var filterbatchsize = 1000;
      var filloutFilteredDataCacheElement = function(element, url, query, filter, filteredData, workflowid, otherid, stepid, type, show, endFunction) {
        jQuery.getJSON(url, query, function(data) {
          var hasMoreData = false;
          var count = ('count' in filter)?filter['count']:0;
          var from = ('from' in filter)?filter['from']:0;
          jQuery.each(data['list'], function (index, item) {
            if (filter['type']) {
              var filtertype = filter['type'];
              if (filter['value']) {
                if ((filtertype in item) && item[filtertype].indexOf(filter['value']) !== -1) {
                  if (count >= from) {
                    filteredData.push(item);
                  }
                  count++;
                }
              }
              else {
                if (!(filtertype in item) || item[filtertype] === '') {
                  if (count >= from) {
                    filteredData.push(item);
                  }
                  count++;
                }
              }
            }
            else {
              jQuery.each(item, function (key, value) {
                if ( key !== 'item nr' && value.indexOf && value.indexOf(filter['value']) !== -1) {
                  if (count >= from) {
                    filteredData.push(item);
                  }
                  count++;
                  return false;
                }
              });
            }
            if (filteredData.length >= batchsize) {
              hasMoreData = true;
              return false;
            }
          });
          filter['count'] = count;
          if (count < (from + batchsize) && data['list'].length > 0) {
            query['startitemnr'] += filterbatchsize;
            query['enditemnr'] += filterbatchsize;
            filloutFilteredDataCacheElement(element, url, query, filter, filteredData, workflowid, otherid, stepid, type, show, endFunction);
          }
          else {
            showDataCache(element, {'list' : filteredData, 'count' : -1}, workflowid, otherid, stepid, type, show, 1, 11, 0, filter);
            setupButtonsDataCache(element, (hasMoreData?10:0), 1, 11, 0);
          }
          if (endFunction) {
            endFunction();
          }
        });
      };
      query['startitemnr'] = 1;
      query['enditemnr'] = filterbatchsize;
      var filteredData = [];
      filloutFilteredDataCacheElement(element, url, query, filter, filteredData, workflowid, otherid, stepid, type, show, endFunction);
    }
  }
}

function filloutDataCacheElementWithData(element, data, workflowid, otherid, stepid, type, show, startitemnr, enditemnr, showfromitemnr) {
  if (jQuery(element).data('count') === undefined) {
    jQuery(element).data('count', data['count']);
  }
  var width = jQuery(element).data('width');
  if (width === undefined) {
    if (jQuery(element).find('> DIV > #widthcalculation').size() == 0) {
      jQuery(element).find('> DIV').html('<DIV id="widthcalculation">Loading...</DIV>');
    }
    width = Math.ceil(jQuery(element).find('#widthcalculation').width());
    if (width > 0) {
      jQuery(element).data('width', width);
    }
    else {
      setTimeout(function() {
        var stepid = jQuery(element).data('stepid');
        console.log("did not find width for step " + stepid);
        filloutDataCacheElementWithData(element, data, type, show, startitemnr, enditemnr, showfromitemnr);
      }, 100);
      return;
    }
  }

  showDataCache(element, data, workflowid, otherid, stepid, type, show, startitemnr, enditemnr, showfromitemnr);
  setupButtonsDataCache(element, data['count'], startitemnr, enditemnr, showfromitemnr);
}

function showDataCache(element, data, workflowid, otherid, stepid, type, show, startitemnr, enditemnr, showfromitemnr, filter) {
  var nrOfRows = data['list'].length;
  var usedKeys = [];
  var header = {};
  var itemcount = data['count'];
  if (itemcount == 0) {
    jQuery(element).find('> DIV').html('<DIV>No data</DIV>');
    return;
  }
  if (showfromitemnr === undefined) {
    showfromitemnr = 0;
  }
  if (type === 2) {
    usedKeys.push('filepath');
  }
  else if (type === 1) {
    if (show === 2) {
      var alreadyUsed = [];
      var l = jQuery(element).data('inputkeys');
      for (var i=0; i<l.length; i++) {
        usedKeys.push(l[i]);
        alreadyUsed[l[i]] = 1;
      }
      l = jQuery(element).data('outputkeys');
      for (var i=0; i<l.length; i++) {
        if (!(l[i] in alreadyUsed)) {
          usedKeys.push(l[i]);
        }
      }
    }
    if (usedKeys.length === 0) {
      var l = jQuery(element).data('allkeys');
      for (var i=0; i<l.length; i++) {
        usedKeys.push(l[i]);
      }
    }
  }
  var width = jQuery(element).data('width');
  var table = '<DIV class="datacache" ' + ((width > 0)?'style="width:'+width+'px;"':'') + '>';
  table += '<TABLE class="' + ((type === 1)?'datalisting':'filelisting') + '">';
  table += '<TR>';
  table += '<TH class="itemnr">item nr</TH>';
  var usedKeysCount = usedKeys.length;
  for (var k=0; k<usedKeysCount; k++) {
    table += '<TH>' + usedKeys[k] + '</TH>';
  }
  table += '</TR>';
  var htmlEncode = function(value) {
    return jQuery('<div>').text(value).html();
  };
  var maxLengthKeyValue = (type === 2)?100:28;
  for (var i=0; i<nrOfRows; i++) {
    var d = data['list'][i];
    table += '<TR>';
    table += '<TH class="itemnr">' + d['item nr'] + '</TH>';
    if (d['item nr'] > showfromitemnr) {
      for (var k=0; k<usedKeysCount; k++) {
        var key = usedKeys[k];
        var value = ((d.hasOwnProperty(key) && d[key] != null)?d[key]:'-');
        var cellhtml = '';
        if (type === 2 && key === 'filepath') {
          cellhtml += '<SPAN class="upi_fullvalue" data-filepath="'+value+'">';
          var shortvalue;
          if (value.length > maxLengthKeyValue) {
            shortvalue = value.substr(0, maxLengthKeyValue/2-1) + '...' + value.substr(-maxLengthKeyValue/2+2);
          }
          else {
            shortvalue = value;
          }
          if (d.hasOwnProperty('removed') && d['removed']) {
            shortvalue = '<s>' + shortvalue + '</s>';
          }
          cellhtml += '</SPAN><SPAN class="upi_shortvalue">' + shortvalue + '</SPAN>';
        }
        else if (value.length > maxLengthKeyValue) {
          var shortvalue = value.substr(0, maxLengthKeyValue/2-1) + '...' + value.substr(-maxLengthKeyValue/2+2);
          shortvalue = htmlEncode(shortvalue);
          value = htmlEncode(value);
	  cellhtml = '<SPAN class="upi_fullvalue">' + value + '</SPAN><SPAN class="upi_shortvalue">' + shortvalue + '</SPAN>';
        }
        else {
          cellhtml = htmlEncode(value);
        }
        table += '<TD><DIV class="celldata">' + cellhtml + '</DIV></TD>';
      }
    }
    else {
      for (var k=0; k<usedKeysCount; k++) {
        table += '<TD>&nbsp;</TD>';
      }
    }
    table += '</TR>';
  }
  for (var i=nrOfRows; i<(enditemnr-startitemnr+1); i++) {
    table += '<TR><TH>&nbsp;</TH><TD colspan="' + usedKeysCount + '">&nbsp;</TD></TR>';
  }
  table += '<TR><TH class="fixedpos">&nbsp;</TH><TH class="fixedpos" colspan="' + usedKeysCount + '">';
  table += '<FORM novalidate>';
  table += '<SPAN id="rangeselection"' + (filter?' style="display:none;"':'') + '>'
  if ((type === 1) && (jQuery(element).data('outputkeys').length > 0)) {
    table += '<BUTTON type="button" id="usebutton">';
    if (show === 1) {
      table += 'Show step data only';
    }
    else {
      table += 'Show all data';
    }
    table += '</BUTTON>';
    table += '&nbsp;';
  }
  table += '<BUTTON type="button" id="startbutton">|&lt;</BUTTON>';
  table += '<BUTTON type="button" id="prevbutton">&lt;</BUTTON>';
  table += '<SPAN id="rangetext">' + startitemnr + ' to ' + Math.min(itemcount, enditemnr) + ' of ' + itemcount + '</SPAN>';
  //table += '<SPAN id="rangeinput"><INPUT name="rangeinputfield" type="number" min="1" max="' + itemcount + '" size="5" value="' + startitemnr + '" step="' + batchsize + '"/><BUTTON type="button" id="rangego">go</BUTTON></SPAN>';
  table += '<SPAN id="rangeinput"><INPUT name="rangeinputfield" value="' + startitemnr + '"/><BUTTON type="button" id="rangego">go</BUTTON></SPAN>';
  table += '<BUTTON type="button" id="nextbutton">&gt;</BUTTON>';
  table += '<BUTTON type="button" id="endbutton">&gt;|</BUTTON>';
  table += '&nbsp;&nbsp;<BUTTON type="button" id="filterbutton">filter</BUTTON>';
  table += '</SPAN>';
  table += '<SPAN id="filterspan"' + (!filter?' style="display:none;"':'') + '>'
  table += '<SELECT name="filtertype" id="filtertype">';
  if (type === 1) {
    table += '<OPTION value="">All values</OPTION>'
  }
  for (var i=0; i<usedKeys.length; i++) {
    table += '<OPTION value="' + usedKeys[i] + '"' + ((filter && usedKeys[i] === filter['type'])?' selected':'') + '>' + usedKeys[i] + '</OPTION>';
  }
  table += '</SELECT>';
  table += '<INPUT name="filtervalue" id="filtervalue" type="text" size="20" value="' + ((filter && 'value' in filter)?filter['value']:'') + '"/>';
  table += '<INPUT name="filterfrom" id="filterfrom" type="hidden" value="' + ((filter && 'from' in filter)?filter['from']:'0') + '"/>';
  table += '<BUTTON type="button" id="filtergo">go</BUTTON>&nbsp;';
  table += '<BUTTON type="button" id="fprevbutton">&lt;</BUTTON>';
  table += '<BUTTON type="button" id="fnextbutton">&gt;</BUTTON>';
  table += '<BUTTON type="button" id="fclosebutton">close</BUTTON>';
  table += '</SPAN>';
  table += '</FORM>';
  table += '</TH></TR>';
  table += '</TABLE>';
  table += '</DIV>';
  jQuery(element).find('> DIV').html(table);
  setupDisplayFullValue(jQuery(element), workflowid, otherid, stepid, type);
}

function setupButtonsDataCache(element, itemcount, startitemnr, enditemnr, showfromitemnr) {
  var reloadForMinMax = function(min, max) {
    jQuery(element).data('startitemnr', min);
    jQuery(element).data('enditemnr', max);
    filloutDataCacheElement(element, false, showfromitemnr);
  };
  jQuery(element).find('TABLE #startbutton').prop('disabled', (startitemnr <= 1)).click(function(e) {
    reloadForMinMax(1, batchsize);
  });
  jQuery(element).find('TABLE #prevbutton').prop('disabled', (startitemnr <= 1)).click(function(e) {
    var newStart = Math.max(1, startitemnr - batchsize);
    var newEnd = newStart + batchsize -1;
    reloadForMinMax(newStart, newEnd);
  });
  jQuery(element).find('TABLE #nextbutton').prop('disabled', (enditemnr >= itemcount)).click(function(e) {
    reloadForMinMax(startitemnr + batchsize, enditemnr + batchsize);
  });
  jQuery(element).find('TABLE #endbutton').prop('disabled', (enditemnr >= itemcount)).click(function(e) {
    var newend = Math.floor((itemcount + batchsize - 1) / batchsize) * batchsize;
    reloadForMinMax(newend - batchsize + 1, newend);
  });
  jQuery(element).find('TABLE #usebutton').click(function(e) {
    var show = jQuery(element).data('show');
    if (show === 1) {
      jQuery(element).data('show', 2);
    }
    else {
      jQuery(element).data('show', 1);
    }
    filloutDataCacheElement(element, true);
  });
  jQuery(element).find('TABLE #rangeinput').hide();
  jQuery(element).find('TABLE #rangetext').click(function(e) {
    jQuery(this).hide();
    jQuery(element).find('TABLE #rangeinput').show();
  });
  jQuery(element).find('TABLE #rangego').click(function(e) {
    var newstartstr = jQuery(element).find('TABLE #rangeinput > INPUT').val();
    if (/^[1-9][0-9]{0,8}$/.test(newstartstr)) {
      var newend = Math.floor((Math.min(parseInt(newstartstr), itemcount) + batchsize - 1) / batchsize) * batchsize;
      reloadForMinMax(newend - batchsize + 1, newend);
    }
    else {
      jQuery(element).find('TABLE #rangeinput').hide();
      jQuery(element).find('TABLE #rangetext').show();
    }
  });
  jQuery(element).find('TABLE #filterbutton, TABLE #fclosebutton').click(function(e) {
    jQuery(element).find('TABLE #filterspan').toggle();
    jQuery(element).find('TABLE #rangeselection').toggle();
    if (jQuery(element).find('TABLE #rangeselection').is(':visible')) {
      reloadForMinMax(startitemnr, enditemnr);
    }
  });
  jQuery(element).find('TABLE #filtergo').click(function(e) {
    var filtertype = jQuery(element).find('TABLE #filtertype').val();
    var filtervalue = jQuery(element).find('TABLE #filtervalue').val();
    var filterfrom = 0;

    if (filtertype || filtervalue) {
      jQuery(element).find('TABLE #filterfrom').val(filterfrom);
      var filter = {type:filtertype, value:filtervalue, from:filterfrom};
      filloutDataCacheElement(element, false, showfromitemnr, undefined, filter);
    }
  });
  var filterfrom = parseInt(jQuery(element).find('TABLE #filterfrom').val());
  jQuery(element).find('TABLE #fprevbutton').prop('disabled', (filterfrom <= 0)).click(function(e) {
    var filtertype = jQuery(element).find('TABLE #filtertype').val();
    var filtervalue = jQuery(element).find('TABLE #filtervalue').val();

    if ((filtertype || filtervalue) && (filterfrom >= batchsize)) {
      filterfrom -= batchsize;
      jQuery(element).find('TABLE #filterfrom').val(filterfrom);
      var filter = {type:filtertype, value:filtervalue, from:filterfrom};
      filloutDataCacheElement(element, false, showfromitemnr, undefined, filter);
    }
  });
  jQuery(element).find('TABLE #fnextbutton').prop('disabled', (itemcount == 0)).click(function(e) {
    var filtertype = jQuery(element).find('TABLE #filtertype').val();
    var filtervalue = jQuery(element).find('TABLE #filtervalue').val();

    if (filtertype || filtervalue) {
      filterfrom += batchsize;
      jQuery(element).find('TABLE #filterfrom').val(filterfrom);
      var filter = {type:filtertype, value:filtervalue, from:filterfrom};
      filloutDataCacheElement(element, false, showfromitemnr, undefined, filter);
    }
  });
}

function setupDisplayFullValue($context, workflowid, otherid, stepid, type) {
  // display full value
  var placementfunc = function(fvdiv) {
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
  };
  var htmlEncode = function(value) {
    return jQuery('<div>').text(value).html();
  };
  $context.find('.upi_shortvalue').click(function(e) {
    e.stopPropagation();
    var fvdiv = jQuery(this).prev('.upi_fullvalue');

    if (type == 2) {
      var path = fvdiv.data('filepath');
      var query = {
        'workflowid'   : workflowid,
        'otherid'      : otherid,
        'stepid'       : stepid,
        'path'         : path,
      };

      var url = '/admin/islandora/prepare_ingest/ajax/files';
      jQuery.getJSON(url, query, function(fd) {
        var cellhtml = path + '<BR/><BR/>';
        if (fd.hasOwnProperty('removed') && fd['removed']) {
          if (fd.hasOwnProperty('type') && fd['type'] === 'directory') {
            cellhtml += 'Directory';
          }
          else {
            cellhtml += 'File';
          }
          cellhtml += ' is removed';
        }
        else if (fd.hasOwnProperty('type') && fd['type'] === 'directory') {
          cellhtml += 'Is a directory';
        }
        else if (fd.hasOwnProperty('content')) {
          if (fd.hasOwnProperty('archivefilepath')) {
            cellhtml += 'Part of archive ' + htmlEncode(fd['archivefilepath']) + '<BR/><BR/>';
          }
          cellhtml += 'Content (' + fd['content'].length + ' bytes):' + '</BR>';
          cellhtml += '<pre>';
          if (fd['content'].length > 10000) {
            cellhtml += htmlEncode(fd['content'].substr(0, 10000) + '...');
          }
          else {
            cellhtml += htmlEncode(fd['content']);
          }
          cellhtml += '</pre>';
        }
        else if (fd.hasOwnProperty('realfilepath')) {
          cellhtml += 'Contains same data as file at ' + fd['realfilepath'];
        }
        fvdiv.html(cellhtml);
        placementfunc(fvdiv);
      });
    }
    else {
      placementfunc(fvdiv);
    }
  });
  $context.find('.upi_fullvalue').click(function(e) {
    jQuery(this).hide();
  });
}

}

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
