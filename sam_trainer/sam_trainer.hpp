#pragma comment(lib, "detours.lib")

#include "memtool.hpp"
#include <psapi.h>
#include <detours.h>
#include <fstream>
#include <mutex>

#define STR_LEN 256

using namespace mem_tool;
using namespace std;

extern FILE* log_file;
extern string log_file_path;

struct sam_process_t;
extern sam_process_t sam_process;

extern bool gui_thread_active;

// Separate variable due to assemble call
extern PVOID get_group_mover_orig_func;

struct s_func_t;
extern shared_ptr<s_func_t> receive_health, receive_armor, hv_handle_to_pointer, get_group_mover;

// gui
extern void sam_gui_init();

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

template<typename... Args>
void print_log_error(const char* format, Args ...args) {
	char* new_format = new char[STR_LEN + 1];
	strncpy(new_format, format, STR_LEN);
	strncat(new_format, "Error: %s\n", STR_LEN);
	DWORD error_message_id = GetLastError();
	LPSTR error_buffer;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, 
		error_message_id, NULL, (LPSTR)&error_buffer, NULL, nullptr);
	print_log(new_format, args ..., error_buffer);
	delete new_format;
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
