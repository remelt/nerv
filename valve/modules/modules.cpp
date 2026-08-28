#include "../../main.hpp"

#define REQUIRE(field, name) \
	field = c_dll(xorstr_(name)); \
	if (!field.get()) { \
		LOG_ERROR(xorstr_("[-] missing %s"), name); \
		MessageBoxA(NULL, name, "Missing module", MB_OK | MB_ICONERROR); \
		return; \
	}

void c_modules::modules_t::initialize() {
	REQUIRE(tier0_dll,			"tier0.dll");
	REQUIRE(client_dll,			"client.dll");
	REQUIRE(input_system,		"inputsystem.dll");
	REQUIRE(schemasystem_dll,	"schemasystem.dll");
	REQUIRE(filesystem_stdio,	"filesystem_stdio.dll");
	REQUIRE(engine_dll,			"engine2.dll");
	REQUIRE(panorama_dll,		"panorama.dll");
	REQUIRE(resourcesystem_dll,	"resourcesystem.dll");
	REQUIRE(rendersystem_dll,	"rendersystemdx11.dll");
	REQUIRE(scenesystem_dll,	"scenesystem.dll");
	afxhooksource2_dll = c_dll(xorstr_("afxhooksource2.dll"));
	localize_dll = c_dll(xorstr_("localize.dll"));
	if (!localize_dll.get())
		LOG_ERROR(xorstr_("[-] localize.dll missing — names will use raw tokens"));
}
