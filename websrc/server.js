import axios from "axios";

export let emulatorType = "desktop"; // either "desktop" or "browser" depending on where the user is emulating from
export let gameLoaded = false;

export function sendActionToServer(payload) {
  if (window.sendMessageToCoreFromFrontend) {
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
    emulatorType = "browser";
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
  return axios.get(`/game/playthroughs/${playthrough_name}/${filename}`, { responseType: 'arraybuffer' })
  .then(function (response) {
    console.info("Response from emulator:",response);
    return response.data;
  })
  .catch(function (error) {
    console.error("Error calling emulator:",error);
  });
}