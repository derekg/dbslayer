/*
	Plotr V0.1.4
	============
	This is a uncompressed version of Plotr.{Base,Chart,Canvas,LineChart,BarChart}, 
	packed by Dean Edwards's Packer.
	
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

/**
 * Plotr.Chart
 * 
 * @alias Plotr.Chart
 * @namespace Plotr 
 */
Plotr.Chart = {
	
	/**
	 * The constructor of Plotr.Chart.
	 * 
	 * @alias initialize
	 * @see Plotr.Canvas.setOptions
	 * @param {String} type - Choose from {'bar'}.
	 * @param {Object} options - Object with options.
	 */
	initialize: function(element, options) {
		this.setOptions(options);
		this.sets = 0;
		this.dataStores = new Array();
		
		if (!Plotr.Base.isNil(this.options.xAxis)) {
	        this.minxval = this.options.xAxis[0];
	        this.maxxval = this.options.xAxis[1];
	        this.xscale = this.maxxval - this.minxval; 
	    } else {
	        this.minxval = 0;
	        this.maxxval = this.xscale = null;
	    }
	
	    if (!Plotr.Base.isNil(this.options.yAxis)) {
	        this.minyval = this.options.yAxis[0];
	        this.maxyval = this.options.yAxis[1];
	        this.yscale = this.maxyval - this.minyval;
	    } else {
	        this.minyval = 0;
	        this.maxyval = this.yscale = null;
	    }
	
	    this.xticks = new Array();
		this.yticks = new Array();
	    this.minxdelta = 0;
	    this.xrange = 1;
		this.yrange = 1;
		
		this._initCanvas(element);
	},
	
	/**
	 * Function adds the array to the dataStores object. Argument must be in 
	 * the form of: {['<setName>': [[0,1],[1,2]...<data>], ..}. The function also 
	 * keeps track of how many sets are added.
	 * 
	 * @alias addDataset
	 * @param {Object} arguments - Object with data
	 */
	addDataset: function(store) {
		for(var name in store) {
			this.dataStores[name] = store[name];
			this.sets++;
		}
	},
	
	/**
	 * This function makes it easy to parse a table and show it's
	 * data in a chart. The upper left corner has coordinates (x=0,y=0).
	 * 
	 * @alias addTable
	 * @param {String|Element} table - table id or element;
	 * @param {Integer} x - xcoordinate to start with data parsing
	 * @param {Integer} y - y coordinate to start with data parsing
	 * @param {Integer} xticks - row number of row with labels for xticks
	 */
	addTable: function(table, x, y, xticks){
		table = $(table);
		
		if(Plotr.Base.isNil(x))	x = 0;
		if(Plotr.Base.isNil(y)) y = 1;
		if(Plotr.Base.isNil(xticks)) xticks = -1;
		
		var tr = table.tBodies[0].rows;
		var store = {};
		var labels = new Array();
		
		for(var i = y, ln = tr.length; i < ln; i++){
			var j = 0;		
			store['row_'+i] = $A(tr[i].cells).reject(function(cell,index){
				return index < x;
			}).collect(function(cell){
				return [j++, parseFloat(cell.innerHTML)];
			});
		}
		if(xticks >= 0){
			var tickIndex = 0;
			this.options.xTicks = $A(tr[xticks].cells).reject(function(cell,index){
				return index < x;
			}).collect(function(cell){
				return {v: tickIndex++, label: cell.innerHTML};
			});
		}
		this.addDataset(store);
	},
	
	/**
	 * This function does all the math. It'll process all the data that has to do
	 * with canvas measures.
	 * 
	 * @alias _eval
	 * @param {Object} options - (optional) evaluate the chart with the given options.
	 */
	_eval: function(options) {
		if(!Plotr.Base.isNil(options)) {
			Object.extend(options,{});
			this.setOptions(options);
		}
		this.stores = Plotr.Base.items(this.dataStores);
		this._evalXY();
		this.setColorscheme();
	},
	
	/**
	 * Processes measures of the bars(/lines/pies).
	 * 
	 * @alias _evalXY
	 */	
	_evalXY: function() {		
		var xdata = this.stores.collect(function(item) {return item.pluck(0)}).flatten();
		if (Plotr.Base.isNil(this.options.xAxis)) {
			this.minxval = (this.options.xOriginIsZero) ? 0 : parseFloat(xdata.min());
			this.maxxval = parseFloat(xdata.max());
		} else {
			this.minxval = this.options.xAxis[0];
	        this.maxxval = this.options.xAxis[1];
			this.xscale = this.maxxval - this.minxval;
		}
		this.xrange = this.maxxval - this.minxval;
		this.xscale = (this.xrange == 0) ? 1.0 : 1/this.xrange;	
		
		var ydata = this.stores.collect(function(item) {return item.pluck(1)}).flatten();
		if (Plotr.Base.isNil(this.options.yAxis)) {
			this.minyval = (this.options.yOriginIsZero) ? 0 : parseFloat(ydata.min());
			this.maxyval = parseFloat(ydata.max());
		} else {
			this.minyval = this.options.yAxis[0];
	        this.maxyval = this.options.yAxis[1];
			this.yscale = this.maxyval - this.minyval;
		}	
	    this.yrange = this.maxyval - this.minyval;
		this.yscale = (this.yrange == 0) ? 1.0 : 1/this.yrange;
	},
	
	/**
	 * Evaluates ticks for X and Y axis.
	 * 
	 * @alias _evalLineTicks
	 */
	_evalLineTicks: function() {		
		this._evalLineTicksForXAxis();
		this._evalLineTicksForYAxis();
	},
	
	/**
	 * Evaluates ticks for X axis.
	 * 
	 * @alias _evalLineTicksForXAxis
	 */
	_evalLineTicksForXAxis: function() {	    
	    if (this.options.xTicks) {	
			this.xticks = this.options.xTicks.collect(function (tick) {
				var label = tick.label;
	            if (Plotr.Base.isNil(label))
	                label = tick.v.toString();
	            var pos = this.xscale * (tick.v - this.minxval);
	            if ((pos >= 0.0) && (pos <= 1.0)) {
	                return [pos, label];
	            }
			}.bind(this));
	    } else if (this.options.xNumberOfTicks) {
	        var uniqx = Plotr.Base.uniqueIndices(this.stores);
	        var roughSeparation = this.xrange / this.options.xNumberOfTicks;
	        var tickCount = 0;
	
	        this.xticks = new Array();
	        for (var i = 0; i <= uniqx.length; i++) {
	            if ((uniqx[i] - this.minxval) >= (tickCount * roughSeparation)) {
	                var pos = this.xscale * (uniqx[i] - this.minxval);
	                if ((pos > 1.0) || (pos < 0.0))
	                    continue;
	                this.xticks.push([pos, uniqx[i]]);
	                tickCount++;
	            }
	            if (tickCount > this.options.xNumberOfTicks)
	                break;
	        }
    	}
	},
	
	/**
	 * Evaluates ticks for Y axis.
	 * 
	 * @alias _evalLineTicksForYAxis
	 */
	_evalLineTicksForYAxis: function() {	    
	    if (this.options.yTicks) {
			this.yticks = this.options.yTicks.collect(function (tick) {
				var label = tick.label;
	            if (Plotr.Base.isNil(label))
	                label = tick.v.toString();
	            var pos = 1.0 - (this.yscale * (tick.v - this.minyval));
	            if ((pos >= 0.0) && (pos <= 1.0)) {
	                return [pos, label];
	            }
			}.bind(this));
	    }else if (this.options.yNumberOfTicks) { 
	        this.yticks = new Array();
			var prec = this.options.yTickPrecision;
			var num = this.yrange/this.options.yNumberOfTicks;
	        var roughSeparation = num.toFixed(this.options.yTickPrecision);
			
	        for (var i = 0; i <= this.options.yNumberOfTicks; i++) {
	            var yval = this.minyval + (i * roughSeparation);
	            var pos = 1.0 - ((yval - this.minyval) * this.yscale);
	            if ((pos > 1.0) || (pos < 0.0))
	                continue;
	            this.yticks.push([pos, yval.toFixed(prec)]);
	        }
    	}
	}
};

