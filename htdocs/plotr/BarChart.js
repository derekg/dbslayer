/*
	Plotr.BarChart
	==============	
	Plotr.BarChart is part of the Plotr Charting Framework.
	
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
	if (typeof(Plotr.Canvas) == 'undefined') throw '';
	if (typeof(Plotr.Chart) == 'undefined') throw '';
} catch(e) {
	throw 'Plotr.BarChart depends on Plotr.{Base,Canvas,Chart}.';
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