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

struct sam_process_t;
extern sam_process_t sam_process;

// Separate variable due to assemble call
extern PVOID get_group_mover_orig_func;

struct s_func_t;
extern shared_ptr<s_func_t> receive_health, receive_armor, hv_handle_to_pointer, get_group_mover;

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

struct sam_process_t {
	HANDLE handle;
	DWORD id;
	HWND window_handle;
	HANDLE imgui_thread_handle;
	DWORD imgui_thread_id;
};

struct s_module_t {
	HANDLE process;
	HMODULE module;
	LPCTSTR name;
	MODULEINFO module_info;
	s_module_t() = delete;
	explicit s_module_t(HANDLE process, LPCTSTR name);
};

struct s_func_t {
	shared_ptr<s_module_t> module;
	PVOID orig_func;
	PVOID detour_func;
	string pattern;
	string mask;
	s_func_t() = delete;
	explicit s_func_t(LPCTSTR func_name, shared_ptr<s_module_t> module, string pattern, string mask,
		PVOID detour_func, bool hook, bool hex_string);
};
