/**
 * @file
 * js/ubl_show_persistent_url.js
 */

jQuery(document).ready(function() {
  // remove button
  jQuery('.remove_step_button').click(function(e) {
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
  jQuery('.ungroup_step_button').click(function(e) {
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
  // move up button
  jQuery('.moveup_step_button').click(function(e) {
    e.preventDefault();
    var $thisstep = jQuery(this).closest('fieldset.workflow_step');
    if ($thisstep.size() == 1) { 
      var $prevstep = $thisstep.prev('.workflow_step');
      var $thisendgroup = $thisstep.next('fieldset.workflow_step.visual_group_end');
      var $goingoutofgroup = false;
      if ($prevstep.size() == 0) {
        // from an open group?
        if ($thisstep.parents('fieldset.workflow_step').size() > 0) {
          $prevstep = $thisstep.parents('fieldset.workflow_step').first(); 
          $goingoutofgroup = true;
        }
      }
      if ($thisstep.find('> div > div.grouped_steps').size() > 0) {
        // a group is moving up
        if (!$goingoutofgroup && $prevstep.hasClass('visual_group_end')) {
          // the step above is another group
          var $prevgroup = $prevstep.prev();
          if ($prevgroup.hasClass('collapsed')) {
            var $prevendgroup = $prevstep;
            var $prevsteps = retrieveStepsBetween($prevgroup,$thisstep);
            var $thissteps = retrieveStepsBetween($thisstep,$thisendgroup);
            $thissteps.push($thisendgroup);
            exchangeStepWeights($prevsteps, $thissteps);
            swapElements($prevendgroup, $thisstep, function() {
              $thisstep.after($prevendgroup);
              setTimeout(function() {
                swapElements($prevgroup, $thisstep, function() {
                  $thisstep.after($prevgroup);
                  setTimeout(function() {
                    swapElements($prevendgroup, $thisendgroup, function() {
                      $thisendgroup.after($prevendgroup); 
                      setTimeout(function() {
                        swapElements($prevgroup, $thisendgroup);
                      }, 10);
                    }, 200);
                  }, 10);
                }, 200);
              }, 10);
            });
          }
          else {
            // going into other group
            var $steps = retrieveStepsBetween($thisstep,$thisendgroup);
            $steps.unshift($prevstep, $prevstep);
            if ($thisendgroup.size() > 0) {
              $steps.push($thisendgroup);
            }
            $steps.push($prevstep);
            moveStepWeightsUp($steps);

            var $stepsgroup = $prevstep.prev().find('> div > div.grouped_steps').first();
            var $lastingroup = jQuery('<DIV/>');
            $stepsgroup.append($lastingroup);
            swapElements($lastingroup, $thisstep, function() {
              $stepsgroup.append($thisstep);
              $lastingroup.remove();
              if ($thisendgroup.size() > 0) {
                $lastingroup = jQuery('<DIV/>');
                $stepsgroup.append($lastingroup);
                setTimeout(function() {
                  swapElements($lastingroup, $thisendgroup, function() {
                    $stepsgroup.append($thisendgroup);
                    $lastingroup.remove();
                  }, 200);
                }, 10);
              }
            });
          }
        }
        else {
          var $steps = retrieveStepsBetween($thisstep,$thisendgroup);
          $steps.unshift($prevstep, $prevstep);
          if ($thisendgroup.size() > 0) {
            $steps.push($thisendgroup);
          }
          $steps.push($prevstep);
          moveStepWeightsUp($steps);

          if ($goingoutofgroup) {
            // this group is going out of another group
            var $prevstepfields = $thisstep.parent().parent().parent().find(' > LEGEND, > DIV > .fields, > DIV > .buttons');
            swapElements($prevstepfields, $thisstep, function() {
              $prevstep.before($thisstep);
              if ($thisendgroup.size() > 0) {
                setTimeout(function() {
                  swapElements($prevstepfields, $thisendgroup, function() {
                    $prevstep.before($thisendgroup);
                  }, 200);
                }, 10);
              }
            }, 200);
          }
          else {
            // this group is just going up
            swapElements($prevstep, $thisstep, function () {
              $thisstep.after($prevstep);
              if ($thisendgroup.size() > 0) {
                setTimeout(function() {
                  swapElements($thisendgroup, $prevstep, function() {
                    $thisendgroup.after($prevstep);
                    $prevstep.css('top' , 'auto');
                    $thisendgroup.css('top' , 'auto');
                  }, 200);
                }, 10);
              }
            }, 200);
          }
        }
      }
      else if (!$goingoutofgroup 
          && $prevstep.prev().hasClass('collapsed')
          && ($prevstep.prev().find('> div > div.grouped_steps').size() > 0)) {
        var $prevprevstep = $prevstep.prev();
        var $steps = retrieveStepsBetween($prevprevstep,$thisstep);
        var len = $steps.length;
        var $pstep = $thisstep;
        for (var index = 0; index < len; index++) {
          var $cstep = $steps[index];
          flipStepWeights($cstep, $pstep);
          $pstep = $cstep;
        }
        swapElements($prevstep, $thisstep, function () {
          $thisstep.after($prevstep);
          setTimeout(function() {
            swapElements($prevprevstep, $thisstep, function() {
              $thisstep.after($prevprevstep); 
            }, 200);
          }, 10);
        }, 200);
      }
      else if ($prevstep.size() == 1) {
        flipStepWeights($thisstep, $prevstep);
        if ($goingoutofgroup) {
          // going out of open group
          var $prevstepfields = $thisstep.parent().parent().parent().find(' > LEGEND, > DIV > .fields, > DIV > .buttons');
          swapElements($prevstepfields, $thisstep, function() { $prevstep.before($thisstep); }); 
        }
        else {
          if ($prevstep.prev().find('> div > div.grouped_steps').size() > 0) {
            // going into an open group
            var $stepsgroup = $prevstep.prev().find('> div > div.grouped_steps').first();
            var $lastingroup = jQuery('<DIV/>');
            $stepsgroup.append($lastingroup);
            swapElements($lastingroup, $thisstep, function() { $stepsgroup.append($thisstep); $lastingroup.remove(); });
          }
          else {
            swapElements($prevstep, $thisstep);
          }
        }
      }
    }
  });

  // move down button
  jQuery('.movedown_step_button').click(function(e) {
    e.preventDefault();
    var $thisstep = jQuery(this).closest('fieldset.workflow_step');
    if ($thisstep.size() == 1) { 
      if ($thisstep.find('> div > div.grouped_steps').size() > 0) {
        // a group is moving down
        var $thisendgroup = $thisstep.next('fieldset.workflow_step.visual_group_end');
        if ($thisendgroup.size() == 0) {
          return; // no end group found, unexpected
        }
        var $nextstep = $thisendgroup.next('fieldset');
        if ($nextstep.size() > 0) {
          if ($nextstep.find('> div > div.grouped_steps').size() > 0) {
            // next step is also a group.
            if ($nextstep.hasClass('collapsed')) {
              // going past a collapsed group
              var $nextendgroup = $nextstep.next('fieldset.workflow_step.visual_group_end');
              var $thissteps = retrieveStepsBetween($thisstep,$nextstep);
              var $nextsteps = retrieveStepsBetween($nextstep,$nextendgroup);
              $nextsteps.push($nextendgroup);
              exchangeStepWeights($thissteps, $nextsteps);
              swapElements($thisendgroup, $nextstep, function() {
                $nextstep.after($thisendgroup);
                setTimeout(function() {
                  swapElements($thisstep, $nextstep, function() {
                    $nextstep.after($thisstep);
                    setTimeout(function() {
                      swapElements($thisendgroup, $nextendgroup, function() {
                        $nextendgroup.after($thisendgroup);
                        setTimeout(function() {
                          swapElements($thisstep, $nextendgroup);
                        }, 10);
                      }, 200);
                    }, 10);
                  }, 200);
                }, 10);
              });
            }
            else {
              // going into an open group
              var $thissteps = retrieveStepsBetween($thisstep,$nextstep);
              exchangeStepWeights($thissteps, $nextstep);
  
              var $nextstepsgroup = $nextstep.find('> div > div.grouped_steps').first();
              var $firstingroup = jQuery('<DIV/>');
              $nextstepsgroup.prepend($firstingroup);
              swapElements($firstingroup, $thisendgroup, function() {
                $nextstepsgroup.prepend($thisendgroup);
                $firstingroup.remove();
                setTimeout(function() {
                  swapElements($thisendgroup, $thisstep, function() {
                    $nextstepsgroup.prepend($thisstep);
                  }, 200);
                }, 10);
              });
            }
          }
          else {
            // just going down
            var $thissteps = retrieveStepsBetween($thisstep,$nextstep);
            exchangeStepWeights($thissteps, $nextstep);
            swapElements($thisendgroup, $nextstep, function () {
              $nextstep.after($thisendgroup);
              setTimeout(function() {
                swapElements($thisstep, $nextstep, function() {
                  $nextstep.after($thisstep);
                }, 200);
              }, 10);
            }, 200);
          }
        }
        else {
          if ($thisstep.parents('fieldset.workflow_step').size() > 0) {
            // going out of a group
            var $thissteps = retrieveStepsBetween($thisstep,$thisendgroup);
            $thissteps.push($thisendgroup);
            $nextstep = $thisstep.parents('fieldset.workflow_step').next('fieldset.workflow_step');
            exchangeStepWeights($thissteps, $nextstep);
            swapElements($thisendgroup, $nextstep, function () {
              $nextstep.after($thisendgroup);
              setTimeout(function() {
                swapElements($thisstep, $nextstep, function() {
                  $nextstep.after($thisstep);
                }, 200);
              }, 10);
            }, 200);
          }
        }
      }
      else {
        var $nextstep = $thisstep.next('.workflow_step');
        var $goingoutofgroup = false;
        if ($nextstep.size() == 0) {
          // from an open group?
          if ($thisstep.parents('fieldset.workflow_step').size() > 0) {
            $nextstep = $thisstep.parents('fieldset.workflow_step').first().next();
            $goingoutofgroep = true;
          }
        }
        if (!$goingoutofgroup
            && $nextstep.hasClass('collapsed')
            && ($nextstep.find('> div > div.grouped_steps').size() > 0)) {
          var $nextnextstep = $nextstep.next('.workflow_step');
          if ($nextnextstep.size() > 0) {
            var $steps = retrieveStepsBetween($thisstep,$nextnextstep);
            $steps.unshift($thisstep);
            $steps.push($nextnextstep, $thisstep);
            moveStepWeightsUp($steps);
            swapElements($thisstep, $nextstep, function () {
              $nextstep.after($thisstep);
              setTimeout(function() {
                swapElements($thisstep, $nextnextstep, function() {
                  $nextnextstep.after($thisstep);
                }, 200);
              }, 10);
            }, 200);
          }
        }
        else if ($nextstep.size() == 1) {
          flipStepWeights($thisstep, $nextstep);
          if ($nextstep.find('> div > div.grouped_steps').size() > 0) {
            var $group = $nextstep.find('> div > div.grouped_steps').first();
            swapElements($thisstep, $nextstep, function() { $group.prepend($thisstep); });
          }
          else {
            swapElements($thisstep, $nextstep);
          }
        }
      } 
    }
  });
  
  function flipStepWeights($step1, $step2) {
    var step1weight = $step1.find('.weight_step').first().val();
    if (step1weight.lastIndexOf(prefix, 0) !== 0) { // does it have the prefix?
      step1weight = prefix + step1weight;
    }
    var step2weight = $step2.find('.weight_step').first().val();
    if (step2weight.lastIndexOf(prefix, 0) !== 0) { // does it have the prefix?
      step2weight = prefix + step2weight;
    }
    $step1.find('.weight_step').first().val(step2weight);
    $step2.find('.weight_step').first().val(step1weight);
  }

  function moveStepWeightsUp($steps) {
    var cstepweight = '';
    for (var index = 0; index < ($steps.length - 1); index++) {
      var $cstep = $steps[index];
      var $nstep = $steps[index+1];
      var nstepweight = $nstep.find('.weight_step').first().val();
      if (nstepweight.lastIndexOf(prefix, 0) !== 0) { // does it have the prefix?
        nstepweight = prefix + nstepweight;
      }
      $nstep.find('.weight_step').first().val(cstepweight);
      cstepweight = nstepweight;
    }
  }

  function exchangeStepWeights($steps1, $steps2) {
    var $allstepscurrentorder;
    if ($steps1.concat && $steps2.concat) {
      $allstepscurrentorder = $steps1.concat($steps2);
    }
    else if ($steps1.concat) {
      $allstepscurrentorder = $steps1.concat([$steps2]); 
    }
    else if ($steps2.concat) {
      $allstepscurrentorder = [$steps1].concat($steps2); 
    }
    else {
      $allstepscurrentorder = [$steps1, $steps2];
    }
    var weights = [];
    jQuery.each($allstepscurrentorder, function(i, $astep) {
      var stepweight = $astep.find('.weight_step').first().val();
      if (stepweight.lastIndexOf(prefix, 0) !== 0) { // does it have the prefix?
        stepweight = prefix + stepweight;
      }
      weights.push(stepweight);
    }); 
    var $allstepsexchanged;
    if ($steps1.concat && $steps2.concat) {
      $allstepsexchanged = $steps2.concat($steps1);
    }
    else if ($steps1.concat) {
      $allstepsexchanged = [$steps2].concat($steps1);
    }
    else if ($steps2.concat) {
      $allstepsexchanged = $steps2.concat([$steps1]);  
    }
    else {
      $allstepsexchanged = [$steps2, $steps1];
    }
    jQuery.each($allstepsexchanged, function(i, $astep) {
      $astep.find('.weight_step').first().val(weights[i]);
    });
  }

  function retrieveStepsBetween($fromElement, $toElement) {
    var $steps = [];
    var $currentstep = $fromElement;
    while ($currentstep != null && $currentstep.size() > 0 && $currentstep.attr('id') != $toElement.attr('id')) {
      $steps.push($currentstep);
      if ($currentstep.find('> div > div.grouped_steps > fieldset.workflow_step').size() > 0) {
        var $firstingroup = $currentstep.find('> div > div.grouped_steps > fieldset.workflow_step').first();
        var $moresteps = retrieveStepsBetween($firstingroup, $toElement);
        var len = $moresteps.length;
        for (var i=0; i<len; i++) {
          $steps.push($moresteps[i]);
        }
      } 
      $currentstep = $currentstep.next('fieldset.workflow_step');
    }

    return $steps;
  }

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
      $textfield.parents('.workflow_step').prevAll('.workflow_step').find('.output_key, .keys, .keystemplate').each(function(i, element) {
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
        else if (jQuery(element).hasClass('keystemplate')) {
          var re = /[^{]*{([^}]+)}/g;
          var match;
          while ((match = re.exec(outputvalue)) != null) {
            menuMaker(match[1]);
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

    setTimeout(function() {
      var hidefunc = function() {
        jQuery('#autosuggestmenu').hide(500);
      };
      jQuery('SELECT, INPUT[type="submit"], BUTTON').one('focus', hidefunc).one('click', hidefunc);
      jQuery('BODY').one('click', hidefunc);
    }, 500);
  };
  jQuery('INPUT, TEXTAREA').change(autosuggestfunc).focus(autosuggestfunc);

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
  jQuery('.upi_fullvalue').click(function(e) {
    jQuery(this).hide();
  });
});

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
