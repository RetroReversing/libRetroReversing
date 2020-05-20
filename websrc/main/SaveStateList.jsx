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
import { reverse } from "lodash";

export default function SaveStateList({save_states, last_frame}) {

  if (!save_states) {
    return null;
  }

  const save_states_reversed = reverse(save_states);
  
  return (
    <List>
      {save_states_reversed.map((state)=>{
          return (<div key={state.name}><ListItem alignItems="flex-start">
          <ListItemAvatar>
            <Avatar />
          </ListItemAvatar>
          <ListItemText
            primary={state.name}
            secondary={
              <React.Fragment>
                <Typography
                  component="span"
                  variant="body2"
                  style={{display: "inline"}}
                  color="textPrimary"
                >
                  Frame {state.frame}
                </Typography>
                {"/"+last_frame}
              </React.Fragment>
            }
          />
          <ListItemSecondaryAction>
              <IconButton edge="end" aria-label="comments">
                <MoreHorizIcon />
              </IconButton>
            </ListItemSecondaryAction>
        </ListItem>
        <Divider variant="inset" component="li" />
        </div>);
        })}
    </List>
  );
}
