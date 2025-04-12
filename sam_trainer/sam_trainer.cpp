#include "sam_trainer.hpp"

FILE* log_file = nullptr;
string log_file_path = "C:\\Users\\p2282\\OneDrive\\Documents\\Visual Studio 2022\\projects\\sam_trainer\\sam_trainer_log.txt";

process_t sam_main, sam_imgui;

shared_ptr<s_module_t> sam_trainer_dll, sam2game_dll, core_dll, engine_dll;

shared_ptr<s_func_t> receive_health, receive_armor, hv_handle_to_pointer, get_group_mover, on_explode,
    update_freezing_exhalation, player_on_step, simulation_step, ray_get_hit_position;

PVOID get_group_mover_orig_func;

s_module_t::s_module_t(HANDLE process, LPCTSTR name) {
    if (!process) {
        print_log("Invalid process\n");
        return;
    }
    this->process = process;
    this->module = GetModuleHandle(name);
    if (!module) {
        print_log_error("Couldn't find module %ls handle\n", name);
        return;
    }
    this->name = name;
    BOOL res = GetModuleInformation(process, module, &module_info, sizeof(MODULEINFO));
    if (!res) {
        print_log_error("Couldn't find module %ls info\n", name);
        return;
    }
    print_log("Module name: %ls, address: 0x%p, size: 0x%x\n", name, module_info.lpBaseOfDll, module_info.SizeOfImage);
}

s_func_t::s_func_t(LPCTSTR func_name, shared_ptr<s_module_t> module, string pattern, string mask, 
    PVOID detour_func, bool hook, bool hex_string) {
    this->module = module;
    this->detour_func = detour_func;
    if (!hex_string) {
        pattern = trim(pattern);
        pattern = str_to_hex_str(pattern);
    }
    this->mask = trim(mask);
    this->orig_func = sig_scan(sam_main.handle, reinterpret_cast<BYTE*>(module->module_info.lpBaseOfDll),
        module->module_info.SizeOfImage, pattern, mask);
    print_log("Function: %ls\n", func_name);
    print_log("Original function address: 0x%p\n", orig_func);

    if (hook) {
        print_log("Detour function address: 0x%p\n", detour_func);
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(reinterpret_cast<PVOID*>(&orig_func), detour_func);
        DetourTransactionCommit();
    }
    print_log("\n");
}

__declspec(naked) void* __cdecl method_to_ptr(...) {
    __asm {
        mov eax, [esp + 4]
        retn
    }
}

void init_module(shared_ptr<s_module_t>* module, LPCTSTR name) {
    *module = make_shared<s_module_t>(sam_main.handle, name);
}

void init_modules() {
    init_module(&sam_trainer_dll, _T("sam_trainer.dll"));
    init_module(&sam2game_dll, _T("Sam2Game.dll"));
    init_module(&core_dll, _T("Core.dll"));
    init_module(&engine_dll, _T("Engine.dll"));
    print_log("\n");
}

