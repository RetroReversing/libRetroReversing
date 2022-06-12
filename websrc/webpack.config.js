const path = require('path');
const webpack = require('webpack')

const HtmlWebpackPlugin = require('html-webpack-plugin');
const MonacoWebpackPlugin = require('monaco-editor-webpack-plugin');

module.exports = {
  mode: 'development',
  entry: {
    app: './main',
  },
  devtool: 'inline-source-map',
  devServer: {
    static: path.join(__dirname, './dist/'), // where dev server will look for static files, not compiled
    proxy: {
      '/postresponse': {
        target: 'http://127.0.0.1:1234/',
        changeOrigin: true,
      },
      '/game': {
        target: 'http://127.0.0.1:1234/',
        changeOrigin: true,
      },
    }
  },
  output: {
    filename: 'libRRUI.bundle.js',
    path: path.resolve(__dirname, './dist/'), // base path where to send compiled assets
    publicPath: '/', // base path where referenced files will be look for
  },
  resolve: {
    extensions: ['*', '.js', '.jsx', '.ts', '.tsx'],
    alias: {
      '@': path.resolve(__dirname, 'src'), // shortcut to reference src folder from anywhere
    },
    fallback: { "vm": require.resolve("vm-browserify") }
  },
  module: {
    rules: [
      {
        test: /\.m?js/,
        resolve: {
            fullySpecified: false
        }
    },
      {
        // config for es6 jsx
        test: /\.(js|jsx)$/,
        exclude: /node_modules/,
        use: {
          // loader: 'babel-loader',
          loader: 'ts-loader',
        },
      },
      {
        // config for es6 jsx
        test: /\.(ts|tsx)$/,
        exclude: /node_modules/,
        use: {
          loader: 'ts-loader',
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
    // fix "process is not defined" error:
    // (do "npm install process" before running the build)
    new webpack.ProvidePlugin({
      process: 'process/browser',
      Buffer: ['buffer', 'Buffer'],
    }),
  ],
};
