import React, { useState } from 'react';
import Button from '@material-ui/core/Button';
import TextField from '@material-ui/core/TextField';
import Dialog from '@material-ui/core/Dialog';
import DialogActions from '@material-ui/core/DialogActions';
import DialogContent from '@material-ui/core/DialogContent';
import DialogContentText from '@material-ui/core/DialogContentText';
import DialogTitle from '@material-ui/core/DialogTitle';
import { sendActionToServer } from '../server';

export default function PauseSaveDialog( { setCurrentDialog, open = true }) {

  const [saveStateName, setSaveStateName] = useState('');

  const handleClose = () => {
    setCurrentDialog('');
  };

  function saveState() {
    if (saveStateName.length < 1) {

    }
    console.error("Going to save with name:", saveStateName);
    const payload = {
      category: 'save_state',
      state: {
        name: saveStateName
      }
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
            If you would like to save a state here enter a name to describe where you are in the game.
          </DialogContentText>
          <TextField
            autoFocus
            margin="dense"
            id="state_name"
            label="State Name"
            type="email"
            fullWidth
            value={saveStateName}
            onChange={(e)=>setSaveStateName(e.target.value)}
          />
        </DialogContent>
        <DialogActions>
          <Button onClick={handleClose} color="error">
            Go back to previous State
          </Button>
          <Button onClick={saveState} color="primary">
            Save to Playthrough
          </Button>
        </DialogActions>
      </Dialog>
    </div>
  );
}