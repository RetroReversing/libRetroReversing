import React from "react";
import Select from '@material-ui/core/Select';
import MenuItem from '@material-ui/core/MenuItem';
import FormControl from '@material-ui/core/FormControl';
import InputLabel from '@material-ui/core/InputLabel';
import TripeDotDropDownMenu from "../pages/util/TripeDotDropDownMenu";

export function PlaythroughSelector(props) {
  return (<div style={{ display: 'flex', float: 'right' }}><FormControl style={{
    minWidth: 200,
    marginBottom: 20,
    float: 'right'
  }}>
    <InputLabel id="playthrough-selector-label">Playthrough</InputLabel><Select
      labelId="playthrough-selector-label"
      id="playthrough-selector"
      value={"Initial"}
      onChange={(v) => console.log("Value changed:", v)}
    >
      <MenuItem value="Initial">Initial </MenuItem>
      <MenuItem value="Full">Full Playthrough</MenuItem>
      <MenuItem value="Speedrun">Speed run</MenuItem>
    </Select></FormControl> <TripeDotDropDownMenu options={["Create New Playthrough", "Import button.log", "Import Bizhawk TAS"]} /></div>);
}
