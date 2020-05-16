import React, { Fragment, useState, useEffect } from 'react';
import ReactDOM from 'react-dom';
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

let currentPixels = [];
let lines = [];
const pixelsPerLine = 8;

function add_pixel(color, pixelsPerLine = 8) {
  let style = {
    'background-color': color,
    width: 25,
    height: 25,
  };
  
  if (currentPixels.length >= pixelsPerLine) {
    // current_line += 1;
    lines.push(<div className="line">{currentPixels}</div>);
    currentPixels = [];
  }
  currentPixels.push(<div className="pixel" style={style}></div>);

}

function show_1bpp(buffer, pixelsPerLine = 8) {
  
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
  return lines;
}

function twoBytesToSingleStringOfBits(byte1, byte2) {
  const byte1AsBinary = padStart((byte1).toString(2), 8, "0");
  const byte2AsBinary = padStart((byte2).toString(2), 8, "0");
  const additionOfBits = byte1AsBinary+byte2AsBinary;
  return additionOfBits;
}

function calculateRGBFrom2Bytes(byte1, byte2) {
  if (!byte2 || ! byte1) {
    return {r:0, g:0, b:0};
  }
    const stringOfBits = twoBytesToSingleStringOfBits(byte1, byte2);
    const red5bits = stringOfBits.substr(0,5);
    const green5bits = stringOfBits.substr(5,10);
    const blue5bits = stringOfBits.substr(10,15);

    // multiply by 8 just to  make sure its a wider range
    const r = parseInt(red5bits, 2) * 8;
    const g = parseInt(green5bits, 2) * 8;
    const b = parseInt(blue5bits, 2) * 8;
    return {r,g,b};
}


function show_16bpp(buffer, pixelsPerLine = 8) {

  for (let i = 0; i < buffer.length+1; i += 2) {
    const { r,g,b } = calculateRGBFrom2Bytes(buffer[i], buffer[i+1]);
    add_pixel('rgb(' + r + ',' + g + ',' + b + ')', pixelsPerLine);
  }
  return lines;
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
  return lines;
}


function show_8bppXterm(buffer, pixelsPerLine = 8) {

  for (let i = 0; i < buffer.length; i += 1) {
    const character = buffer[i];
    let color = xterm[character];
    
    add_pixel(color, pixelsPerLine);
  }
  return lines;
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
  return lines;
}

export function visualiseData(data, visualType) {
  // reset variables
  currentPixels = [];
  lines = [];

  if (visualType === "highlight_printable") {
    lines = show_printable(data);
  } else if (visualType === '1bpp') {
    lines = show_1bpp(data);
  }
  else if (visualType === '8bpp') {
    lines = show_8bppXterm(data);
  }
  else if (visualType === '16bpp') {
    lines = show_16bpp(data);
  }
  else if (visualType === '24bpp_bgr'){
    lines = show_24bpp(data, 'bgr');
  } else {
    lines = show_24bpp(data);
  }
  return lines;
}
