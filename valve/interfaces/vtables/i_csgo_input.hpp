#pragma once
#include "../../../sdk/typedefs/vec_t.hpp"
#include "../../../sdk/vfunc/vfunc.hpp"
#include "../../../utils/utils.hpp"

class i_csgo_input
{
public:

    char pad_0000[0x228];
    bool block_shot;
    bool in_thirdperson;
    char pad_0252[0x6];
    vec3_t third_person_angles;
    char pad_0264[0x04];
    uint64_t button_pressed;
    uint64_t mouse_button_pressed;
    uint64_t button_un_pressed;
    uint64_t keyboard_copy;
    float forward_move;
    float left_move;
    float up_move;
    int mouse_delta_x;
    int mouse_delta_y;
    int32_t subtick_count;

    void* subticks[0xC];
    vec3_t view_angles;
    int32_t target_entity_index;
    char pad_03E0[0x230];
    int32_t attack_history_1;
    int32_t attack_history_2;
    int32_t attack_history_3;
    char pad_061C[0x4];
    int32_t message_size;
    char pad_0624[0x4];
    void* message;

};