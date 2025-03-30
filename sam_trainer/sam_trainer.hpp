#pragma comment(lib, "detours.lib")

#include "memtool.hpp"
#include <psapi.h>
#include <detours.h>
#include <fstream>
#include <mutex>

using namespace mem_tool;
using namespace std;

extern FILE* log_file;
extern string log_file_path;
extern HANDLE sam_process;
extern HWND sam_window;

// Separate variable due to assemble call
extern PVOID get_group_mover_orig_func;

struct s_func;
extern shared_ptr<s_func> receive_health, receive_armor, hv_handle_to_pointer, get_group_mover;

// gui
extern void WINAPI sam_gui_main();

// game functions
class game_functions {
public:
	void __thiscall ReceiveHealth(long health_change, long arg1);
	void __thiscall ReceiveArmor(long armor_change, long arg1);
};
extern void GetGroupMover();

// local functions
extern void* __cdecl method_to_ptr(...);

template<typename... Args>
void print_log(const char* format, Args ...args) {
	log_file = fopen(log_file_path.c_str(), "a");
	fprintf(log_file, format, args ...);
	fclose(log_file);
}

struct s_module {
	HANDLE process;
	HMODULE module;
	LPCTSTR name;
	MODULEINFO module_info;
	s_module() = delete;
	explicit s_module(HANDLE process, LPCTSTR name) {
		if (!process) {
			print_log("Invalid process\n");
			return;
		}
		process = process;
		module = GetModuleHandle(name);
		if (!module) {
			print_log("Couldn't find module %ls handle\n", name);
			return;
		}
		name = name;
		BOOL res = GetModuleInformation(process, module, &module_info, sizeof(MODULEINFO));
		if (!res) {
			print_log("Couldn't find module %ls info\n", name);
			return;
		}
		print_log("Module name: %ls, address: 0x%p, size: 0x%x\n", name, module_info.lpBaseOfDll, module_info.SizeOfImage);
	}
};

struct s_func {
	shared_ptr<s_module> module;
	PVOID orig_func;
	PVOID detour_func;
	string pattern;
	string mask;
	s_func() = delete;
	explicit s_func(LPCTSTR func_name, shared_ptr<s_module> module, string pattern, string mask, PVOID detour_func, bool hook, bool hex_string) {
		this->module = module;
		this->detour_func = detour_func;
		if (!hex_string) {
			pattern = trim(pattern);
			pattern = str_to_hex_str(pattern);
		}
		mask = trim(mask);
		orig_func = sig_scan(sam_process, (BYTE*)module->module_info.lpBaseOfDll, module->module_info.SizeOfImage, pattern, mask);
		print_log("Function: %ls\n", func_name);
		print_log("Original function address: 0x%p\n", orig_func);

		if (hook) {
			print_log("Detour function address: 0x%p\n", detour_func);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach((PVOID*)&orig_func, detour_func);
			DetourTransactionCommit();
		}
	}
};
