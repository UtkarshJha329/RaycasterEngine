#pragma once

#include "UISimulation.h"

void RenderRectangleOnScreen(const Vector3& start, const Vector3& end, const Vector4& uiRectColour, const int& imageWidth, const int& imageHeight, std::vector<unsigned char>& imageData) {

	int startX = std::max((int)start.x, 0);
	int startY = std::max((int)start.y, 0);

	int endX = std::min((int)end.x, imageWidth);
	int endY = std::min((int)end.y, imageHeight);

	for (int y = startY; y < endY; y++)
	{
		for (int x = startX; x < endX; x++)
		{
			int curRedIndex = GetRedFlattenedImageDataSlotForPixel(Vector2Int{ x, screenHeight - y - 1 }, imageWidth);
			//int curRedIndex = GetRedFlattenedImageDataSlotForPixel(Vector2Int{ x, y }, imageWidth);

			imageData[curRedIndex + 0] = uiRectColour.x;
			imageData[curRedIndex + 1] = uiRectColour.y;
			imageData[curRedIndex + 2] = uiRectColour.z;
			imageData[curRedIndex + 3] = uiRectColour.w;
		}
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