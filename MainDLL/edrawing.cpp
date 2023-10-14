#include "environment.hpp"
#include "drawing_lib_structs.hpp"
#include <fonts.hpp>

bool initialize = false;
bool ui_currently_shown = false;

DXGI_SWAP_CHAIN_DESC D11SwapChain;
IDXGISwapChain* D11GSwapChain = nullptr;

ID3D11Present D11Present = nullptr;
ID3D11ResizeBuffers D11Resize = nullptr;
ID3D11WindowProcess D11WindowProcess = nullptr;

ID3D11Device* D11Device = nullptr;
ID3D11DeviceContext* D11Context = nullptr;
ID3D11RenderTargetView* D11RenderTargetView = nullptr;

ID3D11Texture2D* D11TextureBuffer = nullptr;

HWND Window = nullptr;


std::vector<base_t*> drawing_vector;
std::vector<ImFont*> fonts;

extern IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return std::string(); //No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}

#pragma region INTERNAL_UI
auto DrawUI() -> void {
	if (ui_currently_shown == false)
		return;

	ImGui::PushFont(fonts.at(1));
	ImGui::SetNextWindowSize(ImVec2(480, 285), ImGuiCond_FirstUseEver);

	static auto VectorGetter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};



}
#pragma endregion


int drawline = 0, drawcircle = 0, drawsquare = 0, drawtriangle = 0, drawquad = 0, drawtext = 0, drawimage = 0;
auto vector2_new(lua_State* L, const ImVec2& vector) -> void {
	lua_getfield(L, -10002, "Vector2");
	lua_getfield(L, -1, "new");

	lua_pushnumber(L, vector.x);
	lua_pushnumber(L, vector.y);

	lua_pcall(L, 2, 1, 0);
}

auto color3_new(lua_State* L, const ImVec4& color) -> void {
	lua_getfield(L, -10002, "Color3");
	lua_getfield(L, -1, "new");

	lua_pushnumber(L, color.x);
	lua_pushnumber(L, color.y);
	lua_pushnumber(L, color.z);

	lua_pcall(L, 3, 1, 0);
}

auto tovec2(lua_State* L, int index) -> ImVec2 {
	return *reinterpret_cast<const ImVec2*>(lua_topointer(L, index));
}

auto tocol3(lua_State* L, int Index) -> ImVec4 {
	return *reinterpret_cast<const color_t*>(lua_topointer(L, Index));
}
/* namespaces */
namespace drawing_lib {
	namespace base {
		auto remove(lua_State* L) -> int {
			ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TUSERDATA););

			const auto item = reinterpret_cast<base_t*>(lua_touserdata(L, 1));
			if (const auto iter = std::find(drawing_vector.begin(), drawing_vector.end(), item); iter != drawing_vector.end())
				drawing_vector.erase(iter);

