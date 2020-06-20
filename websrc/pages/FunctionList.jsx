import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import PropTypes from 'prop-types';
import { map, filter, noop } from 'lodash';
import Box from '@material-ui/core/Box';
import Collapse from '@material-ui/core/Collapse';
import IconButton from '@material-ui/core/IconButton';
import Table from '@material-ui/core/Table';
import TableBody from '@material-ui/core/TableBody';
import TableCell from '@material-ui/core/TableCell';
import TableContainer from '@material-ui/core/TableContainer';
import TableHead from '@material-ui/core/TableHead';
import TableRow from '@material-ui/core/TableRow';
import Typography from '@material-ui/core/Typography';
import Paper from '@material-ui/core/Paper';
import KeyboardArrowDownIcon from '@material-ui/icons/KeyboardArrowDown';
import KeyboardArrowUpIcon from '@material-ui/icons/KeyboardArrowUp';
import KeyboardArrowRightIcon from '@material-ui/icons/KeyboardArrowRight';
import DescriptionIcon from '@material-ui/icons/Description';
import { MemoryViewer } from './MemoryViewer';
import FrameHintPopOver from '../popovers/FrameHintPopover';
import { VariableSizeList as List } from 'react-window';

function createMemoryViewer(row) {
  const offset = parseInt(row.func_offset, 16);
  return (<MemoryViewer function swapEndian offset={offset} memory={{ start: 0, name: "function", filename: row.func_name, length: 200, offset} } />);
}

const createRow = (openedFunction, setOpenedFunction, { index, style, setOpen = noop }) => {
  const rows =  window.allInformation?.functions;
  const row = rows[index][1];
  return (<div style={{...style, width:'100%', display:'table', maxHeight:52, overflowY:'scroll'}} >
  {/* <Table aria-label="collapsible table">
   <TableBody> */}
   {/* <TableHead>
          <TableRow>
            <TableCell />
            <TableCell></TableCell>
            <TableCell align="right"></TableCell>
            <TableCell align="right"></TableCell>
            <TableCell align="right"></TableCell>
            <TableCell align="right"></TableCell>
          </TableRow>
    </TableHead> */}
  <TableRow style={{borderBottom: 'unset'}}>
        <TableCell style={{width: "5%"}}>
        </TableCell>
        <TableCell style={{width: "25%"}} component="th" scope="row" onClick={() => {
          // setOpenedIndex(index);
          if (openedFunction !== row.func_name) {
            setOpenedFunction(row.func_name);
          } else {
            setOpenedFunction('');
          }
          }}>
        {row.func_name}
        </TableCell>
        <TableCell style={{width: "10%"}} align="right">{row.func_offset}</TableCell>
        <TableCell style={{width: "10%"}} align="right">{row.func_stack} </TableCell>
        {/* <TableCell style={{width: "25%", maxHeight:40}} align="right">{row.stack_trace} </TableCell> */}
        <TableCell align="right"> </TableCell>
        
     </TableRow>
     {/*  </TableBody>
      </Table> */}
      
      {openedFunction=== row.func_name?(
        <TableRow>
      <TableCell align="center" colSpan={12}>
        {createMemoryViewer(row)}
        </TableCell>
      </TableRow>
      ):null}
      
  </div>);
};

export function FunctionList() {

  const [openedFunction, setOpenedFunction] = useState('');
  const listRef = React.createRef();

  const getItemSize = (openedFunction, index) => {
    const rows =  window.allInformation?.functions;
    const row = rows[index][1];
    if (openedFunction === row.func_name) {
      return 600;
    }
    return 52;
  };

  useEffect(()=>{
    listRef.current.resetAfterIndex(0, true);
  }, [openedFunction]);

  const rows = window.allInformation?.functions;
  return (
    <TableContainer component={Paper}>
      <Table aria-label="collapsible table">
        <TableHead>
          <TableRow>
            <TableCell />
            <TableCell>Name</TableCell>
            <TableCell align="right">Location</TableCell>
            <TableCell align="right">First Access</TableCell>
            <TableCell align="right">Latest Access</TableCell>
            <TableCell align="right">Access Count</TableCell>
          </TableRow>
        </TableHead>
        <TableBody>
          {/* {rows.map((row) => (
            <Row key={row?.name} row={row} />
          ))} */}
          
        </TableBody>
        
      </Table>
      <Table aria-label="collapsible table">
      <TableBody>
      <List ref={listRef}
					height={550}
					itemCount={rows.length}
          itemSize={(index)=>getItemSize(openedFunction, index)}
					width={'100%'}
          >
					{createRow.bind(null, openedFunction, setOpenedFunction)}
				</List>
        </TableBody>
        </Table>
      
    </TableContainer>
  );

}