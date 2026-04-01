# PixelArtEditor

This project got developed as a C++ university project by Erik Mettner. It is a simple Pixel Art Editor that features a color picker, different brush shapes and sizes and an option to resize the canvas up to 999 * 999 pixels. 

## How to run
### Inside of Clion

The project requires SFML (Version 3.0.2) and CMake so make sure to have that installed. It was originally written in CLion so I can't guarantee if it will run in other Editors.

Apart from that, a custom font is used that should be recognized by default.
If the font isn't automatically recognized, make sure to navigate to `Run > Edit Configurations > Working Directory` and set it to `path/to/project/src`.

### Using the executable

Alternatively, the directory `/executable` features the needed custom font, `.dll` files and the executable `main.exe` in order to run the software directly.

## Controls

- LMB to draw pixels
- RMB to erase pixels
- MMB and dragging to pan across the canvas
- Scrolling to zoom in and out

Apart from these, there are multiple buttons and panels that can be used for different functionalities but those are pretty self-explanatory or have tips directly written near them.

## Further notices
### Use of AI

I used Claude for helping me with issues I encountered after a SFML update that wasn't properly or at all documented in any tutorial video I watched. 

Apart from that, I used it to optimize my code for the canvas layout and navigation (zooming and dragging) since my original layout code for it was really messy and I had several weird sizing issues trying to move the canvas for zooming and dragging.

### Template used

The base for this project was generated from this public GitHub template:

https://github.com/SFML/cmake-sfml-project
