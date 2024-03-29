import React, { Fragment, useState, useEffect } from 'react';
import ReactDOM from 'react-dom';
import { sendActionToServer } from '../server';
import { useRendersCount } from 'react-use';
import HexViewer from './HexViewer/HexViewer';
import Paper from '@material-ui/core/Paper';
import Tabs from '@material-ui/core/Tabs';
import Tab from '@material-ui/core/Tab';
import Grid from '@material-ui/core/Grid';
import Divider from '@material-ui/core/Divider';
import Typography from '@material-ui/core/Typography';
import InputLabel from '@material-ui/core/InputLabel';
import MenuItem from '@material-ui/core/MenuItem';
import FormHelperText from '@material-ui/core/FormHelperText';
import FormControl from '@material-ui/core/FormControl';
import Select from '@material-ui/core/Select';
import InputAdornment from '@material-ui/core/InputAdornment';
import TextField from '@material-ui/core/TextField';
import IconButton from '@material-ui/core/IconButton';
import NavigateNextIcon from '@material-ui/icons/NavigateNext';
import SkipNextIcon from '@material-ui/icons/SkipNext';
import SkipPreviousIcon from '@material-ui/icons/SkipPrevious';
import NavigateBeforeIcon from '@material-ui/icons/NavigateBefore';
import Menu from '@material-ui/core/Menu';
import MoreVertIcon from '@material-ui/icons/MoreVert';

import { VisualViewer } from './DataVisualiser/VisualiseViewer';
import { ParserViewer } from './ParserViewer';
import { downloadData } from './util/DownloadData';
import { NotesViewer } from './NotesViewer/NotesViewer';
import { StringsViewer } from './StringsViewer/StringsViewer';
import { CodeOverride } from './CodeOverride/CodeOverride';
import { DisasmViewer } from './DisasmViewer/DisasmViewer';

const rowLength = 16;
const numberOfRowsAtOnce = 1200;
const totalBytesPerPage = rowLength * numberOfRowsAtOnce;

