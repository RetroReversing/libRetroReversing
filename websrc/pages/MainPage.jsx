import React, { Fragment, useState, useEffect } from 'react';
import ReactDOM from 'react-dom';
import PropTypes from "prop-types";

import Typography from '@material-ui/core/Typography';
import Card from '@material-ui/core/Card';
import CardContent from '@material-ui/core/CardContent';
import Grid from '@material-ui/core/Grid';

import SaveStateList from '../main/SaveStateList';
import { sendActionToServer } from '../server';

function _MainPage() {
  const [mainState, setMainState] = useState({ 
    current_state: 1,
    last_state: 2,
    save_states: [
      {name: "Start of Game", frame: "0"},
      {name: "Start of Level 1", frame: "3224"},
      {name: "Start of Level 2", frame: "6324"},
    ]
  });

  const current_state = mainState.save_states[mainState.current_state];
  const last_state = mainState.save_states[mainState.last_state];

  const current_state_card = (
    <Grid item xs={12} md={6}>
      <Card>
        <CardContent>
          <Typography color="textSecondary" gutterBottom>
            Current State
          </Typography>
          <Typography variant="h5" component="h2">
          {current_state.name}
          </Typography>
          <Typography color="textSecondary" gutterBottom>
          Frame {current_state.frame} out of {last_state.frame}
          </Typography>
        </CardContent>
      </Card>
    </Grid>
  );

  useEffect(()=>{
    console.error("Requesting Main State:", mainState);
    const payload = {
      category: 'main_states',
      state: mainState
    };
    sendActionToServer(payload);
  }, []);

  return (
    <div>
      <Grid container spacing={2}>
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6">History</Typography>
              <SaveStateList save_states={mainState.save_states} last_frame={last_state.frame} />
            </CardContent>
          </Card>
        </Grid>
        {current_state_card}
      </Grid>
    </div>
  );
}

_MainPage.propTypes = {

}

export const MainPage = React.memo(_MainPage);
