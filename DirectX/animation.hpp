#pragma once

#include "types.hpp"
#include "mesh.hpp"

class Animation : public ObjectInterface {
	class Frame {
	public:
		vector<VERTEX> bvertices;
		vector<DWORD>  bindices;
		UINT indicesCount;

		XMFLOAT3 pt;
		float r;


		Frame(ifstream &inp) {
			UINT tmp;

			inp.read((char*)&tmp, sizeof(UINT));
			bvertices.resize(tmp);
			inp.read((char*)&bvertices[0], sizeof(VERTEX)* tmp);

			inp.read((char*)&indicesCount, sizeof(UINT));
			bindices.resize(indicesCount);
			inp.read((char*)&bindices[0], sizeof(UINT)* indicesCount);

			inp.read((char*)&pt, sizeof(XMFLOAT3));
			inp.read((char*)&r, sizeof(float));
		}
	};

	vector<Frame*> frames;
	VERTEX *vert;
	int cur;

	Circle* circle;
	float r;
	float steep;

	HRESULT UpDateVericesBuffer() {
		if (c_pIndexBuffer)
			c_pIndexBuffer->Release();
		D3D11_BUFFER_DESC bd = {};
		D3D11_SUBRESOURCE_DATA InitData;

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(DWORD)* frames[cur]->indicesCount;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		InitData.pSysMem = &frames[cur]->bindices[0];
		if (FAILED(c_pd3dDevice->CreateBuffer(&bd, &InitData, &c_pIndexBuffer)))
			return -1;

		if (c_pVertexBuffer)
			c_pVertexBuffer->Release();

		memset(&bd, 0, sizeof(D3D11_BUFFER_DESC));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX)* frames[cur]->bvertices.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		InitData.pSysMem = vert;
		if (FAILED(c_pd3dDevice->CreateBuffer(&bd, &InitData, &c_pVertexBuffer)))
			return -1;

		return 1;
	}
	virtual float getHeight(float x, float z) { return pt.y; };
public:
	Animation(float x, float z, Map* mp, float stp, float scl, ID3D11Device *pd3dDevice, ID3D11DeviceContext *context, wchar_t *fname, wchar_t *text, ID3D11VertexShader *pVertexShader, ID3D11PixelShader *pPixelShader, float dy = .1f) : ObjectInterface(pd3dDevice, context, pVertexShader, pPixelShader) {
		ifstream inp(fname, ios::binary | ios::in);
		size_t size;

		if (!inp) {
			meshloaded = false;
			MessageBox(NULL, L"Модель не завантажена!", L"Помилка", NULL);

			return;
		}
		meshloaded = !FAILED(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, text, NULL, NULL, &g_pTextureRV, NULL));

		UINT maxB = 0;
		pt = {0, 0, 0};
		r = 0.f;
		inp.read((char*)&size, sizeof(size_t));
		for (size_t i = 0; i < size; ++i) {
			frames.push_back(new Frame(inp));
			
			r += frames[i]->r / (float)size;
			pt.x += frames[i]->pt.x / (float)size;
			pt.z += frames[i]->pt.z / (float)size;

			if (maxB < frames[i]->indicesCount)
				maxB = frames[i]->indicesCount;
		}
		inp.close();
		vert = new VERTEX[maxB];

		cur = 0;

		map = mp;
		scale = scl;
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
		steep = stp;

		pt.x = x;
		pt.y = mp->GetHSm(x, z);
		pt.z = z;
		GotoXY(pt);
		pt = pt_to;

		updateGoto(pt, 0);
		circle->updateGoto(pt, 0);
	}

	virtual void GotoXY(const XMFLOAT3& ptm) {
		pt_to = ptm;

		if (pt_to.x - r < map->GetLeft())
			pt_to.x = map->GetLeft() + (r + 1);
		if (pt_to.x + r > map->GetRight())
			pt_to.x = map->GetRight() - (r + 1);

		if (pt_to.z - r < map->GetTop())
			pt_to.z = map->GetTop() + (r + 1);
		if (pt_to.z - r < map->GetBot()) //FIXME: not tested
			pt_to.z = map->GetBot() + (r + 1);
	}

	virtual void Draw(bool bl = false) {
    		if (meshloaded) {
			UINT stride = sizeof(VERTEX);
			UINT offset = 0;

			c_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);
			c_pImmediateContext->IASetVertexBuffers(0, 1, &c_pVertexBuffer, &stride, &offset);
			c_pImmediateContext->IASetIndexBuffer(c_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			c_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			c_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
			c_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);

			c_pImmediateContext->DrawIndexed(frames[cur = (cur+1) % frames.size()]->indicesCount, 0, 0);

			if (bl)
				afterDraw();
		}
	}
	virtual void afterDraw() {
		circle->Draw();
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

			updateGoto(pt, a);

			circle->GotoXY(pt);
			circle->whileGotoXY();
		}
	}
	virtual void updateGoto(const XMFLOAT3& pt, float a = .0f) {
		float s = sin(a),
			c = cos(a);

		for (int i = 0; i < frames[cur]->bvertices.size(); ++i) {
			vert[i].position.x = frames[cur]->bvertices[i].position.x * scale * c - frames[cur]->bvertices[i].position.z * scale * s + pt.x;
			vert[i].position.z = frames[cur]->bvertices[i].position.x * scale * s + frames[cur]->bvertices[i].position.z * scale * c - pt.z;
			vert[i].position.y = frames[cur]->bvertices[i].position.y * scale + getHeight(vert[i].position.x, vert[i].position.z);

			vert[i].normal.x = frames[cur]->bvertices[i].normal.x * c - frames[cur]->bvertices[i].normal.z * s;
			vert[i].normal.y = frames[cur]->bvertices[i].normal.y;
			vert[i].normal.z = frames[cur]->bvertices[i].normal.x * s + frames[cur]->bvertices[i].normal.z * c;

			vert[i].texcoord = frames[cur]->bvertices[i].texcoord;
		}

		UpDateVericesBuffer();
	};


	float GetR() {
		return r * scale;
	}
	float GetA() { return a; }
	const XMFLOAT3& GetPos() {
		return pt;
	}

	void newPeriod() { cur = 0; }
	virtual bool gotoEnd() { return pt_to.x == pt.x && pt_to.z == pt.z; }
};