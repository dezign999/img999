var MessageQueue  = require('./MessageQueue');
var dithering     = require('./dithering');
var image_tools   = require('./image_tools');
var JpegImage     = require('./jpg');
var pebble        = require('./pebble');
var Png4Pebble    = require('./generatepng4pebble.js');
var Clay          = require('pebble-clay');
var clayConfig    = require('./config');
var customClay    = require('./custom-clay');
var clay          = new Clay(clayConfig, customClay, { autoHandleEvents: false });
var cachedBitmap;
var defaultUrl    = "https://i.postimg.cc/fb0kJb1S/D45-A9844-9-BD5-48-BA-9-B8-A-04-E8559-A7-C07.jpg";

var oldurl = JSON.parse(localStorage.getItem('oldurl'));

var CHUNK_SIZE = 6000;
var DOWNLOAD_TIMEOUT = 20000;
var app = {};

var sUrl;
var sHourColor;
var sMinuteColor;
var sGradient;
var sGradColor;
var sLayout;

function sendBitmap(bitmap){
  var i = 0;
  var nextSize = bitmap.length-i > CHUNK_SIZE ? CHUNK_SIZE : bitmap.length-i;
  var sliced = bitmap.slice(i, i + nextSize);

  sendMessage({"size": bitmap.length});

  var success = function(){
    if(i>=bitmap.length)
      return;
    i += nextSize;
    // console.log(i + "/" + bitmap.length);
    nextSize = bitmap.length-i > CHUNK_SIZE ? CHUNK_SIZE : bitmap.length-i;
    sliced = bitmap.slice(i, i + nextSize);
   sendMessage(
      {
      "index":i,
      "chunk":sliced
      },
      success,
      null
      );
  };

  sendMessage(
      {
      "index":i,
      "chunk":sliced
      },
      success,
      null
      );
}

function convertImage(rgbaPixels, numComponents, width, height){

  var watch_info;
  if(Pebble.getActiveWatchInfo) {
    watch_info = Pebble.getActiveWatchInfo() || { 'platform' : 'aplite'};
    
  } else {
    watch_info = { 'platform' : 'aplite'};
  }
  // console.log("Watch Info: " + JSON.stringify(watch_info));
  // var ratio = Math.min(144 / width,168 / height);
	var ratio = Math.min(180 / width,180 / height);
  var ratio = Math.min(ratio,1);

  var final_width = Math.floor(width * ratio);
  var final_height = Math.floor(height * ratio);
  var final_pixels = [];
  var bitmap = [];

  if(watch_info.platform === 'aplite' || watch_info.platform === 'diorite') {
    var grey_pixels = image_tools.greyScale(rgbaPixels, width, height, numComponents);
    image_tools.ScaleRect(final_pixels, grey_pixels, width, height, final_width, final_height, 1);
    dithering.floydSteinberg(final_pixels, final_width, final_height, dithering.pebble_nearest_color_to_black_white);
    bitmap = pebble.toPBI(final_pixels, final_width, final_height);
  }
  else {
    image_tools.ScaleRect(final_pixels, rgbaPixels, width, height, final_width, final_height, numComponents);
    dithering.floydSteinberg(final_pixels, final_width, final_height, dithering.pebble_nearest_color_to_pebble_palette);
    var png = Png4Pebble.png(final_width, final_height, final_pixels);
    for(var i=0; i<png.length; i++){
      bitmap.push(png.charCodeAt(i));
    }
  }

  return bitmap;
	
}

function getJpegImage(url) {
    var j = new JpegImage();
    j.onload = function () {
        clearTimeout(xhrTimeout); // Got response, no more need for timeout

        if (j.width > 500 || j.height > 500) {
            sendMessage({ "message": "Large Image Detected" }, null, null);
        }

        var pixels = j.getData(j.width, j.height);
        var bitmap = convertImage(pixels, 3, j.width, j.height);

        // Overwrite the cached bitmap
        // console.log("Saving bitmap to localStorage");
        localStorage.setItem("cachedBitmap", JSON.stringify(bitmap));

        sendBitmap(bitmap);
    };

    var xhrTimeout = setTimeout(function () {
        sendMessage({ "message": "Error : Timeout" }, null, null);
    }, DOWNLOAD_TIMEOUT);

    try {
        j.load(url);
    } catch (e) {
        // console.log("Error : " + e);
    }
}

