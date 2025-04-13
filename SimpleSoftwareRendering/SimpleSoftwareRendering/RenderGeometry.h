#pragma once

#include <queue>

#include "Instrumentor.h"

#include "DebugUtilities.h"

#include "WorldConstants.h"
#include "Model.h"

float LerpFloat(const float& a, const float& b, const float& t) {
	return ((1 - t) * a) + b * t;
}

Vector2 LerpVector2(const Vector2& a, const Vector2& b, const float& t) {
	return ((1 - t) * a) + b * t;
}

Vector3 LerpVector3(const Vector3& a, const Vector3& b, const float& t) {
	return ((1 - t) * a) + b * t;
}

Vector4 LerpVector4(const Vector4& a, const Vector4& b, const float& t) {
	return ((1 - t) * a) + b * t;
}

int GetRedFlattenedImageDataSlotForPixel(Vector2Int pixelPos, int imageWidth) {
	return (pixelPos.x + pixelPos.y * imageWidth) * 4;
}

Vector4 ColourToVector4(const Colour& curColour) {
	return Vector4{ curColour.r, curColour.g, curColour.b, curColour.a };
}

Colour Vector4ToColour(const Vector4& curColour) {
	return Colour{ (unsigned char)curColour.x, (unsigned char)curColour.y, (unsigned char)curColour.z, (unsigned char)curColour.w };
}

int GetFlattenedImageDataSlotForDepthData(Vector2Int pixelPos, int imageWidth) {
	return (pixelPos.x + pixelPos.y * imageWidth);
}

void FillSubPixels(std::vector<unsigned char>& imageData, int imageWidth, Vector2Int pixelCentre, int halfSizeMinusOne, Colour colourToFillWith) {
	
	for (int x = -halfSizeMinusOne; x <= halfSizeMinusOne; x++)
	{
		for (int y = -halfSizeMinusOne; y <= halfSizeMinusOne; y++)
		{
			int index = GetRedFlattenedImageDataSlotForPixel(Vector2Int{ pixelCentre.x + x, pixelCentre.y + y }, imageWidth);
			if (index >= 0 && (index + 3) < imageData.size()) {

				imageData[index + 0] = colourToFillWith.r;
				imageData[index + 1] = colourToFillWith.g;
				imageData[index + 2] = colourToFillWith.b;
				imageData[index + 3] = colourToFillWith.a;
			}
		}
	}
}

void DrawLineSegmentOnScreen(std::vector<unsigned char>& imageData, int imageWidth, Vector2Int a, Vector2Int b, int lineThickness, Colour lineColour) {

	int x0 = a.x;
	int y0 = a.y;
	int x1 = b.x;
	int y1 = b.y;

	bool steep = false;

	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	float dx = (float)(x1 - x0);
	float dy = (float)(y1 - y0);

	if (steep)
	{
		for (int x = x0; x <= x1; x++) {

			float t = (x - x0) / dx;
			int y = y0 + dy * t;

			FillSubPixels(imageData, imageWidth, Vector2Int{ y, x }, lineThickness, lineColour);
		}
	}
	else
	{
		for (int x = x0; x <= x1; x++) {

			float t = (x - x0) / dx;
			int y = y0 + dy * t;

			FillSubPixels(imageData, imageWidth, Vector2Int{ x, y }, lineThickness, lineColour);
		}
	}
}

void DrawCurrentPixelWithInterpValues(const float& imageWidth,
	const float& x, const float& y,
	const float& lightDotTriangleNormal,
	const Vector3& deltaY, const Vector3& deltaX, const Vector3& deltaK,
	const float& areaOfTriangle,
	const Vector3& invDepth,
	const Vector3& invW, const Vector3& texWs,
	const Triangle& curTriangle,
	const float& colourTextureMixFactor,
	const Colour& fixedColour, bool drawFixedColour,
	const Texture* curTex, std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData) {

	//std::cout << "Stuck 4" << std::endl;

	Vector2Int curPoint = Vector2Int{ round(x), round(y) };
	Vector2 curPointFloat = Vector2{ x + 0.5f, y + 0.5f };

	float crossAFloat = ((curPointFloat.x * deltaY.x) - (curPointFloat.y * deltaX.x)) + deltaK.x;
	float crossBFloat = ((curPointFloat.x * deltaY.y) - (curPointFloat.y * deltaX.y)) + deltaK.y;
	float crossCFloat = ((curPointFloat.x * deltaY.z) - (curPointFloat.y * deltaX.z)) + deltaK.z;

	float alpha = crossAFloat / areaOfTriangle;
	float beta = crossBFloat / areaOfTriangle;
	float gamma = crossCFloat / areaOfTriangle;

	//std::cout << "alpha := " << alpha << ", beta := " << beta << ", gamma := " << gamma << std::endl;

	float depth = 1.0f / ((alpha * invDepth.x) + (beta * invDepth.y) + (gamma * invDepth.z));

	float w = 1.0f / ((alpha * invW.x) + (beta * invW.y) + (gamma * invW.z));
	float texW = 1.0f / ((alpha * texWs.x) + (beta * texWs.y) + (gamma * texWs.z));

	Vector2 texCoord = (alpha * curTriangle.a.texCoord) + (beta * curTriangle.b.texCoord) + (gamma * curTriangle.c.texCoord);
	texCoord *= texW;

	Vector3 normal = (alpha * curTriangle.a.normal) + (beta * curTriangle.b.normal) + (gamma * curTriangle.c.texCoord);
	normal *= texW;

	normal = glm::normalize(normal);
	Vector3 lightDir = { 1.0f, 1.0f, 1.0f };
	lightDir = glm::normalize(lightDir);

	float normalDotLightDir = glm::dot(normal, lightDir);

	//Vector4 curColour = (alpha * ColourToVector4(curTriangle.a.colour) * invW.x) + (beta * ColourToVector4(curTriangle.b.colour) * invW.y) + (gamma * ColourToVector4(curTriangle.c.colour) * invW.z);
	Vector4 curColour = (alpha * curTriangle.a.colour) + (beta * curTriangle.b.colour) + (gamma * curTriangle.c.colour);
	curColour *= texW;

	int depthDataIndex = GetFlattenedImageDataSlotForDepthData(curPoint, imageWidth);

	if (depthDataIndex >= 0 && depthDataIndex < imageDepthData.size() && imageDepthData[depthDataIndex] < depth)
	{
		//std::cout << "Pixel has passed depth test." << std::endl;
		{
			//std::cout << "Ready to draw pixel." << std::endl;
			int index = GetRedFlattenedImageDataSlotForPixel(curPoint, imageWidth);
			if (index >= 0 && (index + 3) < imageData.size())
			{
				//curColour = { 255, 255, 255, 255 };

				//std::cout << "Drawing pixel." << std::endl;

				//Colour texelColour = GetColourFromTexCoord(*curTex, curTexCoord);
				Colour texelColour = colourTextureMixFactor < 1.0f ? GetColourFromTexCoord(*curTex, texCoord) : colour_black;
				float r = ((1.0f - colourTextureMixFactor) * texelColour.r + (colourTextureMixFactor * curColour.r));
				float g = ((1.0f - colourTextureMixFactor) * texelColour.g + (colourTextureMixFactor * curColour.g));
				float b = ((1.0f - colourTextureMixFactor) * texelColour.b + (colourTextureMixFactor * curColour.b));

				//r = texCoord.x * 255;
				//g = texCoord.y * 255;
				//b = 0;


				imageData[index + 0] = r * lightDotTriangleNormal;
				imageData[index + 1] = g * lightDotTriangleNormal;
				imageData[index + 2] = b * lightDotTriangleNormal;
				imageData[index + 3] = 255;

				if (drawFixedColour) {
					imageData[index + 0] = fixedColour.r;
					imageData[index + 1] = fixedColour.g;
					imageData[index + 2] = fixedColour.b;
					imageData[index + 3] = 255;
				}
			}

			imageDepthData[depthDataIndex] = depth;
		}
	}

}

