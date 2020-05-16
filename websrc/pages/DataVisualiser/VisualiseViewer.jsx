import React, { Fragment, useState, useEffect } from 'react';
import ReactDOM from 'react-dom';
import Paper from '@material-ui/core/Paper';
import Select from '@material-ui/core/Select';
import MenuItem from '@material-ui/core/MenuItem';
import Grid from '@material-ui/core/Grid';
import { xterm } from './8bppXterm';
import { padStart } from "lodash";
import css from "./visual.css";
import { visualiseData } from './visual';



export function VisualViewer(props) {
  const [visualType, setVisualType] = useState('8bpp');

  const lines = visualiseData(props.buffer, visualType);

  const displaySelector = (
    <Paper>
      <Grid container alignItems="center" justify="space-between">
        <Select
          labelId="demo-simple-select-label"
          id="demo-simple-select"
          onChange={(e) => setVisualType(e.target.value)}
          value={visualType}
        >
          <MenuItem value={'24bpp_rgb'}>24bpp RGB (3 bytes)</MenuItem>
          <MenuItem value={'24bpp_bgr'}>24bpp BGR (3 bytes)</MenuItem>
          <MenuItem value={'16bpp'}>16bpp (2 bytes)</MenuItem>
          <MenuItem value={'8bpp'}>8bpp (1 byte)</MenuItem>
          <MenuItem value={'1bpp'}>1bpp (1 bit)</MenuItem> 
          <MenuItem value={'highlight_printable'}>Highlight Printable</MenuItem>
        </Select>
      </Grid>
    </Paper>
  );

  return (
    <div>
      {displaySelector}
      {lines}
    </div>
  );
}