/**
 * Plotr.Canvas
 * 
 * @alias Plotr.Canvas
 * @namespace Plotr 
 */
Plotr.Canvas = {
	
	/**
	 * Sets options of this chart. Current options are:
	 * - sweetRender: using more advanced renderering techniques the chart will look much sweeter. {Boolean}
	 * - drawBackground: whether a background should be drawn. {Boolean}
	 * - backgroundLineColor: color of backgroundlines. {String}
	 * - backgroundColor: background color. {String}
	 * - padding: padding. {Object}
	 * - colorScheme: Array of colors used for chart. {Array}
	 * - strokeColor: color of a stroke. {String}
	 * - strokeWidth: width of a stroke. {Number}
	 * - shouldFill: whether bars/lines/pies should be filled. {Boolean}
	 * - shouldStroke: whether strokes should be drawn. {Boolean}
	 * - drawXAxis: whether the X axis should be drawn. {Boolean}
	 * - drawYAxis: whether the Y axis should be drawn. {Boolean}
	 * - axisTickSize: size of a tick in pixels. {Number}
	 * - axisLineColor: color of axis lines. {String}
	 * - axisLineWidth: line width of axis. {Number}
	 * - axisLabelColor: axis label color. {String}
	 * - axisLabelFont: font familily used for labels. {String}
	 * - axisLabelFontSize: font size used for labels. {String}
	 * - axisLabelWidth: axis label width. {Number} 
	 * - barWidthFillFraction: sets the bar width (>1 will cause bars to overlap eachother). {Integer} 
	 * - barOrientation: whether bars are horizontal. {'horizontal','vertical'} 
	 * - xOriginIsZero: whether or not the origin of the X axis starts at zero. {Boolean}
	 * - yOriginIsZero: whether or not the origin of the Y axis starts at zero. {Boolean}
	 * - xAxis: values of xAxis {[xmin,xmax]}
	 * - yAxis: values of yAxis {[ymin,ymax]}
	 * - xTicks: labels for the X axis. {[{label: "somelabel", v: value}, ..]} (label = optional)
	 * - yTicks: labels for the Y axis. {[{label: "somelabel", v: value}, ..]} (label = optional)
	 * - xNumberOfTicks: number of ticks on X axis when xTicks is null. {Integer}
	 * - yNumberOfTicks: number of ticks on Y axis when yTicks is null. {Integer}
	 * - xTickPrecision: decimals for the labels on the X axis. {Integer}
	 * - yTickPrecision: decimals for the labels on the Y axis. {Integer}
	 * 
	 * @alias Chart.setOptions
	 * @param {Object} options - Object with options.
	 */
	setOptions: function(options) {
		this.options = Object.extend({
			sweetRender:		true,
	        drawBackground: 	true,
			backgroundLineColor:'#ffffff',
	        backgroundColor: 	'#f5f5f5',
	        padding: 			{left: 30, right: 30, top: 5, bottom: 10},
			colorScheme: 		Plotr.Base.defaultScheme(Math.max(this.sets,3)),
			strokeColor: 		'#ffffff',
	        strokeWidth: 		0.5,
	        shouldFill: 		true,
			shouldStroke: 		true,
	        drawXAxis: 			true,
	        drawYAxis: 			true,			
	        axisTickSize: 		3,
	        axisLineColor: 		'#000000',
	        axisLineWidth: 		0.5,
	        axisLabelColor: 	'#666666',
	        axisLabelFont: 		'Arial',
	        axisLabelFontSize: 	9,
			axisLabelWidth: 	50,
			barWidthFillFraction: 0.75,
			barOrientation: 'vertical',
        	xOriginIsZero: true,
			yOriginIsZero: true,
			xAxis: null,
        	yAxis: null,
			xTicks: null,
			yTicks: null,
			xNumberOfTicks: 10,
			yNumberOfTicks: 10,
			xTickPrecision: 1,
        	yTickPrecision: 1,
			pieRadius: 0.4
	    }, options || {});
	},
	
	/**
	 * Resets options and datasets of this chart.
	 * 
	 * @alias reset
	 */
	reset: function() {
		this.setOptions();
		this.dataStores = new Array();
		if(!Plotr.Base.isNil(this.renderDelay)){
			this.renderDelay.stop();
			this.renderDelay = null;
		}
	},
	
	/**
	 * The constructor of Plotr.Canvas. 
	 * 
	 * @alias initialize
	 * @see setOptions
	 * @param {String} element  - Canvas element ID.
	 * @param {Plotr.Chart} chart - Chart object to render.
	 * @param {Object} options - Options.
	 */
	_initCanvas: function(element) {
		this.canvasNode = $(element);
		this.containerNode = this.canvasNode.parentNode;
		Element.setStyle(this.containerNode,{position: 'relative',width: this.canvasNode.width + 'px'});	
		this.isIE = Plotr.Base.excanvasSupported();
		
	    if (this.isIE && !Plotr.Base.isNil(G_vmlCanvasManager)) {
			this.IEDelay = 0.5;
	        this.maxTries = 10;
	        this.renderDelay = null;
			this.renderStack = new Array();
	        this.canvasNode = G_vmlCanvasManager.initElement(this.canvasNode);			
	    }
		
		if(Plotr.Base.isNil(this.canvasNode))
			throw 'Plotr.Canvas(): Could\'nt find canvas.';	
		if(Plotr.Base.isNil(this.containerNode) || this.containerNode.nodeName.toLowerCase() != 'div')
			throw 'Plotr.Canvas(): Canvas element is not enclosed by a <div> element.';	
		if (!this.isIE && !(Plotr.Base.isSupported(element)))
        	throw "Plotr.Canvas(): Canvas is not supported.";
		
		this.xlabels = new Array();
    	this.ylabels = new Array();
		
		this.area = {
 	        x: this.options.padding.left,
 	        y: this.options.padding.top,
 	        w: this.canvasNode.width - this.options.padding.left - this.options.padding.right,
 	        h: this.canvasNode.height - this.options.padding.top - this.options.padding.bottom
 	    };
	},
	
	/**
	 * This function renders the background in the canvas element.
	 * 
	 * @alias render
	 * @param {String} [element] - (optional) ID of the canvas element to render in.
	 */
	_render: function(element) {
		if(!Plotr.Base.isNil(element)) this._initCanvas(element);
	
		if (this.options.drawBackground) {
			this._renderBackground();
		}
	},
	
	_ieWaitForVML: function(element, options){
		var isNil = Plotr.Base.isNil;
		
		if(!isNil(element)) {
			this.renderStack[element] = options;
		}
		
		try{
			if(!isNil(this.canvasNode)) {	
				this.canvasNode.getContext("2d");					
			} else {
				$(element).getContext("2d");
			}
		} catch(e) {
			if(isNil(this.renderDelay)){
				this.renderDelay = new PeriodicalExecuter(function(pe){
					if(!isNil(this.canvasNode)) {	
						this.render(this.canvasNode,options);					
					} else {
						this.render(element,options);
					}					
				}.bind(this), this.IEDelay);
			}else if(this.maxTries-- <= 0) {
				this.renderDelay.stop();
			}
			return true;
		}
		if(!isNil(this.renderDelay)){
			this.renderDelay.stop();
			delete this.renderStack[element || this.canvasNode];
		}
		
		return false;
	},
	
	/**
	 * Sets the colorScheme used for the chart.
	 * 
	 * @alias setColorScheme 
	 */
	setColorscheme: function() {
		var scheme = this.options.colorScheme;
		
		if(scheme instanceof Array) { 
			return;
		} else if(typeof(scheme) == 'string') {
			if(this.type == 'pie'){
				this.options.colorScheme = Plotr.Base.getColorscheme(scheme, Math.max(this.stores[0].length,3));
			}else{
				this.options.colorScheme = Plotr.Base.getColorscheme(scheme, Math.max(this.sets,3));
			}
		} else { 
			throw 'Plotr.Canvas.setColorscheme(): colorScheme is invalid!';
		}
	},
	
	/**
	 * Renders the background of the chart.
	 * 
	 * @alias _renderBackground
	 */
	_renderBackground: function() {
		var cx = this.canvasNode.getContext('2d');
		cx.save();
	    cx.fillStyle = this.options.backgroundColor;
			
		if(this.options.sweetRender) {
	        cx.fillRect(this.area.x, this.area.y, this.area.w, this.area.h);
	        cx.strokeStyle = this.options.backgroundLineColor;
	        cx.lineWidth = 1.0;
	        
	        var ticks = this.yticks;
	        var horiz = false;
	        if (this.type == 'bar' && this.options.barOrientation == 'horizontal') {
				ticks = this.xticks;
	            horiz = true;
	        }
	        
			var drawBackgroundLines = function(tick) {
				var x1 = x2 = y1 = y2 = 0;
				
				if(horiz) {
					x1 = x2 = tick[0] * this.area.w + this.area.x;
	                y1 = this.area.y;
	                y2 = y1 + this.area.h;
				}else{
					x1 = this.area.x;
	                y1 = tick[0] * this.area.h + this.area.y;
	                x2 = x1 + this.area.w;
	                y2 = y1;
				}
				
				cx.beginPath();
	            cx.moveTo(x1, y1);
	            cx.lineTo(x2, y2);
	            cx.closePath();
            	cx.stroke();
			}.bind(this);			
			ticks.each(drawBackgroundLines);
		} else {
			cx.fillRect(0, 0, this.canvasNode.width, this.canvasNode.height);
		}
		cx.restore();
	},
	
	/**
	 * Renders the axis for line charts.
	 * 
	 * @alias _renderLineAxis
	 */
	_renderLineAxis: function() {
		this._renderAxis();
	},
	
	/**
	 * Renders axis.
	 * 
	 * @alias _renderAxis 
	 */
	_renderAxis: function() {
	    if (!this.options.drawXAxis && !this.options.drawYAxis)
	        return;
	
	    var cx = this.canvasNode.getContext('2d');
	
	    var labelStyle = {
			position: 'absolute',
	        fontSize: this.options.axisLabelFontSize + 'px',			
			fontFamily: this.options.axisLabelFont,
	        zIndex: 10,
	        color: this.options.axisLabelColor,
	        width: this.options.axisLabelWidth + 'px',
	        overflow: 'hidden'
		};
		
	    cx.save();
	    cx.strokeStyle = this.options.axisLineColor;
	    cx.lineWidth = this.options.axisLineWidth;
		
	    if (this.options.drawYAxis) {
	        if (this.yticks) {
				var collectYLabels = function(tick) {
					if(typeof(tick) == 'function') return;
					
	                var x = this.area.x;
	                var y = this.area.y + tick[0] * this.area.h;
					
	                cx.beginPath();
	                cx.moveTo(x, y);
	                cx.lineTo(x - this.options.axisTickSize, y);
	                cx.closePath();
	                cx.stroke();
					
					var label = document.createElement('div');
					label.appendChild(document.createTextNode(tick[1]));	
					Element.setStyle(label, Object.extend(labelStyle,{
						top: (y - this.options.axisLabelFontSize) + 'px',
						left: (x - this.options.padding.left - this.options.axisTickSize) + 'px',
						width: (this.options.padding.left - this.options.axisTickSize * 2) + 'px',
						textAlign: 'right'
					}));				
	                this.containerNode.appendChild(label);
	                return label;
				}.bind(this);
				this.ylabels = this.yticks.collect(collectYLabels);
	        }
	
	        cx.beginPath();
	        cx.moveTo(this.area.x, this.area.y);
	        cx.lineTo(this.area.x, this.area.y + this.area.h);
	        cx.closePath();
	        cx.stroke();
	    }
		
		if(this.options.drawXAxis) {
	        if(this.xticks) {
				var collectXLabels = function(tick) {
					if (typeof(tick) == 'function') return;
					
	                var x = this.area.x + tick[0] * this.area.w;
                	var y = this.area.y + this.area.h;
					
	                cx.beginPath();
	                cx.moveTo(x, y);
	                cx.lineTo(x, y + this.options.axisTickSize);
	                cx.closePath();
	                cx.stroke();
					
	                var label = document.createElement('div');
	                label.appendChild(document.createTextNode(tick[1]));
					
	                Element.setStyle(label, Object.extend(labelStyle,{
						top: (y + this.options.axisTickSize) + 'px',
						left: (x - this.options.axisLabelWidth/2) + 'px',
						width: this.options.axisLabelWidth + 'px',
						textAlign: 'center'
					}));
					
	                this.containerNode.appendChild(label);
	                return label;
				}.bind(this);
				this.xlabels = this.xticks.collect(collectXLabels);
	        }
	
	        cx.beginPath();
	        cx.moveTo(this.area.x, this.area.y + this.area.h);
	        cx.lineTo(this.area.x + this.area.w, this.area.y + this.area.h);
	        cx.closePath();
	        cx.stroke();
	    }		
		cx.restore();
	}
};


