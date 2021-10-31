import axios from "axios";


export function sendActionToServer(payload) {
  return axios.post('/postresponse', payload)
  .then(function (response) {
    console.info("Response from emulator:",response);
    return response.data;
  })
  .catch(function (error) {
    console.error("Error calling emulator:",error);
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