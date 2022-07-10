import React, { EffectCallback, useEffect, useMemo, useState } from "react";
import Typography from '@material-ui/core/Typography';
import { useDropzone } from 'react-dropzone';
import { Box, List, ListItem } from '@material-ui/core';
import { extensions, systems } from "./emulators";
import { MTY, MTY_StrToC, MTY_StrToJS, MTY_Alloc, MTY_Start, MTY_Stop } from "./matoya";
import { JUN_ReadFile, RE_ReadFileFromJS, JUN_WriteFile, JUN_WriteFileFromJS, RE_getAllLocalGames, RE_ReadRomFromJS } from "./database";
import { useEffectOnce } from "react-use";
import {
  useHistory
} from "react-router-dom";
import { blue } from "@material-ui/core/colors";
import { noop } from "lodash";
import settings from "../settings.json";

window["loadedGames"] = {};
window["Buffer"] = Buffer;


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

  // GameDropZone Component
export function GameDropZone() {
  let history = useHistory();
  const [currentConsole, setCurentConsole] = useState("gg");
  const [localGames, setLocalGames] = useState([]);
  const { acceptedFiles, fileRejections, getRootProps, getInputProps, isDragActive, isDragReject, isDragAccept } = useDropzone({
    accept: '.sms,.gg',
  });

  useEffect(()=> {
    console.info("Accepted files are now:", acceptedFiles);
    acceptedFiles.map((file) => {
      console.info("accepted ROM:", file);
      const game      = file.name;
      const extension = game.substring(game.lastIndexOf('.') + 1);
      const system    = extensions[extension];
      const core      = systems[system];
      
      JUN_WriteFileFromJS(file, system);
    } )
  }, [acceptedFiles])

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

  function setPath(gameName) {
    const newPath = "/"+currentConsole+"/"+gameName+"/";
    history.push(newPath);
  }

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
          {localGames.map(gamePath => <li key={gamePath} style={{ color: "blue", cursor: "pointer" }} onClick={()=>{
            const gameName = gamePath.split("/")[1];
            setCurentConsole("gg");
            setPath(gameName); 
            startEmulator(gameName)
          }}>{gamePath}</li>)}
        </ul>
      </aside>}
      <button onClick={pauseGame}>Pause</button>
    </Box>
  );
}
;

function pauseGame() {
  frontend_status.paused = !frontend_status.paused;
  sendMessageToCoreFromFrontend({ category: "pause", state: { paused: frontend_status.paused, fullLogging: false } })
}

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

export async function loadFileFromLocalStorage(path, file_name) {
  const binary_file = await RE_ReadFileFromJS(path);
  if (!binary_file) {
    console.error("Binary file was null for ", file_name, "path:", path);
    return;
  }

  return Buffer.from(binary_file.data, "base64").buffer;
}

// customFetch overrides the default browser fetch, to make sure maytoya loads the game from the browser rather than a URL request
async function customFetch (input: any, init) {
  if (input.includes("settings.json")) {
    return Promise.resolve(new Response(JSON.stringify(settings), { status: 200 }));
  }
  // Returning the local game file if the request matches
  if (input.includes(`/games/`)) {
    
    // return response;
    // return new Promise(resolve => resolve(response));
    return new Promise(resolve => {
      
      (async () => {
        const game_path = input.split("/games/")[1];
        const game_binary = await RE_ReadRomFromJS(game_path);

      if (!game_binary) {
        console.error("Game binary was null");
        return;
      }
      console.error("game_binary:", game_binary);
      let game_file_name = game_path.split("/")[1];
      const file2: any = new File([game_binary.data], ""+game_file_name);
      const response = new Response(file2, { status: 200 });
      resolve(response);
    })();
    });
  }

  return _fetch(input, init);
}

let startedEmulator = false;
function startEmulator(game_name, callback=noop) {
  if (startedEmulator) { 
    stop();
    // return; 
  }
  const extension = game_name.substring(game_name.lastIndexOf('.') + 1);
  const system    = extensions[extension];
  const core      = systems[system];
  window['fetch'] = customFetch as any;
    
  try {
    window["loadedGames"][game_name] = true;
    run(system, core, game_name).then(callback);
  } catch(e) {
    console.error("Error running core:", e)
    startedEmulator = false;
  }
  startedEmulator = true;
  return startedEmulator;
}
window["startEmulator"] = startEmulator;

// TODO: need to hook this up to be changed instead of postResponse
let frontend_status = { paused: false, startAt: 0 };
let game_json = { current_state: { memory_descriptors: []}, playthrough: {}, cd_tracks: [], functions: {}, function_usage: {} };

function getFrontendStatus() {
  return frontend_status;
}

export function sendMessageToCoreFromFrontend(json) {
  const resultString = libRR_parse_message_from_emscripten(JSON.stringify(json)+"\0");
  try {
  return JSON.parse(resultString);
  } catch (e) {
    return {};
  }
}

// TODO: only set these when we are sure we are only using WASM cores
window["isWASM"] = false; // Initially always false, until the first call fails
window["sendMessageToCoreFromFrontend"] = sendMessageToCoreFromFrontend;
window["getFrontendStatus"] = getFrontendStatus;

function libRR_parse_message_from_emscripten(json_string = '{ "category": "play", "state": { "startAt": 0, "paused": false } }\0') {
  if (!MTY.module) {
    console.info("MTY might not be initiated yet", json_string);
    return ""; //game_json;
  }
  MTY.libRRcommand = MTY_Alloc(json_string.length);
  const c_str = MTY_StrToC(json_string, MTY.libRRcommand, json_string.length);
  const result = MTY.module.instance.exports.libRR_parse_message_from_emscripten(c_str);
  console.log("Result back from core, pointer:", result, "String:", MTY_StrToJS(result), "Payload: ",json_string);
  return MTY_StrToJS(result);
}

function handleServerAction(payload) {
  console.error("handleServerAction in the frontend", payload);
  return Promise.resolve(libRR_parse_message_from_emscripten(payload));

}
window["handleServerAction"] = handleServerAction;
window["hasInit"] = false;

function run(system, core, game) {
    //Start core execution
    const promise = MTY_Start(`/${core.library}.wasm`, {
        js_get_host:  (value, length) => MTY_StrToC(window.location.hostname, value, length),
        js_get_port:  ()              => window.location.port ?? 0,
        js_is_secure: ()              => location.protocol.indexOf('https') != -1,

        js_get_system: (value, length) => MTY_StrToC(system,    value, length),
        js_get_core:   (value, length) => MTY_StrToC(core.name, value, length),
        js_get_game:   (value, length) => MTY_StrToC(game,      value, length),

        js_read_file:  JUN_ReadFile,
        js_write_file: JUN_WriteFile,
        retro_deinit: ()=> console.log("retro_deinit"),
        get_frontend_status: () => {
            const json_string = JSON.stringify(frontend_status)+"\0";
            const MAX_COMMAND_LENGTH = 1024;
            MTY.libRRcommand = MTY_Alloc(MAX_COMMAND_LENGTH);
            if (json_string.length >MAX_COMMAND_LENGTH ) {
                console.error("Warning: please increase MAX_COMMAND_LENGTH to at least:", json_string.length);
            }
            const c_str = MTY_StrToC(json_string,MTY.libRRcommand, json_string.length);
            // console.log("get_frontend_status", json_string, c_str);
            return c_str;
        }
    });
    
    return promise;
    //Prevent mobile keyboard
    // MTY.clip.readOnly = true;
}

function stop() {
  MTY_Stop();
}