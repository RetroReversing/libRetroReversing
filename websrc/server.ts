import axios from "axios";
import { loadFileFromLocalStorage, sendMessageToCoreFromFrontend } from "./home/GameDropZone";

export let gameLoaded = false;

export function sendActionToServer(payload) {
  if (window["isWASM"]) {
    const result = sendMessageToCoreFromFrontend(payload);
    return Promise.resolve(result);
  }
  return axios.post('/postresponse', payload)
  .then(function (response) {
    console.info("Response from emulator:",response);
    gameLoaded = true; // TODO: use the response to set this to the correct value
    return response.data;
  })
  .catch(function errorCallingEmulator(error) {
    console.error("Error calling emulator:",error);
    console.info("External emulator not found, using internal wasm emulator instead");
    window["isWASM"] = true; 
    gameLoaded = false;
  });
}

export function loadState(frame) {
  console.log("About to load state:", frame);
  const payload = {
    category: 'load_state',
    state: {
      frame
    },
  };
  sendActionToServer(payload);
};

export function requestFileFromServer(filename, playthrough_name = "Initial") {
  // TODO: get file from localstorage
  const last_part_of_path = `/playthroughs/${playthrough_name}/${filename}`;
  if (window["isWASM"]) {
    console.info("We are running in WASM mode to requesting file from localStorage instead")
    // TODO: need to get full path such as "/system/Genesis Plus GX/RE_projects/GameGear/MegaManUSA//playthroughs/Initial/button_log.bin"	
    const full_path = "/system/Genesis Plus GX/RE_projects/GameGear/MegaManUSA/"+ last_part_of_path;
    return loadFileFromLocalStorage(full_path, filename);
  }
  return axios.get("/game" + last_part_of_path, { responseType: 'arraybuffer' })
  .then(function (response) {
    console.info("Response from emulator:",response);
    return response.data;
  })
  .catch(function (error) {
    console.error("Error calling emulator:",error);
  });
}