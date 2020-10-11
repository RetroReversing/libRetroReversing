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

function baseName(str) {
  if (!str.includes("/")) { return str; }
  return str.substr(str.lastIndexOf("/") + 1);
}

export default function LoadLinkerMapFileDialog( { currentDialogParameters, setCurrentDialog, playerState, setPlayerState, open = true }) {
  const [functionSettings, setFunctionSettings] = React.useState(currentDialogParameters);
  const [mapContents, setMapContents] = React.useState({});

  const handleClose = () => {
    setCurrentDialog('');
  };

  function save() {
    
    console.error("Going to upload map with settings:", mapContents);
    // const newPlayerState = {...playerState, paused:false};
    // setPlayerState(newPlayerState);

    const payload = {
      category: 'upload_linker_map',
      state: mapContents
    };
    sendActionToServer(payload);
    handleClose();
  }

  function readFile(files) {
    var reader = new FileReader();
    console.error("Read File", files);
    reader.onload = function(progressEvent){
      const libraryObjects = {};
      const libraryFunctions = {};

      // By lines
      var lines = this.result.split('\n');
      var currentCategory = "";
      let currentObjectFile = "";
      for(var line = 0; line < lines.length; line++){

        const lineContent = lines[line].trim();
        if (lineContent === "Linker script and memory map") {
          currentCategory = "mem_map";
          continue;
        }
        if (currentCategory === "" || lineContent === "") {
          continue;
        }

        if (lineContent.includes('PROVIDE ')) {
          continue;
        }

        if (lineContent.startsWith(".")) {
          const [section, location, length, ...path] = lineContent.split(" ").filter((item)=>item!=="");
          // const [location, length, ...path] = lineContent.split(" ").filter((item)=>item!=="");
          let library_name = baseName(path.join(' '));
          libraryObjects[location] = { name: library_name, length}
          console.error("Object file line:", libraryObjects[location], { section, location, length } );
          // Object file
          currentObjectFile=library_name;
          continue;
        }
        
        if (lineContent.startsWith("0x")) {
          const [location, ...func_name] = lineContent.split(" ").filter((item)=>item!=="");
          libraryFunctions[location] = { name: func_name.join(" "), file: currentObjectFile}
          // console.error("Function:", {location, func_name, currentObjectFile}, libraryFunctions[location])
          continue;
        }

        // console.log(lineContent);
      }
      setMapContents({ libraryFunctions, libraryObjects });
    };
    reader.readAsText(files[0]);
  }

  return (
    <div>
      <Dialog open={open} onClose={handleClose} aria-labelledby="form-dialog-title">
        <DialogTitle id="form-dialog-title">Load Linker Map File</DialogTitle>
        <DialogContent>
          <DialogContentText>
            Load a *.MAP file generated from a GCC Linker to rename all functions in this project
          </DialogContentText>

        <input
          accept="*.map"
          id="contained-button-file"
          multiple
          type="file"
          onChange={(e)=>readFile(e.target.files)}
        />
        {/* <label htmlFor="contained-button-file">
          <Button variant="contained" color="primary" component="span">
            Upload
          </Button>
        </label> */}
          
        </DialogContent>
        <DialogActions>
          <Button onClick={handleClose} color="error">
            Cancel
          </Button>
          <Button onClick={save} color="primary">
            Save
          </Button>
        </DialogActions>
      </Dialog>
    </div>
  );
}