import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";

export function VisualViewer(props) {

  const pixelsPerLine = 16; 
  let currentPixels = [];
  const lines = [];

  let current_line = 0;
  console.error("LENGTH:", props.buffer.length);
  for (let i=0; i< props.buffer.length; i+=3) {
    const r = props.buffer[i];
    const g = props.buffer[i+1];
    const b = props.buffer[i+2];
    // console.error("R:",r,g,b, current_line);
    let style = {'background-color': "rgb("+r+","+g+","+b+")", width: 25, height: 25};
    currentPixels.push(<div className="pixel" style={style}></div>);
    if (i % 16 === 0) {
      current_line+=1;
      lines.push(<div className="line">{currentPixels}</div>);
      currentPixels = [];
    }
  }

  return <div>{lines}</div>
}