Plotr.BarChart = Class.create();
Object.extend(Plotr.BarChart.prototype, Plotr.Canvas);
Object.extend(Plotr.BarChart.prototype, Plotr.Chart);
Object.extend(Plotr.BarChart.prototype,{
	/**
	 * Type of chart we're dealing with.
	 */
	type: 'bar',
	
	/**
	 * Renders the chart with the specified options. The optional parameters
	 * can be used to render a barchart in a different canvas element with new options.
	 * 
	 * @alias render
	 * @param {String} [element] - (optional) ID of a canvas element.
	 * @param {Object} [options] - (optional) Options for rendering.
	 */
	render: function(element, options) {
		var isNil = Plotr.Base.isNil;
		
		if(this.isIE && this._ieWaitForVML(element,options)){
			return;
		}

		this._evaluate(options);
		this._render(element);
		this._renderBarChart();				
		this._renderBarAxis();
		
		if(this.isIE) {
			for(var el in this.renderStack){
				if(typeof(this.renderStack[el]) == 'function') break;
				this.render(el,this.renderStack[el]);
				break;
			}
		}
	},
	
	/**
	 * This function does all the math. This function evaluates all the data needed
	 * to plot the chart.
	 * 
	 * @alias _evaluate
	 * @param {Object} [options] - (optional) Evaluate the chart with the given options.
	 */
	_evaluate: function(options) {
		this._eval(options);
		if(this.options.barOrientation == 'vertical') this._evalBarChart();
		else this._evalHorizBarChart();
		this._evalBarTicks();
	},
	
	/**
	 * Evaluates measures for vertical bars.
	 * 
	 * @alias _evalBarChart
	 */
	_evalBarChart: function() {		
		var uniqx = Plotr.Base.uniqueIndices(this.stores);		
		var xdelta = 10000000;
	    for (var i = 1; i < uniqx.length; i++) {
	        xdelta = Math.min(Math.abs(uniqx[i] - uniqx[i-1]), xdelta);
	    }
		
		var barWidth = 0;
	    var barWidthForSet = 0;
	    var barMargin = 0;
	    if (uniqx.length == 1) {
	        xdelta = 1.0;
	        this.xscale = 1.0;
	        this.minxval = uniqx[0];
	        barWidth = 1.0 * this.options.barWidthFillFraction;
	        barWidthForSet = barWidth / this.stores.length;
	        barMargin = (1.0 - this.options.barWidthFillFraction)/2;
	    } else {
			this.xscale = (this.xrange == 1) ? 0.5 : (this.xrange == 2) ? 1/3.0 : (1.0 - 1/this.xrange)/this.xrange;
	        barWidth = xdelta * this.xscale * this.options.barWidthFillFraction;
	        barWidthForSet = barWidth / this.stores.length;
	        barMargin = xdelta * this.xscale * (1.0 - this.options.barWidthFillFraction)/2;
	    }
		
		this.minxdelta = xdelta;
		this.bars = new Array();
		
	    var i = 0;
	    for (var name in this.dataStores) {
	        var store = this.dataStores[name];
			if(typeof(store) == 'function') continue;
	        for (var j = 0; j < store.length; j++) {
	            var item = store[j];
	            var rect = {
	                x: ((parseFloat(item[0]) - this.minxval) * this.xscale) + (i * barWidthForSet) + barMargin,
	                y: 1.0 - ((parseFloat(item[1]) - this.minyval) * this.yscale),
	                w: barWidthForSet,
	                h: ((parseFloat(item[1]) - this.minyval) * this.yscale),
	                xval: parseFloat(item[0]),
	                yval: parseFloat(item[1]),
	                name: name
	            };
	            if ((rect.x >= 0.0) && (rect.x <= 1.0) && 
	                (rect.y >= 0.0) && (rect.y <= 1.0)) {
	                this.bars.push(rect);
	            }
	        }
			i++;
	    }
	},
	
	/**
	 * Evaluates measures for vertical bars.
	 * 
	 * @alias _evalHorizBarChart
	 */
	_evalHorizBarChart: function() {
		var uniqx = Plotr.Base.uniqueIndices(this.stores);		
		var xdelta = 10000000;
	    for (var i = 1; i < uniqx.length; i++) {
	        xdelta = Math.min(Math.abs(uniqx[i] - uniqx[i-1]), xdelta);
	    }
		
		var barWidth = 0;
	    var barWidthForSet = 0;
	    var barMargin = 0;
	    if (uniqx.length == 1) {
	        xdelta = 1.0;
	        this.xscale = 1.0;
	        this.minxval = uniqx[0];
	        barWidth = 1.0 * this.options.barWidthFillFraction;
	        barWidthForSet = barWidth / this.stores.length;
	        barMargin = (1.0 - this.options.barWidthFillFraction)/2;
	    } else {
       	 	this.xscale = (1.0 - xdelta/this.xrange)/this.xrange;
        	barWidth = xdelta * this.xscale * this.options.barWidthFillFraction;
        	barWidthForSet = barWidth / this.stores.length;
        	barMargin = xdelta * this.xscale * (1.0 - this.options.barWidthFillFraction)/2;			
		}
		
		this.minxdelta = xdelta;
	    this.bars = new Array();
	    var i = 0;
	    for (var name in this.dataStores) {
	        var store = this.dataStores[name];
	        if (typeof(store) == 'function') continue;
	        for (var j = 0; j < store.length; j++) {
	            var item = store[j];
	            var rect = {
	                y: ((parseFloat(item[0]) - this.minxval) * this.xscale) + (i * barWidthForSet) + barMargin,
	                x: 0.0,
	                h: barWidthForSet,
	                w: ((parseFloat(item[1]) - this.minyval) * this.yscale),
	                xval: parseFloat(item[0]),
	                yval: parseFloat(item[1]),
	                name: name
	            };
				
				rect.y = (rect.y <= 0.0) ? 0.0 : (rect.y >= 1.0) ? 1.0 : rect.y;	            
	            if ((rect.x >= 0.0) && (rect.x <= 1.0)) {
	                this.bars.push(rect);
	            }
	        }
	        i++;
	    }		
	},
	
	/**
	 * Evaluates bar ticks.
	 * 
	 * @alias _evalBarTicks
	 */
	_evalBarTicks: function() {
		this._evalLineTicks();
		this.xticks = this.xticks.collect(function(tick) {
			return [tick[0] + (this.minxdelta * this.xscale)/2, tick[1]];
		}.bind(this));
		
		if (this.options.barOrientation == 'horizontal') {
			var tmp = this.xticks;			
			this.xticks = this.yticks.collect(function(tick) {
				return [1.0 - tick[0], tick[1]];
			}.bind(this));
			this.yticks = tmp;
	    }
	},
	
	/**
	 * Renders a horizontal/vertical bar chart.
	 * 
	 * @alias _renderBarChart
	 */
	_renderBarChart: function() {
		var cx = this.canvasNode.getContext('2d');
		var index = 0;		
		
		for(var storeName in this.dataStores) {
			var drawBar = function(bar) {
				if(bar.name != storeName || typeof(bar) == 'function') return;
				cx.save();
				cx.lineWidth = this.options.strokeWidth;
				cx.fillStyle = this.options.colorScheme[index % this.options.colorScheme.length];
				cx.strokeStyle = this.options.strokeColor;
				var x = this.area.w * bar.x + this.area.x;
	 	    	var y = this.area.h * bar.y + this.area.y;
	 	        	var w = this.area.w * bar.w;
	 	        	var h = this.area.h * bar.h;
			      
	 	       		if ((w < 1) || (h < 1)) return;
	 	        	if (this.options.shouldFill) cx.fillRect(x, y, w, h);
				if (this.options.shouldStroke) cx.strokeRect(x, y, w, h);				
				cx.restore();
			}.bind(this);
			this.bars.each(drawBar);
			index++;
		}
	},
	
	/**
	 * Renders the axis for bar charts.
	 * 
	 * @alias _renderBarAxis
	 */
	_renderBarAxis: function() {
		this._renderAxis();
	}
});

