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
import { n2hexstr } from '../pages/DisasmViewer/DisasmViewer';

function baseName(str) {
  if (!str.includes("/")) { return str; }
  return str.substr(str.lastIndexOf("/") + 1);
}

function parse_simple_sym_file(fileContents) {
  const libraryObjects = {}; // SYm files don't have .o files defined
  const libraryFunctions = {};
  const bank_size = 0x4000; // TODO get this from the emulator core

  // By lines
  var lines = fileContents.split('\n');
  for(var lineNumber = 0; lineNumber < lines.length; lineNumber++) {
    const line = lines[lineNumber].trim();
    if (line.startsWith(";")) {
      continue;
    }
    const [location, ...func_name] = line.split(" ").filter((item)=>item!=="");
    if (!location) {
      continue;
    }
    let [bank, addr] = location.split(":");
    bank = parseInt(bank,16);
    addr = parseInt(addr,16);

    if (bank === 0) {
      libraryFunctions[n2hexstr(addr)] = { name: func_name.join(" ")}
    } else {
      // Remember: addr starts from bank_size, so we need to subtract 1 from bank number
      const full_function_address = addr + ((bank-1)*bank_size);
      libraryFunctions[n2hexstr(full_function_address)] = { name: func_name.join(" ") }
    }
    
  }

  return { libraryObjects, libraryFunctions };
}

function parse_gcc_map_file(result) {
  const libraryObjects = {};
  const libraryFunctions = {};

  // By lines
  var lines = result.split('\n');
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
  return { libraryObjects, libraryFunctions };
}

export default function LoadLinkerMapFileDialog( { currentDialogParameters, setCurrentDialog, playerState, setPlayerState, open = true }) {
  const [functionSettings, setFunctionSettings] = React.useState(currentDialogParameters);
  const [mapContents, setMapContents] = React.useState(null);
  const [format, setFormat] = React.useState('simple');
  const [file, setFile] = React.useState(null);

  const handleClose = () => {
    setCurrentDialog('');
  };

  React.useEffect(()=> {
    console.info("Contents of map have been parsed");

    if (mapContents === null) {
      return;
    }
    console.error("Going to upload map with settings:", mapContents);

    console.info("sending contents to server", mapContents);
    const payload = {
      category: 'upload_linker_map',
      state: mapContents
    };
    sendActionToServer(payload);
    handleClose();

  }, [mapContents]);

  function save() {
    readFile(file);
  }

  function readFile(file) {
    var reader = new FileReader();
    console.info("Reading Symbol File:", file);
    reader.onload = function(progressEvent){
      let libraryFunctionsAndObjects = {};

      if (format === 'map') {
        libraryFunctionsAndObjects = parse_gcc_map_file(this.result);
      } else if (format === 'simple') {
       libraryFunctionsAndObjects = parse_simple_sym_file(this.result);
      }

      setMapContents(libraryFunctionsAndObjects);
      console.info("Finished parsing file");
    };
    reader.readAsText(file);
  }

  return (
    <div>
      <Dialog open={open} onClose={handleClose} aria-labelledby="form-dialog-title">
        <DialogTitle id="form-dialog-title">Load Debug Symbols</DialogTitle>
        <DialogContent>
          <DialogContentText>
            Load a *.MAP/*.SYM file of debug symbols to rename all functions in this project
          </DialogContentText>

      <Box mb={2}>
        <input
          accept="*.map"
          id="contained-button-file"
          multiple
          type="file"
          onChange={(e)=>setFile(e.target.files[0])}
        />
      </Box>
        <FormControl style={{marginRight: 20}}>
              <InputLabel htmlFor="format">Format</InputLabel>
              <Select
                native
                value={format}
                onChange={(e)=>setFormat(e.target.value)}
                inputProps={{
                  name: 'format',
                  id: 'format',
                }}
              >
                <option value="simple">Simple (Bank:Address SymbolName)</option>
                <option value="map">GCC Map file</option>
              </Select>
            </FormControl>
          
        </DialogContent>
        <DialogActions>
          <Button onClick={handleClose} color="error">
            Cancel
          </Button>
          <Button onClick={save} color="primary">
            Apply
          </Button>
        </DialogActions>
      </Dialog>
    </div>
  );
}