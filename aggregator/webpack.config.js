import path from 'path';

export default {
  mode: 'development',
  entry: {
    graph: [ './views/graph.js', './views/graph.scss' ],
    regression: [ './views/regression.js', './views/regression.scss' ]
  },
  output: {
    path: path.resolve('views', 'dist'), 
    filename: '[name].min.js',
    clean: true,
  },
  module: {
    rules: [
      {
        test: /\.scss$/,
        exclude: /node_modules/,
        use: [
            {
                loader: 'file-loader',
                options: { outputPath: 'css/', name: '[name].min.css'}
            },
            'sass-loader'
        ]
      }
    ]
  }
};

