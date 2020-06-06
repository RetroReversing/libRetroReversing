import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import Editor from "rich-markdown-editor";
import { Container, Paper, FormControlLabel, Switch, Button } from '@material-ui/core';
import SaveIcon from '@material-ui/icons/Save';
import { sendActionToServer } from '../../server';

export function NotesViewer( { memory }) {

  const [notes, setNotes] = useState(window.allInformation?.notes?.CD?.[memory.filename]?.notes || "Notes go here");
  const overrideType = "CD";

  function saveNote() {
    const payload = {
      category: 'modify_note',
      state: {
        overrideType, 
        name: memory.filename || memory.name,
        notes
      },
    };
    sendActionToServer(payload);
  }

  return (<Container><Editor
  defaultValue={notes}
  onChange={(e)=> setNotes(e())}
/>
<Paper style={{ marginTop: 30, display: 'flex', justifyContent:'flex-end'}} elevation={0}>
        <Button variant="contained" color="primary" onClick={saveNote}>
          <SaveIcon /> Save
        </Button>
      </Paper>
</Container>)
}