### Compiling and running
```
mkdir build && cd build
cmake .. # for crosscompiling to windows: cmake -DCMAKE_TOOLCHAIN_FILE=../Windows.cmake ..
make
cd ..
build/noeuclid
```

[Explanation Video](https://www.youtube.com/watch?v=tl40xidKF-4)

### Files you care about:

* rooms.txt < the levels.
* scripthelpers.h < helper functions for the level scripts
* tileattributes.txt < File containing the visual appearance for various tiles.

Modifying rooms.txt while the game is running will just cause it to be recompiled in-place.  Where you are.

### Hotkeys:

 Key | Action
-----|---
 'a', 's', 'd', 'w' | Walk around
' ' | jump
'g' | Go into noclip mode.
']', '[' | Go up/down
'0' | Show additional debug info.
|'-' | Go down level.
'+' | Go up level.
'r' | Reload current.
'l' | Load ALL levels right now.
'p' | Pauses.
'8' / '9' | Change mouse sensitivity.

You can add new blocks by modifying tileattributes.txt


Building on Linux:
Be sure to install: (Debian-style)
  build-essential, zlib1g-dev, freeglut3-dev ... maybe more?
