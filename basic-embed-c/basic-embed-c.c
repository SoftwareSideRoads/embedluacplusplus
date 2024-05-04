
#include <stdio.h>

#include "../lualib/lua.h"
#include "../lualib/lauxlib.h"
#include "../lualib/lualib.h"

int main()
{
	printf("Starting basic lua embedded in c example\n");

	lua_State* L = luaL_newstate();
	int err;
	luaopen_string(L);
	luaL_openlibs(L);

	const char* sample = "print(\"Hello World from Lua!\")";

	err = luaL_loadbuffer(L, sample, strlen(sample), "mysample");
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

