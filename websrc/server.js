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