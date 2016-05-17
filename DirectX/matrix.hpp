#pragma once

#include "types.hpp"
#include "window.hpp"
#include "map.hpp"
#include <string>

#define EYE_ANGLE_MAX -0.523598776f
#define EYE_ANGLE_MIN -3.1415 / 3.f
#define CAM_NEAR 0.01f
#define CAM_FAR 100.f

class Matrix {
	static XMMATRIX g_World;		// Матриця світу
	static XMMATRIX g_View;			// Матриця світу
	static XMMATRIX g_Projection;	// Матриця проекції

	XMFLOAT3 From;  // Звідки
	XMFLOAT3 At;    // Куди
	float fi;
	float r;

	float dx;
	float dz;

	struct {
		float top;
		float bot;

		float left;
		float right;
	} mez;

	bool init;

	MatrixConstantBuffer cb;

	ID3D11Buffer *g_pConstantMatrixBuffer;
	ID3D11DeviceContext *g_pImmediateContext;

	float AngleCam;
	float Zb, Zf;

	void GetNewZfZb() {
		float alpha = asinf(From.y / r);

		Zb = sin(AngleCam / 2.f) * r / sin(XM_PI - AngleCam / 2.f - alpha);
		Zf = sin(AngleCam / 2.f) * r / sin(alpha - AngleCam / 2.f);
	}

public:
	Matrix(ID3D11Device *pd3dDevice, ID3D11DeviceContext *pImmediateContext, float h, Window *Wind) {
		g_World = XMMatrixIdentity();
		g_Projection = XMMatrixPerspectiveFovLH(AngleCam = ANGLE_CAM, Wind->GetWindowSize().width / (FLOAT)Wind->GetWindowSize().height, CAM_NEAR, CAM_FAR);

		From = XMFLOAT3(0.f, 0.f, 0.f);
		At = XMFLOAT3(0.f, 0.f, 0.f);

		fi = EYE_ANGLE_MAX;
		r = h;
		UpdateCamFromPos(0.f);


		g_pImmediateContext = pImmediateContext;

		// Створення константного буфера матриць
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(MatrixConstantBuffer);     // розмір буфера == розміру структури
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // тип - константний буфер
		bd.CPUAccessFlags = 0;
		if (FAILED(pd3dDevice->CreateBuffer(&bd, NULL, &g_pConstantMatrixBuffer)))
			return;

		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

		dx = dz = 0.f;

		mez.left = mez.bot = mez.right = mez.top = 0.f;
		init = false;

		SetMatrix();
	}

	void SetMatrix() {
		init = true;

		cb.mWorld = XMMatrixTranspose(g_World);
		cb.mView = XMMatrixTranspose(g_View = XMMatrixLookAtLH(XMVectorSet(From.x, From.y, From.z, 0.0f), XMVectorSet(At.x, At.y, At.z, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
		cb.mProjection = XMMatrixTranspose(g_Projection);

		g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, NULL, &cb, 0, 0);
	}

	XMFLOAT3 screenToWorldPoint(int x, int y, Window* Wind, float mHight) {
		XMVECTOR mousepos = {
			(float)x,
			(float)y,
			0.f,
			1.f
		};

		XMVECTOR vPickRayOrig = XMVector3Unproject(mousepos, 0.f, 0.f, (float)Wind->GetWindowSize().width, (float)Wind->GetWindowSize().height, CAM_NEAR, CAM_FAR, g_Projection, g_View, g_World);
		mousepos.m128_f32[2] = 1.f;
		XMVECTOR vPickRayDir = XMVector3Unproject(mousepos, 0.f, 0.f, Wind->GetWindowSize().width, Wind->GetWindowSize().height, CAM_NEAR, CAM_FAR, g_Projection, g_View, g_World);

		XMVECTOR a = {0.f, mHight, 0.f, 0.f},
			b = {0.f, 1.f, 0.f, 0.f};
		XMVECTOR p = XMPlaneFromPointNormal(a, b);
		p = XMPlaneIntersectLine(p, vPickRayOrig, vPickRayDir);
		return XMFLOAT3(p.m128_f32[0], p.m128_f32[1], -p.m128_f32[2]);
	}

	void UpdateCamFromPos(float f) {
		fi += f;
		if (fi > EYE_ANGLE_MAX)
			fi = EYE_ANGLE_MAX;
		else if (fi < EYE_ANGLE_MIN)
			fi = (float)EYE_ANGLE_MIN;

		From = XMFLOAT3(From.x, At.y + r * cos(fi), At.z + r * sin(fi));

		GetNewZfZb();
		if (At.z - Zb < mez.bot)
			UpdateFromAt(0.f, 0.f, mez.bot - (At.z - Zb));
	}
	void UpdateFromAt(float x, float y, float z) { //FIXME: delete y
		From.x += x;
		From.y += y;
		From.z += z;

		At.x += x;
		At.y += y;
		At.z += z;
	}

	void MoveCamWhile() {
		if (dx && mez.left <= From.x + dx && From.x + dx <= mez.right)
			UpdateFromAt(dx, 0.f, 0.f);

		if (dz && mez.top >= At.z + dz && At.z - Zb + dz > mez.bot)
			UpdateFromAt(0.f, 0.f, dz);
	}
	void SetDXMoveCamWhile(float dx_) {
		dx = dx_;
	}
	void SetDZMoveCamWhile(float dz_) {
		dz = dz_;
	}

	void SetMez(Map *map, Window* window) {
		POINT ptCur = {0, 0};
		XMFLOAT3 pos = this->screenToWorldPoint(ptCur.x, ptCur.y, window, map->getMHeight());

		mez.top = pos.z / 3.f;
		mez.bot = map->GetBot() - pos.z / 6.f;
		if (mez.bot <= At.z)
			UpdateFromAt(0.f, 0.f, mez.top - At.z);

		mez.left = -pos.x * 2.f / 3.f;
		mez.right = map->GetRight() + pos.x * 2.f / 3.f;
		if (mez.left >= At.x)
			UpdateFromAt(mez.left - At.x, 0.f, 0.f);
	}

	const XMFLOAT3& GetAt() {
		return At;
	}
	float GetZb() {
		return Zb;
	}
	float GetZf() {
		return Zf;
	}

	~Matrix() {
		if (g_pConstantMatrixBuffer)
			g_pConstantMatrixBuffer->Release();
	}
};

XMMATRIX Matrix::g_World = {};
XMMATRIX Matrix::g_View = {};
XMMATRIX Matrix::g_Projection = {};