var dataStream = new TimeSeries();

function createPlot() {
	var chart = new SmoothieChart({
		// scaleSmoothing:1,
		// maxValue:20,
		// minValue:-5
	});
	chart.addTimeSeries(dataStream, { strokeStyle: 'rgba(0, 255, 0, 1)', fillStyle: 'rgba(0, 255, 0, 0.2)', lineWidth: 4 });
	chart.streamTo(document.getElementById("chart"), 500);
}