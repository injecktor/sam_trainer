#include "sam_trainer.hpp"

s_module *sam_trainer_dll, *sam2game_dll, *core_dll;

s_func *receive_health, *receive_armor, *hv_handle_to_pointer, *get_player, *get_group_mover;

PVOID get_group_mover_orig_func;

void init_module(s_module** module, LPCTSTR name) {
    *module = new s_module(sam_process, name);
}

void init_modules() {
    init_module(&sam_trainer_dll, _T("sam_trainer.dll"));
    init_module(&sam2game_dll, _T("Sam2Game.dll"));
    init_module(&core_dll, _T("Core.dll"));
}

void deinit() {
    delete sam2game_dll;
    delete core_dll;
    delete receive_health;
}

void init_func(s_func** func, LPCTSTR func_name, s_module* module,
    string pattern, string mask, PVOID detour_func, bool hook = true, bool hex_string = false) {
    *func = new s_func(func_name, module, pattern, mask, detour_func, hook, hex_string);
}

void init_funcs() {
    init_func(&receive_health, _T("ReceiveHealth"), sam2game_dll, 
        "55 8B EC 83 EC 0C 56 8B F1 FF 15 50 29 AA 0E 85 C0 0F 85 8A 00 00 00 8B 86 88 04 00 00 8B 55 08 8B 8E C4 00 00 00 89 45 F4",
        "xxxxxxxxxxxxx??xxxxxxxxxxxxxxxxxxxxxxxxxx",
        reinterpret_cast<PVOID>(method_to_ptr(&Entity::ReceiveHealth)), true);
    init_func(&hv_handle_to_pointer, _T("hvHandleToPointer"), core_dll,
        "55 8B EC 6A FF 68 98 78 BB 00 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 83 EC 0C 6A 01",
        "xxxxxxxx??xxxxxxxxxxxxxxxxxxx",
        nullptr, false);
    init_func(&receive_armor, _T("ReceiveArmor"), sam2game_dll,
        "55 8B EC 83 EC 0C 56 8B F1 FF 15 50 29 D4 0E 85 C0 0F 85 8A 00 00 00 8B 86 8C 04 00 00 8B 55 08 8B 8E C8 00 00 00 89 45 F4",
        "xxxxxxxxxxxxx??xxxxxxxxxxxxxxxxxxxxxxxxxx",
        reinterpret_cast<PVOID>(method_to_ptr(&Entity::ReceiveArmor)), true);
    init_func(&get_player, _T("GetPlayer"), sam2game_dll,
        "55 8B EC 83 EC 08 53 56 33 F6 57 89 4D F8 89 75 FC 83 CB FF 8D 79 24 8B 07 50 FF 15 C8 21",
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 
        reinterpret_cast<PVOID>(method_to_ptr(&Entity::GetPlayer)), true);
    init_func(&get_group_mover, _T("GetGroupMover"), sam2game_dll,
        "55 8B EC 6A FF 68 08 82 5D 10 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 83 EC 14 56 8B F1 \
        8B 46 28 57 8B 3D C8 21 BB 0E 50 FF D7 83 C4 04 85 C0 75 10 5F",
        "xxxxxxxx??xxxxxxxxxxxxxxxxxxxxxxxxxxxx??xxxxxxxxxxx",
        GetGroupMover, true);

    get_group_mover_orig_func = get_group_mover->orig_func;
}

VOID attach() {
    sam_process = GetCurrentProcess();
    if (!sam_process) {
        print_log("Couldn't find process");
        return;
    }
    init_modules();
    init_funcs();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch( fdwReason ) { 
        case DLL_PROCESS_ATTACH:
            log_file = fopen(log_file_path.c_str(), "w");
            fprintf(log_file, "DLL_PROCESS_ATTACH\n");
            fclose(log_file);
            attach();
            break;

        case DLL_THREAD_ATTACH:
            print_log("DLL_THREAD_ATTACH\n");
            break;

        case DLL_THREAD_DETACH:
            print_log("DLL_THREAD_DETACH\n");
            break;

        case DLL_PROCESS_DETACH:
            print_log("DLL_PROCESS_DETACH\n");

            deinit();
            
            if (lpvReserved != nullptr) {
                break; 
            }
            break;
    }
    return TRUE;
}