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
import InputLabel from '@material-ui/core/InputLabel';
import Select from '@material-ui/core/Select';
import Box from '@material-ui/core/Box';


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
          <Box>
            <FormControl mr={20} style={{marginRight: 20}}>
              <InputLabel htmlFor="start-from">Start from</InputLabel>
              <Select
                native
                value={"Start of Game"}
                onChange={()=>null}
                inputProps={{
                  name: 'start',
                  id: 'start-from',
                }}
              >
                <option aria-label="Start of Game" value="Start of Game">Start of Game</option>
                <option value={10}>State 1</option>
              </Select>
            </FormControl>
            <FormControl style={{marginRight: 20}}>
              <InputLabel htmlFor="end-at">End at</InputLabel>
              <Select
                native
                value={"Keep going"}
                onChange={()=>null}
                inputProps={{
                  name: 'end',
                  id: 'end-at',
                }}
              >
                <option value="Manual Pause">Keep going</option>
                <option value={10}>State 1</option>
              </Select>
            </FormControl>
            <FormControl style={{marginRight: 20}}>
              <InputLabel htmlFor="on-end">On End</InputLabel>
              <Select
                native
                value={"Pause game"}
                onChange={()=>null}
                inputProps={{
                  name: 'on-end',
                  id: 'on-end',
                }}
              >
                <option value="Loop from start checkpoint">Loop from start checkpoint</option>
                <option value="Loop from start of game">Loop from start of game</option>
                <option value="Pause game">Pause game</option>
              </Select>
            </FormControl>
            <FormControl style={{marginRight: 20, minWidth:200, marginTop: 20}}>
              <InputLabel htmlFor="game-speed">Game Speed</InputLabel>
              <Select
                native
                ml={5}
                value={playerState.speed}
                onChange={(e)=>setPlayerState({
                  ...playerState, speed:+e.target.value
                })}
                inputProps={{
                  name: 'speed',
                  id: 'game-speed',
                }}
              >
                <option value={100}>100%</option>
                <option value={90}>90%</option>
                <option value={75}>75%</option>
                <option value={50}>50%</option>
                <option value={25}>25%</option>
                <option value={10}>10%</option>
                <option value={1}>1%</option>
              </Select>
            </FormControl>
          </Box>
          
          <Box mt={5}>
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
        </Box>

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