export function MemoryViewer(props) {
  const [data, setData] = useState(Buffer.from([]));
  const [loading, setLoading] = useState(true);
  
  const [subTab, setSubTab] = useState(0);
  const [currentStartAddress, setCurrentStartAddress] = useState(props?.memory?.start?.toString(16) || 0);

  const rendersCount = useRendersCount();

  const [anchorEl, setAnchorEl] = React.useState(null);
  const menuOpen = Boolean(anchorEl);

  const handleMenuClick = (event) => {
    setAnchorEl(event.currentTarget);
  };

  const handleMenuClose = () => {
    setAnchorEl(null);
  };

  useEffect(() => {
    console.error('memory changed', props.memory);
    setCurrentStartAddress(props.memory.start.toString(16));
  }, [props.memory]);

  useEffect(() => {
    console.error('currentStartAddress changed', currentStartAddress);
    let currentStartAddressNumber =  parseInt(currentStartAddress,16);
    if (currentStartAddressNumber < props.memory.start) {
      // user probably hasn't finished typing
      return;
    }

    const offset = props?.offset || currentStartAddressNumber - props.memory.start;
    setLoading(true);
    const payload = {
      category: 'request_memory',
      state: {
        memory: props.memory,
        length: props.fullFile || props.function? props?.memory?.length: totalBytesPerPage,
        offset: offset,
        swapEndian: props.swapEndian || false,
      },
    };
    sendActionToServer(payload).then((memoryContents) => {
      setData(Buffer.from(memoryContents));
      setLoading(false);
    });
  }, [currentStartAddress]);

  const tab_bar = (
    <Tabs
      value={subTab}
      onChange={(e, newValue) => {
        setSubTab(newValue);
      }}
      indicatorColor="primary"
      textColor="primary"
      scrollButtons="auto"
      centered
    >
      <Tab label="Hex" />
      <Tab label="Disassembled" />
      <Tab label="Visual" />
      <Tab label="Override" />
      <Tab label="Notes" />
      <Tab label="Strings" />
      <Tab label="Parser" />
    </Tabs>
  );

  const end_offset = props.memory.start + props.memory.length;

  const subTabs = {
    0: <HexViewer buffer={data} rowLength={rowLength} setLength={4} offset={props?.offset || props?.memory?.start} />,
    1: <DisasmViewer memory={props.memory} functionObj={props.functionObj} />,
    2: <VisualViewer />,
    3: <CodeOverride memory={props.memory} />,
    4: <NotesViewer memory={props.memory} />,
    5: <StringsViewer />,
    6: <ParserViewer />,
  };


  function downloadPageOfData() {
    handleMenuClose();
    downloadData(data, props.memory.name+"_page_"+currentStartAddress+"_to_.bin")
  }

  function downloadAllData() {
    handleMenuClose();
    const payload = {
      category: 'request_memory',
      state: {
        memory: props.memory,
        length: props.memory.length,
        swapEndian: props.swapEndian || false,
        offset: props.fullFile || props.function? props.offset : 0,
      },
    };
    sendActionToServer(payload).then((memoryContents) => {
      downloadData(Buffer.from(memoryContents), props.memory.name+"_full.bin")
    });
    
  }

  const menu = (
    <div>
      <IconButton
        aria-label="more"
        aria-controls="long-menu"
        aria-haspopup="true"
        onClick={handleMenuClick}
      >
        <MoreVertIcon />
      </IconButton>
      <Menu
        id="long-menu"
        keepMounted
        open={menuOpen}
        onClose={handleMenuClose}
        anchorEl={anchorEl}
      >
        
      <MenuItem onClick={downloadPageOfData}>
        Download Page of Data
      </MenuItem>
      <MenuItem onClick={downloadAllData}>
        Download All Data (0x{props.memory.start.toString(16)} to 0x{end_offset.toString(16)})
      </MenuItem>
        
      </Menu>
    </div>
  );

  function setNextStartOffsetManually(value) {
    console.error("setNextStartOffsetManually:", value, parseInt(value, 16));
    if (parseInt(value, 16) < end_offset) {
      setCurrentStartAddress(parseInt(value, 16).toString(16));
    }
  }

  function goBackToStart() {
    setNextStartOffsetManually(props.memory.start.toString(16));
  }
  function goBackToEnd() {
    const end = props.memory.start + props.memory.length;
    setNextStartOffsetManually((end - totalBytesPerPage).toString(16));
  }

  function nextBytes() {
    let currentAddress = parseInt(currentStartAddress, 16);
    currentAddress += totalBytesPerPage;
    setNextStartOffsetManually(currentAddress.toString(16));
  }

  function previousBytes() {
    let currentAddress = parseInt(currentStartAddress, 16);
    currentAddress -= totalBytesPerPage;
    if (currentAddress < props.memory.start) {
      return goBackToStart();
    }
    setNextStartOffsetManually(currentAddress.toString(16));
  }

  const rangeSelector = (
    <Paper>
    <Grid container alignItems="center" justify="center">
        <IconButton aria-label="next">
          <SkipPreviousIcon onClick={goBackToStart} />
        </IconButton>
        <IconButton aria-label="next">
          <NavigateBeforeIcon onClick={previousBytes} />
        </IconButton>
        <TextField
          id="standard-start-adornment"
          InputProps={{
            startAdornment: (
              <InputAdornment position="start">0x</InputAdornment>
            ),
          }}
          onChange={(e)=>setNextStartOffsetManually(e.target.value)}
          value={currentStartAddress}
        />
        <IconButton aria-label="next">
          <NavigateNextIcon onClick={nextBytes} />
        </IconButton>
        <IconButton aria-label="next">
          <SkipNextIcon onClick={goBackToEnd} />
        </IconButton>
      </Grid>
    </Paper>
  );

  return (
    <div>
      Memory Viewer
      <Grid container alignItems="center" justify="space-between">
      {props.memory.name === "file"? null:(
        <Grid item xs={1}>
          <Typography gutterBottom variant="h6">
            {props.memory.name}
          </Typography>
        </Grid>)}
        <Grid item xs={10}>
          {tab_bar}
        </Grid>
        <Grid item xs={1}>
          {menu}
          {/* <Typography gutterBottom variant="h6" style={{ textAlign: 'end' }}>
            0x{props.memory.start.toString(16)} -> 0x{end_offset.toString(16)}
          </Typography> */}
        </Grid>
      </Grid>

      <Grid container alignItems="center" justify="space-between">
        <Grid item xs={12} style={{minHeight: 350}}>
          {subTabs[subTab]}
        </Grid>
        <Grid item xs={12}>
          {rangeSelector}
        </Grid>
      </Grid>
    </div>
  );
}
