# zupedils
A simple sliding puzzle game, written in c, using gtk+-3.

A player's progress can be saved, and the puzzle's image can be changed.

To build and install from the source directory, type:
```bash
mkdir build
cd build
cmake ..
make
(As root) make install
```

If building on Windows using something like MSYS2, you might need:
```bash
mkdir build
cd build
cmake -G "Unix Makefiles" ..
make
(As root) make install
```

![zupedils](https://raw.githubusercontent.com/bit-sorter/zupedils/master/zupedils.png)
