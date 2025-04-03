#include "sam_gui.hpp"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

IDirect3D9 *d3d9;
IDirect3DDevice9 *d3d9_device;

HWND window_overlay_handle = nullptr;
WNDPROC window_process_orig = nullptr;

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

	return CallWindowProc(window_process_orig, hWnd, uMsg, wParam, lParam);
}

bool create_overlay() {
	WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, window_overlay_process, 0L, 0L, GetModuleHandle(nullptr),
		nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
	RegisterClassEx(&wc);

	window_overlay_handle = CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX9 Example",
		WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

	window_process_orig = reinterpret_cast<WNDPROC>(SetWindowLongPtr(sam_process.window_handle,
		GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(window_overlay_process)));

	return window_overlay_handle && window_process_orig;
}

void sam_gui_init() {
	IMGUI_CHECKVERSION();

	if (!create_overlay()) {
		print_log_error("Creating overlay failed\n");
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
	while (true) {
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("My Super menu", &menu_opened);
		{
			ImGui::Text("Hello World!");
		}
		ImGui::End();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		HRESULT result = d3d9_device->Present(nullptr, nullptr, nullptr, nullptr);
		if (result == D3DERR_DEVICELOST) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
	print_log("ImGui drawing ended\n");
}