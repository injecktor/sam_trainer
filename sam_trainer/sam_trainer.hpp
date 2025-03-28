#pragma comment(lib, "detours.lib")

#include "memtool.hpp"
#include <psapi.h>
#include <detours.h>
#include <fstream>
#include <mutex>

using namespace mem_tool;
using namespace std;

FILE* log_file;
string log_file_path = "C:\\Users\\p2282\\OneDrive\\Documents\\Visual Studio 2022\\projects\\sam_trainer\\sam_trainer_log.txt";
HANDLE sam_process;

// Separate variable due to assemble call
extern PVOID get_group_mover_orig_func;

extern class Entity;

__declspec(naked) void* __cdecl method_to_ptr(...) {
	__asm {
		mov eax, [esp + 4]
		retn
	}
}

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
	s_module* module;
	PVOID orig_func;
	PVOID detour_func;
	string pattern;
	string mask;
	s_func() = delete;
	explicit s_func(LPCTSTR func_name, s_module* module, string pattern, string mask, PVOID detour_func, bool hook, bool hex_string) {
		module = module;
		detour_func = detour_func;
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

extern s_func *receive_health, *receive_armor, *hv_handle_to_pointer, *get_player, *get_group_mover;
