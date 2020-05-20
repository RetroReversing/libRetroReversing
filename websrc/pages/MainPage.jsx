import React, { Fragment, useState, useEffect } from 'react';
import ReactDOM from 'react-dom';
import PropTypes from "prop-types";

import Typography from '@material-ui/core/Typography';
import Card from '@material-ui/core/Card';
import CardContent from '@material-ui/core/CardContent';
import Grid from '@material-ui/core/Grid';

import SaveStateList from '../main/SaveStateList';
import { sendActionToServer } from '../server';

function _MainPage( { fullState }) {

  let current_state_card = null;
  
  if (fullState) { 
    current_state_card=(<Grid item xs={12} md={6}>
        <Card>
          <CardContent>
            <Typography color="textSecondary" gutterBottom>
              Current State
            </Typography>
            <Typography variant="h5" component="h2">
            {fullState?.playthrough?.current_state?.name}
            </Typography>
            <Typography color="textSecondary" gutterBottom>
            Frame {fullState?.playthrough?.current_state?.frame} out of {fullState?.playthrough?.last_frame}
            </Typography>
          </CardContent>
        </Card>
      </Grid>
    );
  }

  return (
    <div>
      <Grid container spacing={2}>
        <Grid item xs={12} md={6}>
          <Card>
            <CardContent>
              <Typography variant="h6">History</Typography>
              <SaveStateList save_states={fullState?.playthrough?.states} last_frame={fullState?.playthrough?.last_frame} />
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
