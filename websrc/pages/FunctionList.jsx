import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import PropTypes from 'prop-types';
import { map, filter, noop, sortBy, orderBy as _orderBy } from 'lodash';
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
import { AutoSizer, Column, Table as VirtualTable } from 'react-virtualized';
import ReactVirtualizedTable from './util/VirtualTable';
import VirtualizedTable from './util/VirtualTable';
import {
  BrowserRouter as Router,
  Switch,
  Route,
  useParams,
  useHistory,
  useLocation
} from "react-router-dom";
import { FunctionViewer } from './FunctionViewer/FunctionViewer';
import MapIcon from '@material-ui/icons/Map';


export function FunctionList( { loading, setCurrentDialog, setCurrentDialogParameters }) {

  const listRef = React.createRef();
  let params = useParams();
  let location = useLocation();
  let history = useHistory();
  const [openedFunction, setOpenedFunction] = useState(params.currentSubTab || '');
  const [orderBy, setOrderBy] = React.useState('Location');
  const [order, setOrder] = React.useState('asc');
  const [rows, setRows] = React.useState(window.allInformation?.functions || []);
  console.error("view_function", params, location, params.currentSubTab, order, orderBy);

  const getItemSize = (openedFunction, index) => {
    const rows =  window.allInformation?.functions;
    const row = rows[index][1];
    if (openedFunction === row.func_name) {
      return 600;
    }
    return 52;
  };

  function getSortField(row) {
    const func_name = row[1].func_offset;
    // window.allInformation?.function_usage[row.func_name]
    if (orderBy === "First Access") {
      return window.allInformation?.function_usage[func_name]?.first_frame_access;
    }
    if (orderBy === "Latest Access") {
      return window.allInformation?.function_usage[func_name]?.last_frame_access;
    }
    if (orderBy === "Frame Count") {
      return window.allInformation?.function_usage[func_name]?.number_of_frames;
    }
    if (orderBy === "Calls Per Frame") {
      return window.allInformation?.function_usage[func_name]?.number_of_calls_per_frame;
    }
    if (orderBy === "Name") {
      return row[1].func_name;
    }
    // console.error("sorting by:", row);
    return row[1].func_offset; //func_name;
  }

  useEffect(()=>{
    if (!window.allInformation || window.allInformation?.functions?.length === 0) { return; }
    const mapped = map(window.allInformation?.functions, (row)=>row/*[1]*/);
    const sorted = _orderBy(mapped, getSortField, order);
    console.error("Changed order:", order, orderBy, mapped, sorted);
    setRows(sorted);
  }, [order, orderBy]);

  useEffect(()=>{
  //   // if (!listRef.current) { return; }
  //   listRef.current.resetAfterIndex(0, true);
    setOpenedFunction(params.currentSubTab);
  }, [location]);

  // const rows = window.allInformation?.functions || [];
  // if (!rows) {
  //   return null;
  // }

  if (openedFunction) {
    const row = filter(window.allInformation?.functions, (func)=>func[1].func_name === openedFunction)[0];
    if (row) {
      console.error("openedFunction:", openedFunction, row);
      return <FunctionViewer func={row[1]} setCurrentDialog={setCurrentDialog} setCurrentDialogParameters={setCurrentDialogParameters} />;
      // return createMemoryViewer(row[1]);
    }
  }

  if (rows.length <1) {
    console.error("Chdecking for rows", window.allInformation?.functions);
    if (window.allInformation?.functions) {
      setRows(window.allInformation?.functions);
      console.error("Updated rows", window.allInformation?.functions);
    }
    return null;
  }

  return (
    <div>
    <h1>All Functions 
        <IconButton onClick={()=>setCurrentDialog('load_linker_map')} aria-label="build">
          <MapIcon />
        </IconButton>
    </h1>
    <TableContainer component={Paper}>

    <Paper style={{ height: 600, width: "100%" }}>
      <VirtualizedTable orderBy={orderBy} setOrderBy={setOrderBy} order={order} setOrder={setOrder} rowCount={rows.length} rowGetter={({index})=> {
        // const rows =  window.allInformation?.functions;
        const row = rows[index][1];
        const playthrough_info = window.allInformation?.function_usage[row?.func_offset] || {};
        return { name:<a onClick={()=> {
          // setOpenedFunction(row.func_name); 
          history.push("/functions/"+row.func_name);
        }

        }>{row.func_name}</a>, location:row.func_offset, first_access:<FrameHintPopOver frame={playthrough_info.first_frame_access} />, last_access:<FrameHintPopOver frame={playthrough_info.last_frame_access} />, frame_count:playthrough_info.number_of_frames, calls_per_frame:playthrough_info.number_of_calls_per_frame };
      }}
              columns={[
          {
            width: 200,
            label: "Name",
            dataKey: "name"
          },
          {
            width: 200,
            label: "Location",
            dataKey: "location"
          },
          {
            width: 200,
            label: "First Access",
            dataKey: "first_access"
          },
          {
            width: 200,
            label: "Latest Access",
            dataKey: "last_access"
          },
          {
            width: 200,
            label: "Frame Count",
            dataKey: "frame_count",
            numeric: true
          },
          {
            width: 200,
            label: "Calls Per Frame",
            dataKey: "calls_per_frame",
            numeric: true
          }
        ]} />
        </Paper>
    </TableContainer>
    </div>
  );

}