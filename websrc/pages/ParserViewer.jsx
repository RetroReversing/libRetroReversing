import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
// import MonacoEditor from 'react-monaco-editor';
import Parser from './ParserViewer/binary_parser';
var Buffer = require('buffer/').Buffer  // note: the trailing slash is important!
// const { Parser, FileUnparser, ReactUnparser, Buffer } = window["binary_unparser"];
import { FileUnparser, ReactUnparser } from './ParserViewer/binary_unparser';

const default_code = `
  // Build an IP packet header Parser
  var ipHeader = new Parser.Parser()
    .endianess("big")
    .bit4("version")
    .bit4("headerLength")
    .uint8("tos")
    .uint16("packetLength")
    .uint16("id")
    .bit3("offset")
    .bit13("fragOffset")
    .uint8("ttl")
    .uint8("protocol")
    .uint16("checksum")
    .array("src", {
      type: "uint8",
      length: 4
    })
    .array("dst", {
      type: "uint8",
      length: 4
    });
  
  var buf = Buffer.from("450002c5939900002c06ef98adc24f6c850186d1", "hex");
  
  // Parse buffer and show result
  const result = ipHeader.parse(buf);
  console.log('result:',result);
  
  //var ipHeaderUP = new FileUnparser(ipHeader);
  //console.log("Unparsed:",ipHeaderUP.unparse(result, buf));
  return "Done";
  `;

function runCodeWithDateFunction(obj, data){
  return Function('"use strict";return (' + obj + ')')()(
      Parser, FileUnparser, data
  );
}


export function ParserViewer() {

  const [code, setCode] = useState(default_code);

  return "Enable this in webpack";

  console.error("Parser is:", Parser)

  var ipHeader = new Parser.Parser()
    .endianess("big")
    .uint8("version");

  var buf = Buffer.from("450002c5939900002c06ef98adc24f6c850186d1", "hex");
  window.Buffer = Buffer;
  window.React = React;
  
  // Parse buffer and show result
  const result = ipHeader.parse(buf);
  console.log('result:',result);
  console.log(runCodeWithDateFunction( "function(Parser, FileUnparser, data){ "+code+" \n}"))

  var ipHeaderUP = new ReactUnparser(ipHeader);
  const reactBuffer = [];
  const reactResult = ipHeaderUP.unparse(result, reactBuffer, React);
  console.log("Unparsed:",reactResult);
  // eval(code);
  
    const options = {
      selectOnLineNumbers: true
    };
    // return (
    //   <div>
    //   <MonacoEditor
    //     width="800"
    //     height="600"
    //     language="javascript"
    //     theme="vs-dark"
    //     value={code}
    //     options={options}
    //     onChange={(e)=>setCode(e.target.value)}
    //     editorDidMount={()=>console.error("mount")}
    //   />
    //   {reactResult}
    //   </div>
    // );

  return <div id="parse-container">ParserViewer (enable by commenting back in the Monaco editor)</div>
}