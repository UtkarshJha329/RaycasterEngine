#pragma once

#include <vector>

#include "WorldConstants.h"
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

enum UI_RectState {
	OnHoverEnter,
	OnHovering,
	OnHoverExit,
	OnNotHovering
};

struct UI_Rect {

	int index = -1;

	Vector3 start;
	Vector3 end;

	Vector4 colour;

	AnchorPosition anchorPosition = TopLeft;

	UI_RectState uiRectState = UI_RectState::OnNotHovering;

	Vector3 worldStartPos;
	Vector3 worldEndPos;

	std::vector<unsigned int> children;

	static std::vector<UI_Rect> uiRects;
};

bool PointLiesInsideUIRect(const UI_Rect& uiRect, const Vector2& point) {

	//std::cout << "Checking point in rec := " << uiRect.index << std::endl;

	return point.x >= uiRect.worldStartPos.x &&
		point.x <= uiRect.worldEndPos.x &&
		point.y >= uiRect.worldStartPos.y &&
		point.y <= uiRect.worldEndPos.y;

}

float GetUIRectWidth(const UI_Rect& uiRect) {
	return uiRect.end.x - uiRect.start.x;
}

float GetUIRectHeight(const UI_Rect& uiRect) {
	return uiRect.end.y - uiRect.start.y;
}

void SetUIRectState(UI_Rect& uiRect, const float& mouseX, const float& mouseY) {

	if (PointLiesInsideUIRect(uiRect, { mouseX, mouseY })) {

		if(uiRect.uiRectState == UI_RectState::OnNotHovering)
		{
			std::cout << uiRect.index << " entered hover." << std::endl;
			uiRect.uiRectState = UI_RectState::OnHoverEnter;
		}
		else if (uiRect.uiRectState == UI_RectState::OnHoverEnter) {

			std::cout << uiRect.index << " hovering." << std::endl;
			uiRect.uiRectState = UI_RectState::OnHovering;
		}
	}
	else {
		if (uiRect.uiRectState == UI_RectState::OnHoverEnter || uiRect.uiRectState == UI_RectState::OnHovering) {

			std::cout << uiRect.index << " hover exit." << std::endl;
			uiRect.uiRectState = UI_RectState::OnHoverExit;
		}
		else {

			//std::cout << uiRect.index << " not hovering." << std::endl;
			uiRect.uiRectState = UI_RectState::OnNotHovering;
		}
	}

}

struct UI_CollisionGrid {
	static std::vector<std::vector<unsigned int>> uiRectIndexInCollisionGrid;
};

int GetUICollisionGridIndex(const int& xCoord, const int& yCoord, const int& collisionGridSizeX) {
	return xCoord + (yCoord * collisionGridSizeX);
}

void AddUIRectToCollisionGrid(const UI_Rect& uiRect) {

	//std::cout << "(" << uiRect.worldStartPos.x << ", " << uiRect.worldEndPos.x << ") - (" << uiRect.worldStartPos.y << ", " << uiRect.worldEndPos.y << ")" << std::endl;

	for (int y = uiRect.worldStartPos.y; y <= uiRect.worldEndPos.y; y += collisionGridCellSize.y)
	{
		for (int x = uiRect.worldStartPos.x; x <= uiRect.worldEndPos.x; x += collisionGridCellSize.x)
		{
			Vector2Int collisionGridCoords = { x / (int)collisionGridCellSize.x , (int)(y) / (int)collisionGridCellSize.y };
			//Vector2Int collisionGridCoords = { x / (int)collisionGridCellSize.x , (int)(y) / (int)collisionGridCellSize.y };
			int collisionGridIndex = GetUICollisionGridIndex(collisionGridCoords.x, collisionGridCoords.y, numGridsOnScreen.x);

			//std::cout << "Added in : " << collisionGridIndex << std::endl;

			UI_CollisionGrid::uiRectIndexInCollisionGrid[collisionGridIndex].push_back(uiRect.index);
		}
	}
}