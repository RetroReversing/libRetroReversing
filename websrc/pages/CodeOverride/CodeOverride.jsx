import React, { Fragment, useState, useEffect } from 'react';
import ReactDOM from 'react-dom';
import { TextField, Paper, Grid, Button, Container, FormControlLabel, Switch } from '@material-ui/core';
import SaveIcon from '@material-ui/icons/Save';
import { sendActionToServer } from '../../server';

export function CodeOverride({ memory }) {
  const [enabled, setEnabled] = useState(window.allInformation?.overrides?.CD?.[memory.filename]?.enabled || false);
  const [code, setCode] = useState(
    window.allInformation?.overrides?.CD?.[memory.filename]?.code || "// You have access to variables: lba, buffer, memset(start, length, value)"
  );
  const overrideType = "CD";

  function saveOverride() {
    const start = memory.lba_start || memory.start;
    const end = memory.lba_end || memory.end;
    const payload = {
      category: 'modify_override',
      state: {
        overrideType, 
        name: memory.filename || memory.name,
        code,
        start, end, enabled
      },
    };
    sendActionToServer(payload).then((result)=> {
      console.error("result:", result);
      window.allInformation = result;
    });
  }

  useEffect(()=> {
    saveOverride();
  }, [enabled]);

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
          setCode(e.target.value);
        }}
      />
      <Paper elevation={0}>
        <FormControlLabel
          control={<Switch checked={enabled} onChange={(e)=>setEnabled(e.target.checked)} name="enabled" />}
          label="Enabled"
        />
        <Button variant="contained" color="primary" onClick={saveOverride}>
          <SaveIcon /> Save
        </Button>
      </Paper>
    </Container>
  );
  // return <div>Code Override</div>
}