void DrawLineSegmentOnScreenWithInterpolatedValues(const float& imageWidth,
										Vector2 start, Vector2 end,
										const float& lightDotTriangleNormal,
										const Vector3& deltaY, const Vector3& deltaX, const Vector3& deltaK,
										const float& areaOfTriangle,
										const Vector3& invDepth,
										const Vector3& invW, const Vector3& texWs,
										const Triangle& curTriangle,
										const float& colourTextureMixFactor,
										const Colour& fixedColour, bool drawFixedColour,
										const Texture* curTex, std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData
) {


	float x0 = start.x;
	float y0 = start.y;
	float x1 = end.x;
	float y1 = end.y;

	bool steep = false;

	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	float dx = (float)(x1 - x0);
	float dy = (float)(y1 - y0);

	//x0 = floor(x0);
	//x1 = ceil(x1);
	//y0 = floor(y0);
	//y1 = ceil(y1);

	if (steep)
	{
		for (int x = x0; x <= x1; x++) {

			float t = (x - x0) / dx;
			int y = y0 + dy * t;

			Vector2 curPoint = Vector2{ y, x };

			DrawCurrentPixelWithInterpValues(imageWidth, curPoint.x, curPoint.y, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texWs, curTriangle, colourTextureMixFactor, fixedColour, false, curTex, imageData, imageDepthData);

		}
	}
	else
	{
		for (int x = x0; x <= x1; x++) {

			float t = (x - x0) / dx;
			int y = y0 + dy * t;

			Vector2 curPoint = Vector2{ x, y };

			DrawCurrentPixelWithInterpValues(imageWidth, curPoint.x, curPoint.y, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texWs, curTriangle, colourTextureMixFactor, fixedColour, false, curTex, imageData, imageDepthData);

		}
	}
}

