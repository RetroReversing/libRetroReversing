import React, { useEffect, useState } from "react";
import ReactDOM from "react-dom";
import clsx from 'clsx';
import {
  HashRouter as Router,
  Route,
  Switch,
  useHistory,
  useParams
} from "react-router-dom";

import Typography from '@material-ui/core/Typography';
import Paper from '@material-ui/core/Paper';

import { useStyles, useRRTheme } from './styles';
import { MainPage } from './pages/MainPage';
import { InputHistory } from './pages/InputHistory';
import { GameInformation } from './pages/GameInformation';
import { FunctionList } from './pages/FunctionList';
import { ResourceList } from './pages/ResourceList';
import { DataStructures } from './pages/DataStructures';
import { MemoryViewer } from './pages/MemoryViewer';
import PauseSaveDialog from './dialogs/PauseSaveDialog';
import ResumeDialog from './dialogs/ResumeDialog';

import { Breadcrumbs } from '@material-ui/core';
import Link from '@material-ui/core/Link';
import EditFunctionDialog from './dialogs/EditFunctionDialog';
import ExportFunctionDialog from './dialogs/ExportFunctionDialog';
import LoadLinkerMapFileDialog from './dialogs/LoadLinkerMapFileDialog';
import { RRTitleBar } from "./main/RRTitleBar";
import { PlaythroughSelector } from "./main/PlaythroughSelector";
import { HomePage } from "./HomePage";
import { getEmulatorMetadata } from "./api/getEmulatorMetadata";

function createBreadcrumbs(params) {
  return (<Breadcrumbs aria-label="breadcrumb">
  <Link color="inherit" href="/#/home">
    Home
  </Link>
  <Link color="inherit" href={"/#/"+params.currentTab}>
    {params.currentTab}
  </Link>
  <Typography color="textPrimary">{params.currentSubTab}</Typography>
</Breadcrumbs>);
}

function setupAdditionalGameSpecificTabs(emuMetaData: EmulatorMetaData, tabs) {
  console.error("setupAdditionalGameSpecificTabs", emuMetaData);
  if (!emuMetaData) return tabs;
  emuMetaData?.memory_descriptors?.forEach((mem)=> {
    const tab_name = "memory_"+mem.name;
    // TODO find display name
    tabs[tab_name] = <MemoryViewer memory={mem} />;
  });

  emuMetaData?.cd_tracks?.forEach((mem)=> {
    const tab_name = "memory_"+mem.name;
    tabs[tab_name] = <MemoryViewer memory={{...mem, start: 0} } />;
  });
  return tabs;
}

export function createTabs(loading, { setCurrentDialog, setCurrentDialogParameters, setFullState }, emulatorMetaData, fullState, cdData, allInformation) {
  const tabs =  {
    input: <InputHistory mainState={emulatorMetaData} fullState={fullState} />,
    functions: <FunctionList loading={loading} setCurrentDialog={setCurrentDialog} setCurrentDialogParameters={setCurrentDialogParameters} />,
    data_structures: <DataStructures />,
    home: <GameInformation />,
    game_info: <GameInformation />,
    main: <MainPage fullState={fullState} setFullState={setFullState} />,
    resources: <ResourceList cdData={cdData} />,
  };
  return setupAdditionalGameSpecificTabs(emulatorMetaData, tabs);
}

const initialPlayerState = {
  paused: true,
  speed: 100,
  startAt: -1,
  endAt: -1,
  loopFrame: -1,
  logButtons: false,
  recordInput: false,
  playbackLogged: false,
  fullLogging: false,
  fullFrames: []
};
function App() {
  const classes = useStyles();
  const theme = useRRTheme();
  let history = useHistory();

  const [leftDrawerOpen, setOpen] = React.useState(false);
  const [loading, setLoading] = React.useState(true);
  let params = useParams();

  const [currentTab, _setCurrentTab] = React.useState(params.currentTab || 'home');
  function setCurrentTab(newTab) {
    history.push(newTab);
    _setCurrentTab(newTab);
  }
  const [currentDialog, setCurrentDialog] = React.useState('');
  const [currentDialogParameters, setCurrentDialogParameters] = React.useState('');
  const [emulatorMetaData, setEmulatorMetadata] = useState({});
  const [allInformation, setAllInformation] = useState({ 
    gameName: "AllInfo",
    cd_data: { root_files: [] }
  });
  const [fullState, setFullState] = useState({});
  window['allInformation'] = allInformation;

  const [playerState, setPlayerState] = useState(initialPlayerState);

  const cdData = allInformation?.cd_data?.root_files;
  const tabs = createTabs(loading, { setCurrentDialog, setCurrentDialogParameters, setFullState }, emulatorMetaData, fullState, cdData, allInformation)
  
  const dialogs = {
    'pause_save': <PauseSaveDialog setCurrentDialog={setCurrentDialog} fullState={fullState} />,
    'resume': <ResumeDialog setCurrentDialog={setCurrentDialog} playerState={playerState} setPlayerState={setPlayerState} fullState={fullState} />,
    'edit_function': <EditFunctionDialog currentDialogParameters={currentDialogParameters} setCurrentDialog={setCurrentDialog} playerState={playerState} setPlayerState={setPlayerState} />,
    'export_function': <ExportFunctionDialog currentDialogParameters={currentDialogParameters} setCurrentDialog={setCurrentDialog} playerState={playerState} setPlayerState={setPlayerState} />,
    'load_linker_map': <LoadLinkerMapFileDialog currentDialogParameters={currentDialogParameters} setCurrentDialog={setCurrentDialog} playerState={playerState} setPlayerState={setPlayerState} />
  }

  useEffect(()=>{
    if (params?.gameHash && !window["loadedGames"][params?.gameHash]) {
      console.info("About to Load the Game into the web based emulator");
      const callback = () => {
        console.info("Game now running:", params?.gameHash);
      };
      window["startEmulator"](params?.gameHash, callback);
    }
  }, [params.gameHash])

  React.useEffect(getEmulatorMetadata.bind(null, setEmulatorMetadata, setLoading), [currentDialog, currentTab, leftDrawerOpen]);

  return (
    <div className={classes.root}>
      {RRTitleBar(classes, leftDrawerOpen, emulatorMetaData, setCurrentDialog, playerState, setPlayerState, setCurrentTab, theme, setOpen)}      
      <main
        className={clsx(classes.content, {
          [classes.contentShift]: leftDrawerOpen,
        })}
      >
        <div className={classes.drawerHeader} />
        {dialogs[currentDialog]}
        <PlaythroughSelector />
        {createBreadcrumbs(params)}
        {tabs[currentTab]}
      </main>
      </div>
  );
}

// ReactDOM.render(<Router><App /></Router>, document.querySelector("#app"));
ReactDOM.render(<Router>
<Switch>
  <Route path="/:console/:gameHash/:currentTab/:currentSubTab">
    <App />
    game Editor Subtab
  </Route>
  <Route path="/:console/:gameHash/:currentTab">
    <App />
    Game Editor CurrentTab
  </Route>
  <Route path="/:console/:gameHash">
    <App />
    Game Editor Homepage
  </Route>
  {/* <Route path="/:currentTab/:currentSubTab">
    <App />
    subtab
  </Route>
  <Route path="/:currentTab">
    <App />
    CurrentTab
  </Route> */}
  <Route path="/:console">
    <HomePage />
    Console specific games
  </Route>
  <Route path="/">
    <HomePage />
  </Route>
</Switch></Router>, document.querySelector("#app"));

export interface EmulatorMetaData {
  memory_descriptors: any;
  cd_tracks: any;
}


