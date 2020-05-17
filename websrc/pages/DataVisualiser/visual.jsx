import React, { Fragment, useState, useEffect } from 'react';
import ReactDOM from 'react-dom';
import { padStart } from "lodash";

import { xterm } from './8bppXterm';

var _1bpp_colour_map = {
	0:"rgb("+255+","+255+","+255+")", //white
	1:"rgb("+0+","+0+","+0+")" // black
};

var _2bpp_colour_map = {
	0:"rgb("+255+","+255+","+255+")", //white
	1:"rgb("+192+","+192+","+192+")", //light grey
	2:"rgb("+96+","+96+","+96+")", // dark grey
	3:"rgb("+0+","+0+","+0+")" // black
};

// for saturn its 32 tiles per block
let currentPixels = [];
let lines = [];
let allLines = [];
let tiles = [];
let allTiles = [];
let blocks = [];
const pixelsPerLine = 8;
const linesPerTile = 8;
const tilesPerBlock = 32;

export function getLines() {
  return allLines;
}

export function getTiles() {
  return allTiles;
}

let mode = "2d";

function add_pixel(color, pixelsPerLine = 8, width=5, height=5) {
  let style = {
    'background-color': color,
    width: width,
    height: height,
  };
  
  if (currentPixels.length >= pixelsPerLine) {
    lines.push(<div className="line">{currentPixels}</div>);
    currentPixels = [];
  }
  if (lines.length >= linesPerTile) {
    allLines = [ ...allLines, lines];
    const className = mode==="1d"?"tile":"tile2d";
    tiles.push(<div className={className}>{lines}</div>);
    lines = [];
  }
  if (tiles.length >= tilesPerBlock) {
    allTiles = [ ...allTiles, tiles];
    blocks.push(<div className={"block"}>{tiles}</div>);
    tiles = [];
  }
  currentPixels.push(<div className="pixel" style={style}></div>);

}

function show_1bpp(buffer, pixelsPerLine = 64) {
  
  for (let i = 0; i < buffer.length; i += 3) {

    // Now loop through each bit in the byte
    for(var x = 8; x >0; x--)
	  {
      // Find bit index for this pixel
		  // i.e get a mask which can be applied to a byte, that will return the value of this pixel
      var pixelMask = 1 << (7-x);
      var bitValueOfByte = buffer[i] & pixelMask;
      add_pixel(_1bpp_colour_map[bitValueOfByte], pixelsPerLine);

    }
    
  }
}

function twoBytesToSingleStringOfBits(byte1, byte2) {
  const byte1AsBinary = padStart((byte1).toString(2), 8, "0");
  const byte2AsBinary = padStart((byte2).toString(2), 8, "0");
  const additionOfBits = byte1AsBinary+byte2AsBinary;
  return additionOfBits;
}

function calculateRGBFrom2Bytes(byte1, byte2, ignoreBit="msb", colourOrder="bgr", endian="little") {
  if (!byte2 || ! byte1) {
    return {r:0, g:0, b:0};
  }
    let stringOfBits = twoBytesToSingleStringOfBits(byte2, byte1);
    let addition = 0;
    if (ignoreBit === "msb") {
      addition = 1;
    }

    if (endian === "big") {
      stringOfBits = twoBytesToSingleStringOfBits(byte1, byte2);
    }
    const red5bits = stringOfBits.substr(addition,5);
    const green5bits = stringOfBits.substr(5+addition,5);
    const blue5bits = stringOfBits.substr(10+addition,5);

    // multiply by 8 just to  make sure its a wider range
    let r = parseInt(red5bits, 2) << 3;
    let g = (parseInt(green5bits, 2)) << 3;
    let b = (parseInt(blue5bits, 2)) << 3;
    // const r = (parseInt(red5bits, 2)/31) * 255;
    // const g = (parseInt(green5bits, 2)/31) * 255;
    // const b = (parseInt(blue5bits, 2)/31) * 255;

    // const colour16 = parseInt(stringOfBits,2);
    // let r,g,b;
    // r = (colour16 >> 8) & (255-7);
    // g = (colour16 >> 3) & (255-3);
    // b = (colour16 << 3) & 255;

    if (colourOrder === "bgr") {
      return {r:b, g: g, b: r};
    }

    return {r,g,b};
}


