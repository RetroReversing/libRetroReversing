# libRetroReversing
Library to provide reverse engineering functionality to retroArch libRetro cores

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
