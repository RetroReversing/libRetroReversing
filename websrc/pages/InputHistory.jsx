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
import TextField from '@material-ui/core/TextField';
import { VariableSizeList as List } from 'react-window';
import { map, filter, noop } from 'lodash';
import VirtualizedTable from './util/VirtualTable';
import Checkbox from '@material-ui/core/Checkbox';
import FrameHintPopOver from '../popovers/FrameHintPopover';
import { requestFileFromServer } from '../server';
const RETRO_DEVICE_ID_JOYPAD_B =     0
const RETRO_DEVICE_ID_JOYPAD_Y =        1
const RETRO_DEVICE_ID_JOYPAD_SELECT =   2
const RETRO_DEVICE_ID_JOYPAD_START =    3
const RETRO_DEVICE_ID_JOYPAD_UP =       4
const RETRO_DEVICE_ID_JOYPAD_DOWN =     5
const RETRO_DEVICE_ID_JOYPAD_LEFT =     6
const RETRO_DEVICE_ID_JOYPAD_RIGHT =    7
const RETRO_DEVICE_ID_JOYPAD_A =        8
const RETRO_DEVICE_ID_JOYPAD_X =        9
const RETRO_DEVICE_ID_JOYPAD_L =       10
const RETRO_DEVICE_ID_JOYPAD_R =       11
const RETRO_DEVICE_ID_JOYPAD_L2 =      12
const RETRO_DEVICE_ID_JOYPAD_R2 =      13
const RETRO_DEVICE_ID_JOYPAD_L3 =      14
const RETRO_DEVICE_ID_JOYPAD_R3 =      15

const ButtonToNameMap = {
  [RETRO_DEVICE_ID_JOYPAD_SELECT]: "Select",
  [RETRO_DEVICE_ID_JOYPAD_START]: "Start",
  [RETRO_DEVICE_ID_JOYPAD_B]: "B",
  [RETRO_DEVICE_ID_JOYPAD_A]: "A",
  [RETRO_DEVICE_ID_JOYPAD_Y]: "Y",
  [RETRO_DEVICE_ID_JOYPAD_X]: "X",
  [RETRO_DEVICE_ID_JOYPAD_UP]: "UP",
  [RETRO_DEVICE_ID_JOYPAD_DOWN]: "DOWN",
  [RETRO_DEVICE_ID_JOYPAD_LEFT]: "LEFT",
  [RETRO_DEVICE_ID_JOYPAD_RIGHT]: "RIGHT",
  [RETRO_DEVICE_ID_JOYPAD_L]: "L",
  [RETRO_DEVICE_ID_JOYPAD_R]: "R",
  [RETRO_DEVICE_ID_JOYPAD_L2]: "L2",
  [RETRO_DEVICE_ID_JOYPAD_R2]: "R2",
  [RETRO_DEVICE_ID_JOYPAD_R3]: "R3",
}

export function ShowButtonsForInput({ input }) {
  if (input === 0) {
    return null;
  }
  let resultingString = "";
  for (let i=0; i<16; i++) {
    let name = ButtonToNameMap[i] || "?";
    let isButtonPressed = (input & 1 << i);
    if (isButtonPressed>0) {
      resultingString += name + " "
    }
  }
  return resultingString;
}



export function InputHistory( {mainState, fullState}) {
  const listRef = React.createRef();
  const number_of_rows = fullState?.playthrough?.last_frame || 0;
  const [loading, setLoading] = useState(true);
  const [buttonLog, setButtonLog] = useState(null);
  const [startFrame, setStartFrame] = useState(0);

  useEffect(() => {
    
    setLoading(true);
    requestFileFromServer("button_log.bin").then((buttonLog) => {
      console.error("Go the Button Log!", typeof(buttonLog), buttonLog, new Blob([buttonLog]));
      const view = new DataView(buttonLog);
      setButtonLog(view);
      setLoading(false);
    });
  }, []);

  const columns = [
    {
      width: 200,
      label: "Frame",
      dataKey: "frame"
    },
    {
      width: 200,
      label: "Input",
      dataKey: "input"
    },
    // {
    //   width: 200,
    //   label: "Save",
    //   dataKey: "save"
    // },
    {
      width: 200,
      label: "Breakpoint",
      dataKey: "breakpoint"
    }
  ];

  const [order, setOrder] = React.useState('Location');
  const [orderDirection, setOrderDirection] = React.useState('asc');
  const [rows, setRows] = React.useState(window.allInformation?.functions || []);

  function rowGetter({index}) {    
    const frame_index = startFrame + index;
    const breakpoint =  (<Checkbox
        checked={false}
        onChange={()=>null}
        inputProps={{ 'aria-label': 'primary checkbox' }}
        style={{ marginBottom: 20}}
      />);
    const save =  (<Checkbox
        checked={false}
        onChange={()=>null}
        inputProps={{ 'aria-label': 'primary checkbox' }}
        style={{ marginBottom: 20}}
      />);
    const frame = (<FrameHintPopOver frame={frame_index} />);
    // Technically our button log supports 64bit (8 byte values)
    const left = buttonLog?.getUint32(frame_index*8, true);
    // const right = buttonLog?.getUint32(index*8+ 4, true);
    const input = buttonLog?.getUint32(frame_index*8, true); // 967
    const inputElement = <ShowButtonsForInput input={input} />;
    return { frame, input: inputElement, save, breakpoint };
  }

return (<TableContainer component={Paper}>
  <TextField id="start-frame" label="Start Frame" value={startFrame} onChange={(e)=>setStartFrame(+e.target.value || 0)} />
  <Paper style={{ height: 600, width: "100%" }}>
    <VirtualizedTable orderBy={order} setOrderBy={setOrder} order={orderDirection} setOrder={setOrderDirection} rowCount={number_of_rows-startFrame} rowGetter={rowGetter}
            columns={columns} />
      </Paper>
  </TableContainer>);
}