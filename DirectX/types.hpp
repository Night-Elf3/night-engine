#pragma once

#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#include <Windows.h>
#pragma warning(disable: 4838 4005)
#include <xnamath.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#pragma warning(default: 4838 4005)
#include "resource.h"

#include "window.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")

struct WindowSize {
	UINT height;
	UINT width;

	WindowSize(UINT w = 0u, UINT h = 0u) : width(w), height(h)
	{}
};
struct SimpleVertex {
	XMFLOAT3 Pos;			// Координата
	XMFLOAT3 Normal;		// Вектор нормалі(освітлення)
	XMFLOAT2 Tex;			// Координати текстури

	SimpleVertex() {
		Normal = Pos = XMFLOAT3(0.f, 0.f, 0.f);
		Tex = XMFLOAT2(0.f, 0.f);
	}
};
struct MatrixConstantBuffer {
	XMMATRIX mWorld;		// Матриця світу
	XMMATRIX mView;			// Матриця виду
	XMMATRIX mProjection;	// Матриця проекції
};

#define MAX_LIGHTS_COUNT 3
struct LightConstantBuffer {
	XMFLOAT4 vLightColor[MAX_LIGHTS_COUNT];
	XMFLOAT4 vLightPos[MAX_LIGHTS_COUNT];
	__int32 n;
	__int32 tmp[MAX_LIGHTS_COUNT];
};

#define VK_KEY(a) int(0x41 + (a-'a'))
#define ANGLE_CAM XM_PIDIV4