import { sendActionToServer } from '../server';
import { EmulatorMetaData } from "../main";


export function getEmulatorMetadata(_, setEmulatorMetadata, setAllInformation, tabs, setFullState, setPlayerState, playerState, setLoading) {
  return function () {
    const payload = {
      category: 'emulator_metadata'
    };
    sendActionToServer(payload).then((info) => {
      if (!info) { return; }
      console.error("result: Emulator MetaData:", info.result);
      const game_info: EmulatorMetaData = info.result;
      setEmulatorMetadata(game_info);
      // setAllInformation(info);
      // setFullState(info);
      // if (info.playerState) {
      //   setPlayerState({ ...info.playerState, playerState });
      // }
      setLoading(false);
    });
  };
}
