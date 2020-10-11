import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import PropTypes from 'prop-types';
import { map, filter, noop } from 'lodash';
import { MemoryViewer } from '../MemoryViewer';
import { IconButton } from '@material-ui/core';
import EditIcon from '@material-ui/icons/Edit';

function createMemoryViewer(row) {
  const offset = parseInt(row.func_offset, 16);
  return (<MemoryViewer function swapEndian offset={offset} memory={{ start: 0, name: "function", filename: row.func_name, length: 200, offset} } />);
}

export function FunctionViewer({ func, setCurrentDialog, setCurrentDialogParameters }) {
  // return createMemoryViewer(row[1])
  const playthrough_info = window.allInformation?.function_usage[func?.func_offset] || {};
  return <Fragment>
    <h1>{func.func_name}  <IconButton aria-label="delete" onClick={()=> {
      setCurrentDialogParameters(func);
      setCurrentDialog('edit_function');
      } }>
  <EditIcon />
</IconButton></h1>
    {createMemoryViewer(func)}
    <div>
      {JSON.stringify(playthrough_info.signatures)}
    </div>
  </Fragment>
}
