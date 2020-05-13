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