			return 0;
		}

		auto clear(lua_State* L) -> int {
			ARG_CHECK(L, 0, 0);
			drawing_vector.clear();
			return 0;
		}

		auto index(lua_State* L, base_t* object, const std::string& drawing_object) -> void {
			if (!std::strcmp(drawing_object.c_str(), "Visible"))
				lua_pushboolean(L, object->visible);

			else if (!std::strcmp(drawing_object.c_str(), "ZIndex"))
				lua_pushinteger(L, object->zindex);

			else if (!std::strcmp(drawing_object.c_str(), "Remove"))
				lua_pushcclosure(L, remove, 0, 0);

			else if (!std::strcmp(drawing_object.c_str(), "Destroy"))
				lua_pushcclosure(L, remove, 0, 0);

			else if (!std::strcmp(drawing_object.c_str(), "__OBJECT_EXISTS"))
				lua_pushboolean(L, true);
		}

		auto newindex(lua_State* L, base_t* object, const std::string& drawing_object) -> void {
			if (!std::strcmp(drawing_object.c_str(), "Visible"))
				object->visible = lua_toboolean(L, 3);

			else if (!std::strcmp(drawing_object.c_str(), "ZIndex"))
				object->zindex = lua_tointeger(L, 3);
		}
	}

	namespace line {
		auto index(lua_State* L) -> int {
			ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);
			const auto obj = reinterpret_cast<line_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "From"))
				vector2_new(L, obj->from);

			else if (!std::strcmp(drawing_object, "To"))
				vector2_new(L, obj->to);

			else if (!std::strcmp(drawing_object, "Color"))
				color3_new(L, obj->color);

			else if (!std::strcmp(drawing_object, "Thickness"))
				lua_pushnumber(L, obj->thickness);

			else if (!std::strcmp(drawing_object, "Transparency"))
				lua_pushnumber(L, obj->transparency);
			else
				base::index(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 1;
		}

		auto newindex(lua_State* L) -> int {
			ARG_CHECK(L, 3, 3, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);
			const auto obj = reinterpret_cast<line_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "From"))
				obj->from = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "To"))
				obj->to = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "Color"))
				obj->color = tocol3(L, 3);

			else if (!std::strcmp(drawing_object, "Thickness"))
				obj->thickness = lua_tointeger(L, 3);

			else if (!std::strcmp(drawing_object, "Transparency"))
				obj->transparency = (float)lua_tonumber(L, 3);
			else
				base::newindex(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 0;
		}
	}

	namespace circle {
		auto index(lua_State* L) -> int {
			ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);
			const auto obj = reinterpret_cast<circle_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "Position"))
				vector2_new(L, obj->position);

			else if (!std::strcmp(drawing_object, "Radius"))
				lua_pushnumber(L, obj->radius);

			else if (!std::strcmp(drawing_object, "Color"))
				color3_new(L, obj->color);

			else if (!std::strcmp(drawing_object, "Thickness"))
				lua_pushnumber(L, obj->thickness);

			else if (!std::strcmp(drawing_object, "Filled"))
				lua_pushboolean(L, obj->filled);

			else if (!std::strcmp(drawing_object, "NumSides"))
				lua_pushnumber(L, obj->numsides);

			else if (!std::strcmp(drawing_object, "Transparency"))
				lua_pushnumber(L, obj->transparency);
			else
				base::index(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 1;
		}

		int newindex(lua_State* L)
		{
			ARG_CHECK(L, 3, 3, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);
			const auto obj = reinterpret_cast<circle_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "Position"))
				obj->position = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "Radius"))
				obj->radius = lua_tointeger(L, 3);

			else if (!std::strcmp(drawing_object, "Color"))
				obj->color = tocol3(L, 3);

			else if (!std::strcmp(drawing_object, "Thickness"))
				obj->thickness = lua_tointeger(L, 3);

			else if (!std::strcmp(drawing_object, "Filled"))
				obj->filled = lua_toboolean(L, 3);

			else if (!std::strcmp(drawing_object, "NumSides"))
				obj->numsides = lua_tointeger(L, 3);

			else if (!std::strcmp(drawing_object, "Transparency"))
				obj->transparency = (float)lua_tonumber(L, 3);
			else
				base::newindex(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 0;
		}
	}

	namespace square {
		auto index(lua_State* L) -> int {
			ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);
			const auto obj = reinterpret_cast<square_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "Position"))
				vector2_new(L, obj->position);

			else if (!std::strcmp(drawing_object, "Size"))
				vector2_new(L, obj->size);

			else if (!std::strcmp(drawing_object, "Color"))
				color3_new(L, obj->color);

			else if (!std::strcmp(drawing_object, "Thickness"))
				lua_pushnumber(L, obj->thickness);

			else if (!std::strcmp(drawing_object, "Transparency"))
				lua_pushnumber(L, obj->transparency);

			else if (!std::strcmp(drawing_object, "Filled"))
				lua_pushboolean(L, obj->filled);

			else
				base::index(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 1;
		}

		auto newindex(lua_State* L) -> int {
			ARG_CHECK(L, 3, 3, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);
			const auto obj = reinterpret_cast<square_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "Position"))
				obj->position = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "Size"))
				obj->size = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "Color"))
				obj->color = tocol3(L, 3);

			else if (!std::strcmp(drawing_object, "Thickness"))
				obj->thickness = lua_tointeger(L, 3);

			else if (!std::strcmp(drawing_object, "Transparency"))
				obj->transparency = (float)lua_tonumber(L, 3);

			else if (!std::strcmp(drawing_object, "Filled"))
				obj->filled = lua_toboolean(L, 3);

			else
				base::newindex(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 0;
		}
	}

	namespace triangle {
		auto index(lua_State* L) -> int {
			ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);
			const auto obj = reinterpret_cast<triangle_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "PointA"))
				vector2_new(L, obj->pointa);

			else if (!std::strcmp(drawing_object, "PointB"))
				vector2_new(L, obj->pointb);

			else if (!std::strcmp(drawing_object, "PointC"))
				vector2_new(L, obj->pointc);

			else if (!std::strcmp(drawing_object, "Color"))
				color3_new(L, obj->color);

			else if (!std::strcmp(drawing_object, "Thickness"))
				lua_pushnumber(L, obj->thickness);

			else if (!std::strcmp(drawing_object, "Transparency"))
				lua_pushnumber(L, obj->transparency);

			else if (!std::strcmp(drawing_object, "Filled"))
				lua_pushboolean(L, obj->filled);

			else
				base::index(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 1;
		}

		auto newindex(lua_State* L) -> int {
			ARG_CHECK(L, 3, 3, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);
			const auto obj = reinterpret_cast<triangle_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, ("PointA")))
				obj->pointa = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "PointB"))
				obj->pointb = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "PointC"))
				obj->pointc = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "Color"))
				obj->color = tocol3(L, 3);

			else if (!std::strcmp(drawing_object, "Thickness"))
				obj->thickness = lua_tointeger(L, 3);

			else if (!std::strcmp(drawing_object, "Transparency"))
				obj->transparency = (float)lua_tonumber(L, 3);

			else if (!std::strcmp(drawing_object, "Filled"))
				obj->filled = lua_toboolean(L, 3);

			else
				base::newindex(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 0;
		}
	}

	namespace quad {
		auto index(lua_State* L) -> int {
			ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);
			const auto obj = reinterpret_cast<quad_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "PointA"))
				vector2_new(L, obj->pointa);

			else if (!std::strcmp(drawing_object, "PointB"))
				vector2_new(L, obj->pointb);

			else if (!std::strcmp(drawing_object, "PointC"))
				vector2_new(L, obj->pointc);

			else if (!std::strcmp(drawing_object, "PointD"))
				vector2_new(L, obj->pointd);

			else if (!std::strcmp(drawing_object, "Color"))
				color3_new(L, obj->color);

			else if (!std::strcmp(drawing_object, "Thickness"))
				lua_pushnumber(L, obj->thickness);

			else if (!std::strcmp(drawing_object, "Transparency"))
				lua_pushnumber(L, obj->transparency);

			else if (!std::strcmp(drawing_object, "Filled"))
				lua_pushboolean(L, obj->filled);

			else
				base::index(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 1;
		}

		auto newindex(lua_State* L) -> int {
			ARG_CHECK(L, 3, 3, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);

			const auto obj = reinterpret_cast<quad_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "PointA"))
				obj->pointa = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "PointB"))
				obj->pointb = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "PointC"))
				obj->pointc = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "PointD"))
				obj->pointd = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "Color"))
				obj->color = tocol3(L, 3);

			else if (!std::strcmp(drawing_object, "Thickness"))
				obj->thickness = lua_tointeger(L, 3);

			else if (!std::strcmp(drawing_object, "Transparency"))
				obj->transparency = (float)lua_tonumber(L, 3);

			else if (!std::strcmp(drawing_object, "Filled"))
				obj->filled = lua_toboolean(L, 3);

			else
				base::newindex(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 0;
		}
	}

	namespace text {
		auto index(lua_State* L) -> int {
			ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);

			const auto obj = reinterpret_cast<text_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "Text"))
				lua_pushstring(L, obj->text);

			else if (!std::strcmp(drawing_object, "Position"))
				vector2_new(L, obj->position);

			else if (!std::strcmp(drawing_object, "Color"))
				color3_new(L, obj->color);

			else if (!std::strcmp(drawing_object, "Center"))
				lua_pushboolean(L, obj->center);

			else if (!std::strcmp(drawing_object, "Outline"))
				lua_pushboolean(L, obj->outline);

			else if (!std::strcmp(drawing_object, "OutlineColor"))
				color3_new(L, obj->outlinecolor);

			else if (!std::strcmp(drawing_object, "Size"))
				lua_pushnumber(L, obj->size);

			else if (!std::strcmp(drawing_object, "Transparency"))
				lua_pushnumber(L, obj->transparency);

			else if (!std::strcmp(drawing_object, "Font"))
				lua_pushnumber(L, obj->font);

			else if (!std::strcmp(drawing_object, "TextBounds"))
			{
				ImFont* font = fonts.at(obj->font);

				if (font == nullptr)
					font = fonts.at(0);

				const ImVec2 text_size = font->CalcTextSizeA(obj->size, FLT_MAX, 0.0f, obj->text);
				vector2_new(L, text_size);
			}
			else
				base::index(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 1;
		}

		auto newindex(lua_State* L) -> int {
			ARG_CHECK(L, 3, 3, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);
			const auto obj = reinterpret_cast<text_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "Text"))
			{
				const char* str = luaL_checklstring(L, 3, nullptr);
#pragma warning(suppress : 4996)
				std::strcpy(obj->text, str);
			}

			else if (!std::strcmp(drawing_object, "Position"))
				obj->position = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "Color"))
				obj->color = tocol3(L, 3);

			else if (!std::strcmp(drawing_object, "OutlineColor"))
				obj->outlinecolor = tocol3(L, 3);

			else if (!std::strcmp(drawing_object, "Center"))
				obj->center = lua_toboolean(L, 3);

			else if (!std::strcmp(drawing_object, "Outline"))
				obj->outline = lua_toboolean(L, 3);

			else if (!std::strcmp(drawing_object, "Size"))
				obj->size = (float)lua_tonumber(L, 3);

			else if (!std::strcmp(drawing_object, "Transparency"))
				obj->transparency = (float)lua_tonumber(L, 3);
			else if (!std::strcmp(drawing_object, "Font"))
				obj->font = lua_tointeger(L, 3);
			else if (!std::strcmp(drawing_object, "TextBounds"))
				luaL_error(L, "TextBounds is a read only property for text");
			else
				base::newindex(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 0;
		}
	}

	namespace image {
		auto index(lua_State* L) -> int {
			ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);
			const auto obj = reinterpret_cast<image_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "Data"))
				lua_pushstring(L, obj->data);

			else if (!std::strcmp(drawing_object, "Position"))
				vector2_new(L, obj->position);

			else if (!std::strcmp(drawing_object, "Color"))
				color3_new(L, obj->color);

			else if (!std::strcmp(drawing_object, "Size"))
				vector2_new(L, obj->size);

			else if (!std::strcmp(drawing_object, "Rounding"))
				lua_pushnumber(L, obj->rounding);

			else if (!std::strcmp(drawing_object, "Transparency"))
				lua_pushnumber(L, obj->transparency);

			else
				base::index(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 1;
		}

		auto newindex(lua_State* L) -> int {
			ARG_CHECK(L, 3, 3, luaL_checktype(L, 1, LUA_TUSERDATA););
			luaL_checktype(L, 2, LUA_TSTRING);


			const auto obj = reinterpret_cast<image_t*>(lua_touserdata(L, 1));
			const auto drawing_object = lua_tostring(L, 2);

			if (!std::strcmp(drawing_object, "Data"))
			{
				const char* str = luaL_checklstring(L, 3, nullptr);
#pragma warning(suppress : 4996)
				std::strcpy(obj->data, str);
			}

			else if (!std::strcmp(drawing_object, "Position"))
				obj->position = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "Color"))
				obj->color = tocol3(L, 3);

			else if (!std::strcmp(drawing_object, "Size"))
				obj->size = tovec2(L, 3);

			else if (!std::strcmp(drawing_object, "Rounding"))
				obj->rounding = lua_tointeger(L, 3);

			else
				base::newindex(L, dynamic_cast<base_t*>(obj), drawing_object);

			return 1;
		}
	}
}

