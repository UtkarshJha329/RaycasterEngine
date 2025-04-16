#pragma once

#include <vector>

#include "Geometry.h"

enum AnchorPosition {
	TopLeft,
	TopMiddle,
	TopRight,
	MiddleLeft,
	MiddleMiddle,
	MiddleRight,
	BottomLeft,
	BottomMiddle,
	BottomRight
};

struct UI_Rect {

	Vector3 start;
	Vector3 end;

	Vector4 colour;

	AnchorPosition anchorPosition = TopLeft;

	std::vector<UI_Rect> children;
};

float GetUIRectWidth(const UI_Rect& uiRect) {
	return uiRect.end.x - uiRect.start.x;
}

float GetUIRectHeight(const UI_Rect& uiRect) {
	return uiRect.end.y - uiRect.start.y;
}