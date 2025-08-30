# Overview

Spider model that moves towards different colored lights in a room, it can't climb walls. User can control the camera and toggle the lights on and off using a context menu. There are four lights located in each corner: red, white, green, and blue.

## Requirements

- C++ compiler (g++)
- OpenGL
- FreeGLUT

## Usage

Open project directory, run this command to compile an executable: 
```bash
g++ main.cpp -o main.exe -lfreeglut -lopengl32 -lglu32
```

- Arrow Keys: Move the camera.
- W/S: Move the camera vertically.
- Right Click: Open light control menu.

## The End

That's it! Have fun, kids.
