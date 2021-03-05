import path from 'path';

export default {
  mode: 'development',
  entry: [
    './views/graph.js',
    './views/graph.scss'
  ],
  output: {
    path: path.resolve('views', 'dist'), 
    filename: 'graph.min.js',
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