auto drawing_new(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1, luaL_checktype(L, 1, LUA_TSTRING););
	const auto drawing_object = lua_tostring(L, 1);

	if (!std::strcmp(drawing_object, "Line")) {
		const auto ud = reinterpret_cast<line_t*>(lua_newuserdata(L, sizeof(line_t)));

		*ud = line_t{};
		ud->type = line;

		lua_rawgeti(L, LUA_REGISTRYINDEX, drawline);
		lua_setmetatable(L, -2);

		drawing_vector.push_back(ud);
	}
	else if (!std::strcmp(drawing_object, "Circle")) {
		const auto ud = reinterpret_cast<circle_t*>(lua_newuserdata(L, sizeof(circle_t)));

		*ud = circle_t{};
		ud->type = circle;

		lua_rawgeti(L, LUA_REGISTRYINDEX, drawcircle);
		lua_setmetatable(L, -2);

		drawing_vector.push_back(ud);
	}
	else if (!std::strcmp(drawing_object, "Square")) {
		const auto ud = reinterpret_cast<square_t*>(lua_newuserdata(L, sizeof(square_t)));

		*ud = square_t{};
		ud->type = square;

		lua_rawgeti(L, LUA_REGISTRYINDEX, drawsquare);
		lua_setmetatable(L, -2);

		drawing_vector.push_back(ud);
	}
	else if (!std::strcmp(drawing_object, "Triangle")) {
		const auto ud = reinterpret_cast<triangle_t*>(lua_newuserdata(L, sizeof(triangle_t)));

		*ud = triangle_t{};
		ud->type = triangle;

		lua_rawgeti(L, LUA_REGISTRYINDEX, drawtriangle);
		lua_setmetatable(L, -2);

		drawing_vector.push_back(ud);
	}
	else if (!std::strcmp(drawing_object, "Quad")) {
		const auto ud = reinterpret_cast<quad_t*>(lua_newuserdata(L, sizeof(quad_t)));

		*ud = quad_t{};
		ud->type = quad;

		lua_rawgeti(L, LUA_REGISTRYINDEX, drawquad);
		lua_setmetatable(L, -2);

		drawing_vector.push_back(ud);
	}
	else if (!std::strcmp(drawing_object, "Text")) {
		const auto ud = reinterpret_cast<text_t*>(lua_newuserdata(L, sizeof(text_t)));

		*ud = text_t{};
		ud->type = text;

		lua_rawgeti(L, LUA_REGISTRYINDEX, drawtext);
		lua_setmetatable(L, -2);

		drawing_vector.push_back(ud);
	}
	else if (!std::strcmp(drawing_object, "Image")) {
		const auto ud = reinterpret_cast<image_t*>(lua_newuserdata(L, sizeof(image_t)));

		*ud = image_t{};
		ud->type = image;

		lua_rawgeti(L, LUA_REGISTRYINDEX, drawimage);
		lua_setmetatable(L, -2);

		drawing_vector.push_back(ud);
	}
	else {
		lua_pushnil(L);
	}

	return 1;
}

