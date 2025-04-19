#pragma once

#include <stack> // Using stack for event queue instead of Queue because I want children's events to be cleared up first as traversal for state checking and changing happens from root to child.

#include "Input.h"
#include "UIGeometry.h"

struct UIEventsData {
	int uiRectId;
	UI_RectState state;
};

std::stack<UIEventsData> uiEvents;

// Adds a copy! Not the one provided!
void AddUIRectAsChildToUIRect(UI_Rect& child, const int& parentIndex) {

	child.parentIndex = parentIndex;

	int childIndex = UI_Rect::uiRects.size();
	UI_Rect::uiRects.push_back(child);
	UI_Rect::uiRects[parentIndex].children.push_back(childIndex);
}

bool PointLiesInsideUIRect(const UI_Rect& uiRect, const Vector2& point) {

	//std::cout << "Checking point in rec := " << uiRect.index << std::endl;

	return point.x >= uiRect.worldStartPos.x &&
		point.x <= uiRect.worldEndPos.x &&
		point.y >= uiRect.worldStartPos.y &&
		point.y <= uiRect.worldEndPos.y;

}

void SetUIRectState(UI_Rect& uiRect, const float& mouseX, const float& mouseY) {

	if (PointLiesInsideUIRect(uiRect, { mouseX, mouseY })) {

		if (uiRect.uiRectState == UI_RectState::OnNotHovering)
		{
			//std::cout << uiRect.index << " entered hover." << std::endl;
			uiRect.uiRectState = UI_RectState::OnHoverEnter;
		}
		else if (uiRect.uiRectState == UI_RectState::OnHoverEnter) {

			//std::cout << uiRect.index << " hovering." << std::endl;
			uiRect.uiRectState = UI_RectState::OnHovering;
		}
	}
	else {
		if (uiRect.uiRectState == UI_RectState::OnHoverEnter || uiRect.uiRectState == UI_RectState::OnHovering) {

			//std::cout << uiRect.index << " hover exit." << std::endl;
			uiRect.uiRectState = UI_RectState::OnHoverExit;
		}
		else {

			//std::cout << uiRect.index << " not hovering." << std::endl;
			uiRect.uiRectState = UI_RectState::OnNotHovering;
		}
	}

	uiEvents.push({ uiRect.index, uiRect.uiRectState });

}

bool AddDeltaToUIRectLocalPosition(const int& uiRectIndex, const float& deltaX, const float& deltaY) {

	UI_Rect& uiRect = UI_Rect::uiRects[uiRectIndex];
	UI_Rect& parentUIRect = UI_Rect::uiRects[uiRect.parentIndex];

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

		start = { uiRect.start.x, (heightParent / 2) + uiRect.start.y, uiRect.start.z };
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