#include "environment.hpp"
#include <thread>

using namespace cherry::global;

struct wsocket_t {
	bool closed;
	//sockpp::tcp_connector connection;
	std::vector<int> on_message; /* holds the string used for getting the closures */
	std::vector<int> on_close; /* holds the string used for getting the closures */
};
std::unordered_map<int, wsocket_t> websockets;

auto on_message_caller(int wb_index) -> void {
	if (!websockets.count(wb_index)) {
		return;
	}

	/*char buffer[999999];
	std::string data = "";

	lua_State* L = lua_newthread((lua_State*)task_scheduler->state());
	lua_ref((lua_State*)task_scheduler->state(), -1);
	
	auto second = (std::chrono::seconds)(1);
	while (websockets[wb_index].connection.is_open()) {
		std::this_thread::sleep_for(second);

		if (!websockets[wb_index].connection.is_connected())
			continue;

		size_t str_size = websockets[wb_index].connection.read(buffer, sizeof(buffer));

		if (str_size != -1) {
			data = data + buffer;
			
			for (int index: websockets[wb_index].on_message) {
				lua_rawgeti(L, -10000, index);
				lua_pushstring(L, data.c_str());
				lua_pcall(L, 1, 0, 0);
				lua_settop(L, 0);
			}

			data = "";
		}
	}

	for (int index : websockets[wb_index].on_close) {
		lua_rawgeti(L, -10000, index);
		lua_pushstring(L, data.c_str());
		lua_pcall(L, 1, 0, 0);
		lua_settop(L, 0);
	}*/
}

/* websocke metatable funcs */
auto send(lua_State* L) -> int {
	const auto websocket_idx = *(int*)lua_touserdata(L, 1);
	
	if (!websockets.count(websocket_idx)) {
		return 0;
	}

	if (websockets[websocket_idx].closed) {
		return 0;
	}

	std::string data = luaL_tolstring(L, 1, NULL);
	lua_pop(L, 1);

	/*websockets[websocket_idx].connection.write(data);*/
	return 0;
}

auto close(lua_State* L) -> int {
	const auto websocket_idx = *(int*)lua_touserdata(L, 1);

	if (!websockets.count(websocket_idx)) {
		return 0;
	}

	if (websockets[websocket_idx].closed) {
		return 0;
	}

	//websockets[websocket_idx].connection.close();
	websockets[websocket_idx].closed = true;

	return 0;
}

auto on_message_connect(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TTABLE););
	luaL_checktype(L, 2, LUA_TFUNCTION);

	const auto websocket_idx = *(int*)lua_touserdata(L, -10003);

	if (!websockets.count(websocket_idx)) {
		return 0;
	}

	if (websockets[websocket_idx].closed) {
		return 0;
	}

	websockets[websocket_idx].on_message.push_back(lua_ref(L, 2));
	return 0;
}

auto on_close_connect(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TTABLE););
	luaL_checktype(L, 2, LUA_TFUNCTION);

	const auto websocket_idx = *(int*)lua_touserdata(L, -10003);

	if (!websockets.count(websocket_idx)) {
		return 0;
	}

	if (websockets[websocket_idx].closed) {
		return 0;
	}

	websockets[websocket_idx].on_close.push_back(lua_ref(L, 2));
	return 0;
}


/* websocket start */

auto WebSocket__index(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_checktype(L, 2, LUA_TSTRING);
	const auto websocket_idx = *(int*)lua_touserdata(L, 1);
	std::string index = lua_tostring(L, 2);

	if (index == "Send" || index == "send") {
		lua_pushcclosurek(L, send, NULL, NULL, NULL);
		return 1;
	}
	else if (index == "Close" || index == "close") {
		lua_pushcclosurek(L, close, NULL, NULL, NULL);
		return 1;
	}
	else if (index == "OnMessage" || index == "onmessage") {
		lua_newtable(L);
		
		lua_pushvalue(L, 1);
		lua_pushcclosure(L, on_message_connect, NULL, 1);
		lua_setfield(L, -2, "Connect");

		lua_getfield(L, -1, "Connect");
		lua_setfield(L, -2, "connect");

		lua_setreadonly(L, -1, true);

		return 1;
	}
	else if (index == "OnClose" || index == "onclose") {
		lua_newtable(L);

		lua_pushvalue(L, 1);
		lua_pushcclosure(L, on_close_connect, NULL, 1);
		lua_setfield(L, -2, "Connect");

		lua_getfield(L, -1, "Connect");
		lua_setfield(L, -2, "connect");

		lua_setreadonly(L, -1, true);

		return 1;
	}

	return 0;
}

auto WebSocket_Connect(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
	std::string index = lua_tostring(L, 1);
	index = utilities->replace(index, "ws://", ""); 
	wsocket_t new_socket;
	
	new_socket.closed = false;
	new_socket.on_message = {}; 
	new_socket.on_close = {};

	if (index.contains("localhost")) { /* has a port */
		index = utilities->replace(index, "localhost:", "");
		index = utilities->replace(index, "/", "");
		int16_t port = 12345;

		if (index != "") 
			port = std::stoi(index);

		//std::cout << "[WEBSOCKET LOCALHOST PORT]: " << port << std::endl;

		/*if (!new_socket.connection.connect({ "localhost", port }))
			return 0;*/

		//std::cout << "[PASSED CONNECTION]: " << port << std::endl;

	}
	else { /* most likely doesn't have a port */
		//index = utilities->replace(index, ":", "");
		//sockpp::inet_address addy{ index, 12345 };

		//std::cout << "[WEBSOCKET LOCALHOST PORT]: " << index << " | " << 12345 << std::endl;

		/*if (!new_socket.connection.connect(addy))
			return 0;*/

		//std::cout << "[PASSED CONNECTION]: " << index << " | " << 12345 << std::endl;
	}

	void* userdata = lua_newuserdata(L, sizeof(wsocket_t));
	//*reinterpret_cast<wsocket_t*>(userdata) = std::move(new_socket);

	lua_createtable(L, 0, 0);
	lua_pushcfunction(L, WebSocket__index, 0);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "table");
	lua_setfield(L, -2, "__type");
	lua_setmetatable(L, -2);

	websockets[*(int*)userdata] = std::move(new_socket);
	
	return 1;
}

/* registering */
static const luaL_Reg env_funcs[] = {
	{"connect", WebSocket_Connect},
	{NULL, NULL},
};

auto cherry::environment::luauopen_websocket(lua_State* L) -> void {
	lua_createtable(L, 0, 0);
	lua_setglobal(L, "WebSocket");

	luaL_register(L, "WebSocket", env_funcs);

	lua_getglobal(L, "WebSocket");
	lua_setreadonly(L, -1, true);

	lua_settop(L, 0);
}