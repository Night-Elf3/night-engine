#pragma once
#include "types.hpp"

class Map {
	int height;
	int width;

	HBITMAP hbm;
	HDC hdc;

	ID3D11Device *c_pd3dDevice;
	ID3D11DeviceContext *c_pImmediateContext;

	ID3D11Buffer *c_pVertexBuffer;
	ID3D11Buffer *c_pIndexBuffer;
	int SizeI;

	ID3D11ShaderResourceView *g_pTextureRV;

	ID3D11VertexShader *g_pVertexShader;
	ID3D11PixelShader *g_pPixelShader;

	SimpleVertex *vertices;
	bool loaded;
	float mHeight;

	class VecF32 {
		XMVECTORF32 vert;
	public:
		VecF32() : vert()
		{}

		XMVECTORF32 GetVec() {
			return vert;
		}

		VecF32 operator = (const XMFLOAT3& a) {
			vert.f[0] = a.x;
			vert.f[1] = a.y;
			vert.f[2] = a.z;
			vert.f[3] = 0;

			return *this;
		}
		VecF32 operator -(const VecF32& a) {
			VecF32 v(*this);

			for (int i = 0; i < 4; ++i)
				v.vert.f[i] -= a.vert.f[i];

			return v;
		}
		VecF32 operator +=(const XMVECTOR& a) {
			for (int i = 0; i < 4; ++i)
				vert.f[i] += a.m128_f32[i];

			return *this;
		}
		VecF32 operator /=(float a) {
			for (int i = 0; i < 4; ++i)
				vert.f[i] /= a;

			return *this;
		}
	};

public:
	Map(ID3D11Device *pd3dDevice, ID3D11DeviceContext *context, wchar_t *map_path, wchar_t *text_path, ID3D11VertexShader *pVertexShader, ID3D11PixelShader *pPixelShader) {
		loaded = false;

		DWORD *indices;
		int SizeV;

		c_pd3dDevice = pd3dDevice;
		c_pImmediateContext = context;
		if (!c_pd3dDevice || !c_pImmediateContext) {
			MessageBox(NULL, L"Не переданий пристрій і контекст DirectX!", L"Помилка", NULL);
			return;
		}

		g_pVertexShader = pVertexShader;
		g_pPixelShader = pPixelShader;
		if (!g_pVertexShader || !g_pPixelShader) {
			MessageBox(NULL, L"Не передані шейдери!", L"Помилка", NULL);
			return;
		}

		BITMAP bm;

		hbm = (HBITMAP)LoadImage(NULL, map_path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		if (!hbm) {
			MessageBox(NULL, L"Не знайдено файлу карти!", L"Помилка", NULL);
			return;
		}
		if (FAILED(D3DX11CreateShaderResourceViewFromFile(c_pd3dDevice, text_path, NULL, NULL, &g_pTextureRV, NULL))) {
			MessageBox(NULL, L"Не знайдено файлу текстури!", L"", NULL);
			return;
		}

		GetObject(hbm, sizeof(bm), &bm);
		height = bm.bmHeight;
		width = bm.bmWidth;
		vertices = new SimpleVertex[SizeV = height * width]();

		hdc = CreateCompatibleDC(0);
		SelectObject(hdc, hbm);

		WORD k = 0;
		mHeight = 0;
		for (WORD z = 0, x; z < height; ++z) {
			for (x = 0; x < width; ++x) {
				vertices[k].Pos.x = x;
				mHeight += vertices[k].Pos.y = GetH(x, z);
				vertices[k++].Pos.z = (FLOAT)-z;
			}
		}
		mHeight /= k;


		VecF32 vert[6];
		int nvert,
			ind[6];
		VecF32 res,
			vertStart;
		for (int z = 0, x, i, k = 0, l; z < height; ++z) {
			for (x = 0; x < width; ++x, ++k) {
				res = XMFLOAT3(0.f, 0.f, 0.f);
				nvert = 0;

				vertStart = vertices[z*width + x].Pos;

				if (x - 1 >= 0) {
					ind[nvert] = 0;
					vert[nvert++] = vertices[z*width + (x - 1)].Pos;
				}

				if (z - 1 >= 0) {
					ind[nvert] = 1;
					vert[nvert++] = vertices[(z - 1)*width + x].Pos;

					if (x + 1 < width) {
						ind[nvert] = 2;
						vert[nvert++] = vertices[(z - 1)*width + (x + 1)].Pos;
					}
				}

				if (x + 1 < width) {
					ind[nvert] = 3;
					vert[nvert++] = vertices[z*width + (x + 1)].Pos;
				}

				if (z + 1 < height) {
					ind[nvert] = 4;
					vert[nvert++] = vertices[(z + 1)*width + x].Pos;

					if (x - 1 >= 0) {
						ind[nvert] = 5;
						vert[nvert++] = vertices[(z + 1)*width + (x - 1)].Pos;
					}
				}

				l = -1;
				for (int i = 0; l < 0 && i < nvert - 1; ++i) {
					if (ind[i + 1] == ind[i] + 1)
						l = i;
				}
				if (l < 0)
					l = nvert - 1;

				if (nvert != 6)
					--nvert;
				for (i = 0; i < nvert; ++i, ++l)
					res += XMVector3Normalize(XMVector3Cross((vertStart - vert[l%nvert]).GetVec(), (vertStart - vert[(l + 1) % nvert]).GetVec()));
				res /= (float)nvert;

				vertices[k].Normal.x = res.GetVec().f[0];
				vertices[k].Normal.y = res.GetVec().f[1];
				vertices[k].Normal.z = res.GetVec().f[2];

				vertices[k].Tex = XMFLOAT2((FLOAT)x, (FLOAT)-z);
			}
		}

		indices = new DWORD[SizeI = SizeV * 6]();
		for (int i = 0, j, k = 0, tmp1, tmp2; i < height - 1; ++i) {
			for (j = 0; j < width - 1; ++j) {
				tmp1 = i*width + j;
				tmp2 = (i + 1)*width + j;

				indices[k++] = tmp1;
				indices[k++] = tmp1 + 1;
				indices[k++] = tmp2;

				indices[k++] = tmp1 + 1;
				indices[k++] = tmp2 + 1;
				indices[k++] = tmp2;
			}
		}

		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex)* SizeV;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = vertices;
		if (FAILED(c_pd3dDevice->CreateBuffer(&bd, &InitData, &c_pVertexBuffer))) {
			MessageBox(NULL, L"Не вдалось створити буфер вершин!", L"Помилка", NULL);

			delete[] vertices;
			delete[] indices;
			return;
		}

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(DWORD)* SizeI;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		InitData.pSysMem = indices;
		if (FAILED(c_pd3dDevice->CreateBuffer(&bd, &InitData, &c_pIndexBuffer))) {
			MessageBox(NULL, L"Не вдалось створити буфер індексів!", L"Помилка", NULL);

			delete[] vertices;
			delete[] indices;
			return;
		}

		delete[] indices;

		loaded = true;
	}


