#include <ImGUI/imgui.h>
#include <ImGUI/imgui_internal.h>
#include <ImGUI/imgui_impl_dx11.h>
#include <ImGUI/imstb_truetype.h>
#include <ImGUI/TextEditor.h>
#include <ImGUI/imgui_impl_dx11.h>
#include <ImGUI/imgui_internal.h>
#include <D3D11.h>
#include <d3dcompiler.h>

enum drawing_enum {
	text,
	line,
	square,
	triangle,
	circle,
	quad,
	image
};

class base_t {
public:
	bool visible = false;
	int zindex = 1;
	drawing_enum type;
};

class line_t : public base_t {
public:
	ImVec2 from = { 0.f, 0.f };
	ImVec2 to = { 0.f, 0.f };
	ImVec4 color = { 0.f, 0.f, 0.f, 0.f };

	int thickness = 16;
	float transparency = 1.f;
};

class circle_t : public base_t {
public:
	ImVec2 position = { 0.f, 0.f };
	ImVec4 color = { 0.f, 0.f, 0.f, 0.f };

	bool filled = false;
	float transparency = 1.f;

	int radius = 0;
	int thickness = 15;
	int numsides = 250;
};

class square_t : public base_t {
public:
	ImVec2 position = { 0.f, 0.f };
	ImVec2 size = { 0.f, 0.f };
	ImVec4 color = { 0.f, 0.f, 0.f, 0.f };

	int thickness = 16;

	bool filled = false;

	float transparency = 1.f;
};

class triangle_t : public base_t {
public:
	ImVec2 pointa = { 0.f, 0.f };
	ImVec2 pointb = { 0.f, 0.f };
	ImVec2 pointc = { 0.f, 0.f };

	ImVec4 color = { 0.f, 0.f, 0.f, 0.f };

	int thickness = 15;

	bool filled = false;

	float transparency = 1.f;
};

class quad_t : public base_t {
public:
	ImVec2 pointa = { 0.f, 0.f };
	ImVec2 pointb = { 0.f, 0.f };
	ImVec2 pointc = { 0.f, 0.f };
	ImVec2 pointd = { 0.f, 0.f };

	ImVec4 color = { 0.f, 0.f, 0.f, 0.f };

	int thickness = 16;

	bool filled = false;

	float transparency = 1.f;
};

class text_t : public base_t {
public:
	char text[512];

	ImVec2 position = { 0.f, 0.f };
	ImVec4 color = { 0.f, 0.f, 0.f, 0.f };
	ImVec4 outlinecolor = { 0.f, 0.f, 0.f, 0.f };

	int font = 0;

	bool center = false;
	bool outline = false;

	float size = 16;
	float transparency = 1.f;

	ImVec2 textbounds = { 0.f, 16.f };
};

class image_t : public base_t {
public:
	char data[512];

	ImVec2 position = { 0.f, 0.f };
	ImVec4 color = { 0.f, 0.f, 0.f, 0.f };
	ImVec2 size = { 0.f, 0.f };

	float transparency = 1.f;
	int rounding = 0;
};

struct color_t {
	float r, g, b;

	operator ImVec4() { return { r, g, b, 1.f }; }

	operator ImVec4() const { return { r, g, b, 1.f }; }
};

typedef HRESULT(__stdcall* ID3D11Present)(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT(__stdcall* ID3D11ResizeBuffers)(IDXGISwapChain* SwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
typedef LRESULT(__stdcall* ID3D11WindowProcess)(HWND, UINT, WPARAM, LPARAM);