function show_16bpp(buffer, pixelsPerLine = 8, ignoreBit="msb", colourOrder="bgr") {

  for (let i = 0; i < buffer.length+1; i += 2) {
    const { r,g,b } = calculateRGBFrom2Bytes(buffer[i], buffer[i+1], ignoreBit, colourOrder);
    add_pixel('rgb(' + r + ',' + g + ',' + b + ')', pixelsPerLine);
  }
}

function show_24bpp(buffer, order="rgb", pixelsPerLine = 8) {
  let r,g,b;
  
  for (let i = 0; i < buffer.length; i += 3) {
    if (order === "bgr") {
      b = buffer[i + 2];
      g = buffer[i + 1];
      r = buffer[i];
    } else {
      // rgb
      r = buffer[i +2];
      g = buffer[i + 1];
      b = buffer[i];
    }
    add_pixel('rgb(' + r + ',' + g + ',' + b + ')', pixelsPerLine);
  }
}


function show_digraph(buffer, pixelsPerLine = 8, visibleDigraph=0) {
  // idea from: https://community.wolfram.com/groups/-/m/t/887456
  const digraphs = {};
  for (let i = 0; i < buffer.length+2; i += 2) {
    if (typeof(buffer[i+1])==='undefined') {
      break;
    }
    const digraph = parseInt(twoBytesToSingleStringOfBits(buffer[i], buffer[i+1]), 2);
    if (digraphs[digraph]) {
      digraphs[digraph]+=1;
    } else {
      digraphs[digraph]=1;
    }
    
    let color = 'black';
    if (digraph === visibleDigraph) {
      color = 'green';
    }
    
    add_pixel(color, pixelsPerLine);
  }
  // TODO: sort digraphs by frequency
  return digraphs;
}

function show_8bppXterm(buffer, pixelsPerLine = 8) {

  for (let i = 0; i < buffer.length; i += 1) {
    const character = buffer[i];
    let color = xterm[character];
    
    add_pixel(color, pixelsPerLine);
  }
}
function show_printable(buffer, pixelsPerLine = 32) {

  for (let i = 0; i < buffer.length; i += 1) {
    const character = buffer[i];
    let color = "black";
    if (character === 255) {
      color= "white";
    }
    else if (character >= 32 && character <=126 ) {
      color="blue";
    } else if (character >0 && character < 32) {
      color="green";
    } else if (character > 126) {
      color="red";
    }
    add_pixel(color, pixelsPerLine);
  }
}

export function visualiseData(data, visualType, is2D=true, showAs="tiles") {
  // reset variables
  currentPixels = [];
  lines = [];
  allLines = [];
  allTiles = [];
  tiles = [];
  blocks = [];
  mode = is2D?"2d":"1d";

  if (visualType === "highlight_printable") {
    show_printable(data);
  } 
  else if (visualType === 'digraph') {
    show_digraph(data, pixelsPerLine, 0);
  }
  else if (visualType === '1bpp') {
    show_1bpp(data);
  }
  else if (visualType === '8bpp') {
    show_8bppXterm(data);
  }
  else if (visualType === '16bpp') {
    show_16bpp(data);
  }
  else if (visualType === '16bpp_msb_rgb') {
    show_16bpp(data, pixelsPerLine, "msb", "rgb");
  }
  else if (visualType === '16bpp_lsb_rgb') {
    show_16bpp(data, pixelsPerLine, "lsb", "rgb");
  }
  else if (visualType === '16bpplsb') {
    show_16bpp(data, pixelsPerLine, "lsb");
  }
  else if (visualType === '24bpp_bgr'){
    show_24bpp(data, 'bgr');
  } else {
    show_24bpp(data);
  }

  if (showAs==="tiles") {
    console.error("Tiles", tiles);
    return getTiles();
  } else if (showAs==="blocks") {
    console.error("Blocks", blocks);
    return blocks;
  }
  return getLines();
}
