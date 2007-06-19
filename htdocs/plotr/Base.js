/*
	Plotr.Base
	==========	
	Plotr.Base is part of the Plotr Charting Framework.
	
	For license/info/documentation: http://www.solutoire.com/plotr/
	
	Credits
	-------
	Plotr is partially based on PlotKit (BSD license) by
	Alastair Tse <http://www.liquidx.net/plotkit>.
	
	Copyright
	---------
 	Copyright 2007 (c) Bas Wenneker <sabmann[a]gmail[d]com>
 	For use under the BSD license. <http://www.solutoire.com/plotr>
*/

try {
	if (typeof(Prototype) == 'undefined') throw '';
} catch(e) {
	throw 'Plotr depends on the Prototype framework (version 1.5.0).';
};

if (typeof(Plotr) == 'undefined') {
	Plotr = {};
};

Plotr.name = 'Plotr';
Plotr.version = '0.1.4';
Plotr.author = 'Bas Wenneker';

if (typeof(Plotr.Base) == 'undefined') {
	Plotr.Base = {};
};

/** 
 * Plotr.Base.items puts all (non Function) items of lst into 
 * an Array and returns the Array.
 * 
 * @alias items
 * @param {Object} lst
 * @return {Array} result - Array that contains all nonFunction items of lst.
 */
Plotr.Base.items = function(lst) {
	var result = new Array();
	for(var item in lst){
		if (typeof(lst[item]) == 'function') continue;
		result.push(lst[item]);
	}	
	return result;	
};

/**
 * Check if obj is null or undefined.
 * 
 * @alias isNil
 * @param {Object} obj
 * @return {Boolean} true if null or undefined.
 */
Plotr.Base.isNil = function(obj) {
	return (obj == null || typeof(obj) == 'undefined');
};

/**
 * Check if canvas simulation by ExCanvas is supported by the browser.
 *  
 * @alias excanvasSupported
 * @return {Boolean} true if userAgent is IE
 */
Plotr.Base.excanvasSupported = function() {
     if (/MSIE/.test(navigator.userAgent) && !window.opera) {
         return true;
     }
     return false;
};

/**
 * Check whether or not canvas is supported by the browser.
 * 
 * @alias isSupported
 * @param {String} canvasName - ID of the canvas element.
 * @return {Boolean} true if browser has canvas support supported.
 */
Plotr.Base.isSupported = function(canvasName) {
    var canvas = null;
    try {
        if (Plotr.Base.isNil(canvasName)) 
            canvas = document.createElement('canvas');
        else
            canvas = $(canvasName);
        var context = canvas.getContext('2d');
    }
    catch(e) {
        var ie = navigator.appVersion.match(/MSIE (\d\.\d)/);
        var opera = (navigator.userAgent.toLowerCase().indexOf('opera') != -1);
        if ((!ie) || (ie[1] < 6) || (opera))
            return false;
        return true;
    }
    return true;
};

/**
 * This function checks lst for the element with the largest length and
 * then returns an array with element 0 .. length.
 * 
 * @alias uniqueIndices
 * @param {Array} lst
 * @return {Array} result - Returns an array with unique numbers.
 */
Plotr.Base.uniqueIndices = function(lst) {
	var result = new Array();
	lst.max(function(item){
		return item.length;
	}).times(function(i){
		result.push(i);	
	});	
	return result;	
};

Plotr.Base.sum = function(lst){
	lst = lst.flatten();
	var result = 0;
	for(var i = lst.length-1; i >= 0; i--)
		result += lst[i];
	return result;
};

/**
 * Convert an string with an hexadecimal color code {'#ffffff','ffffff'} 
 * to an RGB object {r: int, g: int, b: int}.
 * 
 * @alias hexToRGB
 * @param {String} hex - String with hexadecimal color code like '#ffffff' or 'ffffff'.
 * @return {Object} rgbObj - Returns an object {r: int, g: int, b: int}.
 */
Plotr.Base.hexToRGB = function(hex) {	
	hex = parseInt(((hex.indexOf('#') > -1) ? hex.substring(1) : hex), 16);
	return {r: hex >> 16, g: (hex & 0x00FF00) >> 8, b: (hex & 0x0000FF)};
};

/**
 * Returns a String representation of rgbObj.
 * 
 * @alias toRGBString
 * @param {Object} rgbObj - Object with RGB values {r: int, g: int, b: int}.
 * @return {String} rgb - Returns a String representation of the rgbObj.
 */
Plotr.Base.toRGBString = function(rgbObj) {
	return '#' + [rgbObj.r, rgbObj.g, rgbObj.b].invoke('toColorPart').join('');
};

/**
 * Convert an string with an hexadecimal color code {'#ffffff','ffffff'} 
 * to an HSB object {h: int, s: int, b: int}.
 * 
 * @alias hexToHSB
 * @param {String} hex - String with hexadecimal color code like '#ffffff' or 'ffffff'.
 * @return {Object} rgbObj - Returns an object {r: int, g: int, b: int}. 
 */
Plotr.Base.hexToHSB = function(hex) {
	return Plotr.Base.rgbToHSB(Plotr.Base.hexToRGB(hex));
};

/** 
 * Modification of Promeths' script. Converts a rgbObj {r: int, g: int, b: int}
 * to a hsbObj {h: int, s: int, b: int}.
 * 
 * @alias rgbToHSB
 * @param {Object} rgbObj - Object with RGB values {r: int, g: int, b: int}.
 * @return {Object} hsbObj - Returns an object with HSB values {h: int, s: int, b: int}.
 * @author Prometh - http://proto.layer51.com/d.aspx?f=1136
 */
