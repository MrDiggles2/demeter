
import Chartist from 'chartist';
import moment from 'moment';
import ChartistPluginLegend from 'chartist-plugin-legend';
new ChartistPluginLegend(); //without this line, you get 'Chartist.plugins undefined'

(async () => {
    const response = await fetch('http://localhost:3000/readings?count=100').then(res => res.json());

    const data = response.data.reduce((acc, point) => {
        const { name, ts, raw } = point;

        if (!acc[name]) {
            acc[name] = [];
        }

        acc[name].push({ x: new Date(ts), y: raw });

        return acc;
    }, {});

    const options = {
        axisX: {
            type: Chartist.FixedScaleAxis,
            divisor: 5,
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
            series: Object.keys(data).map(name => {
                return {
                    name,
                    data: data[name].reverse() // order matters, start with lower values
                };
            })
        },
        options
    );

})();
