
#include <iostream>
#include <map>
#include <string>

extern "C" {
#include "../lualib/lua.h"
#include "../lualib/lauxlib.h"
#include "../lualib/lualib.h"
}

std::map<std::string, std::string> scriptvars;

//ERROR HANDLING NOTE:
//	This code attempts to handle errors gracefully.  What's CORRECT to do will vary by application.
//	In my case, application code often allows the script to continue to run but print error messages to the console
//		and (not done here) sets a flag and possible details to separately log cumulative information about
//		which script(s) are failing and context information.  That's because the typical application I've been building
//		often prefers to run to completion and examine all of the results for that 'transaction' including error state
//		before making a decision about what to do such as persisting data to long term storage.
//	For other applications, it may be important to halt script execution if continuing to run is either pointless or
//		might have an effect like allowing the script to corrupt persisted data.
//
//	The Lua docs indicate that in the event of an error in functions like pcall(), an error object will be created.
//		Because an error could be memory allocation related, this sample allows for the possibility that the error message
//		returned as the error message could be a NULL char*.  (I haven't seen this in the docs, but it seems prudent.)

std::string safestringfromluastack(lua_State* L, int index) {
//lua may return null as the char*
//handle that as an empty string, ""
const char* tmp;
std::string rval;

	tmp = lua_tostring(L, index);
	if (tmp == NULL) {
		rval = "";
	}
	else {
		rval = tmp;
	}
	return(rval);
}

//called by this parent application to make data available within a particular Lua script 
void setstringinluaenvironment(lua_State *L, std::string name, std::string val) {
//checks for fatal problems like L == null or not being properly initialized are the responsibility
//  of the overall application, not each function employed
	//lua does many things by putting the target value(s) on the stack and then taking an action
	lua_pushstring(L, val.c_str());//parm onto the stack
	lua_setglobal(L, name.c_str());//bind the string parm just pushed on the stack to this global identifier
}

//callable by Lua scripts which have this bound with them
//It's possible to directly register Lua functions without declaring them in the C++ application
//	but for many cases in production code it will be valuable have this declared functon available
//	for testing in a C++ environment (regression tests, debugging if necessary might also be easier)
extern "C" int persistlualocalstring(lua_State * L) {
//accepts two parms, the name of the string to persist and the value
//rather than simply using safestringfromluastack, illustrate how nil/null could be handled explicitly
std::string name, val;
char* tmpstr;

	if (lua_gettop(L) != 2) {
		std::cout << "persistlualocalstring requires two and only two arguments.\n";
		return(0);
	}

	tmpstr = (char*)lua_tostring(L, 1);
	if (tmpstr == NULL) {//Lua allows passing nil to functions, which is nonexistent, not the empty string ""
		//a nil named variable doesn't really make sense. as an application logic decision,
		//	treat it as equivalent to an empty string ("") here and handle that case later
		name = "";
	}
	else {
		name = tmpstr;
	}

	tmpstr = (char*)lua_tostring(L, 2);//equivalents exist for integers and floats
	if (tmpstr == NULL) {//Lua allows passing nil to functions, which is nonexistent, not the empty string ""
		//treat nil for VALUES as an empty string. correct behavior is application dependent
		val = "";
	}
	else {
		val = tmpstr;
	}

	if (name != "") {
		scriptvars[name] = val;
	}
	else {
		//we can't assign unnamed variables... how to react would be application dependent:
		//	fail obviously so as to make get the error fixed, abort the script, or just report that
		//	an error has happened and allow things to keep running as best they can
		std::cout << "Lua script attempted to persist to a blank or nil variable name\n";
	}
	return(0);
}

//sample function which accepts input, computes a value, and returns the result
extern "C" int samplemathfunction(lua_State * L) {
//accepts two numbers as input
//a function which requires integers could be written separately
//	OR lua would allow this function to decide what type to return at runtime and push an integer
//	result onto the stack instead.
lua_Number parm1, parm2;

//note: The error check and return(0)'s don't actually return any value to the lua script, which may cause
//  an error in the script.  We could also return a value but note the error.
	if (lua_gettop(L) != 2) {
		std::cout << "samplemathfunction requires two and only two arguments.\n";
		return(0);
	}

	if (lua_isnumber(L, 1) != 1) {
		std::cout << "First parameter of samplemathfunction must be numerical.\n";
		return(0);
	}

	if (lua_isnumber(L, 2) != 1) {
		std::cout << "Second parameter of samplemathfunction must be numerical.\n";
		return(0);
	}

	parm1 = lua_tonumber(L, 1);
	parm2 = lua_tonumber(L, 2);
	lua_pushnumber(L, parm1 * parm2);

	return(1);//the number of values we're returning (just pushed it onto the stack)

}

int main()
{
	lua_State* L = luaL_newstate();
	int err;

	luaopen_string(L);
	luaL_openlibs(L);

	//a Lua script which does three things:
	//	1) print hello world
	//	2) display the string passed into the script
	//	3) store a string in the parent application (this one) via the provided function call
	std::string sample = R"(
		print("Hello World from Lua inside c++!")
		print("The stored value is:" .. tostring(foo))
		persiststringvar("bar", "another test string")
		print("multiplying two numbers: " .. tostring(samplemath(2,3)))
		)";
	//for samplemath(), try:
	//	samplemath(1) which will print a message that the number of parameters is incorrect
	//	samplemath(badvar, 3) which is an undefined variable (nil) in the Lua script and will report that the first parm must be numberocal
 	//	samplemath("bad", 3) which will also report that the first parm must be numerical
	//	samplemath(2a, 3) which doesn't even attempt to call the C function because Lua reports a syntax violation

	err = luaL_loadbuffer(L, sample.c_str(), sample.length(), "sample");
	if (err) {
		printf("Error initializing lua with hello world script: %i\n", err);
		std::cout << safestringfromluastack(L, -1) << "\n"; //the detailed error string
		lua_close(L);
		return(0);
	}

	//make "test string" available within this script call as a string var named "foo"
	setstringinluaenvironment(L, "foo", "test string");
	//allow the script to call a function named 'persiststringvar' which actually calls our own
	//	persistlocalstring
	lua_register(L, "persiststringvar", &persistlualocalstring);
	//likewise for the math function
	lua_register(L, "samplemath", &samplemathfunction);

	err = lua_pcall(L, 0, 0, 0);
	if (err) {
		printf("Error running lua hello world script: %i\n", err);
		std::cout << "\t" << safestringfromluastack(L, -1) << "\n"; //the detailed error string
		lua_close(L);
		return(0);
	}

	//note that this line is printed after the script runs, so even though the persistence is done before the math
	//	call, the math result is printed by Lua during script execution and this is printed afterwards.
	std::cout << "As a result of the script running, value was stored for 'bar': " << scriptvars["bar"] << "\n";

	lua_close(L);

	return(0);

}