Plotr.LineChart = Class.create();
Object.extend(Plotr.LineChart.prototype, Plotr.Canvas);
Object.extend(Plotr.LineChart.prototype, Plotr.Chart);
Object.extend(Plotr.LineChart.prototype,{
	/**
     * Type of chart we're dealing with.
 	 */
	type: 'line',
	
	/**
	 * Renders the chart with the specified options. The optional parameters
	 * can be used to render a linechart in a different canvas element with new options.
	 * 
	 * @alias render
	 * @param {String} [element] - (optional) ID of a canvas element.
	 * @param {Object} [options] - (optional) Options for rendering.
	 */
	render: function(element,options) {		
		if(this.isIE && this._ieWaitForVML(element,options)){
			return;
		}
		
		this._evaluate(options);
		this._render(element);
		this._renderLineChart();
		this._renderLineAxis();
		
		if(this.isIE) {
			for(var el in this.renderStack){
				if(typeof(this.renderStack[el]) == 'function') break;
				this.render(el,this.renderStack[el]);
				break;
			}
		}
	},
	
	/**
	 * This function does all the math. It'll process all the data needed to
	 * plot the chart.
	 * 
	 * @alias evaluate
	 * @param {Object} options - (optional) evaluate the chart with the given options.
	 */
	_evaluate: function(options) {
		this._eval(options);
		this._evalLineChart();
		this._evalLineTicksForXAxis();
		this._evalLineTicksForYAxis();
	},
	
	/**
	 * Processes specific measures for line charts.
	 * 
	 * @alias _evalLineChart
	 */
	_evalLineChart: function() {
	    this.points = new Array();
			
	    for (var name in this.dataStores) {
	        var store = this.dataStores[name];
	        if (typeof(store) == 'function') continue;
	        for (var j = 0; j < store.length; j++) {
	            var item = store[j];
	            var point = {
	                x: ((parseFloat(item[0]) - this.minxval) * this.xscale),
	                y: 1.0 - ((parseFloat(item[1]) - this.minyval) * this.yscale),
	                xval: parseFloat(item[0]),
	                yval: parseFloat(item[1]),
	                name: name
	            };
				point.y = (point.y <= 0.0) ? 0.0 : (point.y >= 1.0) ? 1.0 : point.y;
	            
	            if ((point.x >= 0.0) && (point.x <= 1.0)) {
	                this.points.push(point);
	            }
	        }
	    }
	},
	
	_renderLineChart: function() {
	    var cx = this.canvasNode.getContext("2d");
		var index = 0;
		
		for(var storeName in this.dataStores) {
			cx.save();
			cx.lineWidth = this.options.strokeWidth;
			cx.fillStyle = this.options.colorScheme[index % this.options.colorScheme.length];
		    cx.strokeStyle = this.options.strokeColor;
			
			var preparePath = function() {
				cx.beginPath();
	            cx.moveTo(this.area.x, this.area.y + this.area.h);
	            var addPoint = function(point) {
	                
	            }.bind(this);
				for(var point in this.points) {
					var currPoint = this.points[point];
					if (currPoint.name == storeName)
	                    cx.lineTo(this.area.w * currPoint.x + this.area.x, this.area.h * currPoint.y + this.area.y);
				}
	            cx.lineTo(this.area.w + this.area.x, this.area.h + this.area.y);
	            cx.lineTo(this.area.x, this.area.y + this.area.h);
	            cx.closePath();
			}.bind(this);
			
			if (this.options.shouldFill) {
	            preparePath(cx);
		        cx.fill();
		    }
	        if (this.options.shouldStroke) {
	            preparePath(cx);
	            cx.stroke();
	        }
	
	        cx.restore();	
				
			index++;
		}
	}
});

