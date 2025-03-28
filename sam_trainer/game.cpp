#include "game.hpp"

PVOID serious_engine;
DWORD player_handle;
PVOID player_entity;

void __thiscall Entity::ReceiveHealth(long health_change, long arg1) {
    print_log("Hooked. ");
    print_log("Entity: 0x%p, health_change: %ld\n", this, health_change);
    print_log("PLayer entity: 0x%p\n", player_entity);
    if (player_entity == reinterpret_cast<PVOID>(this) && health_change < 0) {
        health_change = 0;
    }
    auto org_func = reinterpret_cast<void(__thiscall*)(void*, long, long)>(receive_health->orig_func);
    org_func(this, health_change, arg1);
}
void __thiscall Entity::ReceiveArmor(long armor_change, long arg1) {
    print_log("Hooked. ");
    print_log("Entity: 0x%p, armor_change: %ld\n", this, armor_change);
    if (player_entity == reinterpret_cast<PVOID>(this) && armor_change < 0) {
        armor_change = 0;
    }
    auto org_func = reinterpret_cast<void(__thiscall*)(void*, long, long)>(receive_armor->orig_func);
    org_func(this, armor_change, arg1);
}
void __thiscall Entity::GetPlayer(PVOID arg1) {
    print_log("Hooked. ");
    print_log("SeriousEngine: 0x%p, arg1: 0x%p\n", this, arg1);
    auto org_func = reinterpret_cast<void(__thiscall*)(void*, PVOID)>(get_player->orig_func);
    org_func(this, arg1);
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