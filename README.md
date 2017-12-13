## Bouncing Sphere in OpenGL

One sphere bouncing (actually moving up and down) using shader-based OpenGL. Some options are available using the mouse right click, such as:

* See wireframe, shading or with texture
* Gouraud/Phong/Blinn-Phong (shading and illumination model)
* Turn on/off two lights
* Change between plastic/metallic material
* Switch between 2 textures

The textures can be downloaded from [basketball] and  . Also, you can use any [PPM] texture. To run the code, [CMake] is necesary or just manually adding the libraries/environment to work with GLUT (or freeglut).

The required [headers] can be found on the page of authors. Also, Ed Angel is a very famous professor in Computer Graphics (University of New Mexico).

[glm]: <https://glm.g-truc.net/0.9.8/index.html>
[CMake]: <https://cmake.org/>
[headers]: <https://www.cs.unm.edu/~angel/BOOK/INTERACTIVE_COMPUTER_GRAPHICS/SIXTH_EDITION/CODE/include/>
[PPM]: <http://netpbm.sourceforge.net/doc/ppm.html>
[basketball]: <http://ccg.ciens.ucv.ve/~esmitt/files/basketball.ppm>
[earth]: <http://ccg.ciens.ucv.ve/~esmitt/files/earth.ppm>