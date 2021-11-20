
import Chartist from 'chartist';
import moment from 'moment';
import ChartistPluginLegend from 'chartist-plugin-legend';
new ChartistPluginLegend(); //without this line, you get 'Chartist.plugins undefined'

(async () => {
    const response = await fetch(`/regression${window.location.search}`).then(res => res.json());
    
    const data = response.data.reduce((acc, point) => {
        const { rate, tip, lastTs } = point;

        acc.push({ x: new Date(lastTs), y: rate });

        return acc;
    }, []);

    const options = {
        axisX: {
            type: Chartist.FixedScaleAxis,
            // divisor: 5,
            labelInterpolationFnc: function(value) {
                return moment(value).format('MMM D');
            }
        },
        lineSmooth: false,
        plugins: [
            Chartist.plugins.legend()
        ]
    };

    var chart = new Chartist.Line(
        '.ct-chart',
        {
            series: [{
                name: 'reg',
                data: data.reverse(),
            }]
        },
        options
    );

})();
