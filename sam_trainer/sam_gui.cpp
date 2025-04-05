#include "sam_gui.hpp"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

IDirect3D9 *d3d9;
IDirect3DDevice9 *d3d9_device;

HWND window_overlay_handle = nullptr;
WNDPROC window_process_orig = nullptr;
HMENU window_process_id = reinterpret_cast<HMENU>(0xa1b5c7d3);

bool menu_opened;

bool create_device_d3d(HWND window_handle) {
	if ((d3d9 = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr) {
		return false;
	}

	D3DPRESENT_PARAMETERS d3d_params = { };
	d3d_params.Windowed = TRUE;
	d3d_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3d_params.hDeviceWindow = window_handle;
	if (d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3d_params.hDeviceWindow,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3d_params, &d3d9_device) < 0) {
		return false;
	}
	return true;
}

LRESULT WINAPI window_overlay_process(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
		return true;
	}

	print_log("Message: %u\n", uMsg);

	return CallWindowProc(window_process_orig, hWnd, uMsg, wParam, lParam);
}

bool create_overlay() {
	WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, window_overlay_process, 0L, 0L, GetModuleHandle(nullptr),
		nullptr, nullptr, nullptr, _T("ImGui Example"), _T("sam_gui_overlay"), nullptr };
	if (!RegisterClassEx(&wc)) {
		print_log_error("Registering class for window overlay failed\n");
		return false;
	}
	print_log("Registered class instance: 0x%p, name: %ls\n", wc.hInstance, wc.lpszClassName);

	window_process_orig = reinterpret_cast<WNDPROC>(SetWindowLongPtr(sam_process.window_handle,
		GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(window_overlay_process)));

	window_overlay_handle = CreateWindowExW(0,
		wc.lpszClassName, _T("Dear ImGui DirectX9 Example"),
		WS_CHILD | WS_VISIBLE, 100, 100, 300, 300, sam_process.window_handle,
		window_process_id, wc.hInstance, nullptr);

	if (!window_overlay_handle) {
		print_log_error("Creating window overlay failed\n");
		return false;
	}
	if (!window_process_orig) {
		print_log_error("Setting window process failed\n");
		return false;
	}

	return true;
}

void sam_gui_init() {
	IMGUI_CHECKVERSION();

	if (!create_overlay()) {
		print_log("Creating overlay failed\n");
		return;
	}

	if (!create_device_d3d(window_overlay_handle)) {
		print_log("Creating device failed\n");
		return;
	}

	ImGui::CreateContext();
	ImGui_ImplWin32_Init(window_overlay_handle);
	ImGui_ImplDX9_Init(d3d9_device);

	print_log("sam_gui_initted\n");
	sam_gui_run();
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
}

void sam_gui_run() {
	while (gui_thread_active) {
		MSG msg;
		while (GetMessage(&msg, window_overlay_handle, NULL, NULL)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("My Super menu", &menu_opened);
		{
			ImGui::Text("Hello World!");
		}
		ImGui::End();

		ImGui::EndFrame();
		d3d9_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(1, 255, 255, 255), 1.0f, 0);
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
	sam_process.imgui_thread_handle = nullptr;
	sam_process.imgui_thread_id = 0;
	sam_gui_deinit();
	print_log("ImGui drawing ended\n");
}