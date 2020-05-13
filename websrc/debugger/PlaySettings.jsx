import React, { useState, useEffect } from "react";
import ReactDOM from "react-dom";
import Button from "@material-ui/core/Button";
import axios from "axios";

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
  const [playerState, setPlayerState] = useState({ paused: false });

  function pause() {
    const newPlayerState = {...playerState, paused:!playerState.paused};
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

  return  <Button variant="contained" color="primary" onClick={pause}>
  {playerState.paused? "Resume":"Pause"}
</Button>
}