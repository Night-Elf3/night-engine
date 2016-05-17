#include "types.hpp"
#include "map.hpp"
#include <vector>
using std::vector;

struct VERTEX {
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texcoord;
};

class Object : public ObjectInterface {
protected:
	vector<VERTEX> bvertices;
	vector<DWORD>  bindices;
	VERTEX *vert;
	UINT indicesCount;

	float r;
	float steep;

	HRESULT UpDateVericesBuffer() {
		if (c_pVertexBuffer)
			c_pVertexBuffer->Release();

		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX)* bvertices.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = vert;
		if (FAILED(c_pd3dDevice->CreateBuffer(&bd, &InitData, &c_pVertexBuffer)))
			return -1;

		return 1;
	}
	virtual float getHeight(float x, float z) { return pt.y; }
public:
	Object(ID3D11Device *pd3dDevice, ID3D11DeviceContext *context, ID3D11VertexShader *pVertexShader, ID3D11PixelShader *pPixelShader) : ObjectInterface(pd3dDevice, context, pVertexShader, pPixelShader)
	{}

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
	virtual void whileGotoXY() = 0;
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

			c_pImmediateContext->DrawIndexed(indicesCount, 0, 0);

			if (bl)
				afterDraw();
		}
	}
	virtual void afterDraw() = 0;
	void updateGoto(const XMFLOAT3& pt, float a = .0f) {
		float s = sin(a),
			c = cos(a);

		for (int i = 0; i < bvertices.size(); ++i) {
			vert[i].position.x = bvertices[i].position.x * scale * c - bvertices[i].position.z * s + pt.x;
			vert[i].position.z = bvertices[i].position.x * scale * s + bvertices[i].position.z * c - pt.z;
			vert[i].position.y = bvertices[i].position.y * scale + getHeight(vert[i].position.x, vert[i].position.z);

			vert[i].normal.x = bvertices[i].normal.x * c - bvertices[i].normal.z * s;
			vert[i].normal.z = bvertices[i].normal.x * s + bvertices[i].normal.z * c;
		}

		this->pt = pt;

		UpDateVericesBuffer();
	}

	virtual float GetR() {
		return r;
	}
	virtual const XMFLOAT3& GetPos() {
		return pt;
	};

	~Object() {
		if (c_pVertexBuffer)
			c_pVertexBuffer->Release();
		if (c_pIndexBuffer)
			c_pIndexBuffer->Release();
	}
};
