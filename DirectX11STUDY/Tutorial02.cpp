#include <windows.h>
#include <d3d11_1.h>
#include <directxcolors.h>
#include <directxmath.h>
#include <d3dcompiler.h>

#include"resource.h"

using namespace DirectX;

//�\����
struct SimpleVertex
{
	XMFLOAT3 Pos;
};


//�O���|�o���錾
HINSTANCE		g_hInst = nullptr;
HWND			g_hWnd = nullptr;
D3D_DRIVER_TYPE	g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11Device1* g_pd3dDevice1 = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr;
ID3D11DeviceContext1* g_pImmediateContext1 = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
IDXGISwapChain1* g_pSwapChain1 = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
///////////////////////////////////////////////////////
ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixcelShader = nullptr;
ID3D11InputLayout* g_pVertexLayout = nullptr;
ID3D11Buffer* g_pVertexBuffer = nullptr;
//////////////////////////////////////////////////////

//�O���錾
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
void Render();

//�v���O�����ւ̃G���g���|�C���g�B ���ׂĂ����������A���b�Z�[�W�����ɓ���܂�
//���[�v�B �A�C�h�����Ԃ́A�V�[���̃����_�����O�Ɏg�p����܂��B

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

	//���b�Z�[�W���[�v
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}
	CleanupDevice();

	return(int)msg.wParam;
}
//���W�X�^�[�N���X�ƃN���G�C�g�E�B���h�E
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	//���W�X�^�[�N���X
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;//���܂�
	wcex.cbWndExtra = 0;//���܂�
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(nullptr, IDC_ARROW);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"TutorialWindClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	//�E�B���h�E�쐬
	g_hInst = hInstance;
	RECT rc = { 0,0,800,600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"TutorialWindowClass", L"Direct3D 11 Tutorial 1:Direct3D 11 Basics",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);
	return S_OK;
}

// ------------------------------------------------ --------------------------------------
// D3DCompile�ŃV�F�[�_�[���R���p�C�����邽�߂̃w���p�[
//
// VS 11�ł́A����Ƀr���h�ς݂�.cso�t�@�C�������[�h�ł��܂�...
// ------------------------------------------------ ----------------
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef DEBUG
	// D3DCOMPILE_DEBUG�t���O��ݒ肵�āA�f�o�b�O�����V�F�[�_�[�ɖ��ߍ��ށB
	 //���̃t���O��ݒ肷��ƁA�V�F�[�_�[�̃f�o�b�O�G�N�X�y���G���X�����サ�܂����A
	 //�œK������A���m�Ɏ��s�������@�Ŏ��s�����V�F�[�_�[
	 //���̃v���O�����̃����[�X�\���B

	dwShaderFlsgs |= D3DCOMPILE_DEBUG;

	//�œK���𖳌��ɂ��āA�V�F�[�_�[�̃f�o�b�O������ɉ��P���܂�
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // DEBUG
	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return  hr;
	}
	if (pErrorBlob)pErrorBlob->Release();
	return S_OK;

}


//�N���G�C�g�@3D�f�o�C�X�ƃX���b�v�`�F�[��
HRESULT InitDevice()
{
	HRESULT hr = S_OK;
	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0�v���b�g�t�H�[����D3D_FEATURE_LEVEL_11_1��F�����Ȃ����߁A����Ȃ��ōĎ��s����K�v������܂�
			hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		}
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;


	//�f�o�C�X����DXGI�t�@�N�g�����擾���܂��i��L��pAdapter��nullptr���g�p�������߁j

	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;


	//�N���G�C�g�X���b�v�`�F�[��
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		//�_�C���N�g�P�P�D�Pore�ŐV
		hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pd3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(g_pImmediateContext1));
		}


		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(g_pSwapChain));

		}
		dxgiFactory2->Release();
	}
	else
	{
		//DirectX11�V�X�e��
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = g_hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);

	}

	//���̃`���[�g���A���͑S��ʂ̃X���b�v�`�F�[�����������Ȃ����߁AALT + ENTER�V���[�g�J�b�g���u���b�N���Ă��邱�Ƃɒ��ӂ��Ă�������
	dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);
	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	//�N���G�C�g�@�����_�[�^�[�Q�b�g
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

	//�Z�b�g�A�b�v�@�r���[�|�[�g
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0;
	vp.MaxDepth = 1.0;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);


	/////////////////////////////////////////////////////////
	//���_�V�F�[�_�[�̃R���p�C��

	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile(L"Tutorial20.fx", "VS", "vs_4.0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot br compiled.Please run this executable from the directory that contains the FX file", L"Error", MB_OK);
		return hr;
	}

	//Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr,&g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}
	//Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
	};
	UINT numElements = ARRAYSIZE(layout);

	//�C���v�b�g���C�A�E�g�����
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	//�Z�b�g�C���v�b�g���C�A�E�g
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

	//�s�N�Z���V�F�[�_�[���R���p�C������
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Tutorial20.fx", "VS", "vs_4.0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot br compiled.Please run this executable from the directory that contains the FX file", L"Error", MB_OK);
		return hr;
	}
	//�s�N�Z���V�F�[�_�[�����
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize, nullptr, &g_pPixcelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return;

	//�N���G�C�g���_�o�b�t�@�[
	SimpleVertex vertices[] =
	{
		XMFLOAT3(0.0f,0.5f,0.5f),
		XMFLOAT3(0.5f,-0.5f,0.5f),
		XMFLOAT3(-0.5f,-0.5f,0.5f),
	};
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
		return hr;

	//���_�o�b�t�@�[���Z�b�g
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	//�Z�b�g�􉽊w
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	return S_OK;
}
//�I�u�W�F�N�g�������[�X
void CleanupDevice()
{
	if (g_pImmediateContext)g_pImmediateContext->ClearState();

	if (g_pVertexBuffer)g_pVertexBuffer->Release();
	if (g_pVertexShader)g_pVertexShader->Release();
	if (g_pVertexLayout)g_pVertexLayout->Release();
	if (g_pPixcelShader)g_pPixcelShader ->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain1) g_pSwapChain1->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext1) g_pImmediateContext1->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice1) g_pd3dDevice1->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}



//�}�C�t���[���Ă΂�郁�b�Z�[�W�L���[

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		//���E���̂Ȃ��E�B���h�E���쐬����
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//�����_�����O�t���[��

void Render()
{
	//�o�b�t�@���N���A
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);
	g_pSwapChain->Present(0, 0);

	//�����_�[�g���C�A���O��
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPixcelShader, nullptr, 0);
	g_pImmediateContext->Draw(3, 0);

		//�o�b�N�o�b�t�@�[�Ƀ����_�����O���ꂽ�����t�����g�o�b�t�@�[�i��ʁj�ɒ񎦂��܂�
	g_pSwapChain->Present(0, 0);
}
