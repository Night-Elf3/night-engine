#pragma once

#include "drawobjectinterface.h"
#include "mesh.hpp"
#include "animation.hpp"
#include <map>
#include <string>
using std::string;

class Man : public DrawObjectInterface {
private:
	std::map<string, ObjectInterface*> anim;
	string cur;
public:
	Man(const XMFLOAT3& pos, Map* map) {
		anim[cur = "run"] = new Animation (
			pos.x,
			pos.z,
			map,
			.1f,
			.6f,
			Rend->GetID3D11Device(),
			Rend->GetID3D11DeviceContext(),
			L"resource\\mesh\\anim.msh",
			L"resource\\mesh\\ball.png",
			Rend->CreateVSheider("MapVertexShader"),
			Rend->CreatePSheider("MapPixelShader"),
			0.08f
		);
	}
	virtual void GotoXY(const XMFLOAT3& ptm) {
		anim[cur]->GotoXY(ptm);
	}
	virtual void whileGotoXY() {
		if (anim[cur]->gotoEnd()) {
			anim[cur]->newPeriod();
			anim[cur]->updateGoto(anim[cur]->GetPos(), anim[cur]->GetA());
		}

		anim[cur]->whileGotoXY();
	}
	virtual void Draw(bool bl = false) {
		anim[cur]->Draw(bl);
	}
	virtual float GetR() {
		return anim[cur]->GetR();
	}
	virtual float GetA() {
		return anim[cur]->GetA();
	}
	virtual const XMFLOAT3& GetPos() {
		return anim[cur]->GetPos();
	}
};
