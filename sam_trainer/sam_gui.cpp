#include "sam_gui.hpp"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

IDirect3D9 *d3d9;
IDirect3DDevice9 *d3d9_device;

HWND window_overlay_handle = nullptr;
WNDPROC window_process_orig = nullptr;
HMENU window_process_id = reinterpret_cast<HMENU>(0xa1b5c7d3);
WNDCLASSEXW window_overlay_class;

bool menu_opened;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

bool create_device_d3d(HWND window_handle) {
	if ((d3d9 = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr) {
		return false;
	}

	D3DPRESENT_PARAMETERS d3d_params = { };
	d3d_params.Windowed = TRUE;
	d3d_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3d_params.hDeviceWindow = window_handle;
	d3d_params.BackBufferFormat = D3DFMT_UNKNOWN;
	d3d_params.EnableAutoDepthStencil = TRUE;
	d3d_params.AutoDepthStencilFormat = D3DFMT_D16;
	d3d_params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	if (d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3d_params.hDeviceWindow,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3d_params, &d3d9_device) < 0) {
		return false;
	}
	return true;
}

LRESULT WINAPI window_overlay_process(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
		return true;
	}

	if (uMsg == WM_DESTROY) {
		PostQuitMessage(0);
		return true;
	}

	return CallWindowProc(window_process_orig, hWnd, uMsg, wParam, lParam);
}

bool create_overlay() {
	window_overlay_class.cbSize = sizeof(window_overlay_class);
	window_overlay_class.hInstance = GetModuleHandle(nullptr);
	window_overlay_class.lpszClassName = _T("sam_gui_overlay");
	window_overlay_class.lpfnWndProc = window_overlay_process;
	window_overlay_class.style = CS_CLASSDC;
	window_overlay_class.cbClsExtra = NULL;
	window_overlay_class.cbWndExtra = NULL;
	window_overlay_class.lpszMenuName = _T("sam_gui");

	if (!RegisterClassEx(&window_overlay_class)) {
		print_log_error("Registering class for window overlay failed\n");
		return false;
	}
	print_log("Registered class instance: 0x%p, name: %ls\n", window_overlay_class.hInstance, 
		window_overlay_class.lpszClassName);

	window_process_orig = reinterpret_cast<WNDPROC>(SetWindowLongPtr(sam_process.window_handle,
		GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(window_overlay_process)));

	window_overlay_handle = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
		window_overlay_class.lpszClassName, _T("Dear ImGui DirectX9 Example"),
		WS_POPUP | WS_VISIBLE, 200, 100, 300, 300, NULL,
		nullptr, window_overlay_class.hInstance, nullptr);
	SetLayeredWindowAttributes(window_overlay_handle, RGB(0, 0, 0), 0, ULW_COLORKEY);

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

	ShowWindow(window_overlay_handle, SW_SHOWDEFAULT);
	UpdateWindow(window_overlay_handle);

	ImGui::CreateContext();
	static ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
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

	DestroyWindow(window_overlay_handle);
	UnregisterClassW(window_overlay_class.lpszClassName, window_overlay_class.hInstance);
}

void sam_gui_run() {
	while (gui_thread_active) {
		MSG msg;
		while (PeekMessage(&msg, nullptr, NULL, NULL, PM_REMOVE)) {
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
		d3d9_device->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3d9_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		d3d9_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(0, 0, 0, 0);
		d3d9_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
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