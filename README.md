# libRetroReversing
Library to provide reverse engineering functionality to retroArch libRetro cores.

# Notes
The way a reversing emulator works is:
* Initially play the game - the emulator will record each button press
* Every time you pause the game a save state will be created
* It is recommended to name this save state so that you will know where to jump to, e.g "Level 1 start"
* Press stop when you have played the game through the features you want to reverse.
* You can now playback the same button presses but with additional logging
* It will log the following by default:
  * New function calls each frame
  * New memory accesses (Reads & Writes)
  * New Interactions with Audio or Video Hardware

You can also:
* Replace functions with Javascript
  - useful for understanding how the function works
  - if you write in a c-like style then can export to pseudo C code for decompilation
* Create Data structures and use them to parse memory blocks

If you have a very powerful PC you can:
* Log ALL memory accesses for each function
* Log ALL function calls

# TODO
* Ability to name save states such as "Level 1 start"
* Automatically create screenshot for save state

## Playthroughs
In a RE a project can have multiple playthroughs, a playthrough is simply just a log of button presses plus any save states that the user created along the way.

When you click pause you have two options:
* name the current state and save - this will add all button presses the the play through
* ignore recent actions and go back to the previously saved state.

## Dirty Save States
As soon as you manually make a change to the game such as changing memory or disabling a function, the current state will become "dirty".

Save states that you make under this environment will be placed in a seperate bucket known as the "Dirty Bucket".

---
# Types of Data Produced
There are 2 types of data produced by the Reversing Emulator:
* Static Data - is game specific - can be shared with others
* Dynamic Data - is specific to your own play through - not useful to others

## Static Data
Examples of static data are anything that would be in a decompilation project or documentation such as:
* Functions you have named
* Function implementations in Javascript
* Data Structures you have defined
* Resources you have found which are located at a static location in the ROM/ISO
* Cheats you have created
* Translations that can be applied statically to ROM/ISO

## Dynamic Data
Dynamic data is only useful for creating static data as all the data is specific to your initial playthough of the game (button inputs). Some examples of dynamic data include:
* Memory accesses of functions
* Call order of functions
* First and last call of functions
* Save states (including names, screenshots)

---

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
# Developer Notes

## Useful Developer References
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