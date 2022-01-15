import React from "react";
import clsx from 'clsx';
import AppBar from '@material-ui/core/AppBar';
import Toolbar from '@material-ui/core/Toolbar';
import Typography from '@material-ui/core/Typography';
import IconButton from '@material-ui/core/IconButton';
import MenuIcon from '@material-ui/icons/Menu';
import { PlaySettings } from "../debugger/PlaySettings";
import { RRDrawer } from './Drawer';

export function RRTitleBar(classes, open, gameInformation, setCurrentDialog, playerState, setPlayerState, setCurrentTab, theme, setOpen) {

  const handleDrawerOpen = () => {
    setOpen(true);
  };

  const handleDrawerClose = () => {
    setOpen(false);
  };

  return <><AppBar position="fixed"
    className={clsx(classes.appBar, {
      [classes.appBarShift]: open,
    })}>
    <Toolbar>
      <IconButton
        color="inherit"
        aria-label="open drawer"
        onClick={handleDrawerOpen}
        edge="start"
        className={clsx(classes.menuButton, open && classes.hide)}
      >
        <MenuIcon />
      </IconButton>
      <Typography variant="h6">
        {gameInformation?.library_name}RE
      </Typography>

      <PlaySettings setCurrentDialog={setCurrentDialog} playerState={playerState} setPlayerState={setPlayerState} />

    </Toolbar>
  </AppBar>
    <RRDrawer setCurrentTab={setCurrentTab} setCurrentDialog={setCurrentDialog} handleDrawerClose={handleDrawerClose} open={open} theme={theme} allInfo={allInformation} memory_descriptors={gameInformation.memory_descriptors} />
  </>;
}