auto isrenderobj(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1);
	lua_pushboolean(L, (strcmp(luaL_typename(L, 1), "DrawingObject") == 0));
	return 1;
}

auto getrenderproperty(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_checktype(L, 2, LUA_TSTRING);

	const auto drawing_property = reinterpret_cast<base_t*>(lua_touserdata(L, 1));

	switch (drawing_property->type) {
	case line:
	{
		drawing_lib::line::index(L);
		break;
	}
	case circle:
	{
		drawing_lib::circle::index(L);
		break;
	}
	case square:
	{
		drawing_lib::square::index(L);
		break;
	}
	case triangle:
	{
		drawing_lib::triangle::index(L);
		break;
	}
	case quad:
	{
		drawing_lib::quad::index(L);
		break;
	}
	case text:
	{
		drawing_lib::text::index(L);
		break;
	}
	case image:
	{
		drawing_lib::image::index(L);
		break;
	}
	default:
		break;
	}

	return 1;
}

auto setrenderproperty(lua_State* L) -> int {
	ARG_CHECK(L, 3, 3, luaL_checktype(L, 1, LUA_TUSERDATA););
	luaL_checktype(L, 2, LUA_TSTRING);
	luaL_checkany(L, 3);

	const auto drawing_property = reinterpret_cast<base_t*>(lua_touserdata(L, 1));
	switch (drawing_property->type) {
	case line:
	{
		drawing_lib::line::newindex(L);
		break;
	}
	case circle:
	{
		drawing_lib::circle::newindex(L);
		break;
	}
	case square:
	{
		drawing_lib::square::newindex(L);
		break;
	}
	case triangle:
	{
		drawing_lib::triangle::newindex(L);
		break;
	}
	case quad:
	{
		drawing_lib::quad::newindex(L);
		break;
	}
	case text:
	{
		drawing_lib::text::newindex(L);
		break;
	}
	case image:
	{
		drawing_lib::image::newindex(L);
		break;
	}
	default:
		break;
	}

	return 1;
}

