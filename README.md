# Cloth Simulator

A real-time interactive cloth simulation written in C++17, uses OpenGL for rendering and GLEW/GLFW for context and input.  


**Features include:**
- Real-time cloth tearing simulation
- Cloth-object collision detection
- Wind physics simulation
- Adjustable camera controls
- Real-time physics updates
- Wireframe and pause view options

## Demo

<table>
<tr>
  <td align="center">
    <b>Cloth Tearing</b><br>
    <img src="https://github.com/sameersaeed/cloth-simulator/releases/download/demo-clips/cloth-tearing.gif" width="500">
  </td>
  <td align="center">
    <b>Cloth Collision</b><br>
    <img src="https://github.com/sameersaeed/cloth-simulator/releases/download/demo-clips/cloth-collision.gif" width="500">
  </td>
  <td align="center">
    <b>Wind Physics</b><br>
    <img src="https://github.com/sameersaeed/cloth-simulator/releases/download/demo-clips/cloth-wind-physics.gif" width="500">
  </td>
</tr>
</table>


## Prerequisite Libraries / Installations

### CMake
```console
Debian/Ubuntu:
sudo apt-get install cmake

or download it directly from the cmake website:
https://cmake.org/download/
```

### OpenGL, GLEW, GLFW
```console
Debian/Ubuntu:
sudo apt-get install libglew-dev libglfw3-dev libglm-dev -y

Or check the official documentation:
- GLEW: http://glew.sourceforge.net/
- GLFW: https://www.glfw.org/
- GLM: https://github.com/g-truc/glm
```

## **Setup:**
### Clone the repository
```console
git clone "https://github.com/sameersaeed/cloth-simulator"
```


### Build and run the simulator
Starting from project root, run:

```console
mkdir build && cd build
cmake ..
make
./ClothSimulation
```
If you run the binary correctly, you should see a window pop up on your screen with the simulation running
