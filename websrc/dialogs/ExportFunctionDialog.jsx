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
import Checkbox from '@material-ui/core/Checkbox';


export default function ExportFunctionDialog( { currentDialogParameters, setCurrentDialog, playerState, setPlayerState, open = true }) {
  const [functionSettings, setFunctionSettings] = React.useState(currentDialogParameters);

  const handleClose = () => {
    setCurrentDialog('');
  };

  function save() {
    
    console.error("Going to save with settings:", functionSettings);
    // const newPlayerState = {...playerState, paused:false};
    // setPlayerState(newPlayerState);

    const payload = {
      category: 'export_function',
      state: functionSettings
    };
    sendActionToServer(payload);
    handleClose();
  }

  return (
    <div>
      <Dialog open={open} onClose={handleClose} aria-labelledby="form-dialog-title">
        <DialogTitle id="form-dialog-title">Export Function</DialogTitle>
        <DialogContent>
          <DialogContentText>
            This will export the assembly source code for the current function to the source folder, which can then be commited to Git.
          </DialogContentText>
          <form noValidate autoComplete="off">
            <FormControlLabel disabled control={<Checkbox name="exportAsm" />} label="Export Assembly" />
            <FormControlLabel disabled control={<Checkbox name="exportJs" />} label="Export Javascript" />
            <FormControlLabel disabled control={<Checkbox name="exportC" />} label="Export C" />
          </form>
          
        </DialogContent>
        <DialogActions>
          <Button onClick={handleClose} color="error">
            Cancel
          </Button>
          <Button onClick={save} color="primary">
            Save
          </Button>
        </DialogActions>
      </Dialog>
    </div>
  );
}