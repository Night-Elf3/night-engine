#pragma once

#include "types.hpp"
#include "window.hpp"

class Render {
	D3D_DRIVER_TYPE g_driverType;
	D3D_FEATURE_LEVEL g_featureLevel;

	ID3D11Device *g_pd3dDevice;						// ��������� ��������� ����� ����������
	ID3D11DeviceContext *g_pImmediateContext;		// �������� ��� ���������
	IDXGISwapChain *g_pSwapChain;					// ������ ��'������� � �������
	ID3D11RenderTargetView *g_pRenderTargetView;	// ��'��� �������� ������

	ID3D11SamplerState *g_pSamplerLinear = NULL;    // ��������� ��������
	ID3D11Texture2D *m_pDepthStencil;
	ID3D11DepthStencilView *m_pDepthStencilView;

	// ��������� ������� � �����
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut) {
		HRESULT hr = S_OK;
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
		ID3DBlob* pErrorBlob;
		hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
			dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
		if (FAILED(hr))
		{
			if (pErrorBlob != NULL)
				OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			if (pErrorBlob)
				pErrorBlob->Release();

			return hr;
		}
		if (pErrorBlob)
			pErrorBlob->Release();

		return S_OK;
	}

public:
	Render() {
		g_driverType = D3D_DRIVER_TYPE_NULL;
		g_featureLevel = D3D_FEATURE_LEVEL_11_0;

		g_pd3dDevice = NULL;
		g_pImmediateContext = NULL;
		g_pSwapChain = NULL;
		g_pRenderTargetView = NULL;

		m_pDepthStencil = nullptr;
		m_pDepthStencilView = nullptr;
	}

	// ����������� �������� ��� ����������
	HRESULT InitDevice(Window *Wind) { // �����������
		HRESULT hr = S_OK;

		UINT createDeviceFlags = 0;
		#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif

		// ���������� ������ ������������ ����� ���������
		D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};

		// ��������� ��������� ����� �������� ����������� + ����'���� ���� �� ������ ����
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;										// � ��� ���� ����� �����
		sd.BufferDesc.Width = Wind->GetWindowSize().width;		// ������
		sd.BufferDesc.Height = Wind->GetWindowSize().height;	// ������
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;		// ������ ������ � �����
		sd.BufferDesc.RefreshRate.Numerator = 100;				// ������� ���������� ������
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		// ����������� ������ - ����� �����
		sd.OutputWindow = Wind->GetHWnd();						// ��'����� � ����� �����
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = true;										// ��������������� �����

		// ��������� �������
		hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevels, 3, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		if (FAILED(hr))
			return hr;

		// ����� �������� ����� �����
		// RenderTargetOutput - ��� ������� �����
		// RenderTargetView - �����.
		ID3D11Texture2D* pBackBuffer = NULL;
		hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		if (FAILED(hr))
			return hr;
		hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
		pBackBuffer->Release();
		if (FAILED(hr))
			return hr;

		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = Wind->GetWindowSize().width;
		descDepth.Height = Wind->GetWindowSize().height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);
		if (FAILED(hr))
			return hr;

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		hr = g_pd3dDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
		if (FAILED(hr))
			return false;

		// ��������� ����� �����
		g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, m_pDepthStencilView);


		// ��������� ��������
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)Wind->GetWindowSize().width;
		vp.Height = (FLOAT)Wind->GetWindowSize().height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		// ��������� ���� �� ��������
		g_pImmediateContext->RSSetViewports(1, &vp);


		// �������� ������ (��������) ��������
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;      // ��� ����������
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;         // ������ ����������
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		// ������� ��������� ������ ���������������
		hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
		if (FAILED(hr))
			return hr;

		g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
		
		return S_OK;
	}

	// ��������� ���������� �������
	ID3D11VertexShader* CreateVSheider(char *name, wchar_t *file = L"Sheider.fx") {
		ID3D11VertexShader *g_pVertexShader = NULL;
		ID3D11InputLayout *g_pVertexLayout = NULL;
		HRESULT hr = S_OK;
		ID3DBlob* pVSBlob = NULL;

		// ��������� ���������� �������
		hr = CompileShaderFromFile(file, name, "vs_4_0", &pVSBlob);
		if (FAILED(hr)) {
			MessageBox(NULL, L"������ �� �������������!!!.", L"�������", MB_OK);
			return NULL;
		}

		// ��������� ��������� �������
		hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader);
		if (FAILED(hr)) {
			pVSBlob->Release();
			return NULL;
		}
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXTCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		// �������� ������� ������
		hr = g_pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pVertexLayout);
		pVSBlob->Release();
		if (FAILED(hr))
			return NULL;
		// ����������� ������� ������
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

		return g_pVertexShader;
	}
	// ��������� ���������� �������
	ID3D11PixelShader* CreatePSheider(char *name, wchar_t *file = L"Sheider.fx") {
		ID3D11PixelShader *g_pPixelShader;
		HRESULT hr = S_OK;

		// ��������� ���������� �������
		ID3DBlob* pPSBlob = NULL;
		hr = CompileShaderFromFile(file, name, "ps_4_0", &pPSBlob);
		if (FAILED(hr)) {
			MessageBox(NULL, L"������ �� �������������!!!.", L"�������", MB_OK);
			return NULL;
		}
		// �������� ����������� �������
		hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader);
		pPSBlob->Release();
		if (FAILED(hr))
			return NULL;

		return g_pPixelShader;
	}

	// �������� ������
	void Clear() {
		//float ClearColor[4] = { 0.2f, 0.3f, 0.6f, 1.0f };
		float ClearColor[4] = { 0.f, 0.f, 0.f, 1.0f };
		g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
		g_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	// ���������
	void Show() {
		g_pSwapChain->Present(0, 0);
	}

	// ������ ID3D11Device
	ID3D11Device* GetID3D11Device() {
		return g_pd3dDevice;
	}
	// ������ ID3D11DeviceContext
	ID3D11DeviceContext* GetID3D11DeviceContext() {
		return g_pImmediateContext;
	}

	~Render() {
		if (g_pImmediateContext)
			g_pImmediateContext->ClearState();


		if (m_pDepthStencilView)
			m_pDepthStencilView->Release();
		if (m_pDepthStencil)
			m_pDepthStencil->Release();
		if (g_pRenderTargetView)
			g_pRenderTargetView->Release();
		if (g_pSwapChain)
			g_pSwapChain->Release();
		if (g_pImmediateContext)
			g_pImmediateContext->Release();
		if (g_pd3dDevice)
			g_pd3dDevice->Release();
	}
};