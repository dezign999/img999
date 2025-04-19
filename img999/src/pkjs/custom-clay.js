module.exports = function(minified) {
    var clayConfig = this;
    var _ = minified._;
    var $ = minified.$;
    var HTML = minified.HTML;

    function clearUrl() {
        try {
            {clayConfig.getItemByMessageKey('url').set("");}	
        } catch (e) {}
    }
      
    clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
        var sUrl = clayConfig.getItemByMessageKey('url').get();
        var sHourColor = clayConfig.getItemByMessageKey('hourColor').get();
        var sMinuteColor = clayConfig.getItemByMessageKey('minuteColor').get();
        var sGradient = clayConfig.getItemByMessageKey('gradient').get();
        var sGradColor = clayConfig.getItemByMessageKey('gradColor').get();
        var sLayout = clayConfig.getItemByMessageKey('layout').get();

        if (sUrl !== "") {
            try {	
                clayConfig.getItemById('themeCode').set("<span style=\"word-break: break-all;\">" + sUrl + "*" + sHourColor + "*" + sMinuteColor + "*" + sGradient + "*" + sGradColor + "*" + sLayout + "*999</span>");	
            } catch (e) {}
        }

        try {
			var button = clayConfig.getItemById('clearButton');
			button.on('click', clearUrl);
		} catch (e) {}
        
    });  
  };