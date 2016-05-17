#pragma once
#include "types.hpp"
#include "objects.hpp"

class Circle : public Object {
	float dy;

	virtual float getHeight(float x, float z) { return map->GetHSm(x, -z) + dy; }
public:
	Circle(float x, float z, Map* map, float r, ID3D11Device *pd3dDevice, ID3D11DeviceContext *context, wchar_t *text, ID3D11VertexShader *pVertexShader, ID3D11PixelShader *pPixelShader, float dy = .15f) : Object(pd3dDevice, context, pVertexShader, pPixelShader) {
		VERTEX a;

		this->map = map;

		z = -z;

		a.position = XMFLOAT3(0, 0, 0);
		a.normal = XMFLOAT3(0.f, 1.f, 0.f);
		a.texcoord = XMFLOAT2(1.f, 1.f);
		bvertices.push_back(a);


		for (float i = 0.f; i <= 6.4; i += .1f) {
			float _x = r*sin(i),
				_z = r*cos(i);

			a.position = XMFLOAT3(_x, 0, _z);
			bvertices.push_back(a);
		}

		for (int i = 2; i < bvertices.size(); ++i) {
			bindices.push_back(0);
			bindices.push_back(i-1);
			bindices.push_back(i);
		}
		indicesCount = bindices.size();

		this->dy = dy;
		pt_to = pt = XMFLOAT3(x, map->GetHSm(x, z) + dy, z);
		this->r = r;

		D3D11_BUFFER_DESC bd = {};
		D3D11_SUBRESOURCE_DATA InitData;

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(DWORD)* indicesCount;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		InitData.pSysMem = &bindices[0];
		if (FAILED(c_pd3dDevice->CreateBuffer(&bd, &InitData, &c_pIndexBuffer)))
			return;
		vert = new VERTEX[bvertices.size()];
		memcpy_s(vert, bvertices.size() * sizeof(VERTEX), &bvertices[0], bvertices.size() * sizeof(VERTEX));

		D3DX11CreateShaderResourceViewFromFile(pd3dDevice, text, NULL, NULL, &g_pTextureRV, NULL);
		UpDateVericesBuffer();

		meshloaded = true;
	}

	virtual void whileGotoXY() {
		if (pt_to.x != pt.x || pt_to.z != pt.z) {
			pt = pt_to;

			updateGoto(pt);
		}
	}

	void afterDraw() {}
};