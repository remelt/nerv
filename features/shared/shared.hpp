#pragma once

#include "../../main.hpp"
#include "item_schema.hpp"
#include "data.hpp"

#include "../skin_changer/skin_changer.hpp"
#include "../glove_changer/glove_changer.hpp"
#include "../agent_changer/agent_changer.hpp"

class c_changer {
public:
	void run();
	void on_build_material_hook(void* vector, c_econ_item_view* m_item);
};

inline const auto g_changer = std::make_unique<c_changer>();