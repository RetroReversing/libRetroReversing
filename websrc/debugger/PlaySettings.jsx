import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import Button from "@material-ui/core/Button";
import Grid from '@material-ui/core/Grid';
import PauseIcon from '@material-ui/icons/Pause';
import PlayArrowIcon from '@material-ui/icons/PlayArrow';
import RestoreIcon from '@material-ui/icons/Restore';
import StopIcon from '@material-ui/icons/Stop';
import SettingsIcon from '@material-ui/icons/Settings';
import SportsEsportsIcon from '@material-ui/icons/SportsEsports';
import { sendActionToServer } from '../server';

export function PlaySettings( { setCurrentDialog }) {
  const [playerState, setPlayerState] = useState({ 
    paused: true, 
    logButtons: false,
    recordInput: false,
    playbackLogged: false
  });

  function play_or_pause() {
    if (!playerState.paused) {
      // we are just about to pause so lets ask if they want to create a save state
      setCurrentDialog('pause_save');
    }

    const newPlayerState = {...playerState, paused:!playerState.paused};
    setPlayerState(newPlayerState);

    const category = newPlayerState.paused? "pause":"play";

    const payload = {
      category,
      state: newPlayerState
    };
    sendActionToServer(payload);
  }

  function restart() {
    const payload = {
      category: 'restart',
      state: playerState
    };
    sendActionToServer(payload);
  }

  function stop() {
    const payload = {
      category: 'stop',
      state: playerState
    };
    sendActionToServer(payload);
  }

  function changeLogButtons() {
    const newPlayerState = {...playerState, logButtons:!playerState.logButtons};
    setPlayerState(newPlayerState);
  }


  return <Fragment> 
  <Grid container justify="flex-end" spacing={2}>
    <Grid item>
      <Button startIcon={playerState.paused?<PlayArrowIcon />:<PauseIcon />} spacing={2} color="inherit" onClick={play_or_pause}>
      {playerState.paused? "Resume":"Pause"}
      </Button>
    </Grid>

    <Grid item>
      <Button color="inherit" startIcon={<RestoreIcon />} onClick={restart}>
      Restart
    </Button>
    </Grid>

    <Grid item>
      <Button color="inherit" startIcon={<StopIcon />} onClick={stop}>
      Stop
    </Button>
    </Grid>

    <Grid item>
      <Button color="inherit" onClick={changeLogButtons} startIcon={<SportsEsportsIcon />}>
      {playerState.logButtons? "Stop Logging Buttons":"Start Logging Buttons"}
      </Button>
    </Grid>

    <Grid item>
      <Button color="inherit" startIcon={<SettingsIcon />}>
      Settings
      </Button>
    </Grid>
  </Grid>
</Fragment>
}