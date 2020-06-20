import React from 'react';
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem';
import Divider from '@material-ui/core/Divider';
import ListItemText from '@material-ui/core/ListItemText';
import ListItemAvatar from '@material-ui/core/ListItemAvatar';
import Avatar from '@material-ui/core/Avatar';
import Typography from '@material-ui/core/Typography';
import IconButton from '@material-ui/core/IconButton';
import CommentIcon from '@material-ui/icons/Comment';
import ListItemSecondaryAction from '@material-ui/core/ListItemSecondaryAction';
import MoreHorizIcon from '@material-ui/icons/MoreHoriz';
import { reverse, noop, sortBy } from "lodash";

import GridList from '@material-ui/core/GridList';
import GridListTile from '@material-ui/core/GridListTile';
import GridListTileBar from '@material-ui/core/GridListTileBar';
import ListSubheader from '@material-ui/core/ListSubheader';
import InfoIcon from '@material-ui/icons/Info';
import ReplayIcon from '@material-ui/icons/Replay';
import DeleteIcon from '@material-ui/icons/Delete';
import { Grid } from '@material-ui/core';


export function createSaveStateTile(tile,load_state, prefix="", delete_state=noop) {

  return (<GridListTile key={tile.frame}>
    <img src={"/game/playthroughs/Initial%20Playthrough/save_"+tile.frame+".sav.png"} alt={tile.name} />
    <GridListTileBar
      title={prefix+tile.name}
      subtitle={<span>frame: {tile.frame}</span>}
      actionIcon={
        <Grid>
        <IconButton size="small" color="primary" aria-label={`info about ${tile.name}`}>
          <ReplayIcon onClick={()=>load_state(tile.frame)} />
        </IconButton>
        <IconButton size="small" color="secondary" aria-label={`info about ${tile.name}`}>
          <DeleteIcon onClick={()=>delete_state(tile.frame)} />
        </IconButton>
        </Grid>
      }
    />
  </GridListTile>);
}
export default function SaveStateList({save_states, last_frame, load_state, delete_state }) {

  if (!save_states) {
    return null;
  }

  const sorted_save_states = sortBy(save_states, (state)=>state.frame);
  const save_states_reversed = reverse(sorted_save_states);

  return (
    <div>
      <GridList cellHeight={180} >
        <GridListTile key="Subheader" cols={2} style={{ height: 'auto' }}>
          <ListSubheader component="div"></ListSubheader>
        </GridListTile>
        {save_states_reversed.map((el)=>createSaveStateTile(el,load_state, "", delete_state))}
      </GridList>
    </div>
  );
}
