#pragma once

#include "types.hpp"
#include "map.hpp"
#include "drawobjectinterface.h"

class ObjectInterface : public DrawObjectInterface {
protected:
	ID3D11Device *c_pd3dDevice;
	ID3D11DeviceContext *c_pImmediateContext;

	ID3D11Buffer *c_pVertexBuffer;
	ID3D11Buffer *c_pIndexBuffer;
	ID3D11ShaderResourceView *g_pTextureRV;

	ID3D11VertexShader *g_pVertexShader;
	ID3D11PixelShader *g_pPixelShader;

	Map* map;
	XMFLOAT3 pt;
	XMFLOAT3 pt_to;

	bool meshloaded = false;
	float scale = 1.f;
	float a = 0.f;

	virtual HRESULT UpDateVericesBuffer() = 0;
	virtual float getHeight(float x, float z) = 0;

public:
	ObjectInterface() {}
	ObjectInterface(ID3D11Device *pd3dDevice, ID3D11DeviceContext *context, ID3D11VertexShader *pVertexShader, ID3D11PixelShader *pPixelShader) {
		meshloaded = false;
		c_pd3dDevice = pd3dDevice;
		c_pImmediateContext = context;
		c_pVertexBuffer = NULL;
		c_pIndexBuffer = NULL;

		g_pVertexShader = pVertexShader;
		g_pPixelShader = pPixelShader;
		if (!g_pVertexShader || !g_pPixelShader) {
			MessageBox(NULL, L"Не передані шейдери!", L"Помилка", NULL);
			return;
		}
	}
	virtual void updateGoto(const XMFLOAT3& pt, float a = .0f) = 0;

	virtual void GotoXY(const XMFLOAT3& ptm) = 0;
	virtual void whileGotoXY() = 0;
	virtual void Draw(bool bl = false) = 0;
	virtual float GetR() = 0;
	virtual float GetA() { return a; }
	virtual const XMFLOAT3& GetPos() = 0;

	virtual bool gotoEnd() { return pt_to.x == pt.x && pt_to.z == pt.z; }
};