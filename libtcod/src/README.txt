This is the Doryen library source code.

libtcod.h and libtcod.hpp are the main headers, included respectively by 
the C and C++ programs that use libtcod. 

Each "toolkit" is in a separate file set. For each toolkit, you may have :
*  include/<toolkit>.h : the C header
*  include/<toolkit>.hpp : the C++ header
*  src/<toolkit>_c.c : the C source code that contains the actual implementation
*  src/<toolkit>.cpp : the C++ wrapper

Current toolkits :
bresenham : line drawing using the Bresenham algorithm.
bsp       : binary space partition tree module
color     : 24 bits colors operations.
console   : true color console emulator. It relies on some private functions of the sys toolkit.
fov       : field of view calculator.
heightmap : heightmap toolkit.
image     : image manipulation. It relies on some private functions of the sys toolkit.
list      : a fast and lightweight generic container (faster than STL).
mersenne  : pseudorandom number generator using the Mersenne twister algorithm.
mouse     : mouse support.
namegen   : name generator
noise     : various noise generators.
parser    : config file parser.
path      : path finding module.
sys       : system specific functions.
txtfield  : work-in-progress text input utility 
zip       : compression module

Note that some of the sys toolkits rely on system dependant functions. 
They have currently a single working implementation relying on SDL : sys_sdl_c.c
As it is heavily system dependant, the mouse C implementation is in sys_sdl_c.c.
As of 1.4, the image load/save functions are isolated in sys_sdl_XXX.c, 
XXX being the image type (currently bmp and png).
As of 1.5.1, sys_opengl_c.c contains OpenGL and GLSL renderers. Those two
renderers are still dependant on SDL.

libtcod_int.h contains everything that is exported by the toolkits 
(implemented in a toolkit and used in another one) but should not be seen 
by the user of the library.

These are the private (or undocumented) modules :
lex       : a generic lexical parser.
tree      : a minimalist tree toolkit.
sys       : sys also contains an undocumented threading toolkit.
