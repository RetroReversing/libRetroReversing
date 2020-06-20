import React, { Fragment, useState, useEffect } from 'react';
import ReactDOM from 'react-dom';
import Paper from '@material-ui/core/Paper';
import Select from '@material-ui/core/Select';
import MenuItem from '@material-ui/core/MenuItem';
import Grid from '@material-ui/core/Grid';
import FormGroup from '@material-ui/core/FormGroup';
import FormControlLabel from '@material-ui/core/FormControlLabel';
import Switch from '@material-ui/core/Switch';
import { xterm } from './8bppXterm';
import { padStart, debounce } from "lodash";
import css from "./visual.css";
import { visualiseData as originalVisualiseData } from './visual';
import { TextField } from '@material-ui/core';
import { FixedSizeList as List } from 'react-window';

// const visualiseData = debounce(originalVisualiseData, 2000);

export function VisualViewer(props) {
  const [visualType, setVisualType] = useState('8bpp');
  const [is2D, set2D] = useState(true);
  const [pixelsPerLine, setPixelsPerLine] = useState(16);
  // const debouncedSetPixelsPerLine = debounce(setPixelsPerLine, 2000);

  let lines = originalVisualiseData(props.buffer, visualType, is2D, "tiles", pixelsPerLine>8?pixelsPerLine:8);

  // useEffect(
  //   ()=> {
  //     console.error("Pixels per line changed");
  //     lines = visualiseData(props.buffer, visualType, is2D, "tiles", pixelsPerLine);
  //   },
  //   [pixelsPerLine]
  // )

  const displaySelector = (
    <Paper>
      <Grid container alignItems="center" justify="space-between">
        <Select
          labelId="demo-simple-select-label"
          id="demo-simple-select"
          onChange={(e) => setVisualType(e.target.value)}
          value={visualType}
        >
          <MenuItem value={'32bpp_argb'}>32bpp ARGB (4 bytes)</MenuItem>
          <MenuItem value={'32bpp_rgba'}>32bpp RGBA (4 bytes)</MenuItem>
          <MenuItem value={'24bpp_rgb'}>24bpp RGB (3 bytes)</MenuItem>
          <MenuItem value={'24bpp_bgr'}>24bpp BGR (3 bytes)</MenuItem>
          <MenuItem value={'16bpp'}>16bpp MSB BGR (2 bytes)</MenuItem>
          <MenuItem value={'16bpp_msb_rgb'}>16bpp MSB RGB (2 bytes)</MenuItem>
          <MenuItem value={'16bpplsb'}>16bpp LSB BGR (2 bytes)</MenuItem>
          <MenuItem value={'16bpp_lsb_rgb'}>16bpp LSB RGB (2 bytes)</MenuItem>
          <MenuItem value={'8bpp'}>8bpp (1 byte)</MenuItem>
          <MenuItem value={'1bpp'}>1bpp (1 bit)</MenuItem> 
          <MenuItem value={'highlight_printable'}>Highlight Printable</MenuItem>
          <MenuItem value={'digraph'}>DiGraph</MenuItem>
        </Select>
        <TextField id="standard-basic" label="Pixels per Line" value={pixelsPerLine} onChange={(e)=>setPixelsPerLine(e.target.value)} />
        {/* TODO: Add pixel size zooms */}
        <FormControlLabel
        control={<Switch checked={is2D} onChange={(e) => set2D(e.target.checked)} name="2D" />}
        label="2D"
      />
      </Grid>
    </Paper>
  );

  const createRow = ({ index, style }) => {
    const row = lines?.[index];
    return row;
  };

  return (
    <div>
      {displaySelector}
      <Grid container style={{maxWidth: '95vw', display: 'block'}}>
        {lines}
        {/* <List
					height={512}
					itemCount={lines?.length}
					itemSize={15}
					width={'100%'}
				>
					{createRow}
				</List> */}
      </Grid>
    </div>
  );
}
