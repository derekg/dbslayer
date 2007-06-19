/*
	Plotr.Canvas
	============	
	Plotr.Canvas is part of the Plotr Charting Framework.
	
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
	if (typeof(Plotr.Base) == 'undefined') throw '';
} catch(e) {
	throw 'Plotr.Canvas depends on Plotr.Base.';
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