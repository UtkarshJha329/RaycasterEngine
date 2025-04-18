#pragma once

#include <stack>
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
	OnNotHovering,

	OnClick,
	OnClickHeld,
	OnClickRelease
};

struct UI_Rect {

	int index = -1;

	Vector3 start;
	Vector3 end;

	Vector4 colour;

	AnchorPosition anchorPosition = TopLeft;

	UI_RectState uiRectState = UI_RectState::OnNotHovering;

	int parentIndex;

	Vector3 worldStartPos;
	Vector3 worldEndPos;

	Colour normalColour;
	Colour mouseHoverColour;

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
	return abs(uiRect.end.x - uiRect.start.x);
}

float GetUIRectHeight(const UI_Rect& uiRect) {
	return abs(uiRect.end.y - uiRect.start.y);
}

bool AddDeltaToUIRectLocalPosition(const int& uiRectIndex, const float& deltaX, const float& deltaY) {

	UI_Rect& uiRect = UI_Rect::uiRects[uiRectIndex];
	UI_Rect& parentUIRect = UI_Rect::uiRects[uiRect.parentIndex];

	//Vector3 DeltaX = Vector3{ deltaX, 0.0f, 0.0f };
	//Vector3 DeltaY = Vector3{ 0.0f, deltaY, 0.0f };

	//Vector3 modifiedWorldStartX = uiRect.worldStartPos + DeltaX;
	//Vector3 modifiedWorldEndX = uiRect.worldEndPos + DeltaX;

	//bool modifiedXLiesInside = PointLiesInsideUIRect(parentUIRect, modifiedWorldStartX) && PointLiesInsideUIRect(parentUIRect, modifiedWorldEndX);

	//Vector3 modifiedWorldStartY = uiRect.worldStartPos + DeltaY;
	//Vector3 modifiedWorldEndY = uiRect.worldEndPos + DeltaY;

	//bool modifiedYLiesInside = PointLiesInsideUIRect(parentUIRect, modifiedWorldStartY) && PointLiesInsideUIRect(parentUIRect, modifiedWorldEndY);

	Vector3 delta = Vector3{ deltaX, deltaY, 0.0f };

	Vector3 modifiedWorldStart = uiRect.worldStartPos + delta;
	Vector3 modifiedWorldEnd = uiRect.worldEndPos + delta;

	bool modifiedStartAndEndLieInside = PointLiesInsideUIRect(parentUIRect, modifiedWorldStart) && PointLiesInsideUIRect(parentUIRect, modifiedWorldEnd);

	if (modifiedStartAndEndLieInside) {

		uiRect.worldStartPos += delta;
		uiRect.worldEndPos += delta;

		if (uiRect.anchorPosition == AnchorPosition::TopLeft) {
			uiRect.start += delta;
			uiRect.end += delta;
		}
		if (uiRect.anchorPosition == AnchorPosition::TopMiddle) {
			uiRect.start += delta;
			uiRect.end += delta;
		}
		if (uiRect.anchorPosition == AnchorPosition::TopRight) {
			delta.x *= -1.0f;
			uiRect.start += delta;
			uiRect.end += delta;
		}
		if (uiRect.anchorPosition == AnchorPosition::MiddleLeft) {
			uiRect.start += delta;
			uiRect.end += delta;
		}
		if (uiRect.anchorPosition == AnchorPosition::MiddleMiddle) {
			uiRect.start += delta;
			uiRect.end += delta;
		}
		if (uiRect.anchorPosition == AnchorPosition::MiddleRight) {
			delta.x *= -1.0f;
			uiRect.start += delta;
			uiRect.end += delta;
		}
		if (uiRect.anchorPosition == AnchorPosition::BottomLeft) {
			delta.y *= -1.0f;
			uiRect.start += delta;
			uiRect.end += delta;
		}
		if (uiRect.anchorPosition == AnchorPosition::BottomMiddle) {
			delta.y *= -1.0f;
			uiRect.start += delta;
			uiRect.end += delta;
		}
		if (uiRect.anchorPosition == AnchorPosition::BottomRight) {
			delta *= -1.0f;
			uiRect.start += delta;
			uiRect.end += delta;
		}

		return true;
	}

	return false;
}

struct UIEventsData {
	int uiRectId;
	UI_RectState state;
};

std::stack<UIEventsData> uiEvents;


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

	uiEvents.push({ uiRect.index, uiRect.uiRectState });

}

// Adds a copy! Not the one provided!
void AddUIRectAsChildToUIRect(UI_Rect& child, const int& parentIndex) {

	child.parentIndex = parentIndex;

	int childIndex = UI_Rect::uiRects.size();
	UI_Rect::uiRects.push_back(child);
	UI_Rect::uiRects[parentIndex].children.push_back(childIndex);
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