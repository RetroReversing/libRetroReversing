import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import Table from '@material-ui/core/Table';
import TableBody from '@material-ui/core/TableBody';
import TableCell from '@material-ui/core/TableCell';
import TableContainer from '@material-ui/core/TableContainer';
import TableHead from '@material-ui/core/TableHead';
import TableRow from '@material-ui/core/TableRow';
import Typography from '@material-ui/core/Typography';
import Paper from '@material-ui/core/Paper';
import { VariableSizeList as List } from 'react-window';
import { map, filter, noop } from 'lodash';


const createRow = ({ index, style, setOpen = noop }) => {
  // const rows =  window.allInformation?.functions;
  // const row = rows[index][1];
  return ( <TableRow style={{borderBottom: 'unset'}}>
    <TableCell>
  {index}
    </TableCell>
    <TableCell>
  input
    </TableCell>
    <TableCell>
  save
    </TableCell>
    <TableCell>
  breakpoint
    </TableCell>
  </TableRow>);
}

export function InputHistory( {mainState, fullState}) {
  const listRef = React.createRef();
  return (
    <TableContainer component={Paper}>
      <Table aria-label="collapsible table">
        <TableHead>
          <TableRow>
            <TableCell>Frame</TableCell>
            <TableCell align="right">Input</TableCell>
            <TableCell align="right">Save</TableCell>
            <TableCell align="right">Breakpoint</TableCell>
          </TableRow>
        </TableHead>
        <TableBody>
      {/* </Table>
      <Table aria-label="collapsible table">
      <TableBody> */}
      <List ref={listRef}
					height={550}
					itemCount={fullState?.playthrough?.last_frame}
          itemSize={(index)=>30/*getItemSize(openedFunction, index)*/}
					width={'100%'}
          >
					{createRow}
				</List>
        </TableBody>
      </Table>
      
    </TableContainer>
  );
  // return <div>Input History</div>
}