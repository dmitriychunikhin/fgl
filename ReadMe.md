## Project Description
FGL is high performance 3D graphics application framework based on OpenGL, SOIL (Simple OpenGL Image Library), Assimp, Freetype and OpenMesh.

FGL is aimed for VFP developers who interested in adding interactive 3D visualization features in their applications.

FGL supports:
* easy to use scene graph object-oriented interface
* user interaction (mouse, keyboard events)
* rendering performance optimization features like spatial culling, Level Of Details (LOD), vertex buffers and instancing
* material and light system
* text rendering with TrueType and OpenType fonts
* multiple viewports
* skeletal animation
* loading 3D models and textures in wide variety of formats, see [http://assimp.sourceforge.net/main_features_formats.html](http://assimp.sourceforge.net/main_features_formats.html) and [http://www.lonesock.net/soil.html](http://www.lonesock.net/soil.html)

**Short demo of FGL based application**
[http://www.youtube.com/watch?v=voSSELM7oRs](http://www.youtube.com/watch?v=voSSELM7oRs)

FGL is part of [VFPX](http://vfpx.codeplex.com)
![VFPX](Home_vfpxbanner_small.gif|http://vfpx.codeplex.com)


## Installation

1) Download latest FGL release from [http://fgl.codeplex.com/releases](http://fgl.codeplex.com/releases)

2) Copy file **fgl.fll** to the root folder of your project. 

3) Copy **_FGL** folder to the root folder of your project.
* _FGL/fgl.vcx - wrapper around fgl.fll
* _FGL/fglc.vcx - FGL controls library

4) Download and install ["Visual C++ Redistributable Packages for Visual Studio 2013"](http://search.microsoft.com/en-us/DownloadResults.aspx?q=Visual+C%2b%2b+Redistributable+Packages+for+Visual+Studio+2013) (vcredist_x86.exe)


## Usage
1) Integrate library to your project

* Open FGL libraries
{code:vb.net}
SET LIBRARY TO "fgl.fll" ADDITIVE &&FGL API library
SET CLASSLIB TO "_FGL/fgl.vcx" ADDITIVE &&wrapper around fgl.fll
SET CLASSLIB TO "_FGL/fglc.vcx" ADDITIVE &&FGL controls library
{code:vb.net}

* Create new form and put **FGLC.fglc_main** container on it

2) Compile your project and try to execute it. If there is no any errors you can start to use FGL. 
Of cause, you can also take a look at samples applications.

## Todo
* API documetation
* transparent materials rendering
* allow define custom shaders for materials
* layered textures
* bump mapping and parallax occlusion mapping
* heightmaps