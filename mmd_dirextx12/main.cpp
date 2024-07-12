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

    ID3D12CommandAllocator* _cmdAllocator = nullptr;
    if ( _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator)) != S_OK ) {
        MessageBox(NULL, _T("Faild to init _cmdAllocator"), szWindowClass, NULL);
        return -1;
    }

    ID3D12GraphicsCommandList* _cmdList = nullptr;
    if (_dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAllocator, nullptr, IID_PPV_ARGS(&_cmdList)) != S_OK) {
        MessageBox(NULL, _T("Faild to init _cmdList"), szWindowClass, NULL);
        return -1;
    }

    ID3D12CommandQueue* _cmdQueue = nullptr;
    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // タイムアウトなし
    cmdQueueDesc.NodeMask = 0;
    cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // コマンドリストと揃える
    if (_dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue)) != S_OK) {
        MessageBox(NULL, _T("Faild to init _cmdQueue"), szWindowClass, NULL);
        return -1;
    }

    IDXGISwapChain4* _swapchain = nullptr;
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.Width = WINDOW_WIDTH;
    swapchainDesc.Height = WINDOW_HEIGHT;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.Stereo = false;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapchainDesc.BufferCount = 2;
    swapchainDesc.Scaling = DXGI_SCALING_STRETCH; // バックバッファーは伸び縮み可能
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // フリップ後破棄
    swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    if ( _dxgiFactory->CreateSwapChainForHwnd(_cmdQueue, hWnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)&_swapchain) != S_OK) {
        MessageBox(NULL, _T("Faild to init _swapchain"), szWindowClass, NULL);
        return -1;
    }

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NodeMask = 0;
    heapDesc.NumDescriptors = 2;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ID3D12DescriptorHeap* rtvHeaps = nullptr;
    if ( _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps)) != S_OK ) {
		MessageBox(NULL, _T("Faild to init rtvHeaps"), szWindowClass, NULL);
		return -1;
	}

    // 一応上にswapchainDescがあるがリファクタあとのことなどを考えて再取得しておく
    DXGI_SWAP_CHAIN_DESC swcDesc = {};
    if ( _swapchain->GetDesc(&swcDesc) != S_OK) {
		MessageBox(NULL, _T("Faild to get swcDesc"), szWindowClass, NULL);
		return -1;
	}
    std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
    for (size_t idx = 0; idx < swcDesc.BufferCount; ++idx) {
        if (_swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx])) != S_OK) {
            MessageBox(NULL, _T("Faild to get backBuffers"), szWindowClass, NULL);
            return -1;
        }

        _dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
        handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }
    ID3D12Fence* _fence = nullptr;
    UINT64 _fenceVal = 0;
    if ( _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)) != S_OK ) {
		MessageBox(NULL, _T("Faild to init _fence"), szWindowClass, NULL);
		return -1;
	}

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT) {
            break;
        }

        auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

        D3D12_RESOURCE_BARRIER BarrierDesc = {};
        BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
        BarrierDesc.Transition.Subresource = 0;
        BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        _cmdList->ResourceBarrier(1, &BarrierDesc);

        auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
        rtvH.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) * bbIdx;
        _cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

        float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        _cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

        BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        _cmdList->ResourceBarrier(1, &BarrierDesc);

        _cmdList->Close();
        ID3D12CommandList* cmdLists[] = { _cmdList };
        _cmdQueue->ExecuteCommandLists(1, cmdLists);
        _cmdQueue->Signal(_fence, ++_fenceVal);

        if (_fence->GetCompletedValue() != _fenceVal) {
            auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
        }

        _cmdAllocator->Reset(); // キューをクリア
        _cmdList->Reset(_cmdAllocator, nullptr); // 次の準備

        _swapchain->Present(1, 0);
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
