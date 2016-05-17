#pragma once

#include "types.hpp"
#include <vector>
using std::vector;
using std::pair;

class Light {
	LightConstantBuffer lb;

	ID3D11DeviceContext *g_pImmediateContext;
	ID3D11Buffer *g_pConstantLightBuffer;

	vector<pair<XMFLOAT4, XMFLOAT4>> lights;
public:
	Light(ID3D11Device *pd3dDevice, ID3D11DeviceContext *ImmediateContext) {
		g_pImmediateContext = ImmediateContext;

		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(LightConstantBuffer);     // розмір буфера == розміру структури
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // тип - константний буфер
		bd.CPUAccessFlags = 0;

		// Створення константного буфера освітлення
		if (FAILED(pd3dDevice->CreateBuffer(&bd, NULL, &g_pConstantLightBuffer)))
			return;

		g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pConstantLightBuffer);
	}

	void addLight(XMFLOAT4 color, XMFLOAT4 position) {
		lights.push_back(pair<XMFLOAT4, XMFLOAT4>(color, position));
	}

	void setLight() {
		int i = 0;
		lb.n = 0;
		for each (pair<XMFLOAT4, XMFLOAT4> a in lights) {
			if (i < MAX_LIGHTS_COUNT) {
				lb.vLightColor[i] = a.first;
				lb.vLightPos[i] = a.second;
				lb.n++;
			}
			++i;
		}

		g_pImmediateContext->UpdateSubresource(g_pConstantLightBuffer, 0, NULL, &lb, 0, 0);
	}

	~Light() {
		if (g_pConstantLightBuffer)
			g_pConstantLightBuffer->Release();
	}
};