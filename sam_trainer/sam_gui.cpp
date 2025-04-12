#include "sam_gui.hpp"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

IDirect3D9 *d3d9;
IDirect3DDevice9 *d3d9_device;

bool menu_opened;

static bool is_imgui_window_focus = false;
static bool is_button_pressed = false;

LRESULT WINAPI imgui_window_procedure(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param)
{
	if (ImGui_ImplWin32_WndProcHandler(window_handle, message, w_param, l_param)) {
		return true;
	}

	switch (message) {
	case WM_KEYDOWN:
		if (w_param == VK_HOME) {
			is_button_pressed = true;
		}
		break;
	case WM_KEYUP:
		if (w_param == VK_HOME && is_button_pressed) {
			is_imgui_window_focus = !is_imgui_window_focus;
			if (is_imgui_window_focus) {
				ShowWindow(sam_imgui.window_handle, SW_SHOW);
				SetActiveWindow(sam_imgui.window_handle);
			}
			else {
				ShowWindow(sam_imgui.window_handle, SW_HIDE);
				SetActiveWindow(sam_main.window_handle);
			}
			is_button_pressed = false;
		}
		break;
	}

	if (message == WM_DESTROY) {
		PostQuitMessage(0);
		return true;
	}

	return CallWindowProc(sam_main.window_procedure, window_handle, message, w_param, l_param);
}

bool create_device_d3d(HWND window_handle) {
	if ((d3d9 = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr) {
		print_log("Creating direct3d failed\n");
		return false;
	}

	D3DPRESENT_PARAMETERS d3d_params = { };
	d3d_params.Windowed = FALSE;
	d3d_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3d_params.hDeviceWindow = window_handle;
	d3d_params.BackBufferFormat = D3DFMT_UNKNOWN;
	d3d_params.EnableAutoDepthStencil = TRUE;
	d3d_params.AutoDepthStencilFormat = D3DFMT_D16;
	d3d_params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	if (d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3d_params.hDeviceWindow,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3d_params, &d3d9_device) < 0) {
		d3d_params.Windowed = TRUE;
		print_log("Creating fullscreen device failed. Creating windowed..\n");
		if (d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3d_params.hDeviceWindow,
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3d_params, &d3d9_device) < 0) {
			print_log("Creating windowed device failed\n");
			return false;
		}
	}
	return true;
}

void sam_gui_deinit() {
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (d3d9_device) {
		d3d9_device->Release();
		d3d9_device = nullptr;
	}
	if (d3d9) {
		d3d9->Release();
		d3d9 = nullptr;
	}

	if (sam_imgui.window_handle) {
		DestroyWindow(sam_imgui.window_handle);
	}
	if (sam_imgui.window_class.hInstance) {
		UnregisterClassW(sam_imgui.window_class.lpszClassName, sam_imgui.window_class.hInstance);
	}
}