auto __stdcall WindowProcess(const HWND WindowProcess, UINT Message, WPARAM W_Param, LPARAM L_Param) -> HRESULT {
	if (true && ImGui_ImplWin32_WndProcHandler(WindowProcess, Message, W_Param, L_Param))
		return true;

	return CallWindowProc(D11WindowProcess, WindowProcess, Message, W_Param, L_Param);
}

auto __stdcall PresentHook(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags) -> HRESULT {
	if (!initialize)
	{
		if (SUCCEEDED(D11GSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&D11Device))))
		{
			swap_chain->GetDesc(&D11SwapChain);
			swap_chain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&D11Device));
			swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&D11TextureBuffer));

			
			Window = D11SwapChain.OutputWindow;
			D11WindowProcess = (WNDPROC)SetWindowLongPtr(Window, GWL_WNDPROC, (LONG_PTR)WindowProcess);

			D11Device->GetImmediateContext(&D11Context);
			D11Device->CreateRenderTargetView(D11TextureBuffer, nullptr, &D11RenderTargetView);
			D11TextureBuffer->Release();

			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();

			ImFontConfig Config{};
			Config.OversampleH = 3;
			Config.OversampleV = 3;

			fonts.emplace_back(io.Fonts->AddFontFromMemoryCompressedTTF(seoge_ui_compressed_data, seoge_ui_compressed_size, 32.0f, &Config));
			fonts.emplace_back(io.Fonts->AddFontDefault(&Config));
			fonts.emplace_back(io.Fonts->AddFontFromMemoryCompressedTTF(ibm_plex_compressed_data, ibm_plex_compressed_size, 32.0f, &Config));
			fonts.emplace_back(io.Fonts->AddFontFromMemoryCompressedTTF(sometype_mono_compressed_data, sometype_mono_compressed_size, 32.0f, &Config));


			io.Fonts->TexGlyphPadding = 1;
			for (auto n = 0; n < io.Fonts->ConfigData.Size; n++)
			{
				auto* FontCfg = (ImFontConfig*)&io.Fonts->ConfigData[n];
				FontCfg->RasterizerMultiply = 1.0f;
				FontCfg->RasterizerFlags = 0;
			}

			io.Fonts->Build();
			

			io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
			io.IniFilename = NULL;

			ImGui_ImplDX11_Init(Window, D11Device, D11Context);
			ImGui_ImplDX11_CreateDeviceObjects();

			//std::cout << std::hex << " DRAWING LIB WINDOW hWnd - " << ImGui::GetCurrentWindow()-> << std::endl;

			//EnumWindows(EnumWindowsProc, GetCurrentProcessId());

			initialize = true;
		}

		return D11Present(swap_chain, sync_interval, flags);
	}

	if (D11RenderTargetView == nullptr)
	{
		swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&D11TextureBuffer));
		D11Device->CreateRenderTargetView(D11TextureBuffer, nullptr, &D11RenderTargetView);

		D11TextureBuffer->Release();
	}

	ImGui_ImplDX11_NewFrame();
	//ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::Begin("BackBuffer", reinterpret_cast<bool*>(true), ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);

	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);

	auto render_list = ImGui::GetCurrentWindow();
	//render_list->
	for (const auto shape : drawing_vector)
	{
		if (shape->type == line)
		{
			const auto line = reinterpret_cast<line_t*>(shape);

			if (line->visible)
				render_list->DrawList->AddLine({ line->from.x, line->from.y }, { line->to.x, line->to.y }, ImColor(line->color.x, line->color.y, line->color.z, line->transparency), (float)line->thickness);
		}
		if (shape->type == circle)
		{
			const auto circle = reinterpret_cast<circle_t*>(shape);

			if (circle->visible)
			{
				if (circle->filled)
					render_list->DrawList->AddCircleFilled({ circle->position.x, circle->position.y }, (float)circle->radius, ImColor(circle->color.x, circle->color.y, circle->color.z, circle->transparency), circle->numsides);
				else
					render_list->DrawList->AddCircle({ circle->position.x, circle->position.y }, (float)circle->radius, ImColor(circle->color.x, circle->color.y, circle->color.z, circle->transparency), circle->numsides, (float)circle->thickness);
			}
		}
		if (shape->type == square)
		{
			const auto square = reinterpret_cast<square_t*>(shape);

			if (square->visible)
			{
				if (square->filled)
					render_list->DrawList->AddRectFilled({ square->position.x, square->position.y }, { square->position.x + square->size.x, square->position.y + square->size.y }, ImColor(square->color.x, square->color.y, square->color.z, square->transparency));
				else
					render_list->DrawList->AddRect({ square->position.x, square->position.y }, { square->position.x + square->size.x, square->position.y + square->size.y }, ImColor(square->color.x, square->color.y, square->color.z, square->transparency), 0, 0, (float)square->thickness);
			}
		}
		if (shape->type == triangle)
		{
			const auto triangle = reinterpret_cast<triangle_t*>(shape);

			if (triangle->visible)
			{
				if (triangle->filled)
					render_list->DrawList->AddTriangleFilled({ triangle->pointa.x, triangle->pointa.y }, { triangle->pointb.x, triangle->pointb.y }, { triangle->pointc.x, triangle->pointc.y }, ImColor(triangle->color.x, triangle->color.y, triangle->color.z, triangle->transparency));
				else
					render_list->DrawList->AddTriangle({ triangle->pointa.x, triangle->pointa.y }, { triangle->pointb.x, triangle->pointb.y }, { triangle->pointc.x, triangle->pointc.y }, ImColor(triangle->color.x, triangle->color.y, triangle->color.z, triangle->transparency), (float)triangle->thickness);
			}
		}
		if (shape->type == quad)
		{
			const auto quad = reinterpret_cast<quad_t*>(shape);

			if (quad->visible)
			{
				if (quad->filled)
					render_list->DrawList->AddQuadFilled({ quad->pointa.x, quad->pointa.y }, { quad->pointb.x, quad->pointb.y }, { quad->pointc.x, quad->pointc.y }, { quad->pointd.x, quad->pointd.y }, ImColor(quad->color.x, quad->color.y, quad->color.z, quad->transparency));
				else
					render_list->DrawList->AddQuad({ quad->pointa.x, quad->pointa.y }, { quad->pointb.x, quad->pointb.y }, { quad->pointc.x, quad->pointc.y }, { quad->pointd.x, quad->pointd.y }, ImColor(quad->color.x, quad->color.y, quad->color.z, quad->transparency), (float)quad->thickness);
			}
		}
		if (shape->type == text)
		{
			const auto text_data = reinterpret_cast<text_t*>(shape);
			
			if (text_data->visible) {
				ImFont* font = fonts.at(text_data->font);

				if (font == nullptr)
					font = fonts.at(0);

				ImGui::PushFont(font);

				std::stringstream stream(std::string{ text_data->text });
				std::string line;

				auto y = 0.0f;
				auto i = 0;

				while (std::getline(stream, line))
				{
					const auto TextSize = font->CalcTextSizeA(text_data->size, FLT_MAX, 0.0f, line.c_str());

					if (text_data->center)
					{
						if (text_data->outline)
						{
							render_list->DrawList->AddText(
								font,
								text_data->size,
								ImVec2(text_data->position.x - TextSize.x / 2.0f + 1, text_data->position.y + TextSize.y * i + 1),
								ImColor(ImVec4(text_data->outlinecolor.x, text_data->outlinecolor.y, text_data->outlinecolor.z, text_data->transparency)),
								line.c_str()
							);
							
							render_list->DrawList->AddText(font, text_data->size, ImVec2(text_data->position.x - TextSize.x / 2.0f - 1, text_data->position.y + TextSize.y * i - 1), ImColor(ImVec4(text_data->outlinecolor.x, text_data->outlinecolor.y, text_data->outlinecolor.z, text_data->transparency)), line.c_str());
							render_list->DrawList->AddText(font, text_data->size, ImVec2(text_data->position.x - TextSize.x / 2.0f + 1, text_data->position.y + TextSize.y * i - 1), ImColor(ImVec4(text_data->outlinecolor.x, text_data->outlinecolor.y, text_data->outlinecolor.z, text_data->transparency)), line.c_str());
							render_list->DrawList->AddText(font, text_data->size, ImVec2(text_data->position.x - TextSize.x / 2.0f - 1, text_data->position.y + TextSize.y * i + 1), ImColor(ImVec4(text_data->outlinecolor.x, text_data->outlinecolor.y, text_data->outlinecolor.z, text_data->transparency)), line.c_str());
						}

						render_list->DrawList->AddText(font, text_data->size, ImVec2(text_data->position.x - TextSize.x / 2.0f, text_data->position.y + TextSize.y * i), ImColor(ImVec4(text_data->color.x, text_data->color.y, text_data->color.z, text_data->transparency)), line.c_str());
					}
					else
					{
						if (text_data->outline)
						{
							render_list->DrawList->AddText(font, text_data->size, ImVec2(text_data->position.x + 1, text_data->position.y + TextSize.y * i + 1), ImColor(ImVec4(text_data->outlinecolor.x, text_data->outlinecolor.y, text_data->outlinecolor.z, text_data->transparency)), line.c_str());
							render_list->DrawList->AddText(font, text_data->size, ImVec2(text_data->position.x - 1, text_data->position.y + TextSize.y * i - 1), ImColor(ImVec4(text_data->outlinecolor.x, text_data->outlinecolor.y, text_data->outlinecolor.z, text_data->transparency)), line.c_str());
							render_list->DrawList->AddText(font, text_data->size, ImVec2(text_data->position.x + 1, text_data->position.y + TextSize.y * i - 1), ImColor(ImVec4(text_data->outlinecolor.x, text_data->outlinecolor.y, text_data->outlinecolor.z, text_data->transparency)), line.c_str());
							render_list->DrawList->AddText(font, text_data->size, ImVec2(text_data->position.x - 1, text_data->position.y + TextSize.y * i + 1), ImColor(ImVec4(text_data->outlinecolor.x, text_data->outlinecolor.y, text_data->outlinecolor.z, text_data->transparency)), line.c_str());
						}

						render_list->DrawList->AddText(font, text_data->size, ImVec2(text_data->position.x, text_data->position.y + TextSize.y * i), ImColor(ImVec4(text_data->color.x, text_data->color.y, text_data->color.z, text_data->transparency)), line.c_str());
					}

					y = text_data->position.y + TextSize.y * (i + 1);
					i++;
				}

				ImGui::PopFont();
			}
		}
		if (shape->type == image)
		{
			const auto image = reinterpret_cast<image_t*>(shape);

			if (image->visible)
			{

				//ImVec2 display_min = ImVec2(0.0f, 0.0f);
				//ImVec2 display_size = ImVec2(1080.0f, 1920.0f);

				//ImVec2 uv0 = ImVec2(display_min.x / image->size.x, display_min.y / image->size.y);

				//// Normalized coordinates of pixel (110,210) in a 256x256 texture.
				//ImVec2 uv1 = ImVec2((display_min.x + display_size.x) / image->size.x, (display_min.y + display_size.y) / image->size.y);

				//ImTextureID texture = (void*)image->data;

				//ImGuiWindow* window = ImGui::GetCurrentWindow();
				//if (window->SkipItems)
				//	continue;

				//ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + image->size.x, window->DC.CursorPos.y + image->size.y));

				//if (image->transparency > 0.0f)
				//	bb.Max = ImVec2(bb.Max.x + 2, bb.Max.y + 2);

				//ImGui::ItemSize(bb);
				//if (!ImGui::ItemAdd(bb, 0))
				//	continue;

				//if (image->transparency > 0.0f)
				//{
				//	window->DrawList->AddRect(bb.Min, bb.Max, ImColor( ImVec4(image->color.x, image->color.y, image->color.z, image->transparency) ), 0.0f);
				//	window->DrawList->AddImageRounded(
				//		texture, 
				//		ImVec2(bb.Min.x + 1, bb.Min.y + 1),  
				//		ImVec2(bb.Max.x - 1, bb.Max.y - 1), 
				//		uv0, uv1, 
				//		ImGui::GetColorU32(ImVec4(1, 1, 1, 1)),
				//		image->rounding
				//	);
				//}
				//else
				//{
				//	window->DrawList->AddImageRounded(
				//		texture, 
				//		bb.Min, 
				//		bb.Max, 
				//		uv0, uv1, 
				//		ImGui::GetColorU32(ImVec4(1, 1, 1, 1)),
				//		image->rounding
				//	);
				//}

				/*ImGui::Image();
				render_list->DrawList->AddImageRounded(
					texture,
					ImVec2(image->size.x, image->size.x), 
					ImVec2(image->size.y, image->size.y), 
					uv0,
					uv1,
					ImColor( ImVec4(image->color.x, image->color.y, image->color.z, image->transparency) ), 
					image->rounding
				);*/

				/*auto size = 0;
				const auto TextSize = ImGui::CalcTextSize(image->data, nullptr, true);

				render_list->DrawList->AddText(fonts.at(1), 16, ImVec2(image->position.x / 1.1f + 15, image->position.y), ImColor(image->color.x, image->color.y, image->color.z, image->transparency), image->data);*/
			}
		}
	}

	auto WWindow = ImGui::GetCurrentWindow();
	WWindow->DrawList->PushClipRectFullScreen();

	ImGui::End();
	ImGui::PopStyleColor();
	//ImGui::EndFrame();
	ImGui::Render();

	D11Context->OMSetRenderTargets(1, &D11RenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return D11Present(swap_chain, sync_interval, flags);
}

