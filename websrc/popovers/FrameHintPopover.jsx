import React from 'react';
import Popover from '@material-ui/core/Popover';
import Typography from '@material-ui/core/Typography';
import Button from '@material-ui/core/Button';
import { each, noop } from 'lodash';
import { createSaveStateTile } from '../main/SaveStateList';
import { loadState } from '../pages/MainPage';
import { Grid, GridList, Paper, Popper } from '@material-ui/core';

function findClosestStates(frame, states, load_state) {
  let closest_before = { frame: -1 };
  let closest_after = { frame: 9999999999999 };
  let exact_match = false;
  each(states, state => {
    if (state.frame === frame) {
      exact_match = true;
      closest_before = state;
      closest_after = state;
      return;
    }

    if (state.frame < frame) {
      // console.log("state.frame < frame, frame:", frame, "<", state.frame, " closest_before:", closest_before.frame, frame > closest_before.frame)
      if (state.frame > closest_before.frame) {
        closest_before = state;
      }
      console.log("state.frame < frame, frame:", frame, "<", state.frame, " closest_before:", closest_before.frame, frame > closest_before.frame)
      return;
    }
    if (state.frame > frame) {
      if (state.frame < closest_after.frame) {
        closest_after = state;
        return;
      }
      // console.log("state frame:", state.frame, "<", frame, "but <", closest_before)
      return;
    }
    console.error("State:", state.frame, frame);
  })
  return (<GridList cellHeight={180} >
      {createSaveStateTile(closest_before, load_state, "After: ")}
      {createSaveStateTile(closest_after, load_state, "But Before: ")}
  </GridList>)
  // return {closest_before, closest_after, exact_match};
}

export default function FrameHintPopOver(props) {
  const [anchorEl, setAnchorEl] = React.useState(null);
  const frame = props?.frame;

  const handleClick = (event) => {
    setAnchorEl(event.currentTarget);
  };

  const handleClose = () => {
    setAnchorEl(null);
  };

  const open = Boolean(anchorEl);
  const id = open ? 'simple-popover' : undefined;

  if (!frame) { return null; }

  const mainElement = (<Typography aria-owns={open ? 'mouse-over-popover' : undefined}
  aria-haspopup="true" aria-describedby={id} variant="contained" onClick={handleClick}>{frame || 0}</Typography>);

  // if (!open) {
  //   return mainElement;
  // }
  let popOver = null;

  if (open && frame) {
    popOver = ( <Popover
      id="mouse-over-popover"
      open={open}
      anchorEl={anchorEl}
      onClose={handleClose}
      anchorOrigin={{
        vertical: 'bottom',
        horizontal: 'center',
      }}
      transformOrigin={{
        vertical: 'top',
        horizontal: 'center',
      }}
    >
      <Paper>
        <Typography>Closest named frames to {frame}</Typography>
        {findClosestStates(frame, window.allInformation?.playthrough?.states, loadState)}
      </Paper>
    </Popover>);
  }

  return (
    <div>
      {mainElement}
      {/* <Button aria-describedby={id} variant="contained" color="primary" onClick={handleClick}>
        Open Popover
      </Button> */}
      {/* <Popper id={"mouse-over-popover"} open={open} anchorEl={anchorEl}>
        <div>The content of the Popper.</div>
      </Popper> */}
      {open?popOver:null}
    </div>
  );
}