void GeneratePointsAlongLineSegment(Vector2Int start, Vector2Int end, std::vector<Vector2Int>& pointsAlongLineSegment) {

	int x0 = start.x;
	int y0 = start.y;
	int x1 = end.x;
	int y1 = end.y;

	if (y0 - y1 == 0 && x0 - x1 == 0) {
		return;
	}

	bool steep = false;

	if (std::abs(x0 - x1) < std::abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	float dx = (float)(x1 - x0) + 0.001f;
	float dy = (float)(y1 - y0);

	if (steep)
	{
		for (int x = x0; x <= x1; x++) {

			float t = (x - x0) / dx;
			int y = y0 + dy * t;

			pointsAlongLineSegment.push_back(Vector2Int{ y, x });
		}
	}
	else
	{
		for (int x = x0; x <= x1; x++) {

			float t = (x - x0) / dx;
			int y = y0 + dy * t;

			pointsAlongLineSegment.push_back(Vector2Int{ x, y });
		}
	}
}

// Returns a floating point slope generated between the given x0 and x1 for the required number of steps.
float GenerateSlopeWithFixedSteps(const int& x0, const int& x1, const int& numSteps) {
	return (x1 - x0) / numSteps;
}

bool IsLineTopOrLeft(Vector2Int start, Vector2Int end) {

	Vector2Int edge = end - start;

	bool isTopEdge = edge.y == 0 && edge.x > 0;
	bool isLeftEdge = edge.y < 0;

	return isTopEdge || isLeftEdge;
}

void DrawCurrentTrianglePixel(const float& imageWidth,
	const float& x, const float& y,
	const float& lightDotTriangleNormal,
	const Vector3& deltaY, const Vector3& deltaX, const Vector3& deltaK,
	const float& areaOfTriangle,
	const Vector3& invDepth,
	const Vector3& invW, const Vector3& texWs,
	const Triangle& curTriangle,
	const float& colourTextureMixFactor,
	const Colour& fixedColour, bool drawFixedColour,
	const Texture* curTex, std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData) {

	//std::cout << "Stuck 4" << std::endl;

	Vector2Int curPoint = Vector2Int{ x, y };
	Vector2 curPointFloat = Vector2{ curPoint.x + 0.5f, curPoint.y + 0.5f};

	float crossAFloat = ((curPointFloat.x * deltaY.x) - (curPointFloat.y * deltaX.x)) + deltaK.x;
	float crossBFloat = ((curPointFloat.x * deltaY.y) - (curPointFloat.y * deltaX.y)) + deltaK.y;
	float crossCFloat = ((curPointFloat.x * deltaY.z) - (curPointFloat.y * deltaX.z)) + deltaK.z;	

	float alpha = crossAFloat / areaOfTriangle;
	float beta = crossBFloat / areaOfTriangle;
	float gamma = crossCFloat / areaOfTriangle;

	//std::cout << "alpha := " << alpha << ", beta := " << beta << ", gamma := " << gamma << std::endl;

	float depth = 1.0f / ((alpha * invDepth.x) + (beta * invDepth.y) + (gamma * invDepth.z));

	float w = 1.0f / ((alpha * invW.x) + (beta * invW.y) + (gamma * invW.z));
	float texW = 1.0f / ((alpha * texWs.x) + (beta * texWs.y) + (gamma * texWs.z));

	Vector2 texCoord = (alpha * curTriangle.a.texCoord) + (beta * curTriangle.b.texCoord) + (gamma * curTriangle.c.texCoord);
	texCoord *= texW;

	Vector3 normal = (alpha * curTriangle.a.normal) + (beta * curTriangle.b.normal) + (gamma * curTriangle.c.texCoord);
	normal *= texW;

	//std::cout << curTriangle.a.normal.x << ", " << curTriangle.a.normal.y << ", " << curTriangle.a.normal.z << std::endl;

	//std::cout << normal.x << ", " << normal.y << ", " << normal.z << std::endl;

	Vector4 curColour = (alpha * curTriangle.a.colour) + (beta * curTriangle.b.colour) + (gamma * curTriangle.c.colour);
	//curColour *= texW;

	int depthDataIndex = GetFlattenedImageDataSlotForDepthData(curPoint, imageWidth);

	if (depthDataIndex >= 0 && depthDataIndex < imageDepthData.size() && imageDepthData[depthDataIndex] < depth)
	{
		//std::cout << "Pixel has passed depth test." << std::endl;
		float cutOffValueFloat = 0.0f;
		if ((crossAFloat >= cutOffValueFloat && crossBFloat >= cutOffValueFloat && crossCFloat >= cutOffValueFloat)
			|| (crossAFloat <= cutOffValueFloat && crossBFloat <= cutOffValueFloat && crossCFloat <= cutOffValueFloat))
		{
			//std::cout << "Ready to draw pixel." << std::endl;
			int index = GetRedFlattenedImageDataSlotForPixel(curPoint, imageWidth);
			if (index >= 0 && (index + 3) < imageData.size())
			{
				//curColour = { 255, 255, 255, 255 };

				//std::cout << "Drawing pixel." << std::endl;

				//Colour texelColour = GetColourFromTexCoord(*curTex, curTexCoord);
				Colour texelColour = colourTextureMixFactor < 1.0f ? GetColourFromTexCoord(*curTex, texCoord) : colour_black;
				float r = ((1.0f - colourTextureMixFactor) * texelColour.r + (colourTextureMixFactor * curColour.r));
				float g = ((1.0f - colourTextureMixFactor) * texelColour.g + (colourTextureMixFactor * curColour.g));
				float b = ((1.0f - colourTextureMixFactor) * texelColour.b + (colourTextureMixFactor * curColour.b));

				//r = texCoord.x * 255;
				//g = texCoord.y * 255;
				//b = 0;

				imageData[index + 0] = r * lightDotTriangleNormal;
				imageData[index + 1] = g * lightDotTriangleNormal;
				imageData[index + 2] = b * lightDotTriangleNormal;
				imageData[index + 3] = 255;

				if (drawFixedColour) {
					imageData[index + 0] = fixedColour.r;
					imageData[index + 1] = fixedColour.g;
					imageData[index + 2] = fixedColour.b;
					imageData[index + 3] = 255;
				}
			}

			imageDepthData[depthDataIndex] = depth;
		}
		//else {
		//	int index = GetRedFlattenedImageDataSlotForPixel(curPoint, imageWidth);

		//	imageData[index + 0] = 0;
		//	imageData[index + 1] = 0;
		//	imageData[index + 2] = 0;
		//	imageData[index + 3] = 255;

		//	imageDepthData[depthDataIndex] = depth;
		//}
		//else {
		//	std::cout << "crossA := " << crossAFloat << " crossB := " << crossBFloat << " crossC := " << crossCFloat << std::endl;
		//}
	}

}

void DrawCurrentFlatLine(const float& imageWidth,
	const float& y, const float& yTop,
	const float& startX, const float& endX,
	const Vector2& boundingBoxMin, const Vector2& boundingBoxMax,
	const float& lightDotTriangleNormal,
	const Vector3& deltaY, const Vector3& deltaX, const Vector3& deltaK,
	const float& areaOfTriangle,
	const Vector3& invDepth,
	const Vector3& invW, const Vector3& texW,
	const float& colourTextureMixFactor, const Colour& fixedColour,
	const Triangle& triangle,
	const Texture* curTex, std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData)
{
	float x1 = startX;
	float x2 = endX;
	if (x1 > x2) {
		std::swap(x1, x2);
	}

	int numPixelsForPadding = 3;
	////x1 -= numPixelsForPadding;
	x2 += numPixelsForPadding;

	////x1 = std::max(boundingBoxMin.x, x1);
	x2 = std::min(boundingBoxMax.x, x2);

	//x1 = std::max(x1, 0.0f);
	for (float x = x1; x <= x2; x++)
	{
		//std::cout << "Stuck 3 := " <<  x << std::endl;
		DrawCurrentTrianglePixel(imageWidth, x, y, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, triangle, colourTextureMixFactor, fixedColour, false, curTex, imageData, imageDepthData);
		//if (abs(x - x1) <= 2 || abs(x - x2) == 2) {
		//	DrawCurrentPixel(imageWidth, x, y, deltaY, deltaX, deltaK, biasEdgeFloat, areaOfTriangle, invDepth, invW, texW, triangle, colourTextureMixFactor, colour_black, true, curTex, imageData, imageDepthData);
		//}
		//else {
		//	DrawCurrentPixel(imageWidth, x, y, deltaY, deltaX, deltaK, biasEdgeFloat, areaOfTriangle, invDepth, invW, texW, triangle, colourTextureMixFactor, colour_white, false, curTex, imageData, imageDepthData);
		//}
	}

}

int printRate = 1000;
int printCounter = 0;

void DrawFlatBottomTriangle(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight,
							const Vector2& boundingBoxMin, const Vector2& boundingBoxMax,
							const Vector3& d,
							const float& lightDotTriangleNormal,
							const Vector3& deltaY, const Vector3& deltaX, const Vector3& deltaK,
							const float& areaOfTriangle,
							const Vector3& invDepth,
							const Vector3& invW, const Vector3& texW,
							const Triangle& triangle, const Texture* curTex,
							const float& colourTextureMixFactor,
							const Colour& fixedColour, bool drawFixedColour,
							int lineThickness) {

	float startScanLineXTop = triangle.c.position.x;
	float endScanLineXTop = triangle.c.position.x;

	float slope1DeltaX = triangle.b.position.x - triangle.c.position.x;
	float slope1DeltaY = triangle.b.position.y - triangle.c.position.y;

	float slope2DeltaX = d.x - triangle.c.position.x;
	float slope2DeltaY = triangle.b.position.y - triangle.c.position.y;
	bool slope2Steep = abs(slope2DeltaX) < abs(slope2DeltaY);

	float slope1Top = (slope1DeltaX) / (slope1DeltaY);
	float slope2Top = (slope2DeltaX) / (slope2DeltaY);

	for (int y = triangle.c.position.y; y <= triangle.b.position.y; y++)
	{
		DrawCurrentFlatLine(imageWidth, y, triangle.c.position.y, startScanLineXTop, endScanLineXTop, boundingBoxMin, boundingBoxMax, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, colourTextureMixFactor, colour_blue, triangle, curTex, imageData, imageDepthData);
		//DrawCurrentFlatLine(imageWidth, y, triangle.c.position.y, boundingBoxMin.x, boundingBoxMax.x, boundingBoxMin, boundingBoxMax, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, colourTextureMixFactor, colour_blue, triangle, curTex, imageData, imageDepthData);

		startScanLineXTop += (slope1Top);
		endScanLineXTop += (slope2Top);
	}

	DrawLineSegmentOnScreenWithInterpolatedValues(imageWidth, triangle.c.position, triangle.b.position, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, triangle, colourTextureMixFactor, colour_pink, false, curTex, imageData, imageDepthData);
	DrawLineSegmentOnScreenWithInterpolatedValues(imageWidth, triangle.b.position, d, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, triangle, colourTextureMixFactor, colour_pink, false, curTex, imageData, imageDepthData);
	DrawLineSegmentOnScreenWithInterpolatedValues(imageWidth, d, triangle.c.position, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, triangle, colourTextureMixFactor, colour_pink, false, curTex, imageData, imageDepthData);

}

void DrawFlatTopTriangle(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight,
	const Vector2& boundingBoxMin, const Vector2& boundingBoxMax,
	const Vector3& d,
	const float& lightDotTriangleNormal,
	const Vector3& deltaY, const Vector3& deltaX, const Vector3& deltaK,
	const float& areaOfTriangle,
	const Vector3& invDepth,
	const Vector3& invW, const Vector3& texW,
	const Triangle& triangle, const Texture* curTex,
	const float& colourTextureMixFactor,
	const Colour& fixedColour, bool drawFixedColour,
	int lineThickness) {


	float startScanLineXBottom = triangle.a.position.x;
	float endScanLineXBottom = triangle.a.position.x;

	float slope1Bottom = (triangle.a.position.x - triangle.b.position.x) / (triangle.a.position.y - triangle.b.position.y);
	float slope2Bottom = (triangle.a.position.x - d.x) / (triangle.a.position.y - triangle.b.position.y);

	for (int y = triangle.a.position.y; y >= triangle.b.position.y; y--)
	{
		DrawCurrentFlatLine(imageWidth, y, triangle.b.position.y, startScanLineXBottom, endScanLineXBottom, boundingBoxMin, boundingBoxMax, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, colourTextureMixFactor, colour_red, triangle, curTex, imageData, imageDepthData);
		//DrawCurrentFlatLine(imageWidth, y, triangle.b.position.y, boundingBoxMin.x, boundingBoxMax.x, boundingBoxMin, boundingBoxMax, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, colourTextureMixFactor, colour_red, triangle, curTex, imageData, imageDepthData);

		startScanLineXBottom -= (slope1Bottom);
		endScanLineXBottom -= (slope2Bottom);
	}

	DrawLineSegmentOnScreenWithInterpolatedValues(imageWidth, triangle.b.position, triangle.a.position, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, triangle, colourTextureMixFactor, colour_pink, false, curTex, imageData, imageDepthData);
	DrawLineSegmentOnScreenWithInterpolatedValues(imageWidth, triangle.a.position, d, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, triangle, colourTextureMixFactor, colour_pink, false, curTex, imageData, imageDepthData);
	DrawLineSegmentOnScreenWithInterpolatedValues(imageWidth, d, triangle.b.position, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, triangle, colourTextureMixFactor, colour_pink, false, curTex, imageData, imageDepthData);

}

void DrawTriangleOnScreenFromScreenSpaceBoundingBoxMethod(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData,
	int imageWidth, int imageHeight,
	int curTriangleIndex, int currentTextureIndex,
	const Triangle& drawTriangle, const float& lightDotTriangleNormal,
	Vector3 invDepth, Vector3 invW,
	int lineThickness)
{
	PROFILE_FUNCTION();

	Triangle triangle = drawTriangle;
	//PrintColour(triangle.a.colour);
	//PrintColour(triangle.b.colour);
	//PrintColour(triangle.c.colour);
	//triangle.a.colour = colour_blue;
	//triangle.b.colour = colour_red;
	//triangle.c.colour = colour_green;

	// Sort points based on their y coordinates to end up with := [TopOnScreen = C, Middle = b, BottomOnScreen = a]
	if (triangle.a.position.y < triangle.b.position.y) {
		std::swap(triangle.a, triangle.b);
		std::swap(invDepth.x, invDepth.y);
		std::swap(invW.x, invW.y);
	}
	if (triangle.a.position.y < triangle.c.position.y) {
		std::swap(triangle.a, triangle.c);
		std::swap(invDepth.x, invDepth.z);
		std::swap(invW.x, invW.z);
	}
	if (triangle.b.position.y < triangle.c.position.y) {
		std::swap(triangle.b, triangle.c);
		std::swap(invDepth.y, invDepth.z);
		std::swap(invW.y, invW.z);
	}

	//float areaOfTriangle = (cFloat.x * aFloat.y - aFloat.x * cFloat.y) * 0.5f;
	float areaOfTriangle = EdgeFunction(triangle.a.position, triangle.b.position, triangle.c.position) * 0.5f;

	Texture* curTex = &Model::textures[currentTextureIndex];

	int divisionPointY = triangle.b.position.y;

	float colourTextureMixFactor = 0.0f;
	//colourTextureMixFactor = 1.0f;
	//colourTextureMixFactor = 0.5f;

	Vector3 texW = { triangle.a.texCoord.z, triangle.b.texCoord.z, triangle.c.texCoord.z };

	float deltaYA = triangle.c.position.y - triangle.b.position.y;
	float deltaXA = triangle.c.position.x - triangle.b.position.x;
	float deltaKA = (triangle.b.position.y * triangle.c.position.x) - (triangle.b.position.x * triangle.c.position.y);

	float deltaYB = triangle.a.position.y - triangle.c.position.y;
	float deltaXB = triangle.a.position.x - triangle.c.position.x;
	float deltaKB = (triangle.c.position.y * triangle.a.position.x) - (triangle.c.position.x * triangle.a.position.y);

	float deltaYC = triangle.b.position.y - triangle.a.position.y;
	float deltaXC = triangle.b.position.x - triangle.a.position.x;
	float deltaKC = (triangle.a.position.y * triangle.b.position.x) - (triangle.a.position.x * triangle.b.position.y);

	Vector3 deltaY = { deltaYA, deltaYB, deltaYC };
	Vector3 deltaX = { deltaXA, deltaXB, deltaXC };
	Vector3 deltaK = { deltaKA, deltaKB, deltaKC };

	Vector2 boundingBoxMin = Vector2{ std::min(triangle.a.position.x, triangle.b.position.x), std::min(triangle.a.position.y, triangle.b.position.y) };
	Vector2 boundingBoxMax = Vector2{ std::max(triangle.a.position.x, triangle.b.position.x), std::max(triangle.a.position.y, triangle.b.position.y) };

	boundingBoxMin = Vector2{ std::min(boundingBoxMin.x, triangle.c.position.x), std::min(boundingBoxMin.y, triangle.c.position.y) };
	boundingBoxMax = Vector2{ std::max(boundingBoxMax.x, triangle.c.position.x), std::max(boundingBoxMax.y, triangle.c.position.y) };

	float x4 = triangle.c.position.x + ((triangle.b.position.y - triangle.c.position.y) / (triangle.a.position.y - triangle.c.position.y)) * (triangle.a.position.x - triangle.c.position.x);
	Vector3 d = { x4, triangle.b.position.y, 0.0f };

	DrawFlatTopTriangle(imageData, imageDepthData, imageWidth, imageHeight, boundingBoxMin, boundingBoxMax, d, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, triangle, curTex, colourTextureMixFactor, colour_red, false, lineThickness);

	DrawFlatBottomTriangle(imageData, imageDepthData, imageWidth, imageHeight, boundingBoxMin, boundingBoxMax, d, lightDotTriangleNormal, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, triangle, curTex, colourTextureMixFactor, colour_red, false, lineThickness);

	//if (printRateCounter % printRate == 0) {
	//    //std::cout << printString << std::endl;
	//}

	//printCounter++;
	//std::cout << printCounter % printRate << std::endl;
	//lineThickness = 1;
	//DrawLineSegmentOnScreen(imageData, imageWidth, triangle.a.position, triangle.b.position, lineThickness, colour_black);
	//DrawLineSegmentOnScreen(imageData, imageWidth, triangle.b.position, triangle.c.position, lineThickness, colour_black);
	//DrawLineSegmentOnScreen(imageData, imageWidth, triangle.c.position, triangle.a.position, lineThickness, colour_black);

}


int TriangleClipAgainstPlane(const Plane& plane, const Triangle& in_tri, std::vector<Triangle>& outputTriangles, bool test = false)
{
	PROFILE_FUNCTION();

	float da = DistanceFromPointToPlane(in_tri.a, plane);
	float db = DistanceFromPointToPlane(in_tri.b, plane);
	float dc = DistanceFromPointToPlane(in_tri.c, plane);

	if (da > 0.0f && db > 0.0f && dc > 0.0f) {
		outputTriangles.push_back(in_tri);
		return 1;
	}
	if (da < 0.0f && db < 0.0f && dc < 0.0f) {
		return 0;
	}

	if (da > 0.0f) {

		//first point is a.

		if (db > 0.0f) {

			// second point is b

			// c is divided into two points d and e which are interpolated from a and b to c respectively.
			Point d;
			LinePlaneIntersection(in_tri.a.position, in_tri.c.position, plane, d.position);
			float ld = glm::length(d.position - in_tri.a.position) / glm::length(in_tri.c.position - in_tri.a.position);
			d.texCoord = LerpVector3(in_tri.a.texCoord, in_tri.c.texCoord, ld);
			d.colour = LerpVector4(in_tri.a.colour, in_tri.c.colour, ld);
			d.normal = LerpVector3(in_tri.a.normal, in_tri.c.normal, ld);

			Point e;
			LinePlaneIntersection(in_tri.b.position, in_tri.c.position, plane, e.position);
			float le = glm::length(e.position - in_tri.b.position) / glm::length(in_tri.c.position - in_tri.b.position);
			e.texCoord = LerpVector3(in_tri.b.texCoord, in_tri.c.texCoord, le);
			e.colour = LerpVector4((in_tri.b.colour), (in_tri.c.colour), le);
			e.normal = LerpVector3((in_tri.b.normal), (in_tri.c.normal), le);

			// make two triangles := { a, b, d} & {d, b, e}
			Triangle clippedTriangleABD;
			clippedTriangleABD.a = in_tri.a;
			clippedTriangleABD.b = in_tri.b;
			clippedTriangleABD.c = d;

			Triangle clippedTriangleDBE;
			clippedTriangleDBE.a = d;
			clippedTriangleDBE.b = in_tri.b;
			clippedTriangleDBE.c = e;

			outputTriangles.push_back(clippedTriangleABD);
			outputTriangles.push_back(clippedTriangleDBE);
			//std::cout << "Two triangles : = { a, b, d } & { d, b, e }" << std::endl;

			return 2; // Since we are making 2 triangles.
		}
		if (dc > 0.0f) {

			// second point is C

			// b is divided into two points d and e which are interpolated from a and c to b respectively.
			Point d;
			LinePlaneIntersection(in_tri.a.position, in_tri.b.position, plane, d.position);
			float ld = glm::length(d.position - in_tri.a.position) / glm::length(in_tri.b.position - in_tri.a.position);
			d.texCoord = LerpVector3(in_tri.a.texCoord, in_tri.b.texCoord, ld);
			//d.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.a.colour), ColourToVector4(in_tri.b.colour), ld));
			d.colour = (LerpVector4((in_tri.a.colour), (in_tri.b.colour), ld));
			d.normal = (LerpVector3((in_tri.a.normal), (in_tri.b.normal), ld));


			Point e;
			LinePlaneIntersection(in_tri.c.position, in_tri.b.position, plane, e.position);
			float le = glm::length(e.position - in_tri.c.position) / glm::length(in_tri.b.position - in_tri.c.position);
			e.texCoord = LerpVector3(in_tri.c.texCoord, in_tri.b.texCoord, le);
			//e.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.c.colour), ColourToVector4(in_tri.b.colour), le));
			e.colour = (LerpVector4((in_tri.c.colour), (in_tri.b.colour), le));
			e.normal = (LerpVector3((in_tri.c.normal), (in_tri.b.normal), le));

			// make two triangles := {a, c, d} & {d, c, e}
			Triangle clippedTriangleACD;
			clippedTriangleACD.a = in_tri.a;
			clippedTriangleACD.b = in_tri.c;
			clippedTriangleACD.c = d;

			Triangle clippedTriangleDCE;
			clippedTriangleDCE.a = d;
			clippedTriangleDCE.b = in_tri.c;
			clippedTriangleDCE.c = e;

			outputTriangles.push_back(clippedTriangleACD);
			outputTriangles.push_back(clippedTriangleDCE);
			//std::cout << "Two triangles : = {a, c, d} & {d, c, e}" << std::endl;

			return 2; // Since we are making 2 triangles.
		}
		if (db < 0.0f && dc < 0.0f) {

			// second and third points are interpolated between b and c to a.
			Point d;
			LinePlaneIntersection(in_tri.a.position, in_tri.b.position, plane, d.position);
			float ld = glm::length(d.position - in_tri.a.position) / glm::length(in_tri.b.position - in_tri.a.position);
			d.texCoord = LerpVector3(in_tri.a.texCoord, in_tri.b.texCoord, ld);
			//d.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.a.colour), ColourToVector4(in_tri.b.colour), ld));
			d.colour = (LerpVector4((in_tri.a.colour), (in_tri.b.colour), ld));
			d.normal = (LerpVector3((in_tri.a.normal), (in_tri.b.normal), ld));

			Point e;
			LinePlaneIntersection(in_tri.a.position, in_tri.c.position, plane, e.position);
			float le = glm::length(e.position - in_tri.a.position) / glm::length(in_tri.c.position - in_tri.a.position);
			e.texCoord = LerpVector3(in_tri.a.texCoord, in_tri.c.texCoord, le);
			//e.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.a.colour), ColourToVector4(in_tri.c.colour), le));
			e.colour = (LerpVector4((in_tri.a.colour), (in_tri.c.colour), le));
			e.normal = (LerpVector3((in_tri.a.normal), (in_tri.c.normal), le));

			// make one triangle := {a, d, e}
			Triangle clippedTriangleADE;
			clippedTriangleADE.a = in_tri.a;
			clippedTriangleADE.b = d;
			clippedTriangleADE.c = e;

			outputTriangles.push_back(clippedTriangleADE);
			//std::cout << "One triangle : = {a, d, e}" << std::endl;

			return 1; // Since we are making 1 triangles.

		}

	}
	if (db > 0.0f) {

		if (dc > 0.0f) {

			// second point is c

			// a is divided into two points d and e which are interpolated from b and c to a respectively.
			Point d;
			LinePlaneIntersection(in_tri.b.position, in_tri.a.position, plane, d.position);
			float ld = glm::length(d.position - in_tri.b.position) / glm::length(in_tri.a.position - in_tri.b.position);
			d.texCoord = LerpVector3(in_tri.b.texCoord, in_tri.a.texCoord, ld);
			//d.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.b.colour), ColourToVector4(in_tri.a.colour), ld));
			d.colour = (LerpVector4((in_tri.b.colour), (in_tri.a.colour), ld));
			d.normal = (LerpVector3((in_tri.b.normal), (in_tri.a.normal), ld));


			Point e;
			LinePlaneIntersection(in_tri.c.position, in_tri.a.position, plane, e.position);
			float le = glm::length(e.position - in_tri.c.position) / glm::length(in_tri.a.position - in_tri.c.position);
			e.texCoord = LerpVector3(in_tri.c.texCoord, in_tri.a.texCoord, le);
			//e.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.c.colour), ColourToVector4(in_tri.a.colour), le));
			e.colour = (LerpVector4((in_tri.c.colour), (in_tri.a.colour), le));
			e.normal = (LerpVector3((in_tri.c.normal), (in_tri.a.normal), le));

			// make two triangles := {b, c, d} & {d, c, e}
			Triangle clippedTriangleBCD;
			clippedTriangleBCD.a = in_tri.b;
			clippedTriangleBCD.b = in_tri.c;
			clippedTriangleBCD.c = d;

			Triangle clippedTriangleDCE;
			clippedTriangleDCE.a = d;
			clippedTriangleDCE.b = in_tri.c;
			clippedTriangleDCE.c = e;

			outputTriangles.push_back(clippedTriangleBCD);
			outputTriangles.push_back(clippedTriangleDCE);
			//std::cout << "Two triangles : = {b, c, d} & {d, c, e}" << std::endl;

			return 2; // Since we are making 2 triangles.
		}
		if (dc < 0.0f && da < 0.0f) {

			// second and third points are interpolated between c and a to b.
			Point d;
			LinePlaneIntersection(in_tri.c.position, in_tri.b.position, plane, d.position);
			float ld = glm::length(d.position - in_tri.c.position) / glm::length(in_tri.b.position - in_tri.c.position);
			d.texCoord = LerpVector3(in_tri.c.texCoord, in_tri.b.texCoord, ld);
			//d.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.c.colour), ColourToVector4(in_tri.b.colour), ld));
			d.colour = (LerpVector4((in_tri.c.colour), (in_tri.b.colour), ld));
			d.normal = (LerpVector3((in_tri.c.normal), (in_tri.b.normal), ld));

			Point e;
			LinePlaneIntersection(in_tri.a.position, in_tri.b.position, plane, e.position);
			float le = glm::length(e.position - in_tri.a.position) / glm::length(in_tri.b.position - in_tri.a.position);
			e.texCoord = LerpVector3(in_tri.a.texCoord, in_tri.b.texCoord, le);
			//e.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.a.colour), ColourToVector4(in_tri.b.colour), le));
			e.colour = (LerpVector4((in_tri.a.colour), (in_tri.b.colour), le));
			e.normal = (LerpVector3((in_tri.a.normal), (in_tri.b.normal), le));

			// make two triangles := {b, d, e}
			Triangle clippedTriangleBDE;
			clippedTriangleBDE.a = in_tri.b;
			clippedTriangleBDE.b = d;
			clippedTriangleBDE.c = e;

			//std::cout << "One triangle : = {b, d, e}" << std::endl;
			outputTriangles.push_back(clippedTriangleBDE);

			return 1; // Since we are making 1 triangles.
		}
	}
	if (dc > 0.0f) {
		if (db < 0.0f && da < 0.0f) {

			// second and third points are interpolated between b and a to c.
			Point d;
			LinePlaneIntersection(in_tri.a.position, in_tri.c.position, plane, d.position);
			float ld = glm::length(d.position - in_tri.a.position) / glm::length(in_tri.c.position - in_tri.a.position);
			d.texCoord = LerpVector3(in_tri.a.texCoord, in_tri.c.texCoord, ld);
			//d.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.a.colour), ColourToVector4(in_tri.c.colour), ld));
			d.colour = (LerpVector4((in_tri.a.colour), (in_tri.c.colour), ld));
			d.normal = (LerpVector3((in_tri.a.normal), (in_tri.c.normal), ld));

			Point e;
			LinePlaneIntersection(in_tri.b.position, in_tri.c.position, plane, e.position);
			float le = glm::length(e.position - in_tri.b.position) / glm::length(in_tri.c.position - in_tri.b.position);
			e.texCoord = LerpVector3(in_tri.b.texCoord, in_tri.c.texCoord, le);
			//e.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.b.colour), ColourToVector4(in_tri.c.colour), le));
			e.colour = (LerpVector4((in_tri.b.colour), (in_tri.c.colour), le));
			e.normal = (LerpVector3((in_tri.b.normal), (in_tri.c.normal), le));

			// make one triangles := {c, d, e}
			Triangle clippedTriangleCDE;
			clippedTriangleCDE.a = in_tri.c;
			clippedTriangleCDE.b = d;
			clippedTriangleCDE.c = e;

			outputTriangles.push_back(clippedTriangleCDE);
			//std::cout << "One triangles : = {c, d, e}" << std::endl;

			return 1;
		}
	}
}

void DrawTriangleOnScreenFromWorldTriangleWithClipping(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight
	, int curTriangleIndex, int currentTextureIndex, Triangle& modelTriangle, Mat4x4& modelMatrix
	, Vector3 cameraPosition, Vector3 cameraDirection
	, const Mat4x4& viewMatrix, const Mat4x4& projectionMatrix
	, int lineThickness, Colour lineColour, bool debugDraw = false) {

	//cameraPosition.y *= -1.0f;
	PROFILE_FUNCTION();

	Triangle transformedTriangle = ApplyTransformToTriangle(modelTriangle, modelMatrix);
	transformedTriangle.a.position.y *= -1.0f;
	transformedTriangle.b.position.y *= -1.0f;
	transformedTriangle.c.position.y *= -1.0f;

	Mat4x4 normalTransformMatrix;
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			normalTransformMatrix[x][y] = modelMatrix[x][y];
		}
	}

	for (int y = 0; y < 4; y++)
	{
		normalTransformMatrix[3][y] = 0;
	}

	//std::cout << modelTriangle.a.normal.x << ", " << modelTriangle.a.normal.y << ", " << modelTriangle.a.normal.z << std::endl;

	ApplyTransformToTriangleNormals(transformedTriangle, normalTransformMatrix);

	//std::cout << modelTriangle.a.normal.x << ", " << modelTriangle.a.normal.y << ", " << modelTriangle.a.normal.z << std::endl;

	//transformedTriangle.a.texCoord.y *= -1.0f;
	//transformedTriangle.b.texCoord.y *= -1.0f;
	//transformedTriangle.c.texCoord.y *= -1.0f;

	Vector3 trianglePos = 0.33f * transformedTriangle.a.position + 0.33f * transformedTriangle.b.position + 0.33f * transformedTriangle.c.position;
	Vector3 lightPos = { 150.0f, 50.0f, 150.0f };
	Vector3 lightDirFromTriangle = glm::normalize(lightPos - trianglePos);

	Vector3 normal, lineA, lineB;
	lineA = transformedTriangle.b.position - transformedTriangle.a.position;
	lineB = transformedTriangle.c.position - transformedTriangle.b.position;

	normal = glm::cross(lineA, lineB);
	normal = glm::normalize(normal);

	//Vector3 trianglePosRelativeToCamera = cameraPosition - transformedTriangle.a.position;
	Vector3 trianglePosRelativeToCamera = transformedTriangle.a.position - cameraPosition;
	trianglePosRelativeToCamera = glm::normalize(trianglePosRelativeToCamera);

	bool curTriangleIsVisibleFromCamera = glm::dot(normal, trianglePosRelativeToCamera) > 0.0f;
	bool cameraCouldSeeTriangle = glm::dot(trianglePosRelativeToCamera, cameraDirection) > 0.0f;

	if (cameraCouldSeeTriangle && curTriangleIsVisibleFromCamera)
	{
		//Mat4x4 projectionViewMatrix = projectionMatrix * viewMatrix;

		float lightDotTriangleNormal = glm::max(glm::dot(lightDirFromTriangle, normal), 0.1f);

		// Transform into view space.
		Vector4 viewTransformedA = { viewMatrix * Vector4{transformedTriangle.a.position, 1.0f} };
		Vector4 viewTransformedB = { viewMatrix * Vector4{transformedTriangle.b.position, 1.0f} };
		Vector4 viewTransformedC = { viewMatrix * Vector4{transformedTriangle.c.position, 1.0f} };

		// Clip Viewed Triangle against near plane, this could form two additional
		// additional triangles. 
		int nClippedTriangles = 0;
		Triangle curLargeTriangle = Triangle{ {viewTransformedA, modelTriangle.a.texCoord, modelTriangle.a.colour, modelTriangle.a.normal}, {viewTransformedB, modelTriangle.b.texCoord, modelTriangle.b.colour, modelTriangle.b.normal}, {viewTransformedC, modelTriangle.c.texCoord, modelTriangle.c.colour, modelTriangle.c.normal }, colour_white };

		std::vector<Triangle> zClippingOutputTriangles;
		nClippedTriangles = TriangleClipAgainstPlane(planeNear, curLargeTriangle, zClippingOutputTriangles);

		{
			PROFILE_SCOPE("TESTING FOR CLIPPING.");
			for (int n = 0; n < nClippedTriangles; n++)
			{
				// Transform into Homogeneous space.
				Vector4 projectedPointA = { projectionMatrix * Vector4 {zClippingOutputTriangles[n].a.position, 1.0f} };
				Vector4 projectedPointB = { projectionMatrix * Vector4 {zClippingOutputTriangles[n].b.position, 1.0f} };
				Vector4 projectedPointC = { projectionMatrix * Vector4 {zClippingOutputTriangles[n].c.position, 1.0f} };

				// Get W for interpolation in Screen Space.
				Vector3 invW = Vector3{ 1.0f / projectedPointA.w, 1.0f / projectedPointB.w, 1.0f / projectedPointC.w };

				//Transform into NDC space.
				projectedPointA = projectedPointA * invW.x;
				projectedPointB = projectedPointB * invW.y;
				projectedPointC = projectedPointC * invW.z;

				// Get depth for Z-Buffer
				Vector3 invDepth = Vector3{ 1.0f / projectedPointA.z, 1.0f / projectedPointB.z, 1.0f / projectedPointC.z };

				zClippingOutputTriangles[n].a.texCoord *= invW.x;
				zClippingOutputTriangles[n].b.texCoord *= invW.y;
				zClippingOutputTriangles[n].c.texCoord *= invW.z;

				zClippingOutputTriangles[n].a.texCoord.z = invW.x;
				zClippingOutputTriangles[n].b.texCoord.z = invW.y;
				zClippingOutputTriangles[n].c.texCoord.z = invW.z;

				zClippingOutputTriangles[n].a.normal *= invW.x;
				zClippingOutputTriangles[n].b.normal *= invW.y;
				zClippingOutputTriangles[n].c.normal *= invW.z;

				zClippingOutputTriangles[n].a.colour *= invW.x;
				zClippingOutputTriangles[n].b.colour *= invW.y;
				zClippingOutputTriangles[n].c.colour *= invW.z;

				//Vector4 interpolatableColourA = ColourToVector4(zClippingOutputTriangles[n].a.colour) * invW.x;
				//Vector4 interpolatableColourB = ColourToVector4(zClippingOutputTriangles[n].b.colour) * invW.y;
				//Vector4 interpolatableColourC = ColourToVector4(zClippingOutputTriangles[n].c.colour) * invW.z;

				//zClippingOutputTriangles[n].a.colour = Vector4ToColour(interpolatableColourA);
				//zClippingOutputTriangles[n].b.colour = Vector4ToColour(interpolatableColourB);
				//zClippingOutputTriangles[n].c.colour = Vector4ToColour(interpolatableColourC);

				//Transform into Screen Space
				projectedPointA.x += 1.0f; projectedPointA.y += 1.0f;
				projectedPointB.x += 1.0f; projectedPointB.y += 1.0f;
				projectedPointC.x += 1.0f; projectedPointC.y += 1.0f;

				projectedPointA.x *= (0.5 * imageWidth); projectedPointA.y *= (0.5 * imageHeight);
				projectedPointB.x *= (0.5 * imageWidth); projectedPointB.y *= (0.5 * imageHeight);
				projectedPointC.x *= (0.5 * imageWidth); projectedPointC.y *= (0.5 * imageHeight);

				//Triangle curLargeScreenSpaceTriangle = { {projectedPointA, clipped[n].a.texCoord * invW.x }, {projectedPointB, clipped[n].b.texCoord * invW.y }, {projectedPointC, clipped[n].c.texCoord * invW.z}, clipped[n].colour};
				Triangle curLargeScreenSpaceTriangle = { {projectedPointA, zClippingOutputTriangles[n].a.texCoord, zClippingOutputTriangles[n].a.colour, zClippingOutputTriangles[n].a.normal },
														 {projectedPointB, zClippingOutputTriangles[n].b.texCoord, zClippingOutputTriangles[n].b.colour, zClippingOutputTriangles[n].b.normal },
														 {projectedPointC, zClippingOutputTriangles[n].c.texCoord, zClippingOutputTriangles[n].c.colour, zClippingOutputTriangles[n].c.normal },
														 zClippingOutputTriangles[n].colour };

				std::vector<Triangle> bottomScreenPlaneClippingResult;
				TriangleClipAgainstPlane(planeBottomScreenSpace, curLargeScreenSpaceTriangle, bottomScreenPlaneClippingResult);
				
				std::vector<Triangle> topScreenPlaneClippingResult;
				for (int i = 0; i < bottomScreenPlaneClippingResult.size(); i++)
				{
					TriangleClipAgainstPlane(planeTopScreenSpace, bottomScreenPlaneClippingResult[i], topScreenPlaneClippingResult);
				}

				std::vector<Triangle> leftScreenPlaneClippingResult;
				for (int i = 0; i < topScreenPlaneClippingResult.size(); i++)
				{
					TriangleClipAgainstPlane(planeLeftScreenSpace, topScreenPlaneClippingResult[i], leftScreenPlaneClippingResult);
				}

				std::vector<Triangle> rightScreenPlaneClippingResult;
				for (int i = 0; i < leftScreenPlaneClippingResult.size(); i++)
				{
					TriangleClipAgainstPlane(planeRightScreenSpace, leftScreenPlaneClippingResult[i], rightScreenPlaneClippingResult);
				}
				//std::cout << "Drawing something." << std::endl;
				//DrawTriangleOnScreenFromScreenSpaceBoundingBoxMethod(imageData, imageDepthData, imageWidth, imageHeight, curTriangleIndex, currentTextureIndex, curLargeScreenSpaceTriangle, normal, invDepth, invW, lineThickness);
				for (int i = 0; i < rightScreenPlaneClippingResult.size(); i++)
				{
					DrawTriangleOnScreenFromScreenSpaceBoundingBoxMethod(imageData, imageDepthData, imageWidth, imageHeight, curTriangleIndex, currentTextureIndex, rightScreenPlaneClippingResult[i], lightDotTriangleNormal, invDepth, invW, lineThickness);
				}
			}
		}
	}
}

void DrawMeshOnScreenFromWorldWithTransform(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight, Mesh& currentMesh, Mat4x4& modelMatrix, Vector3 cameraPosition, Vector3 cameraDirection, Mat4x4& viewMatrix, Mat4x4& projectionMatrix, int lineThickness, Colour lineColour, bool debugDraw = false) {

	PROFILE_FUNCTION();

	for (int i = 0; i < currentMesh.triangles.size(); i++)
	{
		//std::cout << "READING MESH TEXTURE INDEX 0 : " << currentMesh.textureIndex << std::endl;
		DrawTriangleOnScreenFromWorldTriangleWithClipping(imageData, imageDepthData, imageWidth, imageHeight, i, currentMesh.textureIndex, currentMesh.triangles[i], modelMatrix, cameraPosition, cameraDirection, viewMatrix, projectionMatrix, lineThickness, lineColour, debugDraw);
	}
}