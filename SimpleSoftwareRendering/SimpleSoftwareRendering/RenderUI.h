#pragma once

#include "RenderGeometry.h"

#include "UIGeometry.h"

void RenderRectangleOnScreen(const Vector3& start, const Vector3& end, const Colour& uiRectColour, const int& imageWidth, const int& imageHeight, std::vector<unsigned char>& imageData) {

	int startX = std::min((int)start.x, 0);
	int startY = std::min((int)start.y, 0);

	int endX = std::min((int)end.x, imageWidth);
	int endY = std::min((int)end.y, imageHeight);

	for (int y = startY; y < endY; y++)
	{
		for (int x = startX; x < endX; x++)
		{
			int curRedIndex = GetRedFlattenedImageDataSlotForPixel(Vector2Int{ x, y }, imageWidth);
			imageData[curRedIndex + 0] = uiRectColour.r;
			imageData[curRedIndex + 1] = uiRectColour.g;
			imageData[curRedIndex + 2] = uiRectColour.b;
			imageData[curRedIndex + 3] = uiRectColour.a;
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
		end = { (widthParent / 2) + (width / 2), uiRect.start.y, uiRect.start.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::TopRight) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		start = { widthParent - width, uiRect.start.y, uiRect.start.z };
		end = { widthParent, uiRect.start.y, uiRect.start.z };
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
		end = { (widthParent / 2) + (width / 2), (heightParent / 2) + (height / 2), uiRect.start.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::MiddleRight) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { widthParent - width, (heightParent / 2) - (height / 2), uiRect.start.z };
		end = { widthParent, (heightParent / 2) + (height / 2), uiRect.start.z };

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

void RenderUITree(const UI_Rect& rootUIRect, const int& imageWidth, const int& imageHeight, std::vector<unsigned char>& imageData) {



}