bool create_imgui_window() {
	sam_imgui.handle = nullptr;
	sam_imgui.id = NULL;
	sam_imgui.window_procedure = imgui_window_procedure;

	sam_imgui.window_class.cbSize = sizeof(sam_imgui.window_class);
	sam_imgui.window_class.hInstance = GetModuleHandle(nullptr);
	sam_imgui.window_class.lpszClassName = _T("sam_gui_window");
	sam_imgui.window_class.lpfnWndProc = sam_imgui.window_procedure;
	sam_imgui.window_class.style = CS_CLASSDC;
	sam_imgui.window_class.cbClsExtra = NULL;
	sam_imgui.window_class.cbWndExtra = NULL;
	sam_imgui.window_class.lpszMenuName = _T("sam_gui");

	if (!RegisterClassEx(&sam_imgui.window_class)) {
		print_log_error("Registering class for imgui window failed\n");
		return false;
	}

	sam_main.window_procedure = reinterpret_cast<WNDPROC>(SetWindowLongPtr(sam_main.window_handle,
		GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(imgui_window_procedure)));

	RECT sam_window_area;
	GetWindowRect(sam_main.window_handle, &sam_window_area);

	sam_imgui.window_handle = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
		sam_imgui.window_class.lpszClassName, _T("Dear ImGui DirectX9 Example"),
		WS_POPUP, sam_window_area.left, sam_window_area.top, sam_window_area.right - sam_window_area.left, 
		sam_window_area.bottom - sam_window_area.top, sam_main.window_handle,
		nullptr, sam_imgui.window_class.hInstance, nullptr);
	SetLayeredWindowAttributes(sam_imgui.window_handle, RGB(0, 0, 0), 0, ULW_COLORKEY);

	if (!sam_imgui.window_handle) {
		print_log_error("Creating imgui window failed\n");
		return false;
	}
	if (!sam_imgui.window_procedure) {
		print_log_error("Setting window process failed\n");
		return false;
	}

	print_log("Sam window procedure: 0x%p\n", reinterpret_cast<PVOID>(sam_main.window_procedure));
	print_log("ImGui window handle: 0x%p\n", reinterpret_cast<PVOID>(sam_imgui.window_handle));
	print_log("ImGui window procedure: 0x%p\n", reinterpret_cast<PVOID>(sam_imgui.window_procedure));
	print_log("ImGui window class: 0x%p, name: %ls\n", sam_imgui.window_class.hInstance,
		sam_imgui.window_class.lpszClassName);
	print_log("ImGui thread handle: 0x%p\n", reinterpret_cast<PVOID>(sam_imgui.thread_handle));
	print_log("ImGui thread id: %u\n", sam_imgui.thread_id);

	return true;
}

void sam_gui_init() {
	IMGUI_CHECKVERSION();
	sam_imgui.is_thread_active = true;

	if (!create_imgui_window()) {
		sam_gui_deinit();
		return;
	}

	if (!create_device_d3d(sam_imgui.window_handle)) {
		sam_gui_deinit();
		return;
	}

	d3d9_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	d3d9_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	d3d9_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	ImGui::CreateContext();
	static ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	ImGui_ImplWin32_Init(sam_imgui.window_handle);
	ImGui_ImplDX9_Init(d3d9_device);

	print_log("sam_gui_initted\n");
	sam_gui_run();
}

void gui_draw() {
	ImGui::Begin("My Super menu", &menu_opened);
	{
		ImGui::Checkbox("Invincibility (hp)", &disable_receive_health);
		ImGui::Checkbox("Invincibility (armor)", &disable_receive_armor);
		ImGui::SetNextItemWidth(200);
		ImGui::SetCursorPos(ImVec2(200, 20));
		if (ImGui::SliderInt("hp", &hp, 10, 1000)) {
			set_entity_property_by_offset(sam_offsets::hp, &hp);
		}
		ImGui::SetNextItemWidth(200);
		ImGui::SetCursorPos(ImVec2(200, 40));
		if (ImGui::SliderInt("armor", &armor, 10, 1000)) {
			set_entity_property_by_offset(sam_offsets::armor, &armor);
		}
		ImGui::SetNextItemWidth(200);
		ImGui::SetCursorPos(ImVec2(200, 60));
		if (ImGui::SliderInt("max_hp", &max_hp, 10, 1000)) {
			set_entity_property_by_offset(sam_offsets::max_hp, &max_hp);
		}
		ImGui::SetNextItemWidth(200);
		ImGui::SetCursorPos(ImVec2(200, 80));
		if (ImGui::SliderInt("max_armor", &max_armor, 10, 1000)) {
			set_entity_property_by_offset(sam_offsets::max_armor, &max_armor);
		}
	}
	ImGui::End();
}

void sam_gui_run() {
	while (sam_imgui.is_thread_active) {
		MSG msg;
		while (PeekMessage(&msg, nullptr, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		if (GetActiveWindow() == sam_imgui.window_handle) {
			gui_draw();
		}
		else {
			is_imgui_window_focus = false;
		}
		ImGui::EndFrame();
		d3d9_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);
		if (d3d9_device->BeginScene() >= 0) {
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			d3d9_device->EndScene();
		}

		HRESULT result = d3d9_device->Present(nullptr, nullptr, nullptr, nullptr);
		if (result == D3DERR_DEVICELOST) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
	sam_imgui.thread_handle = nullptr;
	sam_imgui.thread_id = 0;
	sam_gui_deinit();
	print_log("ImGui drawing ended\n");
}