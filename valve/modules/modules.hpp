#pragma once

#include <Windows.h>
#include <memory>

class c_dll {
	HMODULE m_dll;
	size_t m_size;
	char* m_name;
public:
	c_dll() : m_dll(nullptr), m_size(0), m_name(nullptr) { }
	c_dll(const char* name) : m_dll(GetModuleHandleA(name)), m_size(0), m_name(nullptr) {
		if (!m_dll)
			return;

		auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(m_dll);
		auto nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uintptr_t>(m_dll) + dos_header->e_lfanew);
		m_size = nt_header->OptionalHeader.SizeOfImage;

		if (!name)
			return;

		m_name = new char[256];
		strcpy_s(m_name, 256, name);
	}

	uintptr_t get() {
		return reinterpret_cast<uintptr_t>(m_dll);
	}

	const char* get_name() {
		return m_name;
	}
};

class c_modules {
private:
	struct modules_t {
		c_dll tier0_dll{};
		c_dll client_dll{};
		c_dll input_system{};
		c_dll schemasystem_dll{};
		c_dll filesystem_stdio{};
		c_dll localize_dll{};
		c_dll engine_dll{};
		c_dll panorama_dll{};
		c_dll resourcesystem_dll{};
		c_dll rendersystem_dll{};
		c_dll scenesystem_dll{};
		c_dll afxhooksource2_dll{};
		void initialize();
	};

public:
	modules_t m_modules{};
};

inline const auto g_modules = std::make_unique<c_modules>();
