import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import {map, max, padStart, keys } from 'lodash';
import { FixedSizeList as List } from 'react-window';
import "./DisasmViewer.css";

export function n2hexstr(offset) {
  if (!offset) {return 0;}
  return padStart(offset.toString(16).toUpperCase(), 8, '0');
}

const mapInstructionToPseudo = {
  "MOV": "%A1 = %A0",
  "MOV.L": "Write_32bits(%A1, %A0)",
  "MOV.W": "Write_16bits(%A1, %A0)",
  "STS.L": "Write_32bits(%A1, %A0)",
  "LDS.L": "%A1 = Read_32(%A0)",
  "JSR": "JumpToSubRoutine(%A0)",
  "TST": "if ((%A1 & %A0) == 0) T = 1; else T = 0",
  "BF": "if (T == 0) goto(%A0)",
  "NOP": "",
  "RTS": "// return after next instruction",
}

function getInstructionSize(instruction_name) {
  if (instruction_name.endsWith(".L")) {
    return 4;
  }
  if (instruction_name.endsWith(".W")) {
    return 2;
  }
  return 1;
}

function convertToPseudoCode(asm) {
  const [instruction_name, params] = asm.split('(');
  const paramsArray = params.replace(')','').split(',');
  if (paramsArray.length < 1) {
    return instruction_name;
  }
  let mappedValue = mapInstructionToPseudo[instruction_name] || instruction_name;
  let prefix = '';
  paramsArray.forEach((value, i)=>{
    if (value.startsWith("@-")) {
      prefix = value.replace('@-','')+"-="+getInstructionSize(instruction_name)+"; ";
      value = value.replace('@-','address_of(')+")";
    }
    else if (value.startsWith("@")) {
      value = value.replace('@','address_of(')+")"
    }
    mappedValue = prefix+ mappedValue.replace('%A'+i, value.replace('#',''));
  });
  return mappedValue+';';
}

const createRow = (start_offset, { index, style }) => {
  index = index*2;
  const offset = start_offset+index;
  const offset_str = padStart(offset.toString(16).toUpperCase(), 8, '0');
  // console.error("offset:",offset, offset.toString(16), offset_str, window?.allInformation?.assembly[offset_str]);
  const assemblyInstruction = window?.allInformation?.assembly?.[offset_str];

  let rowInner = <div className="disasmCol">{JSON.stringify(window?.allInformation?.assembly?.[offset_str]) || 'Not executed'}</div>;

  if (assemblyInstruction && Object.keys(assemblyInstruction)?.length === 1) {
    const firstKey = Object.keys(assemblyInstruction)[0];
    rowInner = (<Fragment>
      <div className="disasmCol disasmColBytes">0x{assemblyInstruction[firstKey].bytes}</div>
      <div className="disasmCol">{firstKey.replace(',', ', ')}</div>
      <div className="disasmCol">{convertToPseudoCode(firstKey)}</div>
    </Fragment>);
  }

  return <div style={style} className="disasmRow">
    <div className="disasmColOffset">{offset.toString(16)}</div>
    {rowInner}
  </div>;
}

export function DisasmViewer({ memory, functionObj }) {

  const start_offset = memory.offset;
  const function_name = n2hexstr(start_offset); //memory.filename;
  const disassembly = window?.allInformation?.assembly; //window?.allInformation?.functions_playthough?.[function_name];
  const returns = window?.allInformation?.function_usage?.[function_name]?.returns || {};
  const max_return = max(map(keys(returns), (val)=> parseInt(val.substring(1), 16)));
  // const max_return = map(values(returns), (val)=> parseInt(val.substring(1), 16));

  // const returnsumBy(returns, (a,b)=>console.error("sumBy",a,b)) TODO: just get max return
  console.error(function_name, returns, memory, n2hexstr(start_offset), max_return, returns, "asm:",window?.allInformation?.assembly, disassembly);
  // const end_offset = memory.offset+20;
  const end_offset = max_return+2;
  // const rows= window?.allInformation?.assembly;

  let length_of_function = (end_offset - start_offset)/2; // divided by 2 for saturn but will vary per console

  if (length_of_function>300) {
    length_of_function = 300;
  }

  return (<div>
  <div className="disasmRow disasmHeaderRow">
    <div className="disasmCol">Location</div>
    <div className="disasmCol disasmColBytes">Bytes</div>
    <div className="disasmCol">Assembly</div>
    <div className="disasmCol">Pseudo</div>
  </div>
  <List
  height={350}
  itemCount={length_of_function}
  itemSize={25}
  width={'100%'}
>
  {createRow.bind(null, start_offset)}
</List></div>)
  
  // return <div>DisasmViewer {function_name} {JSON.stringify(disassembly,2,2)}
  // <div>
  // Return statements:
  // <ul>
  //   {Object.keys(returns)?.map((key)=> { return <li> {key} = {returns[key]}</li>})}
  // </ul>
  // </div>
  // </div>
}