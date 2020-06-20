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
        <DialogTitle id="form-dialog-title">Save State</DialogTitle>
        <DialogContent>
          <DialogContentText>
            This will resume the game at a save state, you have some options for what logging to do:
            * Full function logging
            * Full memory logging
          </DialogContentText>
          <FormControl component="fieldset">
            <FormLabel component="legend">Additional Logging</FormLabel>
            <FormGroup>
              <FormControlLabel
                control={<Switch checked={playerState.fullLogging} onChange={(e)=>setPlayerState({...playerState, fullLogging:e.target.checked})} name="fullFunctionLogging" />}
                label="Full Function Logging"
              />
              {JSON.stringify(playerState)}
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