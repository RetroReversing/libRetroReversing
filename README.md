# libRetroReversing
Library to provide reverse engineering functionality to retroArch libRetro cores

# Building the Web source

## Building for first time
```bash
cd libRetroReversing/websrc
npm install
npm run production 
```

## Developing with the websrc
```bash
npm run dev
```

Now go to `http://localhost:8081`

---
# To Add to a new core

## Add submodule to new cores repository
```bash
git submodule add https://github.com/RetroReversing/libRetroReversing.git
```

## Include the makefile
Find the main Makefile for your core, for example it may be called `Makefile.libretro`.

Find the line that imports the common makefile:
```
include $(BUILD_DIR)/Makefile.common
```

Add the following line after it:
```
include ./libRetroReversing/Makefile.retroreversing
```

## Running with RetroArch
make && /Applications/RetroArch.app/Contents/MacOS/RetroArch -L pokemini_libretro.dylib ftff36a.min

---
# General Notes

## Useful References
JSON - https://github.com/nlohmann/json

## We don't use retro_get_memory_data
Initially `retro_get_memory_data` seemed perfect for our Reversing Emulator, however it is too limited, it only allows 4 different types of memory defined below:
```c
#define RETRO_MEMORY_SAVE_RAM    0
#define RETRO_MEMORY_RTC         1
#define RETRO_MEMORY_SYSTEM_RAM  2
#define RETRO_MEMORY_VIDEO_RAM   3
```
We need access to all the different specific memory regions such as Boot ROM etc which are too platform specific for libretro itself.