#pragma once

#include "types.hpp"

class DrawObjectInterface {
public:
	virtual void GotoXY(const XMFLOAT3& ptm) = 0;
	virtual void whileGotoXY() = 0;
	virtual void Draw(bool bl = false) = 0;
	virtual void afterDraw() {};
	virtual float GetR() = 0;
	virtual float GetA() = 0;
	virtual const XMFLOAT3& GetPos() = 0;
	virtual void SetPos(const XMFLOAT3&, float = 0.f) {};

	virtual void newPeriod() {}
	virtual bool gotoEnd() { return true; }
	virtual void updateGoto(const XMFLOAT3& pt, float a = .0f) {};
};