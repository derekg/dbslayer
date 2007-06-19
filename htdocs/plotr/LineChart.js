/*
	Plotr.LineChart
	===============	
	Plotr.LineChart is part of the Plotr Charting Framework.
	
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
	throw 'Plotr.LineChart depends on Plotr.{Base,Canvas,Chart}.';
};

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