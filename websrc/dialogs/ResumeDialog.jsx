import React, { useState } from 'react';
import Button from '@material-ui/core/Button';
import TextField from '@material-ui/core/TextField';
import Dialog from '@material-ui/core/Dialog';
import DialogActions from '@material-ui/core/DialogActions';
import DialogContent from '@material-ui/core/DialogContent';
import DialogContentText from '@material-ui/core/DialogContentText';
import DialogTitle from '@material-ui/core/DialogTitle';
import { sendActionToServer } from '../server';
import { FormControl, FormLabel, FormGroup, FormControlLabel, Switch } from '@material-ui/core';
import ChipInput from 'material-ui-chip-input'


export default function ResumeDialog( { setCurrentDialog, playerState, setPlayerState, open = true }) {

  const handleClose = () => {
    setCurrentDialog('');
  };

  function run() {
    
    console.error("Going to run with with settings:", playerState);
    const newPlayerState = {...playerState, paused:false};
    setPlayerState(newPlayerState);

    const payload = {
      category: 'play',
      state: newPlayerState
    };
    sendActionToServer(payload);
    handleClose();
  }

  return (
    <div>
      <Dialog open={open} onClose={handleClose} aria-labelledby="form-dialog-title">
        <DialogTitle id="form-dialog-title">Run</DialogTitle>
        <DialogContent>
          <DialogContentText>
            This will resume the game at the chosen save state, input will be replayed until the end of the log.
          </DialogContentText>
          
          <FormControl component="fieldset">
            <FormLabel component="legend">Additional Logging</FormLabel>
            <FormGroup>
            <FormControlLabel
                  control={<Switch checked={playerState.allowOverrides} onChange={(e)=>setPlayerState({...playerState, allowOverrides:e.target.checked})} name="allowOverrides" />}
                  label="Allow Overrides"
                />
              <FormControlLabel
                control={<Switch checked={playerState.fullLogging} onChange={(e)=>setPlayerState({...playerState, fullLogging:e.target.checked})} name="fullFunctionLogging" />}
                label="All Functions"
              />
              <FormControlLabel
                labelPlacement={"start"}
                control={<ChipInput
                  value={playerState.fullFrames || []}
                  onChange={(chips) =>setPlayerState({...playerState, fullFrames:chips})}
                />}
                label="Full Frames: "
              />
            </FormGroup>
          </FormControl>
          
        </DialogContent>
        <DialogActions>
          <Button onClick={handleClose} color="error">
            Cancel
          </Button>
          <Button onClick={run} color="primary">
            Run
          </Button>
        </DialogActions>
      </Dialog>
    </div>
  );
}