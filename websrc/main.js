import React, {Fragment} from "react";
import ReactDOM from "react-dom";
import Button from "@material-ui/core/Button";
import { PlaySettings } from "./debugger/PlaySettings";

function App() {
  return (<Fragment>
    <PlaySettings />
  </Fragment>);
}

ReactDOM.render(<App />, document.querySelector("#app"));
