#include <amp.h>
#include <amp_graphics.h>

#include <d3d11.h>
#include <atlcomcli.h>

#pragma comment(lib, "d3d11")

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	WNDCLASSEX wcex = { sizeof(wcex) };
	wcex.lpfnWndProc = DefWindowProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = _T(__FUNCTION__);

	HWND hWnd = CreateWindow(
		MAKEINTATOM(RegisterClassEx(&wcex)), wcex.lpszClassName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd) return 1;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	CComPtr<ID3D11Device> dev;
	CComPtr<ID3D11DeviceContext> ctx;
	CComPtr<IDXGISwapChain> swapchain;

	DXGI_SWAP_CHAIN_DESC desc = {};
	desc.BufferCount = 1;
	desc.SampleDesc.Count = 1;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.OutputWindow = hWnd;
	desc.Windowed = TRUE;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;// | DXGI_USAGE_SHADER_INPUT;

	D3D11CreateDeviceAndSwapChain(
		0, D3D_DRIVER_TYPE_HARDWARE, 
		0, 0&D3D11_CREATE_DEVICE_DEBUG, 
		0, 0, D3D11_SDK_VERSION, 
		&desc, &swapchain, &dev, 0, &ctx);

	if (!swapchain) return 1;

	CComPtr<ID3D11Texture2D> backbuffer;
	swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer));

	using namespace concurrency::direct3d;
	using namespace concurrency::graphics;
	using namespace concurrency::graphics::direct3d;

	auto tex = make_texture< unorm4, 2>(create_accelerator_view(dev), backbuffer);
	texture_view<unorm4, 2> tv(tex);
	const auto ext = tv.extent;

	MSG msg;
	while (::IsWindow(hWnd))
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) DispatchMessage(&msg);
		else
		{
			const float t = (msg.time%1000)*0.001f;
			concurrency::parallel_for_each(
				tv.accelerator_view, ext, 
				[=](concurrency::index<2> idx) restrict(amp)
			{
				unorm4 val(1.f*idx[1]/ext[1], 1.f*idx[0]/ext[0], t, 1);
				tv.set(idx, val);
			});

			swapchain->Present(1, 0);
		}
	return 0;
}
