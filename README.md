# Axe DnD Map Viewer

A simple application that allows you to input an image of a dnd map, hide specific tiles, and show them in a second window for your players to view.

## Description

TODO

## Getting Started

### Dependencies

* [Allegro 5.2.7](https://github.com/liballeg/allegro5)
* GCC Compiler
* CMake
* On Windows - MSYS2 + MinGW

### Installing

#### Windows

Follow the instructions [here](https://www.msys2.org/) to install and set up MSYS2 on Windows.

##### MSYS2
```pacman -S mingw-w64-x86_64-allegro```

TODO

#### Linux

##### APT
``` sudo apt install liballegro5-dev```

* Download the source, then cd into axe-map-editor.
* Create a build/ directory and cd into it. Run `cmake ..` then `make all` to build the program.

### Executing program

In the build folder run:
```
./axe-map-editor <image-path> <tile-size>
```

## Help

This is a very early version of this program. Do not expect things to always work.

Keybindings (All keybinds are input in the editor window. Some affect the viewer window)
* F1 to open the viewer window.
* Mouse Wheel Up/Down to zoom.
* Middle Mouse drag the editor view.
* W A S D to move the editor view.
* Ctrl+Middle Mouse to set the viewer view.
* Space to show hidden tiles in editor.
* Drag with Left Mouse to show hidden tiles.
* Drag with Right Mouse to hide visible tiles
* Shift+Drag with Left or Right Mouse to edit a rectangle of tiles.
* G shows/hides the grid (Ctrl+G does the same in the viewer).
* Ctrl+Z Undo
* Ctrl+Y Redo
* Ctrl+S Save (Saves file to map-save.mdf)
* Ctrl+L Load (Loads file from map-save.mdf)
* Ctrl+C Resets the view to 0, 0.
* R Resets the view scale.
* Up/Down Arrows scale the view in the viewer window.
* U Sends the tile visibility set in the editor to the viewer window.

## Authors

Contributors names and contact info

Taurous
Contact info here

## Version History

* 0.1.1-beta
    * Map data was being saved to the directory above the executable. Changed to current working directory.

* 0.1.0-beta
    * Initial Release

## License

This project is licensed under the GPL-3.0 License - see the LICENSE.md file for details

## Acknowledgments

Inspiration, code snippets, etc.
* [DomPizzie/README-Template.md](https://gist.github.com/DomPizzie/7a5ff55ffa9081f2de27c315f5018afc)
