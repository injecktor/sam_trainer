#include "sam_funcs.hpp"

bool disable_receive_health = false;
bool disable_receive_armor = false;

PVOID serious_engine = nullptr;
DWORD player_handle = 0;
PVOID player_entity = nullptr;

void __thiscall game_functions::ReceiveHealth(long health_change, long arg1) {
    print_log("Hooked. ");
    print_log("Entity: 0x%p, health_change: %ld\n", this, health_change);
    if (player_entity) {
        print_log("PLayer entity: 0x%p\n", player_entity);
    }
    if (disable_receive_health && player_entity == reinterpret_cast<PVOID>(this) && health_change < 0) {
        health_change = 0;
    }
    auto org_func = reinterpret_cast<void(__thiscall*)(void*, long, long)>(receive_health->orig_func);
    org_func(this, health_change, arg1);
}
void  __thiscall game_functions::ReceiveArmor(long armor_change, long arg1) {
    print_log("Hooked. ");
    print_log("Entity: 0x%p, armor_change: %ld\n", this, armor_change);
    if (disable_receive_armor && player_entity == reinterpret_cast<PVOID>(this) && armor_change < 0) {
        armor_change = 0;
    }
    auto org_func = reinterpret_cast<void(__thiscall*)(void*, long, long)>(receive_armor->orig_func);
    org_func(this, armor_change, arg1);
}

__declspec(naked) void GetGroupMover() {
    __asm {
        push ebp
        mov ebp, esp
        call get_group_mover_orig_func
        mov ecx, edi
        mov serious_engine, ecx
        mov ecx, [ecx + 0x28]
        mov player_handle, ecx
        mov player_entity, eax
        leave
        ret
    }
}