	float getMHeight() { return mHeight; }
	float GetHSm(float x, float z) {
		int i = (int)z * width + (int)x;
		if (x >= 0 && z >= 0) {
			XMVECTOR a = { vertices[i].Pos.x, vertices[i].Pos.y, vertices[i].Pos.z, 0.f },
				b = { vertices[i].Normal.x, vertices[i].Normal.y, vertices[i].Normal.z, 0.f };
			XMVECTOR p = XMPlaneFromPointNormal(a, b);

			a = { x, 0.f, -z, 0.f };
			b = { x, 1.f, -z, 0.f };
			p = XMPlaneIntersectLine(p, a, b);

			return p.m128_f32[1];
		}
		return 0.f;
	}
	float GetH(float x, float z) {
		COLORREF pix = GetPixel(hdc, (int)x, (int)fabs(z));

		return (float)(GetRValue(pix) + GetGValue(pix) + GetBValue(pix)) / 4.f / 100.f;
	}

	void Draw() {
		if (loaded) {
			UINT stride = sizeof(SimpleVertex);
			UINT offset = 0;

			c_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
			c_pImmediateContext->IASetVertexBuffers(0, 1, &c_pVertexBuffer, &stride, &offset);
			c_pImmediateContext->IASetIndexBuffer(c_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			c_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			c_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
			c_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);

			c_pImmediateContext->DrawIndexed(SizeI, 0, 0);
		}
	}

	float GetTop() {
		return 0;
	}
	float GetLeft() {
		return 0;
	}
	float GetBot() {
		return -1.f*height;
	}
	float GetRight() {
		return (float)width;
	}

	~Map() {
		if (c_pVertexBuffer)
			c_pVertexBuffer->Release();
		if (c_pIndexBuffer)
			c_pIndexBuffer->Release();

		DeleteObject(hbm);
		ReleaseDC(0, hdc);
	}
};