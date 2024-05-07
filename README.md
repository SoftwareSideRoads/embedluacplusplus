# embedluac++

This set of examples illustrates how to embed Lua scripts into C/C++ applications. This example files are included in Visual Studio 2022 projects but the C/C++ and Lua code are easily portable to other environments.  An explanation of intent and context is posted at
https://softwaresideroads.com/2024/05/04/embedding-lua-in-c-c/ for the basic embedding of Lua in C/C++ and
https://softwaresideroads.com/2024/05/06/passing-data-between-lua-and-cplusplus/ for calling C++ functions from Lua, returning values, and persisting global variables in Lua script instances.

Stock Lua library files (.c and .h, not the visual studio related files) for 5.4 are included in the lualib directory and the copyrights to those files belong solely to the Lua project.  As of the time of this writing (2024-05-04) they also use the MIT license with the entity name "Lua.org, PUC-Rio" as stated at https://www.lua.org/license.html
