import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import Button from "@material-ui/core/Button";
import Grid from '@material-ui/core/Grid';
import axios from "axios";
import PauseIcon from '@material-ui/icons/Pause';
import PlayArrowIcon from '@material-ui/icons/PlayArrow';
import RestoreIcon from '@material-ui/icons/Restore';
import StopIcon from '@material-ui/icons/Stop';
import SettingsIcon from '@material-ui/icons/Settings';
import SportsEsportsIcon from '@material-ui/icons/SportsEsports';

function sentActionToServer(payload) {
  axios.post('/postresponse', payload)
  .then(function (response) {
    console.log(response);
  })
  .catch(function (error) {
    console.log(error);
  });
}

export function PlaySettings() {
  const [playerState, setPlayerState] = useState({ 
    paused: true, 
    logButtons: false,
    recordInput: false,
    playbackLogged: false
  });

  function pause() {
    const newPlayerState = {...playerState, paused:!playerState.paused};
    setPlayerState(newPlayerState);
  }

  function changeLogButtons() {
    const newPlayerState = {...playerState, logButtons:!playerState.logButtons};
    setPlayerState(newPlayerState);
  }

  useEffect(()=>{
    console.info("Sending state:", playerState);
    const payload = {
      category: 'player_settings',
      state: playerState
    };
    sentActionToServer(payload);
  }, [playerState]);

  return <Fragment> 
  <Grid container justify="flex-end" spacing={2}>
    <Grid item>
      <Button startIcon={playerState.paused?<PlayArrowIcon />:<PauseIcon />} spacing={2} color="inherit" onClick={pause}>
      {playerState.paused? "Resume":"Pause"}
      </Button>
    </Grid>

    <Grid item>
      <Button color="inherit" startIcon={<RestoreIcon />}>
      Restart
    </Button>
    </Grid>

    <Grid item>
      <Button color="inherit" startIcon={<StopIcon />}>
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