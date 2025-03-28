#include "game.hpp"

PVOID serious_engine;
DWORD player_handle;
PVOID player_entity;

void __fastcall ReceiveHealth(PVOID entity, long health_change, long arg1) {
    print_log("Hooked. ");
    print_log("Entity: 0x%p, health_change: %ld\n", entity, health_change);
    print_log("PLayer entity: 0x%p\n", player_entity);
    if (player_entity == reinterpret_cast<PVOID>(entity) && health_change < 0) {
        health_change = 0;
    }
    auto org_func = reinterpret_cast<void(__thiscall*)(void*, long, long)>(receive_health->orig_func);
    org_func(entity, health_change, arg1);
}
void __fastcall ReceiveArmor(PVOID entity, long armor_change, long arg1) {
    print_log("Hooked. ");
    print_log("Entity: 0x%p, armor_change: %ld\n", entity, armor_change);
    if (player_entity == reinterpret_cast<PVOID>(entity) && armor_change < 0) {
        armor_change = 0;
    }
    auto org_func = reinterpret_cast<void(__thiscall*)(void*, long, long)>(receive_armor->orig_func);
    org_func(entity, armor_change, arg1);
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