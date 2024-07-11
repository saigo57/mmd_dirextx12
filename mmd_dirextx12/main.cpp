#include <windows.h>
#include <tchar.h>

#include <string>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600
HINSTANCE hInst;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	static TCHAR szWindowClass[] = _T("mmd_directx12");
    static TCHAR szTitle[] = _T("MMD with DirectX12");

    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, _T("RegisterClassExの処理に失敗しました"), szWindowClass, NULL);

        return -1;
    }

    hInst = hInstance;

    RECT wrc = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

    HWND hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wrc.right - wrc.left,
        wrc.bottom - wrc.top,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        MessageBox(NULL, _T("ウィンドウ生成に失敗しました!"), szWindowClass, NULL);
        return -1;
    }

    ID3D12Device* _dev = nullptr;
    IDXGIFactory6* _dxgiFactory = nullptr;
    // IDXGISwapchain4* _swapchain = nullptr;

    // アダプタを探す
    auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
    if (!_dxgiFactory) {
		MessageBox(NULL, _T("CreateDXGIFactory1の生成に失敗しました"), szWindowClass, NULL);
		return -1;
	}

    std::vector<IDXGIAdapter*> adaptors;
    IDXGIAdapter* adaptor = nullptr;
    for (int i = 0; _dxgiFactory->EnumAdapters(i, &adaptor) != DXGI_ERROR_NOT_FOUND; i++) {
        adaptors.push_back(adaptor);
    }

    for (auto ad : adaptors) {
        DXGI_ADAPTER_DESC desc = {};
		ad->GetDesc(&desc);
		
        std::wstring strDesc = desc.Description;
        if (strDesc.find(L"NVIDIA") != std::string::npos) {
            adaptor = ad;
            break;
        }
	}

    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
    };
    D3D_FEATURE_LEVEL featureLevel;
    for (auto lv : levels) {
        if (D3D12CreateDevice(adaptor, lv, IID_PPV_ARGS(&_dev)) == S_OK) {
            featureLevel = lv;
			break;
        }
    }

    if (!_dev) {
        MessageBox(NULL, _T("Direct3D12デバイスの生成に失敗しました"), szWindowClass, NULL);
		return -1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}