Plotr.Base.rgbToHSB = function(rgbObj) {
	var r = rgbObj.r;
	var g = rgbObj.g;
	var b = rgbObj.b;
	var hsb = {};
	
	hsb.b = Math.max(Math.max(r,g),b);
	hsb.s = (hsb.b <= 0) ? 0 : Math.round(100*(hsb.b - Math.min(Math.min(r,g),b))/hsb.b);
	hsb.b = Math.round((hsb.b /255)*100);
	if((r==g) && (g==b)) hsb.h = 0;
	else if(r>=g && g>=b) hsb.h = 60*(g-b)/(r-b);
	else if(g>=r && r>=b) hsb.h = 60  + 60*(g-r)/(g-b);
	else if(g>=b && b>=r) hsb.h = 120 + 60*(b-r)/(g-r);
	else if(b>=g && g>=r) hsb.h = 180 + 60*(b-g)/(b-r);
	else if(b>=r && r>=g) hsb.h = 240 + 60*(r-g)/(b-g);
	else if(r>=b && b>=g) hsb.h = 300 + 60*(r-b)/(r-g);
	else hsb.h = 0;
	hsb.h = Math.round(hsb.h);
	return hsb;
};

/** 
 * Modification of Promeths' script. Converts a hsbObj {h: int, s: int, b: int}
 * to a rgbObj {r: int, g: int, b: int}.
 * 
 * @alias hsbToRGB
 * @param {Object} hsbObj - Object with HSB value {h: int, s: int, b: int}.
 * @return {Object} rgbObj - Returns an Object with RGB values {r: int, g: int, b: int}.
 * @author Prometh - http://proto.layer51.com/d.aspx?f=1135
 */
Plotr.Base.hsbToRGB = function(hsbObj) {
	var r,g,b;
	var h = Math.round(hsbObj.h);
	var s = Math.round(hsbObj.s*255/100);
	var v = Math.round(hsbObj.b*255/100);
	if(s == 0) {
		r = g = b = v;
	} else {
		var t1 = v;	
		var t2 = (255-s)*v/255;	
		var t3 = (t1-t2)*(h%60)/60;
		if(h==360) h = 0;
		if(h<60) {r=t1;	b=t2; g=t2+t3}
		else if(h<120) {g=t1; b=t2;	r=t1-t3}
		else if(h<180) {g=t1; r=t2;	b=t2+t3}
		else if(h<240) {b=t1; r=t2;	g=t1-t3}
		else if(h<300) {b=t1; g=t2;	r=t2+t3}
		else if(h<360) {r=t1; g=t2;	b=t1-t3}
		else {r=0; g=0;	b=0}
	}
	return {r:Math.round(r), g:Math.round(g), b:Math.round(b)};
};

/**
 * Plotr.Base.levelHSB takes an hsbObj alters the brightness of it.
 * The brightness will be the max(initial brightness + level, 100).
 * 
 * @alias levelHSB
 * @param {Object} hsbObj - Object with HSB values {h: int, s: int, b: int}.
 * @param {Object} level - Increase Brightness with level.
 * @return {String} rgbString - Returns a RGB string representation of the leveled hsbObj.
 */
Plotr.Base.levelHSB = function(hsbObj, level) {
	var hsb = {
		h: hsbObj.h,
		s: hsbObj.s,
		b: Math.min(hsbObj.b + level, 100)
	};
	return Plotr.Base.toRGBString(Plotr.Base.hsbToRGB(hsb));
};

/** 
 * Plotr.Base.generateColorscheme returns an Array with string representations of
 * colors. The colors start from 'hex' and every color after hex has the same Hue
 * and Saturation but a leveled Brightness. So colors go from dark to light. 
 * If reverse is set to true, the colors go from light to dark. 
 * 
 * @alias generateColorscheme
 * @param {String} hex - String with hexadecimal color code like '#ffffff' or 'ffffff'.
 * @param {Integer} size - Size of the colorScheme.
 * @param {Boolean} reverse - True if you want the colorScheme to be reversed.
 * @return {Array} result - Returns a colorScheme Array of length 'size'.
 */
Plotr.Base.generateColorscheme = function(hex, size, reverse) {
	var hsb = Plotr.Base.hexToHSB(hex);
	var result = new Array();
	var level = (100 - hsb.b)/size;
	
	size.times(function(index){
		result.push(Plotr.Base.levelHSB(hsb, level*index));
	});
	
	return (reverse) ? result.reverse() : result;
};

/**
 * Returns the default (green) colorScheme.
 * 
 * @alias defaultScheme
 * @param {Integer} size - Size of the colorScheme to return.
 * @return {Array} colorScheme - Returns an Array of colors of length 'size'.
 */
Plotr.Base.defaultScheme = function(size) {
	return Plotr.Base.generateColorscheme('#3c581a',size)
};

/**
 * Function returns a colorScheme based on 'color' of length 'size'.
 * 
 * @alias getColorscheme
 * @see Plotr.Base.colorSchemes 
 * @param {String} color
 * @param {Object} size - Size the colorScheme that's returned should have.
 * @return {Array} colorScheme - Returns an Array of colors of length 'size'.
 */
Plotr.Base.getColorscheme = function(color, size) {
	var scheme = Plotr.Base.colorSchemes[color];
	if(Plotr.Base.isNil(scheme)){
		return Plotr.Base.generateColorscheme(color,(size) ? size : 3);
	}
	return Plotr.Base.generateColorscheme(scheme,(size) ? size : 3);
};

/**
 * Storage of colors for predefined colorSchemes.
 * 
 * @alias colorSchemes
 */
Plotr.Base.colorSchemes = {
	red: '#6d1d1d',
	green: '#3c581a',
	blue: '#224565',
	grey: '#444444',
	black: '#000000'
};