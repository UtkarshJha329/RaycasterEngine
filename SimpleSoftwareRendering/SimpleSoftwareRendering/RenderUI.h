#pragma once

#include "RenderGeometry.h"

#include "UIGeometry.h"

void RenderRectangleOnScreen(const Vector3& start, const Vector3& end, const Vector4& uiRectColour, const int& imageWidth, const int& imageHeight, std::vector<unsigned char>& imageData) {

	int startX = std::max((int)start.x, 0);
	int startY = std::max((int)start.y, 0);

	int endX = std::min((int)end.x, imageWidth);
	int endY = std::min((int)end.y, imageHeight);

	for (int y = startY; y < endY; y++)
	{
		for (int x = startX; x < endX; x++)
		{
			int curRedIndex = GetRedFlattenedImageDataSlotForPixel(Vector2Int{ x, screenHeight - y }, imageWidth);
			imageData[curRedIndex + 0] = uiRectColour.x;
			imageData[curRedIndex + 1] = uiRectColour.y;
			imageData[curRedIndex + 2] = uiRectColour.z;
			imageData[curRedIndex + 3] = uiRectColour.w;
		}
	}
}

void TransformUIPositionsToParentSpaceBasedOnAnchor(Vector3& start, Vector3& end, const UI_Rect& uiRect, const UI_Rect& parentUIRect) {

	if (uiRect.anchorPosition == AnchorPosition::TopLeft) {

		start = uiRect.start;
		end = uiRect.end;
	}
	else if (uiRect.anchorPosition == AnchorPosition::TopMiddle) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		start = { (widthParent / 2) - (width / 2), uiRect.start.y, uiRect.start.z };
		end = { (widthParent / 2) + (width / 2), uiRect.end.y, uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::TopRight) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		start = { widthParent - width, uiRect.start.y, uiRect.start.z };
		end = { widthParent, uiRect.end.y, uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::MiddleLeft) {

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { uiRect.start.x, (heightParent / 2) - (height / 2), uiRect.start.z};
		end = { uiRect.end.x, (heightParent / 2) + (height / 2), uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::MiddleMiddle) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { (widthParent / 2) - (width / 2), (heightParent / 2) - (height / 2), uiRect.start.z };
		end = { (widthParent / 2) + (width / 2), (heightParent / 2) + (height / 2), uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::MiddleRight) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { widthParent - width, (heightParent / 2) - (height / 2), uiRect.start.z };
		end = { widthParent, (heightParent / 2) + (height / 2), uiRect.end.z };

	}
	else if (uiRect.anchorPosition == AnchorPosition::BottomLeft) {

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { uiRect.start.x, heightParent - height, uiRect.start.z };
		end = { uiRect.end.x, heightParent, uiRect.end.z };

	}
	else if (uiRect.anchorPosition == AnchorPosition::BottomMiddle) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { (widthParent / 2) - (width / 2), heightParent - height, uiRect.start.z };
		end = { (widthParent / 2) + (width / 2), heightParent, uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::BottomRight) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { widthParent - width, heightParent - height, uiRect.start.z };
		end = { widthParent, heightParent, uiRect.end.z };
	}
}

void RenderUIRect(UI_Rect& uiRect, const UI_Rect& parentUIRect, const int& imageWidth, const int& imageHeight, std::vector<unsigned char>& imageData) {

	Vector3 start = uiRect.start;
	Vector3 end = uiRect.end;

	TransformUIPositionsToParentSpaceBasedOnAnchor(start, end, uiRect, parentUIRect);

	uiRect.worldStartPos = parentUIRect.worldStartPos + start;
	uiRect.worldEndPos = parentUIRect.worldStartPos + end;

	RenderRectangleOnScreen(uiRect.worldStartPos, uiRect.worldEndPos, uiRect.colour, imageWidth, imageHeight, imageData);

	for (int i = 0; i < uiRect.children.size(); i++)
	{
		RenderUIRect(UI_Rect::uiRects[uiRect.children[i]], uiRect, imageWidth, imageHeight, imageData);
	}
}

void RenderUIRoot(UI_Rect& rootUIRect, const int& imageWidth, const int& imageHeight, std::vector<unsigned char>& imageData) {

	Vector3 start = rootUIRect.start;
	Vector3 end = rootUIRect.end;

	UI_Rect fakeScreenRect;
	fakeScreenRect.anchorPosition = TopLeft;
	fakeScreenRect.start = { 0.0f, 0.0f, 0.0f };
	fakeScreenRect.end = { screenWidth, screenHeight, 0.0f };

	TransformUIPositionsToParentSpaceBasedOnAnchor(start, end, rootUIRect, fakeScreenRect);

	rootUIRect.worldStartPos = start;
	rootUIRect.worldEndPos = end;

	RenderRectangleOnScreen(start, end, rootUIRect.colour, imageWidth, imageHeight, imageData);

}

void RenderUITree(UI_Rect& rootUIRect, const int& imageWidth, const int& imageHeight, std::vector<unsigned char>& imageData) {

	RenderUIRoot(rootUIRect, imageWidth, imageHeight, imageData);

	for (int i = 0; i < rootUIRect.children.size(); i++)
	{
		RenderUIRect(UI_Rect::uiRects[rootUIRect.children[i]], rootUIRect, imageWidth, imageHeight, imageData);
	}
}

void AddUITreeToCollisionGrid() {

	for (int i = 0; i < UI_Rect::uiRects.size(); i++)
	{
		//std::cout << UI_Rect::uiRects.size() << std::endl;
		AddUIRectToCollisionGrid(UI_Rect::uiRects[i]);
	}
}

void HighlightMouseHoveringOverRect(const float& mouseX, const float& mouseY) {

	if (mouseX >= 0 && mouseX < screenWidth && mouseY >= 0 && mouseY <= screenHeight) {

		//std::cout << "Valid Mouse Position." << std::endl;

		Vector2Int collisionGridCoords = { (int)mouseX / (int)collisionGridCellSize.x, (int)mouseY / (int)collisionGridCellSize.y };

		int collisionGridIndex = GetUICollisionGridIndex(collisionGridCoords.x, collisionGridCoords.y, numGridsOnScreen.x);
		if (collisionGridIndex >= 0 && collisionGridIndex < UI_CollisionGrid::uiRectIndexInCollisionGrid.size()) {

			//std::cout << "Found Valid Grid Cell Position." << collisionGridIndex << ", " << UI_CollisionGrid::uiRectIndexInCollisionGrid[collisionGridIndex].size() << std::endl;
			//std::cout << "Found Valid Grid Cell Position := " << collisionGridIndex << std::endl;

			for (int i = 0; i < UI_CollisionGrid::uiRectIndexInCollisionGrid[collisionGridIndex].size(); i++)
			{
				//std::cout << "Going to check uiRectIndex := " << UI_CollisionGrid::uiRectIndexInCollisionGrid[collisionGridIndex][i] << std::endl;
				if (PointLiesInsideUIRect(UI_Rect::uiRects[UI_CollisionGrid::uiRectIndexInCollisionGrid[collisionGridIndex][i]], { mouseX, screenHeight - mouseY })) {
					std::cout << "Point lies inside rect := ";
					std::cout << UI_Rect::uiRects[UI_CollisionGrid::uiRectIndexInCollisionGrid[collisionGridIndex][i]].index << std::endl;
				}
			}
		}
	}
}