Plotr.PieChart = Class.create();
Object.extend(Plotr.PieChart.prototype, Plotr.Canvas);
Object.extend(Plotr.PieChart.prototype, Plotr.Chart);
Object.extend(Plotr.PieChart.prototype,{
	/**
     * Type of chart we're dealing with.
 	 */
	type: 'pie',
	
	/**
	 * Renders the chart with the specified options.
	 * 
	 * @alias render
	 * @param {String} [element] - (optional) ID of a canvas element.
	 * @param {Object} [options] - (optional) Options for rendering.
	 */
	render: function(element,options) {
		var isNil = Plotr.Base.isNil;
		
		if(this.isIE && this._ieWaitForVML(element,options)){
			return;
		}

		this._evaluate(options);
		this._render(element);
		this._renderPieChart();
		this._renderPieAxis();
		
		if(this.isIE) {
			for(var el in this.renderStack){
				if(typeof(this.renderStack[el]) == 'function') break;
				this.render(el,this.renderStack[el]);
				break;
			}
		}
	},
	
	
	/**
	 * This function does all the math. This function evaluates all the data needed
	 * to plot the chart.
	 * 
	 * @alias _evaluate
	 * @param {Object} [options] - (optional) Evaluate the chart with the given options.
	 */
	_evaluate: function(options) {
		this._eval(options);
		this._evalPieChart();
		this._evalPieTicks();
	},
	
	_evalPieChart: function(){
		var store = this.stores[0];
		var sum = Plotr.Base.sum(store.pluck(1));
		
		var angle = 0.0;
		this.slices = new Array();
		for(var i = 0, slice = null, fraction = null; i < store.length; i++){
			slice = store[i];
			if(slice[1] > 0){
				fraction = slice[1]/sum;
				this.slices.push({
					fraction: fraction,
					xval: slice[0],
					yval: slice[1],
					startAngle: 2 * angle * Math.PI,
					endAngle: 2 * (angle + fraction) * Math.PI
				});
				angle += fraction;
			}
		}
	},
	
	_renderPieChart: function(){
		var cx = this.canvasNode.getContext('2d');
				
		var centerx = this.area.x + this.area.w * 0.5;
    	var centery = this.area.y + this.area.h * 0.5;
		var radius = Math.min(this.area.w * this.options.pieRadius, this.area.h * this.options.pieRadius);
		
		if(this.isIE) {
	        centerx = parseInt(centerx);
	        centery = parseInt(centery);
	        radius = parseInt(radius);
	    }
		
		for(var i = 0, ln = this.slices.length; i < ln; i++) {			
			cx.save();
			cx.fillStyle = this.options.colorScheme[i % this.options.colorScheme.length];
			
			var drawPie = function(){
				cx.beginPath();
				cx.moveTo(centerx, centery);
				cx.arc(centerx, centery, radius, 
                        this.slices[i].startAngle - Math.PI/2,
                        this.slices[i].endAngle - Math.PI/2,
                        false);
				cx.lineTo(centerx, centery);
           		cx.closePath();
			}.bind(this);
			
			if(Math.abs(this.slices[i].startAngle - this.slices[i].endAngle) > 0.001) {
				
				if(this.options.shouldFill) {
                	drawPie();
	                cx.fill();
	            }
	            
	            if (this.options.shouldStroke) {
	                drawPie();
	                cx.lineWidth = this.options.strokeWidth;
	                if (this.options.strokeColor)
	                    cx.strokeStyle = this.options.strokeColor;	       
	                cx.stroke();
	            }
			}
			cx.restore();				
		}
	},
	
	_evalPieTicks: function() {
		this.xticks = new Array();
		
		var toPercentage = function(n){
			n *= 100;
			return n.toFixed(1)+'%';
		};
		
		if(this.options.xTicks) {
			var lookup = new Array();
			for (var i = 0; i < this.slices.length; i++) {
				lookup[this.slices[i].xval] = this.slices[i];
			}
			
			for(var i = 0; i < this.options.xTicks.length; i++) {
				var tick = this.options.xTicks[i];
				var slice = lookup[tick.v]; 
	            var label = tick.label;
				if(slice) {
	                if (Plotr.Base.isNil(label))
	                    label = tick.v.toString();
					label += " (" + toPercentage(slice.fraction) + ")";
					this.xticks.push([tick.v, label]);
				}
			}
		}else{
			for(var i =0; i < this.slices.length; i++) {
				var slice = this.slices[i];
				var label = slice.xval + " (" + toPercentage(slice.fraction) + ")";
				this.xticks.push([slice.xval, label]);
			}
		}
	},
	
	_renderPieAxis: function() {
		if (!this.options.drawXAxis)
        	return;
		
		if(this.xticks){
			var lookup = new Array();
			for (var i = 0; i < this.slices.length; i++) {
				lookup[this.slices[i].xval] = this.slices[i];
			}
			var centerx = this.area.x + this.area.w * 0.5;
		    var centery = this.area.y + this.area.h * 0.5;
		    var radius = Math.min(this.area.w * this.options.pieRadius,
		                          this.area.h * this.options.pieRadius);
			var labelWidth = this.options.axisLabelWidth;
			
			for(var i = 0; i < this.xticks.length; i++) {
				var slice = lookup[this.xticks[i][0]];
				if(Plotr.Base.isNil(slice))
					continue;
					
				var angle = (slice.startAngle + slice.endAngle)/2;
				// normalize the angle
				var normalisedAngle = angle;
				if (normalisedAngle > Math.PI * 2)
					normalisedAngle = normalisedAngle - Math.PI * 2;
				else if (normalisedAngle < 0)
					normalisedAngle = normalisedAngle + Math.PI * 2;
					
				var labelx = centerx + Math.sin(normalisedAngle) * (radius + 10);
		        var labely = centery - Math.cos(normalisedAngle) * (radius + 10);
				
				var labelStyle = {
			        position: 'absolute',
			        zIndex: 11,
			        width: labelWidth + 'px',
			        fontFamily: this.options.axisLabelFont,
			        fontSize: this.options.axisLabelFontSize + 'px',
			        overflow: 'hidden',
			        color: this.options.axisLabelColor
			    };
				
				if(normalisedAngle <= Math.PI * 0.5) {
		            // text on top and align left
					Object.extend(labelStyle, {
						textAlign: 'left',
						verticalAlign: 'top',
						left: labelx + 'px',
						top: (labely - this.options.axisLabelFontSize) + 'px'
					});
		        }else if ((normalisedAngle > Math.PI * 0.5) && (normalisedAngle <= Math.PI)) {
		            // text on bottom and align left
					Object.extend(labelStyle, {
						textAlign: 'left',
						verticalAlign: 'bottom',
						left: labelx + 'px',
						top: labely + 'px'
					});	
		        }else if ((normalisedAngle > Math.PI) && (normalisedAngle <= Math.PI*1.5)) {
		            // text on bottom and align right
					Object.extend(labelStyle, {
						textAlign: 'right',
						verticalAlign: 'bottom',
						left: (labelx  - labelWidth) + 'px',
						top: labely + 'px'
					});
		        }else {
		            // text on top and align right
					Object.extend(labelStyle, {
						textAlign: 'right',
						verticalAlign: 'bottom',
						left: (labelx  - labelWidth) + 'px',
						top: (labely - this.options.axisLabelFontSize) + 'px'
					});
		        }
				var label = document.createElement('div');
				label.appendChild(document.createTextNode(this.xticks[i][1]));
				Element.setStyle(label, labelStyle);	
                this.containerNode.appendChild(label);
				this.xlabels.push(label);
		  }
		
		}
	}
});