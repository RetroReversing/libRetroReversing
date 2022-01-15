import React, { useState } from "react";
import ReactDOM from "react-dom";
import clsx from 'clsx';

import Typography from '@material-ui/core/Typography';
// Icons


import { useStyles, useRRTheme } from './styles';
import { MainPage } from './pages/MainPage';
import { sendActionToServer } from './server';
import { InputHistory } from './pages/InputHistory';
import { GameInformation } from './pages/GameInformation';
import { FunctionList } from './pages/FunctionList';
import { ResourceList } from './pages/ResourceList';
import { DataStructures } from './pages/DataStructures';
import { MemoryViewer } from './pages/MemoryViewer';
import PauseSaveDialog from './dialogs/PauseSaveDialog';
import ResumeDialog from './dialogs/ResumeDialog';

import {
  HashRouter as Router,
  Route,
  Switch,
  useLocation,
  useHistory,
  useParams
} from "react-router-dom";
import { Breadcrumbs } from '@material-ui/core';
import Link from '@material-ui/core/Link';
import EditFunctionDialog from './dialogs/EditFunctionDialog';
import ExportFunctionDialog from './dialogs/ExportFunctionDialog';
import LoadLinkerMapFileDialog from './dialogs/LoadLinkerMapFileDialog';
import { RRTitleBar } from "./main/RRTitleBar";
import { PlaythroughSelector } from "./main/PlaythroughSelector";

function createBreadcrumbs(params) {
  return (<Breadcrumbs aria-label="breadcrumb">
  <Link color="inherit" href="/#/main">
    Home
  </Link>
  <Link color="inherit" href={"/#/"+params.currentTab}>
    {params.currentTab}
  </Link>
  <Typography color="textPrimary">{params.currentSubTab}</Typography>
</Breadcrumbs>);
}

function setupAdditionalTabs(allInfo, tabs) {
  const gameInfo = allInfo.current_state;
  console.error("setupAdditionalTabs GameInfo:",allInfo);
  if (!gameInfo) return tabs;
  gameInfo.memory_descriptors.forEach((mem)=> {
    const tab_name = "memory_"+mem.name;
    // TODO find display name
    tabs[tab_name] = <MemoryViewer memory={mem} />;
  });

  allInfo.cd_tracks.forEach((mem)=> {
    const tab_name = "memory_"+mem.name;
    tabs[tab_name] = <MemoryViewer memory={{...mem, start: 0} } />;
  });
  console.error("Tabs:", tabs);
  return tabs;
}

function createTabs(loading, { setCurrentDialog, setCurrentDialogParameters, setFullState }, gameInformation, fullState, cdData, allInformation) {
  const tabs =  {
    input: <InputHistory mainState={gameInformation} fullState={fullState} />,
    functions: <FunctionList loading={loading} setCurrentDialog={setCurrentDialog} setCurrentDialogParameters={setCurrentDialogParameters} />,
    data_structures: <DataStructures />,
    game_info: <GameInformation />,
    main: <MainPage mainState={gameInformation} fullState={fullState} setFullState={setFullState} />,
    resources: <ResourceList cdData={cdData} />,
    // additional tabs are setup using: setupAdditionalTabs
  };
  return setupAdditionalTabs(allInformation, tabs);
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
  let location = useLocation();
  let history = useHistory();

  const [open, setOpen] = React.useState(false);
  const [loading, setLoading] = React.useState(true);
  let params = useParams();

  const [currentTab, _setCurrentTab] = React.useState(params.currentTab || 'main');
  function setCurrentTab(newTab) {
    history.push("/"+newTab);
    _setCurrentTab(newTab);
  }
  const [currentDialog, setCurrentDialog] = React.useState('');
  const [currentDialogParameters, setCurrentDialogParameters] = React.useState('');
  const [gameInformation, setGameInformation] = useState({ 
    gameName: ""
  });
  const [allInformation, setAllInformation] = useState({ 
    gameName: ""
  });
  const [fullState, setFullState] = useState({});
  window.allInformation = allInformation;

  const [playerState, setPlayerState] = useState(initialPlayerState);

  const cdData = allInformation?.cd_data?.root_files;
  const tabs = createTabs(loading, { setCurrentDialog, setCurrentDialogParameters, setFullState }, gameInformation, fullState, cdData, allInformation)
  
  const dialogs = {
    'pause_save': <PauseSaveDialog setCurrentDialog={setCurrentDialog} fullState={fullState} />,
    'resume': <ResumeDialog setCurrentDialog={setCurrentDialog} playerState={playerState} setPlayerState={setPlayerState} fullState={fullState} />,
    'edit_function': <EditFunctionDialog currentDialogParameters={currentDialogParameters} setCurrentDialog={setCurrentDialog} playerState={playerState} setPlayerState={setPlayerState} />,
    'export_function': <ExportFunctionDialog currentDialogParameters={currentDialogParameters} setCurrentDialog={setCurrentDialog} playerState={playerState} setPlayerState={setPlayerState} />,
    'load_linker_map': <LoadLinkerMapFileDialog currentDialogParameters={currentDialogParameters} setCurrentDialog={setCurrentDialog} playerState={playerState} setPlayerState={setPlayerState} />
  }

  React.useEffect(getGameInformation(gameInformation, setGameInformation, setAllInformation, tabs, setFullState, setPlayerState, playerState, setLoading), [currentDialog, currentTab]);


  return (
    <div className={classes.root}>
      {RRTitleBar(classes, open, gameInformation, setCurrentDialog, playerState, setPlayerState, setCurrentTab, theme, setOpen)}      
      <main
        className={clsx(classes.content, {
          [classes.contentShift]: open,
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

ReactDOM.render(<Router>
<Switch>
  <Route path="/:currentTab/:currentSubTab">
    <App />
  </Route>
  <Route path="/:currentTab">
    <App />
  </Route>
  <Route path="/">
    <App />
  </Route>
</Switch></Router>, document.querySelector("#app"));


function getGameInformation(gameInformation, setGameInformation, setAllInformation, tabs, setFullState, setPlayerState, playerState, setLoading) {
  return function requestGameInformation() {
    console.error("Requesting Game Information:", gameInformation);
    const payload = {
      category: 'game_information',
      state: gameInformation
    };
    sendActionToServer(payload).then((info) => {
      if (!info) { return; }
      console.error("result: gameInformation:", info);
      setGameInformation(info.current_state);
      setAllInformation(info);
      setupAdditionalTabs(info, tabs);
      setFullState(info);
      if (info.playerState) {
        setPlayerState({ ...info.playerState, playerState });
      }
      setLoading(false);
    });
  };
}

