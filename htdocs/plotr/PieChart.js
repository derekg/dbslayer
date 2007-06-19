/*
	Plotr.PieChart
	==============	
	Plotr.PieChart is part of the Plotr Charting Framework.
	
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
	
	/**
	 * Processes specific measures for pie charts.
	 * 
	 * @alias _evalPieChart
	 */
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