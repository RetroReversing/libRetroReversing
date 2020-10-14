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
import Box from '@material-ui/core/Box';

import FrameHintPopOver from '../popovers/FrameHintPopover';
import { requestFileFromServer, sendActionToServer } from '../server';
import "./InputHistory.css";


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
  [RETRO_DEVICE_ID_JOYPAD_SELECT]: "Vita_Select.png", 
  [RETRO_DEVICE_ID_JOYPAD_START]: "Vita_Start.png",
  [RETRO_DEVICE_ID_JOYPAD_B]: "XboxOne_B.png",
  [RETRO_DEVICE_ID_JOYPAD_A]: "XboxOne_A.png",
  [RETRO_DEVICE_ID_JOYPAD_Y]: "XboxOne_Y.png",
  [RETRO_DEVICE_ID_JOYPAD_X]: "XboxOne_X.png",
  [RETRO_DEVICE_ID_JOYPAD_UP]: "XboxOne_Dpad_Up.png",
  [RETRO_DEVICE_ID_JOYPAD_DOWN]: "XboxOne_Dpad_Down.png",
  [RETRO_DEVICE_ID_JOYPAD_LEFT]: "XboxOne_Dpad_Left.png",
  [RETRO_DEVICE_ID_JOYPAD_RIGHT]: "XboxOne_Dpad_Right.png",
  [RETRO_DEVICE_ID_JOYPAD_L]: "XboxOne_LB.png",
  [RETRO_DEVICE_ID_JOYPAD_R]: "XboxOne_RB.png",
  [RETRO_DEVICE_ID_JOYPAD_L2]: "XboxOne_LT.png",
  [RETRO_DEVICE_ID_JOYPAD_R2]: "XboxOne_RT.png",
  [RETRO_DEVICE_ID_JOYPAD_L3]: "XboxOne_Left_Stick_Click.png",
  [RETRO_DEVICE_ID_JOYPAD_R3]: "XboxOne_Right_Stick_Click.png",
}

export function ShowButtonsForInput({ input, frame, buttonChanges, setButtonChanges }) {

  function changeButtonState(frame, buttonNumber, disabled) {
    let newValue = input;
    newValue ^= 1 << buttonNumber;
    const newButtonChanges = {...buttonChanges, [frame]: newValue};
    setButtonChanges(newButtonChanges);
  }
  
  function getButtonElement(frame, buttonNumber, input, disabled) {
    let image_name = ButtonToNameMap[buttonNumber] || "?";
    let classNames = "inputButton-img";
    if (disabled) {
      classNames += " disabled";
    }
    return <img src={"/images/controller-icons/"+image_name} className={classNames} onClick={()=>changeButtonState(frame, buttonNumber, !disabled)}/>;
  }

  let resultingElements = [];
  for (let i=0; i<16; i++) {
    let isButtonPressed = (input & 1 << i);
      const element = getButtonElement(frame, i, input, isButtonPressed===0); //<span onClick={changeButtonState} disabled={isButtonPressed===0}>{name}</span>
      resultingElements.push(element);
  }
  return resultingElements;
}



export function InputHistory( {mainState, fullState}) {
  const listRef = React.createRef();
  const number_of_rows = fullState?.playthrough?.last_frame || 0;
  const [loading, setLoading] = useState(true);
  const [buttonLog, setButtonLog] = useState(null);
  const [startFrame, setStartFrame] = useState(0);
  const [buttonChanges, setButtonChanges] = useState({});

  useEffect(() => {
    
    setLoading(true);
    requestFileFromServer("button_log.bin").then((buttonLog) => {
      const view = new DataView(buttonLog);
      setButtonLog(view);
      setLoading(false);
    });
  }, []);

  useEffect(()=> {
    if (Object.keys(buttonChanges).length < 1) {
      console.info("No changes to buttons");
      return;
    }

    const payload = {
      category: 'change_input_buttons',
      state: {
        buttonChanges
      },
    };
    sendActionToServer(payload).then((result) => {
      console.info("Should have saved to server", result);
    });

  }, [buttonChanges]);

  const columns = [
    {
      width: 200,
      label: "Frame",
      dataKey: "frame"
    },
    {
      width: 500,
      label: "Input",
      dataKey: "input"
    },
    // {
    //   width: 200,
    //   label: "Save",
    //   dataKey: "save"
    // },
    // {
    //   width: 200,
    //   label: "Breakpoint",
    //   dataKey: "breakpoint"
    // }
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
    let input = buttonLog?.getUint32(frame_index*8, true);

    if (frame_index in buttonChanges) {
      input = buttonChanges[frame_index];
    }

    const inputElement = <ShowButtonsForInput input={input} frame={frame_index} buttonChanges={buttonChanges} setButtonChanges={setButtonChanges} />;
    return { frame, input: inputElement, save, breakpoint };
  }

return (<Box>
  <Box p={2}>
    <TextField ml={2} id="start-frame" label="Start Frame" value={startFrame} onChange={(e)=>setStartFrame(+e.target.value || 0)} />
  </Box>
  <TableContainer component={Paper}>
  <Paper style={{ height: 600, width: "100%" }}>
    <VirtualizedTable orderBy={order} setOrderBy={setOrder} order={orderDirection} setOrder={setOrderDirection} rowCount={number_of_rows-startFrame} rowGetter={rowGetter}
            columns={columns} />
      </Paper>
  </TableContainer></Box>);
}