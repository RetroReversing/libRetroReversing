import React from "react";
import Typography from '@material-ui/core/Typography';
import Container from '@material-ui/core/Container';
import AppBar from '@material-ui/core/AppBar';
import Toolbar from '@material-ui/core/Toolbar';
import { ThemeProvider } from '@material-ui/core/styles';
import { theme } from './styles';
import { GameDropZone } from "./home/GameDropZone";

//  The HomePage will become a game selection interface
export function HomePage() {
  return <div>
    <ThemeProvider theme={theme}>
      <AppBar position="relative">
        <Toolbar>
          {/* <CameraIcon sx={{ mr: 2 }} /> */}
          <Typography variant="h6" color="inherit" noWrap>
            Reversing Emulator
          </Typography>
        </Toolbar>
      </AppBar>
      <Container>
        <Typography variant="h6" color="inherit" noWrap>Add Games</Typography>
        <Typography align="center">Welcome to the Online version of the Reversing Emulator, simply drop a ROM into the box below to start Reversing!</Typography>
        <GameDropZone />
      </Container>
    </ThemeProvider>
  </div>;
}
