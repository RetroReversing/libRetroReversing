import React, { Fragment, useState, useEffect } from 'react';
import ReactDOM from 'react-dom';
import { TextField, Paper, Grid, Button, Container } from '@material-ui/core';
import SaveIcon from '@material-ui/icons/Save';

export function CodeOverride() {
  const [code, setCode] = useState(
    "console.log('Executed with parameters', params)"
  );

  return (
    <Container>
      <TextField
        id="standard-multiline-flexible"
        label="Code to run on file access to override content "
        multiline
        row={10}
        rowsMax={40}
        style={{ width: '100%', paddingBottom: 20 }}
        value={code}
        onChange={(e) => {
          console.error('Changed Override Code:', e.target.value);
          setCode(e.target.value);
        }}
      />
      <Paper elevation={0}>
        <Button variant="contained" color="primary">
          <SaveIcon /> Save
        </Button>
      </Paper>
    </Container>
  );
  // return <div>Code Override</div>
}