void deinit() {
    if (sam_imgui.is_thread_active) {
        sam_imgui.is_thread_active = false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void init_func(shared_ptr<s_func_t>* func, LPCTSTR func_name, shared_ptr<s_module_t> module,
    string pattern, string mask, PVOID detour_func, bool hook = true, bool hex_string = false) {
    *func = make_shared<s_func_t>(func_name, module, pattern, mask, detour_func, hook, hex_string);
}

void init_funcs() {
    init_func(&receive_health, _T("ReceiveHealth"), sam2game_dll, 
        "55 8B EC 83 EC 0C 56 8B F1 FF 15 50 29 AA 0E 85 C0 0F 85 8A 00 00 00 8B 86 88 04 00 00 8B 55 08 8B 8E C4 00 00 00 89 45 F4",
        "xxxxxxxxxxxxx??xxxxxxxxxxxxxxxxxxxxxxxxxx",
        reinterpret_cast<PVOID>(method_to_ptr(&game_functions::ReceiveHealth)), true);
    init_func(&hv_handle_to_pointer, _T("hvHandleToPointer"), core_dll,
        "55 8B EC 6A FF 68 98 78 BB 00 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 83 EC 0C 6A 01",
        "xxxxxxxx??xxxxxxxxxxxxxxxxxxx",
        nullptr, false);
    init_func(&receive_armor, _T("ReceiveArmor"), sam2game_dll,
        "55 8B EC 83 EC 0C 56 8B F1 FF 15 50 29 D4 0E 85 C0 0F 85 8A 00 00 00 8B 86 8C 04 00 00 8B 55 08 8B 8E C8 00 00 00 89 45 F4",
        "xxxxxxxxxxxxx??xxxxxxxxxxxxxxxxxxxxxxxxxx",
        reinterpret_cast<PVOID>(method_to_ptr(&game_functions::ReceiveArmor)), true);
    init_func(&get_group_mover, _T("GetGroupMover"), sam2game_dll,
        "55 8B EC 6A FF 68 08 82 5D 10 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 83 EC 14 56 8B F1 \
        8B 46 28 57 8B 3D C8 21 BB 0E 50 FF D7 83 C4 04 85 C0 75 10 5F",
        "xxxxxxxx??xxxxxxxxxxxxxxxxxxxxxxxxxxxx??xxxxxxxxxxx",
        GetGroupMover, true);
    init_func(&on_explode, _T("OnExplode"), sam2game_dll,
        "55 8B EC 6A FF 68 82 A1 39 10 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 81 EC 9C 00 00 00",
        "xxxxxxxx??xxxxxxxxxxxxxxxxxxxx",
        nullptr, false);
    init_func(&update_freezing_exhalation, _T("UpdateFreezingExhalation"), sam2game_dll,
        "55 8B EC 83 EC 34 56 8B F1 8B 06 FF 90 14 04 00 00 85 C0 0F 84 24 02 00 00 8B 8E A0 08 00",
        "xxxxxxxx??xxxxxxxxxxxxx??xxxxx",
        nullptr, false);
    init_func(&player_on_step, _T("CPlayerPuppetEntity::OnStep"), sam2game_dll,
        "55 8B EC 6A FF 68 D9 99 CA 0E 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 83 EC 14 53 56 8B",
        "xxxxxxxx??xxxxxxxxxxxxx??xxxxx",
        nullptr, false);
    init_func(&simulation_step, _T("CSimulation::Step"), engine_dll,
        "55 8B EC 83 EC 14 53 56 57 8B F9 8B 5F 04 85 DB 8D 77 04 74 21 F6 43 04 01 74 1B 8B 03 8B",
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
        nullptr, false);
    init_func(&ray_get_hit_position, _T("rayGetHitPosition"), engine_dll,
        "55 8B EC 6A FF 68 42 1A C6 00 64 A1 00 00 00 00 50 64 89 25 00 00 00 00 83 EC 10 A1 64 31",
        "xxxxxxxx??xxxxxxxxxxxxxxxxxxxx",
        nullptr, false);

    get_group_mover_orig_func = get_group_mover->orig_func;
}

VOID attach() {
    sam_main.handle = GetCurrentProcess();
    if (!sam_main.handle) {
        print_log_error("Get process failed\n");
        return;
    }
    sam_main.id = GetCurrentProcessId();
    if (!sam_main.id) {
        print_log_error("Get process id failed\n");
        return;
    }
    sam_main.window_handle = get_window_handle(sam_main.id);
    if (!sam_main.window_handle) {
        print_log("Get window handle failed\n");
        return;
    }
    sam_main.window_procedure = nullptr;
    sam_main.window_class = WNDCLASSEXW();
    sam_main.thread_handle = nullptr;
    sam_main.thread_id = NULL;
    sam_main.is_thread_active = false;

    print_log("Sam process handle: 0x%p\n", reinterpret_cast<PVOID>(sam_main.handle));
    print_log("Sam process id: %u\n", sam_main.id);
    print_log("Sam window handle: 0x%p\n", reinterpret_cast<PVOID>(sam_main.window_handle));

    init_modules();
    init_funcs();

    sam_imgui.thread_handle = CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(sam_gui_init),
        nullptr, NULL, &sam_imgui.thread_id);
    if (!sam_imgui.thread_handle || !sam_imgui.thread_id) {
        print_log_error("Get imgui thread failed\n");
        return;
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch( fdwReason ) { 
        case DLL_PROCESS_ATTACH:
            log_file = fopen(log_file_path.c_str(), "w");
            fprintf(log_file, "DLL_PROCESS_ATTACH\n");
            fclose(log_file);
            CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(attach),
                nullptr, NULL, nullptr);
            DisableThreadLibraryCalls(hinstDLL);
            break;

        case DLL_THREAD_ATTACH:
            //print_log("DLL_THREAD_ATTACH\n");
            break;

        case DLL_THREAD_DETACH:
            //print_log("DLL_THREAD_DETACH\n");
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