### Compiling and running
```
make -j
./noeuclid
```
[Explanation Video](https://www.youtube.com/watch?v=tl40xidKF-4)

### Prerequisites

You'll probably want to compile your own copy of TinyCC, you'll need to build tcc 0.9.26.

Right now we use GLUT, But we plant to (already have moved?) to SDL.

And you'll need a modern GPU.  Some intel GPUs kind of work, most post-2013 NVIDIA and ATI GPUs work, too.  (And higher end ones from before)

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
Needed packages:

    sudo apt-get install build-essential libtcc-dev libsfml-dev libglew-dev
