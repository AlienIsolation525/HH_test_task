# Game_of_life - https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life

This simple programm( made using sdk2 library and cmake) represents the life of bacteria in Petri dishes. Player can capture the environment by microorganisms, selecting starting points.

SPACEBAR - Pause the game.
MOUSECLICK - Select \ unselect cell on grid.

Build and enjoy

```mkdir build```


```cmake -DCMAKE_TOOLCHAIN_FILE=path/to\vcpkg\scripts\buildsystems\vcpkg.cmake .```# I installed SDL via vcpkg and it helped me finding required packages. Think that pure cmake call also gonna work


```#Now build .sln file and run executable from /build/ dir```


Exec command line arguments :


'--radius' - petri radius

'--cell_size' - grid_cell 

'--speed' - speed

'--width'

'--height'
