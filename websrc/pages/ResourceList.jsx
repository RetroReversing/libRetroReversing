import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import PropTypes from 'prop-types';
import { map, filter} from 'lodash';
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

function createData(name, lba, first_access, last_access, access_count, file_flags, files, file) {
  return {
    name,
    lba,
    first_access,
    last_access,
    access_count,
    directory: file_flags === 2? true: false,
    files,
    ...file
  };
}

function createMemoryViewer(row) {
  return (<TableContainer component={Paper}>
  <MemoryViewer fullFile offset={row.extent_location_bytes} memory={{ start: 0, name: "file", filename: row.name, length: row.extent_length, offset:row.extent_location_bytes} } />
  </TableContainer>);
}

function createSubDirectoryTree(row, open) {
  if (!open) return null;
  // if (!row.files) return createMemoryViewer(row);
  return (<TableRow>
    <TableCell style={{ paddingBottom: 0, paddingTop: 0 }} colSpan={6}>
      <Collapse in={open} timeout="auto" unmountOnExit>
        <Box margin={1}>
          {/* <Typography variant="h6" gutterBottom component="div">
            {row.name}
          </Typography> */}
        {row.files && row.files.length>0? (<ResourceList cdData={row.files} />): (
        createMemoryViewer(row))}
        </Box>
      </Collapse>
    </TableCell>
  </TableRow>);
}

export function Row({row}) {
  const [open, setOpen] = React.useState(false);
  // const classes = useRowStyles();
  if (!row || row.name.length<1) { return null; }

  return (
    <React.Fragment>
      <TableRow style={{borderBottom: 'unset'}}>
        <TableCell>
        {!row.directory? <DescriptionIcon />: (
          <IconButton aria-label="expand row" size="small" onClick={() => setOpen(!open)}>
            {open ? <KeyboardArrowDownIcon /> : <KeyboardArrowRightIcon />}
          </IconButton>)}
        </TableCell>
        <TableCell component="th" scope="row" onClick={() => setOpen(!open)}>
          {row.name.replace(';1','')}
        </TableCell>
        <TableCell align="right">{row.lba} -> {row.extent_location_end_lba} ({1 + row.extent_location_end_lba-row.lba})</TableCell>
        <TableCell align="right"><FrameHintPopOver frame={row.first_access} /></TableCell>
        <TableCell align="right"><FrameHintPopOver frame={row.last_access} /></TableCell>
        <TableCell align="right">{row.access_count}</TableCell>
      </TableRow>
      { createSubDirectoryTree(row, open)}
    </React.Fragment>
  );
}

Row.propTypes = {
  row: PropTypes.shape({
    lba: PropTypes.number.isRequired,
    last_access: PropTypes.number.isRequired,
    first_access: PropTypes.number.isRequired,
    history: PropTypes.arrayOf(
      PropTypes.shape({
        amount: PropTypes.number.isRequired,
        customerId: PropTypes.string.isRequired,
        date: PropTypes.string.isRequired,
      }),
    ).isRequired,
    name: PropTypes.string.isRequired,
    price: PropTypes.number.isRequired,
    access_count: PropTypes.number.isRequired,
  }).isRequired,
};



export function ResourceList( { cdData }) {
  let rows = filter(cdData, data=>{
    return data.name.trim().length>1;});
  rows = map(rows, file => {
    if (!file) return;
    return createData(file.name, file.extent_location_lba, file.first_access, file.last_access, file.access_count, file.file_flags, file.files, file);
  } )

  return (
    <TableContainer component={Paper}>
      <Table aria-label="collapsible table">
        <TableHead>
          <TableRow>
            <TableCell />
            <TableCell>Name</TableCell>
            <TableCell align="right">Location (LBA)</TableCell>
            <TableCell align="right">First Access</TableCell>
            <TableCell align="right">Latest Access</TableCell>
            <TableCell align="right">Access Count</TableCell>
          </TableRow>
        </TableHead>
        <TableBody>
          {rows.map((row) => (
            <Row key={row?.name} row={row} />
          ))}
        </TableBody>
      </Table>
    </TableContainer>
  );
}