function endsWith(str, suffix) {
    return str.indexOf(suffix, str.length - suffix.length) !== -1;
}

function getImage(url) {

    if (endsWith(url, ".jpg") || endsWith(url, ".jpeg") || endsWith(url, ".JPG") || endsWith(url, ".JPEG") || endsWith(url, ".png") || endsWith(url, ".PNG")) {
        getJpegImage(url);
    } else {
        sendMessage({ "message": "Error : Bad Direct Link" }, null, null);
    }
}

Pebble.addEventListener("ready", function (e) {

  cachedBitmap = localStorage.getItem("cachedBitmap");

  if (cachedBitmap) {
    sendMessage({ "message": "Loading Cache" }, null, null);
    var bitmap = JSON.parse(cachedBitmap);
    sendBitmap(bitmap); // Send the cached bitmap directly
  } else {
    if (oldurl === null || oldurl == "")
      oldurl = defaultUrl;
    localStorage.setItem('oldurl', JSON.stringify(oldurl));
    getImage(oldurl);
  }
});

function sendMessage(data, success, failure) {
  MessageQueue.sendAppMessage(
    data,
    function(e) {
      if(success)
        success();
    },
    function(e) {
      if(failure)
          failure();
    });
}

Pebble.addEventListener('showConfiguration', function(e) {
  
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {	

  if (e && !e.response) {
    return;
  }

  clay.getSettings(e.response, false);  // This triggers the update in localStorage
  app.settings = JSON.parse(localStorage.getItem('clay-settings'));
  var url = app.settings.url;
  oldurl = JSON.parse(localStorage.getItem('oldurl'));
  sendMessage({ "message": "Received Changes" }, null, null);

  if (url !== oldurl && url !== "") {

    if (endsWith(url, ".jpg") || endsWith(url, ".jpeg") || endsWith(url, ".JPG") || endsWith(url, ".JPEG") || 
        endsWith(url, ".png") || endsWith(url, ".PNG")) {

      sendMessage({ "message": "Loading Image" }, null, null);
      localStorage.setItem('oldurl', JSON.stringify(url));
      getImage(url);

      Pebble.sendAppMessage(app.settings, function(e) { }, function(e) { });

    }
    
    if (endsWith(url, "*999")) {
      sendMessage({ "message": "Loading Theme" }, null, null);
      send = 0;
      var parts = url.split("*");

      sUrl = parts[0];
      sHourColor = parseInt(parts[1], 10);
      sMinuteColor = parseInt(parts[2], 10);
      sGradient = parts[3];
      sGradColor = parseInt(parts[4], 10);
      sLayout = parts[5];
      
      getImage(sUrl);

      const payload = {
        "hourlyVibe": app.settings.hourlyVibe,
        "url": sUrl,
        "hourColor": sHourColor,
        "minuteColor": sMinuteColor,
        "gradient": sGradient,
        "gradColor": sGradColor,
        "comp1": app.settings.comp1,
        "comp2": app.settings.comp2,
        "layout": sLayout,
      };
    
      // Save the payload to localStorage
      localStorage.setItem('clay-settings', JSON.stringify(payload));
      localStorage.setItem('oldurl', JSON.stringify(sUrl));
    
      // Send the payload to the Pebble app
      Pebble.sendAppMessage(payload, function() {
          // console.log('Payload sent successfully: ', JSON.stringify(payload));
          sendMessage({ "message": "" }, null, null);
      }, function(e) { });

    } else {
      sendMessage({ "message": "Error : Bad Direct Link" }, null, null);
    }
  } else {
    Pebble.sendAppMessage(app.settings, function(e) { 
      sendMessage({ "message": "" }, null, null);
    }, function(e) { });
  }

});