auto __stdcall ResizeBuffersHook(IDXGISwapChain* this_ptr, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags) -> HRESULT {
	if (D11RenderTargetView) {
		D11RenderTargetView->Release();
		D11RenderTargetView = nullptr;
	}

	Window = D11SwapChain.OutputWindow;

	return D11Resize(this_ptr, buffer_count, width, height, new_format, swap_chain_flags);
}

auto lua_member(lua_State* rl, const char* name, lua_CFunction function) -> void {
	lua_pushcclosure(rl, function, nullptr, 0);
	lua_setfield(rl, -2, name);
}


auto luaLR_newmetatable(lua_State* L, int idx = 0) -> int {
	if (idx != 0) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, idx); // get registry.idx

		if (!lua_isnil(L, -1))                     // idx already in use?
			return 0;

		lua_pop(L, 1);
	}

	lua_newtable(L); // create metatable
	return lua_ref(L, -1);
}

LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/* registering */
auto cherry::environment::luauopen_drawing(lua_State* L) -> void {
	drawline = luaLR_newmetatable(L);
	lua_member(L, "__index", drawing_lib::line::index);
	lua_member(L, "__newindex", drawing_lib::line::newindex);

	lua_pushstring(L, "DrawingObject");
	lua_setfield(L, -2, "__type");

	drawcircle = luaLR_newmetatable(L);
	lua_member(L, "__index", drawing_lib::circle::index);
	lua_member(L, "__newindex", drawing_lib::circle::newindex);

	lua_pushstring(L, "DrawingObject");
	lua_setfield(L, -2, "__type");

	drawsquare = luaLR_newmetatable(L);
	lua_member(L, "__index", drawing_lib::square::index);
	lua_member(L, "__newindex", drawing_lib::square::newindex);

	lua_pushstring(L, "DrawingObject");
	lua_setfield(L, -2, "__type");

	drawtriangle = luaLR_newmetatable(L);
	lua_member(L, "__index", drawing_lib::triangle::index);
	lua_member(L, "__newindex", drawing_lib::triangle::newindex);

	lua_pushstring(L, "DrawingObject");
	lua_setfield(L, -2, "__type");

	drawquad = luaLR_newmetatable(L);
	lua_member(L, "__index", drawing_lib::quad::index);
	lua_member(L, "__newindex", drawing_lib::quad::newindex);

	lua_pushstring(L, "DrawingObject");
	lua_setfield(L, -2, "__type");

	drawtext = luaLR_newmetatable(L);
	lua_member(L, "__index", drawing_lib::text::index);
	lua_member(L, "__newindex", drawing_lib::text::newindex);

	lua_pushstring(L, "DrawingObject");
	lua_setfield(L, -2, "__type");

	drawimage = luaLR_newmetatable(L);
	lua_member(L, "__index", drawing_lib::image::index);
	lua_member(L, "__newindex", drawing_lib::image::newindex);

	lua_pushstring(L, "DrawingObject");
	lua_setfield(L, -2, "__type");

	lua_newtable(L);
	lua_member(L, "new", drawing_new);
	lua_member(L, "clear", drawing_lib::base::clear);

	lua_newtable(L);

	lua_pushnumber(L, 0);
	lua_setfield(L, -2, "UI");
	lua_pushnumber(L, 1);
	lua_setfield(L, -2, "System");
	lua_pushnumber(L, 2);
	lua_setfield(L, -2, "Plex");
	lua_pushnumber(L, 3);
	lua_setfield(L, -2, "Monospace");

	lua_setfield(L, -2, "Fonts");

	lua_setfield(L, -10002, "Drawing");


	lua_pushcclosure(L, drawing_lib::base::clear, nullptr, 0);
	lua_setfield(L, LUA_GLOBALSINDEX, "cleardrawcache");

	lua_pushcclosure(L, isrenderobj, nullptr, 0);
	lua_setfield(L, LUA_GLOBALSINDEX, "isrenderobj");

	lua_pushcclosure(L, getrenderproperty, nullptr, 0);
	lua_setfield(L, LUA_GLOBALSINDEX, "getrenderproperty");

	lua_pushcclosure(L, setrenderproperty, nullptr, 0);
	lua_setfield(L, LUA_GLOBALSINDEX, "setrenderproperty");

	/* -------------------------------------------- */
	//Window = FindWindowExW(NULL, NULL, NULL, L"Roblox");

	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL obtained_level;

	DXGI_SWAP_CHAIN_DESC SwapChain;
	ZeroMemory(&SwapChain, sizeof(SwapChain));

	//const char* window_name = global::utilities->random_string(24).c_str();
	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX", NULL};
	RegisterClassExA(&wc);
	Window = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, FindWindowExW(NULL, NULL, NULL, L"Roblox"), NULL, wc.hInstance, NULL);

	//std::cout << std::hex << " DRAWING LIB WINDOW hWnd - " << Window << std::endl;

	/*if (SetWindowDisplayAffinity(Window, WDA_EXCLUDEFROMCAPTURE) == FALSE) {
		std::cout << GetLastErrorAsString() << std::endl;
	}*/

	SwapChain.BufferCount = 2;
	SwapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChain.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChain.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	SwapChain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	SwapChain.OutputWindow = Window;
	SwapChain.SampleDesc.Count = 1;
	SwapChain.Windowed = TRUE;
	SwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	SwapChain.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChain.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChain.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	SwapChain.BufferDesc.Width = 0;
	SwapChain.BufferDesc.Height = 0;

	SwapChain.BufferDesc.RefreshRate.Numerator = 60;
	SwapChain.BufferDesc.RefreshRate.Denominator = 1;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, 2, D3D11_SDK_VERSION, &SwapChain, &D11GSwapChain, &D11Device, &obtained_level, &D11Context);
	SwapChain.OutputWindow = Window;

	uintptr_t* vt_swapchain;
	std::memcpy(&vt_swapchain, reinterpret_cast<LPVOID>(D11GSwapChain), sizeof(uintptr_t));

	typedef BOOL(WINAPI* VirtualProtectFunction)(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
	VirtualProtectFunction vt_protect = reinterpret_cast<VirtualProtectFunction>(reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"kernelbase.dll"), "VirtualProtect"))); // make sure to xorstring

	DWORD old_protection;
	vt_protect(vt_swapchain, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &old_protection);

	D11Present = reinterpret_cast<decltype(D11Present)>(vt_swapchain[8]);
	D11Resize = reinterpret_cast<decltype(D11Resize)>(vt_swapchain[13]);

	vt_swapchain[8] = reinterpret_cast<std::uintptr_t>(&PresentHook);
	vt_swapchain[13] = reinterpret_cast<std::uintptr_t>(&ResizeBuffersHook);

	vt_protect(vt_swapchain, sizeof(std::uintptr_t), old_protection, &old_protection);
}