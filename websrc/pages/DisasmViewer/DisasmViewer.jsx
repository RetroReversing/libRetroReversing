import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import {map, max, padStart, keys } from 'lodash';
import { FixedSizeList as List } from 'react-window';

const createRow = (start_offset, { index, style }) => {
  index = index*2;
  const offset = start_offset+index;
  const offset_str = 'x'+padStart(offset.toString(16).toUpperCase(), 8, '0');
  // console.error("offset:",offset, offset.toString(16), offset_str, window?.allInformation?.assembly[offset_str]);
  return <div style={style}>{JSON.stringify(window?.allInformation?.assembly?.[offset_str]) || 'Not executed'}</div>;
}

export function DisasmViewer({ memory }) {

  const function_name = memory.filename;
  const disassembly = window?.allInformation?.functions_playthough?.[function_name];
  const returns = disassembly?.returns || {};
  const max_return = max(map(keys(returns), (val)=> parseInt(val.substring(1), 16)));
  // const max_return = map(values(returns), (val)=> parseInt(val.substring(1), 16));

  // const returnsumBy(returns, (a,b)=>console.error("sumBy",a,b)) TODO: just get max return
  console.error("Disasm Memory:", memory, returns, max_return, max(max_return));
  const start_offset = memory.offset;
  // const end_offset = memory.offset+20;
  const end_offset = max_return+2;
  // const rows= window?.allInformation?.assembly;

  let length_of_function = (end_offset - start_offset)/2; // divided by 2 for saturn but will vary per console

  if (length_of_function>300) {
    length_of_function = 300;
  }

  return (<List
  height={350}
  itemCount={length_of_function}
  itemSize={25}
  width={'100%'}
>
  {createRow.bind(null, start_offset)}
</List>)
  
  return <div>DisasmViewer {function_name} {JSON.stringify(disassembly,2,2)}
  <div>
  Return statements:
  <ul>
    {Object.keys(returns)?.map((key)=> { return <li> {key} = {returns[key]}</li>})}
  </ul>
  </div>
  </div>
}