#pragma once
#include "types.hpp"
#include "objects.hpp"
#include "circle.hpp"
#include <vector>
#include <fstream>
using std::vector;
using std::pair;
using std::ios;
using std::ifstream;

extern Map* Mp;
extern Render* Rend;

class Mesh : public Object {
	Circle* circle;

	HRESULT LoadMeshFromMSH(wchar_t *fname) {
		UINT tmp;
		ifstream inp(fname, ios::binary | ios::in);

		inp.read((char*)&tmp, sizeof(UINT));
		bvertices.resize(tmp);
		inp.read((char*)&bvertices[0], sizeof(VERTEX)* tmp);

		inp.read((char*)&indicesCount, sizeof(UINT));
		bindices.resize(indicesCount);
		inp.read((char*)&bindices[0], sizeof(UINT)* indicesCount);

		inp.read((char*)&pt, sizeof(XMFLOAT3));
		inp.read((char*)&r, sizeof(float));
		inp.close();

		D3D11_BUFFER_DESC bd = {};
		D3D11_SUBRESOURCE_DATA InitData;

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(DWORD)* indicesCount;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		InitData.pSysMem = &bindices[0];
		if (FAILED(c_pd3dDevice->CreateBuffer(&bd, &InitData, &c_pIndexBuffer)))
			return -1;

		vert = new VERTEX[bvertices.size()];
		memcpy_s(vert, bvertices.size() * sizeof(VERTEX), &bvertices[0], bvertices.size() * sizeof(VERTEX));

		return UpDateVericesBuffer();
	}

public:
	Mesh(float x, float z, Map* mp, float stp, float scl, ID3D11Device *pd3dDevice, ID3D11DeviceContext *context, wchar_t *fname, wchar_t *text, ID3D11VertexShader *pVertexShader, ID3D11PixelShader *pPixelShader, float dy = .1f) : Object(pd3dDevice, context, pVertexShader, pPixelShader) {
		meshloaded = !FAILED(LoadMeshFromMSH(fname)) && !FAILED(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, text, NULL, NULL, &g_pTextureRV, NULL));

		map = mp;
		circle = new Circle(
			x, 
			z, 
			mp, 
			r * scale, 
			pd3dDevice, 
			context, 
			text, 
			Rend->CreateVSheider("CircleVertexShader"), 
			Rend->CreatePSheider("CirclePixelShader"),
			dy
		);
		this->steep = stp;

		pt.x = x;
		pt.z = z;
		pt.y = mp->GetHSm(x, z);

		pt_to = pt;
		scale = scl;
		r *= scale;

		updateGoto(pt, 0);
		circle->GotoXY(pt);
		circle->whileGotoXY();

		if (!meshloaded)
			MessageBox(NULL, L"Модель не завантажена!", L"Помилка", NULL);
	}

	virtual void whileGotoXY() {
		if (pt_to.x != pt.x || pt_to.z != pt.z) {
			float dx = pt_to.x - pt.x,
				dz = pt_to.z - pt.z;

			if (dx == 0 && dz == 0)
				a = 0.f;
			else if (dz > 0)
				a = atan(dx / dz);
			else if (dx != 0)
				a = (dx < 0 ? -3.1415 / 2.f : 3.1415 / 2.f) - atan(dz / dx);
			else
				a = 3.1415 * 1.5f;

			if (sqrt(dx*dx + dz*dz) >= steep) {
				pt.x += steep*sin(a);
				pt.z += steep*cos(a);
			}
			else
				pt = pt_to;

			pt.y = map->GetHSm(pt.x, pt.z);

			SetPos(pt, a);
		}
	}

	virtual void afterDraw() {
		circle->Draw();
	}
	virtual void SetPos(const XMFLOAT3& pt, float a = 0.f) {
		this->pt = pt;
		this->pt_to = pt;
		updateGoto(pt, a);

		circle->GotoXY(pt);
		circle->whileGotoXY();
	};
};