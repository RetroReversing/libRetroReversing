import React, { EffectCallback, useEffect, useMemo, useState } from "react";
import Typography from '@material-ui/core/Typography';
import { useDropzone } from 'react-dropzone';
import { Box, List, ListItem } from '@material-ui/core';
import { extensions, systems } from "./emulators";
import { MTY, MTY_StrToC, MTY_Alloc, MTY_Start, MTY_Stop } from "./matoya";
import { JUN_ReadFile, RE_ReadFileFromJS, JUN_WriteFile, JUN_WriteFileFromJS, RE_getAllLocalGames } from "./database";
import { useEffectOnce } from "react-use";

const baseStyle = {
    flex: 1,
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    padding: '20px',
    borderWidth: 2,
    borderRadius: 2,
    borderColor: '#eeeeee',
    borderStyle: 'dashed',
    backgroundColor: '#fafafa',
    color: '#bdbdbd',
    outline: 'none',
    transition: 'border .24s ease-in-out',
  };
  
  const activeStyle = {
    borderColor: '#2196f3',
  };
  
  const acceptStyle = {
    borderColor: '#00e676',
  };
  
  const rejectStyle = {
    borderColor: '#ff1744',
  };

export function GameDropZone() {
  const [localGames, setLocalGames] = useState([]);
  const { acceptedFiles, fileRejections, getRootProps, getInputProps, isDragActive, isDragReject, isDragAccept } = useDropzone({
    accept: '.sms,.gg',
  });

  useEffectOnce(loadLocalGames(setLocalGames))

  const style:any = useMemo(
    () => ({
      ...baseStyle,
      ...(isDragActive ? activeStyle : {}),
      ...(isDragAccept ? acceptStyle : {}),
      ...(isDragReject ? rejectStyle : {}),
    }),
    [isDragActive, isDragReject, isDragAccept],
  );

  return (
    <Box style={{ marginTop: 20 }}>
      <Box {...getRootProps({ style })}>
        <input {...getInputProps()} />
        <Typography>Drag 'n' drop some games here, or click to select files</Typography>
        <em>(Only *.sms and *.gg ROMs will be accepted)</em>
      </Box>
     {localGames.length === 0 ? null : <aside>
        <Typography component="h4" variant="inherit">
          Your Games
        </Typography>
        <em>These games are stored in your Browsers local storage</em>
        <ul>
          {acceptedFiles.map(file => <li key={file.name} ><a href="#" onClick={()=>startEmulator(file.name)}>{file.name}</a></li>)}
          {localGames.map(gameName => <li key={gameName} ><a href="#" onClick={()=>startEmulator(gameName.split("/")[1])}>{gameName}</a></li>)}
        </ul>
      </aside>}
    </Box>
  );
}
;

const _fetch = window.fetch;
function loadLocalGames(setLocalGames) {
  return function _loadLocalGames() {

    (async () => {
      const allGames = await RE_getAllLocalGames();
      const localGames = allGames.map(game => game.path);
      console.error("get local games", localGames);
      setLocalGames(localGames);
    })();
  };
}

async function customFetch (input: any, init) {
  //Returning the local game file if the request matches
  if (input.includes(`/games/`)) {
    
    // return response;
    // return new Promise(resolve => resolve(response));
    return new Promise(resolve => {
      
      (async () => {const game_path = input.split("/games/")[1];
      const game_binary = await RE_ReadFileFromJS(game_path);

      if (!game_binary) {
        console.error("Game binary was null");
        return;
      }
      console.error("game_binary:", game_binary);
      let game_file_name = game_path.split("/")[1];
      // game_file_name = "Columns (USA, Europe).gg";
      // console.error("GAME APTH:", game_file_name, "Columns (USA, Europe).gg");
      // debugger;
      const file2: any = new File([game_binary.data], ""+game_file_name);
      const response = new Response(file2, { status: 200 });
      resolve(response);})();
    });
  }

  return _fetch(input, init);
}

let startedEmulator = false;
function startEmulator(game_name) {
  if (startedEmulator) { 
    stop();
    // return; 
  }
  const extension = game_name.substring(game_name.lastIndexOf('.') + 1);
  const system    = extensions[extension];
  const core      = systems[system];
  window['fetch'] = customFetch as any;
    
  try {
    run(system, core, game_name);
  } catch(e) {
    console.error("Error running core:", e)
    startedEmulator = false;
  }
  startedEmulator = true;
}

function stopEmulator() {
  console.error("How do we stop the emulator from running the wasm?");
}

function mapAcceptedFiles(): (value: any, index: number, array: File[]) => JSX.Element {
  
  
  
  return file => { 
    console.info("accepted ROM:", file);
    const game      = file.name;
    const extension = game.substring(game.lastIndexOf('.') + 1);
    const system    = extensions[extension];
    const core      = systems[system];
    
    JUN_WriteFileFromJS(file, system);
    // startEmulator(file.name);
    
    return (
    <ListItem key={file.path}>
     {game} {extension} {system} {file.path} - {file.size} bytes
    </ListItem>
  );}
}

function mapFileRejection() {
    return ({ file, errors }) => { 
        console.error(file, errors);
        return (
        <ListItem key={file.path}>
            {file.path} - {file.size} bytes
            <List>
                {errors.map(e => (
                    <ListItem key={e.code}>{e.message}</ListItem>
                ))}
            </List>
        </ListItem>
    );}
}

let frontend_status = { paused: false, startAt: 0 };
        

function run(system, core, game) {
    //Start core execution
    MTY_Start(`/${core.library}.wasm`, {
        js_get_host:  (value, length) => MTY_StrToC(window.location.hostname, value, length),
        js_get_port:  ()              => window.location.port ?? 0,
        js_is_secure: ()              => location.protocol.indexOf('https') != -1,

        js_get_system: (value, length) => MTY_StrToC(system,    value, length),
        js_get_core:   (value, length) => MTY_StrToC(core.name, value, length),
        js_get_game:   (value, length) => MTY_StrToC(game,      value, length),

        js_read_file:  JUN_ReadFile,
        js_write_file: JUN_WriteFile,
        retro_deinit: ()=> console.log("retro_deinit"),
        get_frontend_status: ()=> {
            const json_string = JSON.stringify(frontend_status)+"\0";
            const MAX_COMMAND_LENGTH = 1024;
            MTY.libRRcommand = MTY_Alloc(MAX_COMMAND_LENGTH);
            if (json_string.length >MAX_COMMAND_LENGTH ) {
                console.error("Warning: please increase MAX_COMMAND_LENGTH to at least:", json_string.length);
            }
            const c_str = MTY_StrToC(json_string,MTY.libRRcommand, json_string.length);
            console.log("get_frontend_status", json_string, c_str);
            return c_str;
        }
    });
    

    //Prevent mobile keyboard
    MTY.clip.readOnly = true;
}

function stop() {
  MTY_Stop();
}