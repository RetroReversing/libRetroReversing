import React, { Fragment, useState, useEffect } from "react";
import ReactDOM from "react-dom";
import clsx from 'clsx';
import CssBaseline from '@material-ui/core/CssBaseline';

import Button from "@material-ui/core/Button";
import AppBar from '@material-ui/core/AppBar';
import Toolbar from '@material-ui/core/Toolbar';
import Typography from '@material-ui/core/Typography';
import Divider from '@material-ui/core/Divider';
import Drawer from '@material-ui/core/Drawer';
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem';
import ListItemIcon from '@material-ui/core/ListItemIcon';
import ListItemText from '@material-ui/core/ListItemText';
import Hidden from '@material-ui/core/Hidden';
import IconButton from '@material-ui/core/IconButton';
// Icons
import MailIcon from '@material-ui/icons/Mail';
import MenuIcon from '@material-ui/icons/Menu';
import InboxIcon from '@material-ui/icons/MoveToInbox';
import ChevronLeftIcon from '@material-ui/icons/ChevronLeft';
import ChevronRightIcon from '@material-ui/icons/ChevronRight';
import MemoryIcon from '@material-ui/icons/Memory';

import { PlaySettings } from "./debugger/PlaySettings";
import { RRDrawer } from './main/Drawer';
import { useStyles, useRRTheme } from './styles';
import { MainPage } from './pages/MainPage';
import { sendActionToServer } from './server';
import { InputHistory } from './pages/InputHistory';
import { GameInformation } from './pages/GameInformation';
import { FunctionList } from './pages/FunctionList';
import { ResourceList } from './pages/ResourceList';
import { DataStructures } from './pages/DataStructures';
import { MemoryViewer } from './pages/MemoryViewer';

function setupAdditionalTabs(gameInfo, tabs) {
  console.error("GameInfo:",gameInfo.memory_descriptors);
  gameInfo.memory_descriptors.forEach((mem)=> {
    const tab_name = "memory_"+mem.name;
    // TODO find display name
    console.error("tab_name", tab_name);
    tabs[tab_name] = <MemoryViewer memory={mem} />;
  });
  console.error("Tabs:", tabs);

}

const tabs = {
  main: <MainPage />,
  input: <InputHistory />,
  functions: <FunctionList />,
  resources: <ResourceList />,
  data_structures: <DataStructures />,
  game_info: <GameInformation />,
  memory: <MemoryViewer />,
  memory_BOOTROM: <MemoryViewer memory={{test:1}} />
}

function App() {
  const classes = useStyles();
  const theme = useRRTheme();

  const [open, setOpen] = React.useState(false);
  const [currentTab, setCurrentTab] = React.useState('main');
  const [gameInformation, setGameInformation] = useState({ 
    gameName: ""
  });

  
  console.error("original Tabs:", tabs);

  const handleDrawerOpen = () => {
    setOpen(true);
  };

  const handleDrawerClose = () => {
    setOpen(false);
  };

  React.useEffect(()=>{
    console.error("Requesting Game Information:", gameInformation);
    const payload = {
      category: 'game_information',
      state: gameInformation
    };
    sendActionToServer(payload).then((info)=>{
      if (!info) { return; }
      console.error("result: gameInformation:", info);
      setGameInformation(info);
      setupAdditionalTabs(info, tabs)
    });
  }, []);


  return (
    <div className={classes.root}>
      {/* <CssBaseline /> */}
      <AppBar position="fixed"
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
            SaturnRE
          </Typography>
          <PlaySettings />
        </Toolbar>
      </AppBar>
      <RRDrawer setCurrentTab={setCurrentTab} handleDrawerClose={handleDrawerClose} open={open} theme={theme} memory_descriptors={gameInformation.memory_descriptors} />
      <main
        className={clsx(classes.content, {
          [classes.contentShift]: open,
        })}
      >
        <div className={classes.drawerHeader} />
        {tabs[currentTab]}
      </main>
      </div>
  );
}

ReactDOM.render(<App />, document.querySelector("#app"));
