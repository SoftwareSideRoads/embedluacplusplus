
#include <iostream>

extern "C" {
#include "../lualib/lua.h"
#include "../lualib/lauxlib.h"
#include "../lualib/lualib.h"
}

int main()
{
    std::cout << "Starting basic lua embedded in c++ example\n";
	lua_State* L = luaL_newstate();
	int err;
	luaopen_string(L);
	luaL_openlibs(L);

	std::string sample = "print(\"Hello World from Lua inside c++!\")";
	
	err = luaL_loadbuffer(L, sample.c_str(), sample.length(), "mysample");
	if (err) {
		printf("Error initializing lua with hello world script: %i", err);
		return(0);
	}

	err = lua_pcall(L, 0, 0, 0);
	if (err) {
		printf("Error running lua hello world script: %i", err);
		return(0);
	}

	printf("Success running hello world script\n");
	return(0);
}
