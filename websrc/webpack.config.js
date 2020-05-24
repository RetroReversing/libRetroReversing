const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');

module.exports = {
  mode: 'development',
  entry: {
    app: './main.js',
  },
  devtool: 'inline-source-map',
  devServer: {
    contentBase: path.join(__dirname, './'), // where dev server will look for static files, not compiled
    publicPath: '/', //relative path to output path where  devserver will look for compiled files
  },
  output: {
    filename: 'libRRUI.bundle.js',
    path: path.resolve(__dirname, './dist/'), // base path where to send compiled assets
    publicPath: '/', // base path where referenced files will be look for
  },
  resolve: {
    extensions: ['*', '.js', '.jsx'],
    alias: {
      '@': path.resolve(__dirname, 'src'), // shortcut to reference src folder from anywhere
    },
  },
  module: {
    rules: [
      {
        // config for es6 jsx
        test: /\.(js|jsx)$/,
        exclude: /node_modules/,
        use: {
          loader: 'babel-loader',
        },
      },
      {
        test: /\.css$/i,
        use: ['style-loader', 'css-loader'],
      },
      {
        // config for images
        test: /\.(png|svg|jpg|jpeg|gif)$/,
        use: [
          {
            loader: 'file-loader',
            options: {
              outputPath: 'images',
            },
          },
        ],
      },
      {
        // config for fonts
        test: /\.(woff|woff2|eot|ttf|otf)$/,
        use: [
          {
            loader: 'file-loader',
            options: {
              outputPath: 'fonts',
            },
          },
        ],
      },
    ],
  },
  plugins: [
    // new MonacoWebpackPlugin(), // reenable this when you want to use parser
    new HtmlWebpackPlugin({
      template: './index.html',
      filename: 'index.html',
      title: 'Reversing Emulator',
    }),
  ],
};
