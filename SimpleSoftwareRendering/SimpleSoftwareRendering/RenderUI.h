#pragma once

#include "Input.h"

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
			int curRedIndex = GetRedFlattenedImageDataSlotForPixel(Vector2Int{ x, screenHeight - y - 1 }, imageWidth);
			//int curRedIndex = GetRedFlattenedImageDataSlotForPixel(Vector2Int{ x, y }, imageWidth);

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

		start = { (widthParent / 2) + (uiRect.start.x), uiRect.start.y, uiRect.start.z };
		end = { (widthParent / 2) + (uiRect.end.x), uiRect.end.y, uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::TopRight) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		start = { widthParent - uiRect.end.x, uiRect.start.y, uiRect.start.z };
		end = { widthParent - uiRect.start.x, uiRect.end.y, uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::MiddleLeft) {

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { uiRect.start.x, (heightParent / 2) + uiRect.start.y, uiRect.start.z};
		end = { uiRect.end.x, (heightParent / 2) + uiRect.end.y, uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::MiddleMiddle) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { (widthParent / 2) + (uiRect.start.x), (heightParent / 2) + (uiRect.start.y), uiRect.start.z };
		end = { (widthParent / 2) + (uiRect.end.x), (heightParent / 2) + (uiRect.end.y), uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::MiddleRight) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { widthParent - uiRect.start.x, (heightParent / 2) + (uiRect.start.y), uiRect.start.z };
		end = { widthParent - uiRect.end.x, (heightParent / 2) + (uiRect.end.y), uiRect.end.z };

	}
	else if (uiRect.anchorPosition == AnchorPosition::BottomLeft) {

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { uiRect.start.x, heightParent - uiRect.start.y, uiRect.start.z };
		end = { uiRect.end.x, heightParent - uiRect.end.y, uiRect.end.z };

	}
	else if (uiRect.anchorPosition == AnchorPosition::BottomMiddle) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { (widthParent / 2) + (uiRect.start.x), heightParent - uiRect.start.y, uiRect.start.z };
		end = { (widthParent / 2) + (uiRect.end.x), heightParent - uiRect.end.y, uiRect.end.z };
	}
	else if (uiRect.anchorPosition == AnchorPosition::BottomRight) {

		float width = GetUIRectWidth(uiRect);
		float widthParent = GetUIRectWidth(parentUIRect);

		float height = GetUIRectHeight(uiRect);
		float heightParent = GetUIRectHeight(parentUIRect);

		start = { widthParent - uiRect.start.x, heightParent - uiRect.start.y, uiRect.start.z };
		end = { widthParent - uiRect.end.x, heightParent - uiRect.end.y, uiRect.end.z };
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

void SetUIRectStatesToNotHovering(UI_Rect& rootUiRect) {

	rootUiRect.uiRectState = UI_RectState::OnNotHovering;

	for (int i = 0; i < rootUiRect.children.size(); i++)
	{
		SetUIRectStatesToNotHovering(UI_Rect::uiRects[rootUiRect.children[i]]);
	}

}

void UpdateUIRectStates(UI_Rect& uiRect, const float& mouseX, const float& mouseY) {
	SetUIRectState(uiRect, mouseX, mouseY);
	if (uiRect.uiRectState == UI_RectState::OnHovering) {
		uiRect.colour = ColourToVector4(uiRect.mouseHoverColour);
	}
	else if (uiRect.uiRectState == UI_RectState::OnHoverExit) {
		uiRect.colour = ColourToVector4(uiRect.normalColour);
	}
}

void UpdateUITreeStates(UI_Rect& uiRect, const float& mouseX, const float& mouseY) {

	//HighlightMouseHoveringOverRect(oldMouseX, oldMouseY, mouseX, mouseY);
	//HighlightMouseHoveringOverRect(mouseX, mouseY, mouseX, mouseY);

	UpdateUIRectStates(uiRect, mouseX, mouseY);
	//SetUIRectState(uiRect, mouseX, mouseY);
	if (uiRect.uiRectState != UI_RectState::OnNotHovering)
	{
		for (int i = 0; i < uiRect.children.size(); i++)
		{
			UpdateUITreeStates(UI_Rect::uiRects[uiRect.children[i]], mouseX, mouseY);
		}
	}
	else
	{
		for (int i = 0; i < uiRect.children.size(); i++)
		{
			SetUIRectStatesToNotHovering(UI_Rect::uiRects[uiRect.children[i]]);
		}
	}

	//for (int i = 0; i < UI_Rect::uiRects.size(); i++)
	//{
	//	SetUIRectState(UI_Rect::uiRects[i], mouseX, mouseY);
	//}
}

void MakeHoveringUIRectsFollowCursorMovement(const int& childIndex, const float& mouseDeltaX, const float& mouseDeltaY) {

	if (childIndex != 0) {
		AddDeltaToUIRectLocalPosition(childIndex, mouseDeltaX, mouseDeltaY);
	}
}

int curSelectedUIRectIndex = -1;

void HandleUIEvents(const float& mouseDeltaX, const float& mouseDeltaY) {

	while (!uiEvents.empty()) {
		if (uiEvents.top().state == UI_RectState::OnHovering && GetKeyHeld(MOUSE_BUTTON_LEFT)) {

			if (curSelectedUIRectIndex == -1 || UI_Rect::uiRects[uiEvents.top().uiRectId].index == curSelectedUIRectIndex) {
				curSelectedUIRectIndex = uiEvents.top().uiRectId;
				MakeHoveringUIRectsFollowCursorMovement(uiEvents.top().uiRectId, mouseDeltaX, mouseDeltaY);
			}
		}

		uiEvents.pop();
	}

}