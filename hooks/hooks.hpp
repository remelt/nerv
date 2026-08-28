#pragma once

class CLightBinnerGpu;
class CAggregateSceneObject;
class CSceneObjectInfo;


class c_hook {
	void* m_function = nullptr;
	void* m_detour = nullptr;
	void* m_trampoline = nullptr;
public:
	bool hook(void* target, void* dtr) {
		m_function = target;
		m_detour = dtr;

		MH_STATUS create_status = MH_CreateHook(m_function, m_detour, &m_trampoline);
		if (create_status != MH_OK) {
			LOG_ERROR(xorstr_("[c_hook] MH_CreateHook failed: %d"), create_status);
			return false;
		}

		MH_STATUS enable_status = MH_EnableHook(m_function);
		if (enable_status != MH_OK) {
			LOG_ERROR(xorstr_("[c_hook] MH_EnableHook failed: %d"), enable_status);
			return false;
		}

		return true;
	}

	bool hook(uintptr_t target, void* dtr) {
		return hook((void*)target, dtr);
	}

	template <typename tOriginal>
	tOriginal get_original() {
		return reinterpret_cast<tOriginal>(m_trampoline);
	}

	void unhook() {
		if (!m_function || !m_detour)
			return;

		MH_DisableHook(m_function);
	}
};

enum e_frame_stage {
	FRAME_UNDEFINED = -1,
	FRAME_START,
	FRAME_NET_UPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	FRAME_NET_UPDATE_END,
	FRAME_RENDER_START,
	FRAME_RENDER_END
};

class i_csgo_input;

namespace hooks {

	namespace mouse_input_enabled {
		inline c_hook m_mouse_input_enabled;
		bool __fastcall hk_mouse_input_enabled(void* rcx);
	}

	namespace enable_cursor {
		inline c_hook m_enable_cursor;
		inline bool m_enable_cursor_input = true;
		void* __fastcall hk_enable_cursor(void* rcx, bool active);
	}

	namespace frame_stage_notify {
		inline c_hook m_frame_stage_notify;
		void hk_frame_stage_notify(void* source_to_client, int stage);
	}

	namespace fire_event_client_side {
		inline c_hook m_fire_event_client_side;
		bool __fastcall hk_fire_event_client_side(void* p_game_event_manager, void* p_game_event);
	}

	namespace level_init {
		inline c_hook m_level_init;
		__int64 __fastcall hk_level_init(void* rcx, void* rdx);
	}

	namespace smoke_voxel_draw {
		inline c_hook m_smoke_voxel_draw;
		void* __fastcall hk_smoke_voxel_draw(void* a1, void* a2, int a3, int a4, void* a5, void* a6);
	}

	namespace draw_flashbang_overlay {
		inline c_hook m_draw_flashbang_overlay;
		void __fastcall hk_draw_flashbang_overlay(void* a1, int a2, __int64* a3, __int64 a4, __m128* a5);
	}

	namespace present {
		inline c_hook m_present;
		HRESULT hk_present(IDXGISwapChain* swap_chain, unsigned int sync_interval, unsigned int flags);
	}

	namespace resize_buffers {
		inline c_hook m_resize_buffers;
		HRESULT hk_resize_buffers(IDXGISwapChain* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags);
	}

	namespace create_swap_chain {
		inline c_hook m_create_swap_chain;
		HRESULT __stdcall hk_create_swap_chain(IDXGIFactory* factory, IUnknown* device, DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swap_chain);
	}

	namespace build_material
	{
		inline c_hook m_build_material;
		void* __fastcall hk_build_material(void* rcx, void* weapon, void* vector);
	}

	namespace draw_array_light
	{
		inline c_hook m_draw_array_light;
		void __fastcall hk_draw_array_light(CLightBinnerGpu* pLightBinnerGPU, CAggregateSceneObject* pAggregateSceneObject, CSceneObjectInfo* a3);
	}

}

class c_hooks {
public:
	bool initialize();
	void destroy();
};

inline const auto g_hooks = std::make_unique<c_hooks>();
