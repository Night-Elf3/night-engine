#pragma once

#include "drawobjectinterface.h"
#include "objectinterface.h"
#include "object.hpp"

class Objects {
	vector<DrawObjectInterface*> objects;
	vector<int> active;

	int hover_id;

	inline float triangleSqr(const XMFLOAT3& p1, const XMFLOAT3& p2, const XMFLOAT3& p3) {
		#define LEN(X1, Y1, X2, Y2) sqrt(pow(X2 - X1, 2.) + pow(Y2 - Y1, 2.))
		float a = LEN(p1.x, p1.z, p2.x, p2.z),
			b = LEN(p2.x, p2.z, p3.x, p3.z),
			c = LEN(p3.x, p3.z, p1.x, p1.z),
			p = (a + b + c) / 2.f;

		return sqrt(p*(p - a)*(p - b)*(p - c));
	}
public:
	Objects() {
		hover_id = -1;
	};

	int push(DrawObjectInterface* a) {
		objects.push_back(a);

		return objects.size() - 1;
	}

	void draw() {
		for (int i = 0; i < objects.size(); ++i) {
			objects[i]->whileGotoXY();
			objects[i]->Draw(active.end() != find(active.begin(), active.end(), i) || i == hover_id);
		}
	}
	void draw(int i, bool active = false) {
		if (0 <= i && i < objects.size())
			objects[i]->Draw(active);
	}

	void gotoXY(int i, XMFLOAT3& pos) {
		if (0 <= i && i < objects.size()) {
			objects[i]->GotoXY(pos);
		}
	}
	void gotoXY(XMFLOAT3& pos) {
		XMFLOAT3 pos_r = {0.f, 0.f, 0.f},
			pos_v = {};

		for each (int i in active) {
			pos_r.x += objects[i]->GetPos().x / (double)active.size();
			pos_r.y += objects[i]->GetPos().y / (double)active.size();
			pos_r.z += objects[i]->GetPos().z / (double)active.size();
		}
		pos_v = {
			pos.x - pos_r.x, 
			pos.y - pos_r.y, 
			pos.z - pos_r.z
		};

		for each (int i in active) {
			XMFLOAT3 pos_t = {
				objects[i]->GetPos().x + pos_v.x,
				objects[i]->GetPos().y + pos_v.y,
				objects[i]->GetPos().z + pos_v.z,
			};

			objects[i]->GotoXY(pos_t);
		}
	}

	void setHoverId(int i) {
		if (0 <= i && i < objects.size()) {
			hover_id = i;
		}
	}

	int getHoverId(XMFLOAT3& pos) {
		hover_id = -1;

		for (int i = 0; hover_id < 0 && i < objects.size(); ++i) {
			if (sqrt(pow(objects[i]->GetPos().x - pos.x, 2.) + pow(objects[i]->GetPos().z - pos.z, 2.)) <= objects[i]->GetR()) {
				hover_id = i;
				break;
			}
		}

		return hover_id;
	}

	bool pickObj(XMFLOAT3& pos, bool shift = false) {
		if (!shift) {
			active.clear();

			if (hover_id >= 0)
				active.push_back(hover_id);

			return active.empty();
		}

		for (int i = 0; i < objects.size(); ++i) {
			if (sqrt(pow(objects[i]->GetPos().x - pos.x, 2.) + pow(objects[i]->GetPos().z - pos.z, 2.)) <= objects[i]->GetR()) {
				active.push_back(i);
				break;
			}
		}

		return active.empty();
	}
	bool pickObj(const XMFLOAT3& pt_lb, const XMFLOAT3& pt_lt, const XMFLOAT3& pt_rt, const XMFLOAT3& pt_rb, bool shift = false) {
		hover_id = -1;
		if (!shift) {
			active.clear();
		}

		//float s = fabs(((pt_rb.x - pt_lb.x) + (pt_rb.x - pt_lb.x)) * (pt_lt.z - pt_lb.z) / 2.f);
		float s = triangleSqr(pt_lb, pt_lt, pt_rt) + triangleSqr(pt_lb, pt_rb, pt_rt);

		if (s < 1.f)
			return false;

		for (int i = 0; i < objects.size(); ++i) {
			float _s = s;

			_s -= triangleSqr(pt_lb, pt_lt, objects[i]->GetPos());
			_s -= triangleSqr(pt_lt, pt_rt, objects[i]->GetPos());
			_s -= triangleSqr(pt_rt, pt_rb, objects[i]->GetPos());
			_s -= triangleSqr(pt_rb, pt_lb, objects[i]->GetPos());

			if (fabs(_s) < objects[i]->GetR()) {
				active.push_back(i);
			}
		}

		return true;
	}
};