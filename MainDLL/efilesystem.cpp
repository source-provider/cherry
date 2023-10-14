#include "environment.hpp"
using namespace cherry::global;

std::vector<std::string> disallowed_extensions = {
			".exe", ".bat", ".com", ".csh", ".msi", ".vb", ".vbs", ".vbe", ".ws", ".wsf", ".wsh", ".ps1"
};

auto readfile(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
	std::string path = lua_tostring(L, 1);

	utilities->create_workspace();

	if (path.find("..") != std::string::npos) {
		luaL_error(L, "attempt to escape directory");
		return 0;
	}

	path = (utilities->location() + "\\workspace\\" + path);

	//std::cout << "reading at - " << path << "\n";

	if (!std::filesystem::exists(path.c_str())) {
		//std::cout << "file does not exist!\n";
		luaL_error(L, "file does not exist");
		return 0;
	}

	std::string output = utilities->read_file(path);
	lua_pushlstring(L, output.c_str(), output.size());
	return 1;
}

auto writefile(lua_State* L) -> int { 
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TSTRING);)
	luaL_checktype(L, 2, LUA_TSTRING);

	std::string path = lua_tostring(L, 1);
	std::string content = lua_tostring(L, 2);

	utilities->create_workspace();

	const std::string extention = PathFindExtensionA(path.c_str());
	for (std::string& test : disallowed_extensions) {
		if (utilities->equals_ignore_case(extention, test)) {
			luaL_error(L, "attempt to escape directory");
			return 0;
		}
	}

	if (path.find("..") != std::string::npos) {
		luaL_error(L, "attempt to escape directory"); 
		return 0;
	}

	path = (utilities->location() + "\\workspace\\" + path);

	std::ofstream out(path);
	out << content;
	out.close();

	return 0;
}

auto appendfile(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TSTRING);)
		luaL_checktype(L, 2, LUA_TSTRING);

	std::string path = lua_tostring(L, 1);
	std::string content = lua_tostring(L, 2);

	const std::string extention = PathFindExtensionA(path.c_str());
	for (std::string& test : disallowed_extensions) {
		if (utilities->equals_ignore_case(extention, test)) {
			luaL_error(L, "attempt to escape directory");
			return 0;
		}
	}

	if (path.find("..") != std::string::npos) {
		luaL_error(L, "attempt to escape directory");
		return 0;
	}

	path = (utilities->location() + "\\workspace\\" + path);

	std::ofstream out;
	out.open(path, std::ios_base::app | std::ios_base::binary);
	out.write(content.c_str(), content.size());
	out.close();

	return 0;
}

auto loadfile(lua_State* L) -> int { 
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
	std::string path = lua_tostring(L, 1);

	utilities->create_workspace();

	if (path.find("..") != std::string::npos) {
		luaL_error(L, "attempt to escape directory");
		return 0;
	}

	path = (utilities->location() + "\\workspace\\" + path);

	if (!std::filesystem::exists(path.c_str())) {
		luaL_error(L, "file does not exist");
		return 0;
	}

	std::string output = utilities->read_file(path);
	return execution->send((int)L, output, false);
}

auto dofile(lua_State* L) -> int { 
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
	std::string path = lua_tostring(L, 1);

	utilities->create_workspace();

	if (path.find("..") != std::string::npos) {
		luaL_error(L, "attempt to escape directory");
		return 0;
	}

	path = (utilities->location() + "\\workspace\\" + path);

	if (!std::filesystem::exists(path.c_str())) {
		luaL_error(L, "file does not exist");
		return 0;
	}

	std::string output = utilities->read_file(path);
	execution->send(NULL, output, true);
	return 0;
}

auto listfiles(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
	std::string path = lua_tostring(L, 1);

	utilities->create_workspace();

	if (path.find("..") != std::string::npos) {
		luaL_error(L, "attempt to escape directory"); 
		return 0;
	}

	path = (utilities->location() + "\\workspace\\" + path);

	if (!std::filesystem::is_directory(path.c_str())) {
		luaL_error(L, "directory does not exist");
		return 0;
	}

	lua_newtable(L);

	int idx = 0;
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		auto file_path = entry.path().string();
		file_path = utilities->replace(file_path, (utilities->location() + "\\workspace\\"), "");

		lua_pushinteger(L, ++idx);
		lua_pushstring(L, file_path.c_str());
		lua_settable(L, -3);
	}

	return 1;
}

auto isfolder(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
	std::string path = lua_tostring(L, 1);

	utilities->create_workspace();

	if (path.find("..") != std::string::npos) {
		luaL_error(L, "attempt to escape directory"); 
		return 0;
	}

	path = (utilities->location() + "\\workspace\\" + path);

	lua_pushboolean(L, std::filesystem::is_directory(path));
	return 1;
}

auto isfile(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
	std::string path = lua_tostring(L, 1);

	utilities->create_workspace();

	if (path.find("..") != std::string::npos) {
		luaL_error(L, "attempt to escape directory"); 
		return 0;
	}

	path = (utilities->location() + "\\workspace\\" + path);

	lua_pushboolean(L, std::filesystem::is_regular_file(path));
	return 1;
}

auto makefolder(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
	std::string path = lua_tostring(L, 1);

	utilities->create_workspace();

	if (path.find("..") != std::string::npos) {
		luaL_error(L, "attempt to escape directory"); 
		return 0;
	}

	path = (utilities->location() + "\\workspace\\" + path);

	std::filesystem::create_directories(path);
	return 0;
}

auto delfolder(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
	std::string path = lua_tostring(L, 1);

	utilities->create_workspace();

	if (path.find("..") != std::string::npos) {
		luaL_error(L, "attempt to escape directory");
		return 0;
	}

	path = (utilities->location() + "\\workspace\\" + path);

	if (!std::filesystem::remove_all(path)) {
		luaL_error(L, "folder does not exist");
		return 0;
	}

	return 0;
}

auto delfile(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING);)
	std::string path = lua_tostring(L, 1);

	utilities->create_workspace();

	if (path.find("..") != std::string::npos) {
		luaL_error(L, "attempt to escape directory"); 
		return 0;
	}

	path = (utilities->location() + "\\workspace\\" + path);

	if (!std::filesystem::remove(path)) {
		luaL_error(L, "file does not exist");
		return 0;
	}

	return 0;
}

/* registring */
static const luaL_Reg env_funcs[] = {
	{"readfile", readfile},
	{"writefile", writefile},
	{"appendfile", appendfile},
	{"loadfile", loadfile},
	{"dofile", dofile},
	{"listfiles", listfiles},
	{"isfolder", isfolder},
	{"isfile", isfile},
	{"makefolder", makefolder},
	{"delfolder", delfolder},
	{"delfile", delfile},
	{NULL, NULL}
};

auto cherry::environment::luauopen_filesystem(lua_State* L) -> void {
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, NULL, env_funcs);

	lua_pop(L, 1);
}