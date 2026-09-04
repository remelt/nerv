#include <windowsx.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include "../main.hpp"
#include "directx.hpp"
#include "../menu/menu.hpp"
#include "../hooks/hooks.hpp"
#include"../sdk/config_system/config_system.hpp"

using namespace hooks;

extern bool g_waiting_for_key;

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK wnd_proc_hook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	
	if (!g_directx->get_imgui_init_status())
		return true;

	if (g_waiting_for_key && msg == WM_KEYDOWN)
	{
		
		if (wparam == VK_ESCAPE)
		{
			g_waiting_for_key = false;
		}
		else
		{
			
			g_cfg->misc.m_menu_key = (int)wparam;
			g_waiting_for_key = false;
			extern void save_last_state(const std::string & config_name);
			save_last_state(g_config_system->m_selected_config);
		}

		return true; 
	}


	if (msg == WM_KEYDOWN && wparam == g_cfg->misc.m_menu_key)
	{
		g_menu->m_opened = !g_menu->m_opened;
		const bool now_open = g_menu->m_opened;
		auto& io = ImGui::GetIO();

		if (!now_open) {
			
			io.MouseDown[0] = io.MouseDown[1] = io.MouseDown[2] = false;
		}

		using fn = decltype(&hooks::enable_cursor::hk_enable_cursor);
		auto original = hooks::enable_cursor::m_enable_cursor.get_original<fn>();
		if (original) {
			
			const bool requested_game_cursor_state = now_open ? false : true;
			__try {
				original(g_interfaces->m_input_system, g_menu->m_opened ? false : enable_cursor::m_enable_cursor_input);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {}
		}
		return true;
	}

	if (g_menu->m_opened) {
		
		ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);

		auto& io = ImGui::GetIO();
		if (io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput) {
			if (msg == WM_SETCURSOR) return 1;
			if (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) return true;
			if (msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_CHAR ||
				msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP) return true;
		}
	}

	return CallWindowProcA(g_directx->get_wnd_proc_original(), hwnd, msg, wparam, lparam);
}
void c_directx::initialize() {

	if (!g_interfaces->m_swap_chain)
		return;

	// initializing device
	g_interfaces->m_swap_chain->pDXGISwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&m_device);
	CHECK(xorstr_("Device Instance"), m_device);

	if (!m_device)
		return;

	m_device->GetImmediateContext(&m_device_context);
	CHECK(xorstr_("Device Context"), m_device_context);

	m_present_address = vmt::get_v_method(g_interfaces->m_swap_chain->pDXGISwapChain, 8);
	CHECK(xorstr_("Present"), m_present_address);

	m_resize_buffers_address = vmt::get_v_method(g_interfaces->m_swap_chain->pDXGISwapChain, 13);
	CHECK(xorstr_("Resize Buffer"), m_resize_buffers_address);

	// initializing wndproc
	DXGI_SWAP_CHAIN_DESC desc;
	g_interfaces->m_swap_chain->pDXGISwapChain->GetDesc(&desc);
	m_window = desc.OutputWindow;

	m_wnd_proc_original = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(m_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wnd_proc_hook)));

	// creating swap chain hook
	IDXGIDevice* pDXGIDevice = NULL;
	m_device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));

	IDXGIAdapter* pDXGIAdapter = NULL;
	pDXGIDevice->GetAdapter(&pDXGIAdapter);

	IDXGIFactory* pIDXGIFactory = NULL;
	pDXGIAdapter->GetParent(IID_PPV_ARGS(&pIDXGIFactory));

	m_create_swap_chain_address = vmt::get_v_method(pIDXGIFactory, 10);

	pDXGIDevice->Release();
	pDXGIDevice = nullptr;
	pDXGIAdapter->Release();
	pDXGIAdapter = nullptr;
	pIDXGIFactory->Release();
	pIDXGIFactory = nullptr;
}

void c_directx::uninitialize() {
	if (m_wnd_proc_original && m_window) {
		SetWindowLongPtrA(m_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_wnd_proc_original));
		m_wnd_proc_original = nullptr;
	}

	destroy_render_target();

	if (m_imgui_initialized) {
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		m_imgui_initialized = false;
	}

	if (m_device_context) { m_device_context->Release(); m_device_context = nullptr; }
	if (m_device) { m_device->Release(); m_device = nullptr; }

	m_swap_chain = nullptr;
	m_window = nullptr;
	m_started = false;
	m_initial_cursor_synced = false;
}

void c_directx::create_render_target() {
	if (!m_swap_chain || !m_device)
		return;

	ID3D11Texture2D* back_buffer = nullptr;
	if (SUCCEEDED(m_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer)))) {
		if (back_buffer) {
			m_device->CreateRenderTargetView(back_buffer, nullptr, &m_render_target);
			back_buffer->Release();
			back_buffer = nullptr;
		}
	}
}

void c_directx::destroy_render_target() {
	if (m_render_target) {
		m_render_target->Release();
		m_render_target = nullptr;
	}
}

void c_directx::update_dpi_scale() {
	if (!m_imgui_initialized || !m_window)
		return;

	RECT rect;
	if (!GetClientRect(m_window, &rect))
		return;

	int height = rect.bottom - rect.top;

	float scale;
	if (height <= 0)
		scale = 1.f;
	else
		scale = height / 1080.0f;

	scale = std::clamp(scale, 0.5f, 4.f);

	if (std::abs(scale - g_menu->get_dpi_scale()) > 0.01f) {
		g_menu->rebuild_fonts(scale);
	}
}

void c_directx::start_frame(IDXGISwapChain* swap_chain)
{
	m_swap_chain = swap_chain;

	if (!m_render_target)
		create_render_target();

	if (m_render_target)
		m_device_context->OMSetRenderTargets(1, &m_render_target, nullptr);

	if (!m_started)
	{
		m_swap_chain = swap_chain;

		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::StyleColorsDark();
		ImGui_ImplWin32_Init(m_window);
		ImGui_ImplDX11_Init(m_device, m_device_context);

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = m_window;

		m_imgui_initialized = true;
		m_started = true;
	}

	update_dpi_scale();
}

void c_directx::new_frame() {
	if (!m_imgui_initialized)
		return;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void c_directx::end_frame() {
	if (!m_imgui_initialized)
		return;

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}