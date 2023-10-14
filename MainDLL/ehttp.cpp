#include "environment.hpp"
#include "cpr/cpr.h"
#include <cpr/HttpStatus.hpp>

enum request_methods
{
	H_GET,
	H_HEAD,
	H_POST,
	H_PUT,
	H_DELETE,
	H_OPTIONS
};

std::map<std::string, request_methods> request_method_map =
{
	{ "get", H_GET },
	{ "head", H_HEAD },
	{ "post", H_POST },
	{ "put", H_PUT },
	{ "delete", H_DELETE },
	{ "options", H_OPTIONS }
};

std::map<uint32_t, std::string> http_cache = {};
auto cherry::environment::httpget(lua_State* L) -> int {
	ARG_CHECK(L, 2, 3, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_checktype(L, 2, LUA_TSTRING);

	bool block_cache = false;
	if (lua_gettop(L) == 3) {
		luaL_checktype(L, 3, LUA_TBOOLEAN);
		block_cache = lua_toboolean(L, 3);
	}

	roblox_yield_t ryield(L);
	std::string url = luaL_checkstring(L, 2);

	if (url.find("http") != 0) {
		luaL_error(L, "Invalid protocol specified (expected 'http://' or 'https://')");
		return 0;
	}

	uint32_t HASH = FNV1A(url.c_str());
	std::string data = http_cache[HASH];

	if (http_cache[HASH].length() != 0 && block_cache == false) {
		lua_pushlstring(L, data.c_str(), data.size());
		return 1;
	}

	return ryield.execute([url, HASH]() {
		auto result = cpr::Get(cpr::Url{ url }, cpr::Header{ {"User-Agent", "Roblox/WinInet"} });

		return [result, url, HASH](lua_State* L) {
			if (HttpStatus::IsError(result.status_code)) {
				auto err = "Http Error " + std::to_string(result.status_code) + " - " + HttpStatus::ReasonPhrase(result.status_code);
				luaL_error(L, err.c_str());
				return 0;
			}

			http_cache[HASH] = result.text;

			lua_pushlstring(L, result.text.c_str(), result.text.size());
			return 1;
		};
		});
}

int cherry::environment::httppost(lua_State* L)
{
	ARG_CHECK(L, 4, 4, luaL_checktype(L, 1, LUA_TUSERDATA););
	std::string url = luaL_checkstring(L, 2);
	std::string data = lua_tostring(L, 3);
	auto content_type = luaL_checkstring(L, 4);

	if (url.find("http") != 0) {
		luaL_error(L, "Invalid protocol specified (expected 'http://' or 'https://')");
		return 0;
	}

	 roblox_yield_t ryield(L);

	return ryield.execute([url, data, content_type]() {
		auto result = cpr::Post(cpr::Url{ url },
			cpr::Header{ {"User-Agent", "Roblox/WinInet"} },
			cpr::Body{ data },
			cpr::Header{ { "Content-Type", content_type} }
		);

		return [result](lua_State* L) {
			if (HttpStatus::IsError(result.status_code)) {
				auto err = "Http Error " + std::to_string(result.status_code) + " - " + HttpStatus::ReasonPhrase(result.status_code);
				luaL_error(L, err.c_str());
				return 0;
			}

			lua_pushlstring(L, result.text.c_str(), result.text.size());
			return 1;
		};
	});
}

int request(lua_State* L)
{
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TTABLE););

	lua_getfield(L, 1, "Url");
	if (lua_type(L, -1) != LUA_TSTRING)
	{
		luaL_error(L, "Invalid or no 'Url' field specified in request table");
		return 0;
	}

	std::string url = lua_tostring(L, -1);

	if (url.find("http") != 0)
	{
		luaL_error(L, "Invalid protocol specified (expected 'http://' or 'https://')");
		return 0;
	}

	lua_pop(L, 1);

	auto method = H_GET;
	lua_getfield(L, 1, "Method");
	if (lua_type(L, -1) == LUA_TSTRING)
	{
		std::string methods = luaL_checkstring(L, -1);
		std::transform(methods.begin(), methods.end(), methods.begin(), tolower);

		if (!request_method_map.count(methods))
		{
			luaL_error(L, "Request type '%s' is not a valid http request type."), methods.c_str();
			return 0;
		}

		method = request_method_map[methods];
	}

	lua_pop(L, 1);

	cpr::Header headers;

	lua_getfield(L, 1, "Headers");
	if (lua_type(L, -1) == LUA_TTABLE)
	{
		lua_pushnil(L);

		while (lua_next(L, -2))
		{
			if (lua_type(L, -2) != LUA_TSTRING || lua_type(L, -1) != LUA_TSTRING)
			{
				luaL_error(L, "'Headers' table must contain string keys/values.");
				return 0;
			}
			std::string header_key = luaL_checkstring(L, -2);
			auto header_copy = std::string(header_key);
			std::transform(header_key.begin(), header_key.end(), header_key.begin(), tolower);

			if (header_copy == "content-length")
			{
				luaL_error(L, "Headers: 'Content-Length' header cannot be overwritten.");
				return 0;
			}

			std::string header_value = luaL_checkstring(L, -1);

			headers.insert({ header_key, header_value });

			lua_pop(L, 1);
		}

	}

	lua_pop(L, 1);

	cpr::Cookies cookies;
	lua_getfield(L, 1, "Cookies");

	if (lua_type(L, -1) == LUA_TTABLE)
	{
		std::map<std::string, std::string> rcookies;
		lua_pushnil(L);

		while (lua_next(L, -2))
		{
			if (lua_type(L, -2) != LUA_TSTRING || lua_type(L, -1) != LUA_TSTRING)
			{
				luaL_error(L, "'Cookies' table must contain string keys/values.");
				return 0;
			}

			std::string cookie_key = luaL_checkstring(L, -2);
			std::string cookie_value = luaL_checkstring(L, -1);

			rcookies[cookie_key] = cookie_value;

			lua_pop(L, 1);
		}

		cookies = rcookies;
	}

	lua_pop(L, 1);

	auto has_user_agent = false;
	for (auto& header : headers)
	{
		auto header_name = header.first;
		std::transform(header_name.begin(), header_name.end(), header_name.begin(), tolower);

		if (header_name == "user-agent")
			has_user_agent = true;
	}

	if (!has_user_agent)
	{
		headers.insert({ "User-Agent", std::string( cherry::configuration::source_name ) });
	}

	std::string body;
	lua_getfield(L, 1, "Body");
	if (lua_type(L, -1) == LUA_TTABLE)
	{
		if (method == H_GET || method == H_HEAD)
		{
			luaL_error(L, "'Body' cannot be present in GET or HEAD requests.");
			return 0;
		}

		size_t body_size;
		const auto body_cstr = luaL_checklstring(L, -1, &body_size);
		body = std::string(body_cstr, body_size);
	}

	lua_pop(L, 1);



	roblox_yield_t ryield(L);

	return ryield.execute([method, url, headers, cookies, body]()
		{
			cpr::Response response;

			switch (method)
			{
			case H_GET:
			{
				response = cpr::Get(
					cpr::Url{ url },
					cookies,
					headers
				);

				break;
			}

			case H_HEAD:
			{
				response = cpr::Head(
					cpr::Url{ url },
					cookies,
					headers
				);

				break;
			}

			case H_POST:
			{
				response = cpr::Post(
					cpr::Url{ url },
					cpr::Body{ body },
					cookies,
					headers
				);

				break;
			}

			case H_PUT:
			{
				response = cpr::Put(
					cpr::Url{ url },
					cpr::Body{ body },
					cookies,
					headers
				);

				break;
			}

			case H_DELETE:
			{
				response = cpr::Delete(
					cpr::Url{ url },
					cpr::Body{ body },
					cookies,
					headers
				);

				break;
			}

			case H_OPTIONS:
			{
				response = cpr::Options(
					cpr::Url{ url },
					cpr::Body{ body },
					cookies,
					headers
				);

				break;
			}

			default:
			{
				throw std::exception("invalid request type");
			}
			}

			return [response](lua_State* L)
			{
				lua_newtable(L);

				lua_pushboolean(L, HttpStatus::IsSuccessful(response.status_code));
				lua_setfield(L, -2, "Success");

				lua_pushinteger(L, response.status_code);
				lua_setfield(L, -2, "StatusCode");

				std::string phrase = HttpStatus::ReasonPhrase(response.status_code);
				lua_pushlstring(L, phrase.c_str(), phrase.size());
				lua_setfield(L, -2, "StatusMessage");

				lua_newtable(L);

				for (auto& header : response.header)
				{
					lua_pushlstring(L, header.first.c_str(), header.first.size());
					lua_pushlstring(L, header.second.c_str(), header.second.size());

					lua_settable(L, -3);
				}

				lua_setfield(L, -2, "Headers");

				lua_newtable(L);

				for (auto& cookie : response.cookies.map_)
				{
					lua_pushlstring(L, cookie.first.c_str(), cookie.first.size());
					lua_pushlstring(L, cookie.second.c_str(), cookie.second.size());

					lua_settable(L, -3);
				}

				lua_setfield(L, -2, "Cookies");

				lua_pushlstring(L, response.text.c_str(), response.text.size());
				lua_setfield(L, -2, "Body");

				return 1;
			};
		});
}

/* registering */

/* I have to fix request not resuming */
static const luaL_Reg env_funcs[] = { // {"NAME", FUNC},
	{"httpget", cherry::environment::httpget},
	{"httppost", cherry::environment::httppost},
	{"httpgetasync", cherry::environment::httpget},
	{"httppostasync", cherry::environment::httppost},
	{"request", request},
	{"http_request", request},
	{NULL, NULL},
};

void cherry::environment::luauopen_http(lua_State* L)
{
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, NULL, env_funcs);

	lua_newtable(L);

	lua_pushcclosure(L, request, NULL, 0);
	lua_setfield(L, -2, "request");

	lua_setreadonly(L, -1, true);
	lua_setglobal(L, "http");

	lua_pop(L, 1);
}