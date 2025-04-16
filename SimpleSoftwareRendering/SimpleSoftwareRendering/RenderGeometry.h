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

Vector3 GetVector3FromMat3x3(const Mat3x3& matrix, const int& columnNumber) {
	return matrix[columnNumber];
}

void SetVector3InMat3x3(Mat3x3& matrix, const int& columnNumber, const Vector3& vector) {
	matrix[columnNumber] = vector;
}

void MultiplyFloatToVectorInMat3x3(Mat3x3& matrix, const int& columnNumber, const float& multiplier) {
	matrix[columnNumber][0] *= multiplier;
	matrix[columnNumber][1] *= multiplier;
	matrix[columnNumber][2] *= multiplier;
}

void SwapVectorsInMat3x3(Mat3x3& matrix, const int& columnNumberA, const int& columnNumberB) {
	Vector3 temp = GetVector3FromMat3x3(matrix, columnNumberA);
	SetVector3InMat3x3(matrix, columnNumberA, GetVector3FromMat3x3(matrix, columnNumberB));
	SetVector3InMat3x3(matrix, columnNumberB, temp);
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
	const Vector3& lightDotTriangleNormals,
	const Vector3& deltaY, const Vector3& deltaX, const Vector3& deltaK,
	const float& areaOfTriangle,
	const Vector3& invDepth,
	const Vector3& invW, const Vector3& texWs,
	Mat3x3& vertexWorldPositions,
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

	Vector3 normal = (alpha * curTriangle.a.normal) + (beta * curTriangle.b.normal) + (gamma * curTriangle.c.normal);
	normal *= texW;

	Vector3 worldPositionOfFragment = (alpha * GetVector3FromMat3x3(vertexWorldPositions, 0)) + (beta * GetVector3FromMat3x3(vertexWorldPositions, 1)) + (gamma * GetVector3FromMat3x3(vertexWorldPositions, 2));
	worldPositionOfFragment *= texW;

	float lightDotTriangleNormal = (alpha * lightDotTriangleNormals.x) + (beta * lightDotTriangleNormals.y) + (gamma * lightDotTriangleNormals.z);
	lightDotTriangleNormal *= texW;

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
				//Colour texelColour = colourTextureMixFactor < 1.0f ? GetColourFromTexCoord(*curTex, texCoord) : colour_black;
				Colour texelColour = GetColourFromTexCoord(*curTex, texCoord);
				float r = ((1.0f - colourTextureMixFactor) * texelColour.r + (colourTextureMixFactor * curColour.r));
				float g = ((1.0f - colourTextureMixFactor) * texelColour.g + (colourTextureMixFactor * curColour.g));
				float b = ((1.0f - colourTextureMixFactor) * texelColour.b + (colourTextureMixFactor * curColour.b));

				//r = texCoord.x * 255;
				//g = texCoord.y * 255;
				//b = 0;

				//imageData[index + 0] = r;
				//imageData[index + 1] = g;
				//imageData[index + 2] = b;
				//imageData[index + 3] = 255;

				//imageData[index + 0] = 255 * normal.x;
				//imageData[index + 1] = 255 * normal.y;
				//imageData[index + 2] = 255 * normal.z;
				//imageData[index + 3] = 255;

				Vector3 lightPos = { 5.0f, -10.0f, -5.0f };
				Vector3 lightDirFromFragment = glm::normalize(lightPos - worldPositionOfFragment);
				float lightDotTriangleNormal = glm::max(glm::dot(lightDirFromFragment, normal), 0.1f);

				imageData[index + 0] = r * lightDotTriangleNormal;
				imageData[index + 1] = g * lightDotTriangleNormal;
				imageData[index + 2] = b * lightDotTriangleNormal;
				imageData[index + 3] = 255;

				//imageData[index + 0] = 255 * lightDotTriangleNormal;
				//imageData[index + 1] = 255 * lightDotTriangleNormal;
				//imageData[index + 2] = 255 * lightDotTriangleNormal;
				//imageData[index + 3] = 255;

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

void BresenhamLineDrawer(Vector2 start, Vector2 end, std::vector<Vector2>& outputPixels) {

	float dx = end.x - start.x;
	float dy = end.y - start.y;

	float step = std::max(abs(dx), abs(dy));

	if (step != 0) {
		float stepX = dx / step;
		float stepY = dy / step;
		for (int i = 0; i < step + 1; i++) {

			outputPixels.push_back({ (float)round(start.x + (i * stepX)), (float)round(start.y + (i * stepY)) });

		}
	}
}

void BresenhamTriangleDrawer(const Vector3& c, const Vector3& b, const Vector3& d,
								const float& imageWidth,
								const Vector3& lightDotTriangleNormals,
								const Vector3& deltaY, const Vector3& deltaX, const Vector3& deltaK,
								const float& areaOfTriangle,
								const Vector3& invDepth,
								const Vector3& invW, const Vector3& texW,
								Mat3x3& vertexWorldPositions,
								const Triangle& triangle,
								const float& colourTextureMixFactor,
								const Colour& fixedColour, bool drawFixedColour,
								const Texture* curTex, std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData
) {

	std::vector<Vector2> outputPixelsCB;
	BresenhamLineDrawer(c, b, outputPixelsCB);

	std::vector<Vector2> outputPixelsCD;
	BresenhamLineDrawer(c, d, outputPixelsCD);

	int indexPointCB = 0;
	int indexPointCD = 0;
	while (indexPointCB < outputPixelsCB.size() && indexPointCD < outputPixelsCD.size()) {

		int x0 = outputPixelsCB[indexPointCB].x;
		int x1 = outputPixelsCD[indexPointCD].x;

		int curYCB = outputPixelsCB[indexPointCB].y;
		while (curYCB == outputPixelsCB[indexPointCB].y) {
			DrawCurrentPixelWithInterpValues(imageWidth, outputPixelsCB[indexPointCB].x, outputPixelsCB[indexPointCB].y, lightDotTriangleNormals, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, vertexWorldPositions, triangle, colourTextureMixFactor, colour_blue, false, curTex, imageData, imageDepthData);
			indexPointCB++;
		}

		int curYCD = outputPixelsCD[indexPointCD].y;
		while (curYCB == curYCD && curYCD == outputPixelsCD[indexPointCD].y) {
			DrawCurrentPixelWithInterpValues(imageWidth, outputPixelsCD[indexPointCD].x, outputPixelsCD[indexPointCD].y, lightDotTriangleNormals, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, vertexWorldPositions, triangle, colourTextureMixFactor, colour_red, false, curTex, imageData, imageDepthData);
			indexPointCD++;
		}

		if (x0 > x1) {
			std::swap(x0, x1);
		}

		if (x0 > x1) {
			std::cout << x0 << ", " << x1 << std::endl;
		}

		// No need to draw last pixel because the next triangle with the same points and edge to the left will draw it anyway?
		for (int x = x0; x < x1; x++) {
			DrawCurrentPixelWithInterpValues(imageWidth, x, curYCB, lightDotTriangleNormals, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, vertexWorldPositions, triangle, colourTextureMixFactor, colour_green, false, curTex, imageData, imageDepthData);
		}

	}
}

// Slower and unstable.
void BresenhamTriangleDrawerAdvanced(const Vector2& c, const Vector2& b, const Vector2& d,
									const float& imageWidth,
									const Vector3& lightDotTriangleNormals,
									const Vector3& deltaY, const Vector3& deltaX, const Vector3& deltaK,
									const float& areaOfTriangle,
									const Vector3& invDepth,
									const Vector3& invW, const Vector3& texW,
									Mat3x3& vertexWorldPositions,
									const Triangle& triangle,
									const float& colourTextureMixFactor,
									const Colour& fixedColour, bool drawFixedColour,
									const Texture* curTex, std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData)
{

	float startY = round(c.y);
	float endY = round(b.y);

	float startXCB = round(c.x);
	float startXCD = round(c.x);

	float endXCB = round(b.x);
	float endXCD = round(d.x);

	if (endXCB > endXCD) {
		std::swap(endXCB, endXCD);
	}

	float dXCB = (endXCB - startXCB);
	float dXCD = (endXCD - startXCD);
	float dY = (endY - startY);
	
	if (dY != 0) {

		float dir = dY / abs(dY);

		float stepCB = std::max(abs(dXCB), abs(dY));
		float stepCD = std::max(abs(dXCD), abs(dY));

		float stepXCB = dXCB / stepCB;
		float stepYCB = dY / stepCB;

		float stepXCD = dXCD / stepCD;
		float stepYCD = dY / stepCD;

		int indexStepCB = 0;
		int indexStepCD = 0;

		float curYCB = startY;
		float curXCB = startXCB;
		float curYCD = startY;
		float curXCD = startXCD;

		float curY = round(startY);
		while (curY != round(endY + dir)) {

			float x0 = round(curXCB);
			while (indexStepCB <= stepCB && curY == curYCB) {

				//pixelsToDraw.push_back({(float)round(curXCB), (float)(round(curYCB))});
				DrawCurrentPixelWithInterpValues(imageWidth, curXCB, curYCB, lightDotTriangleNormals, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, vertexWorldPositions, triangle, colourTextureMixFactor, colour_red, false, curTex, imageData, imageDepthData);

				curXCB = round(startXCB + (indexStepCB * stepXCB));
				curYCB = round(startY + (indexStepCB * stepYCB));

				indexStepCB++;
			}

			float x1 = round(curXCD);
			while (indexStepCD <= stepCD && curY == curYCD) {

				//pixelsToDraw.push_back({(float)round(curXCD), (float)(round(curYCD))});
				DrawCurrentPixelWithInterpValues(imageWidth, curXCD, curYCD, lightDotTriangleNormals, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, vertexWorldPositions, triangle, colourTextureMixFactor, colour_red, false, curTex, imageData, imageDepthData);

				curXCD = round(startXCD + (indexStepCD * stepXCD));
				curYCD = round(startY + (indexStepCD * stepYCD));

				//std::cout << "curY := " << curY << ", curYCB := " << curYCB << ", curYCD := " << curYCD << std::endl;

				indexStepCD++;
			}

			// No need to draw last pixel because the next triangle with the same points and edge to the left will draw it anyway?
			for (int x = x0; x <= x1; x++) {
				//pixelsToDraw.push_back({ (float)x, (float)curY });
				DrawCurrentPixelWithInterpValues(imageWidth, x, curY, lightDotTriangleNormals, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, vertexWorldPositions, triangle, colourTextureMixFactor, colour_red, false, curTex, imageData, imageDepthData);
			}

			curY += dir;
		}
	}
}


bool IsLineTopOrLeft(Vector2Int start, Vector2Int end) {

	Vector2Int edge = end - start;

	bool isTopEdge = edge.y == 0 && edge.x > 0;
	bool isLeftEdge = edge.y < 0;

	return isTopEdge || isLeftEdge;
}

void DrawCurrentTrianglePixel(const float& imageWidth,
	const float& x, const float& y,
	const Vector3& lightDotTriangleNormals,
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

	//std::cout << lightDotTriangleNormals.x << ", " << lightDotTriangleNormals.y << ", " << lightDotTriangleNormals.z << std::endl;

	float lightDotTriangleNormal = (alpha * lightDotTriangleNormals.x) + (beta * lightDotTriangleNormals.y) + (gamma * lightDotTriangleNormals.z);
	lightDotTriangleNormal *= texW;

	//std::cout << lightDotTriangleNormal << std::endl;

	//std::cout << curTriangle.a.normal.x << ", " << curTriangle.a.normal.y << ", " << curTriangle.a.normal.z << std::endl;

	//std::cout << normal.x << ", " << normal.y << ", " << normal.z << std::endl;

	Vector4 curColour = (alpha * curTriangle.a.colour) + (beta * curTriangle.b.colour) + (gamma * curTriangle.c.colour);
	//curColour *= texW;

	int depthDataIndex = GetFlattenedImageDataSlotForDepthData(curPoint, imageWidth);

	if (depthDataIndex >= 0 && depthDataIndex < imageDepthData.size() && imageDepthData[depthDataIndex] < depth)
	{
		//std::cout << "Pixel has passed depth test." << std::endl;
		float cutOffValueFloat = 1.0f;
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
				//Colour texelColour = colourTextureMixFactor < 1.0f ? GetColourFromTexCoord(*curTex, texCoord) : colour_black;
				Colour texelColour = GetColourFromTexCoord(*curTex, texCoord);
				float r = ((1.0f - colourTextureMixFactor) * texelColour.r + (colourTextureMixFactor * curColour.r));
				float g = ((1.0f - colourTextureMixFactor) * texelColour.g + (colourTextureMixFactor * curColour.g));
				float b = ((1.0f - colourTextureMixFactor) * texelColour.b + (colourTextureMixFactor * curColour.b));

				//r = texCoord.x * 255;
				//g = texCoord.y * 255;
				//b = 0;

				//imageData[index + 0] = r;
				//imageData[index + 1] = g;
				//imageData[index + 2] = b;
				//imageData[index + 3] = 255;

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

int printRate = 1000;
int printCounter = 0;

void DrawTriangleOnScreenFromScreenSpaceBresenhamMethod(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData,
	int imageWidth, int imageHeight,
	int curTriangleIndex, int currentTextureIndex,
	const Triangle& drawTriangle, Vector3 lightDotTriangleNormals,
	Mat3x3& vertexWorldPositions,
	Vector3 invDepth, Vector3 invW,
	int lineThickness)
{
	PROFILE_FUNCTION();

	Triangle triangle = drawTriangle;

	// Sort points based on their y coordinates to end up with := [TopOnScreen = C, MiddleOnScreen = b, BottomOnScreen = a]
	if (triangle.a.position.y < triangle.b.position.y) {
		std::swap(triangle.a, triangle.b);
		std::swap(invDepth.x, invDepth.y);
		std::swap(invW.x, invW.y);
		std::swap(lightDotTriangleNormals.x, lightDotTriangleNormals.y);

		SwapVectorsInMat3x3(vertexWorldPositions, 0, 1);
	}
	if (triangle.a.position.y < triangle.c.position.y) {
		std::swap(triangle.a, triangle.c);
		std::swap(invDepth.x, invDepth.z);
		std::swap(invW.x, invW.z);
		std::swap(lightDotTriangleNormals.x, lightDotTriangleNormals.z);

		SwapVectorsInMat3x3(vertexWorldPositions, 0, 2);
	}
	if (triangle.b.position.y < triangle.c.position.y) {
		std::swap(triangle.b, triangle.c);
		std::swap(invDepth.y, invDepth.z);
		std::swap(invW.y, invW.z);
		std::swap(lightDotTriangleNormals.y, lightDotTriangleNormals.z);

		SwapVectorsInMat3x3(vertexWorldPositions, 1, 2);
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

	if (round(triangle.a.position.y) == round(triangle.b.position.y)) {
		BresenhamTriangleDrawer(triangle.c.position, triangle.a.position, triangle.b.position, imageWidth, lightDotTriangleNormals, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, vertexWorldPositions, triangle, colourTextureMixFactor, colour_red, false, curTex, imageData, imageDepthData);
	}
	else if (round(triangle.c.position.y) == round(triangle.b.position.y)) {
		BresenhamTriangleDrawer(triangle.a.position, triangle.c.position, triangle.b.position, imageWidth, lightDotTriangleNormals, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, vertexWorldPositions, triangle, colourTextureMixFactor, colour_red, false, curTex, imageData, imageDepthData);
	}
	else {
		{
			//Top triangle.
			BresenhamTriangleDrawer(triangle.c.position, triangle.b.position, d, imageWidth, lightDotTriangleNormals, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, vertexWorldPositions, triangle, colourTextureMixFactor, colour_red, false, curTex, imageData, imageDepthData);

		}

		{
			//Bottom triangle.
			BresenhamTriangleDrawer(triangle.a.position, triangle.b.position, d, imageWidth, lightDotTriangleNormals, deltaY, deltaX, deltaK, areaOfTriangle, invDepth, invW, texW, vertexWorldPositions, triangle, colourTextureMixFactor, colour_red, false, curTex, imageData, imageDepthData);
		}
	}


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


int TriangleClipAgainstPlane(const Plane& plane, const Triangle& in_tri, Vector3 inLightDotNormal, Mat3x3& interpWorldVertexPositions, std::vector<Triangle>& outputTriangles, std::vector<Vector3>& outLightDotNormal, std::vector<Mat3x3>& outWorldVertexPositions, bool test = false)
{
	PROFILE_FUNCTION();

	float da = DistanceFromPointToPlane(in_tri.a, plane);
	float db = DistanceFromPointToPlane(in_tri.b, plane);
	float dc = DistanceFromPointToPlane(in_tri.c, plane);

	if (da > 0.0f && db > 0.0f && dc > 0.0f) {
		outputTriangles.push_back(in_tri);
		outLightDotNormal.push_back(inLightDotNormal);
		outWorldVertexPositions.push_back(interpWorldVertexPositions);
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
			
			float dLightNormal = LerpFloat(inLightDotNormal.x, inLightDotNormal.z, ld);
			Vector3 dPosition = LerpVector3(GetVector3FromMat3x3(interpWorldVertexPositions, 0), GetVector3FromMat3x3(interpWorldVertexPositions, 2), ld);

			Point e;
			LinePlaneIntersection(in_tri.b.position, in_tri.c.position, plane, e.position);
			float le = glm::length(e.position - in_tri.b.position) / glm::length(in_tri.c.position - in_tri.b.position);
			e.texCoord = LerpVector3(in_tri.b.texCoord, in_tri.c.texCoord, le);
			e.colour = LerpVector4((in_tri.b.colour), (in_tri.c.colour), le);
			e.normal = LerpVector3((in_tri.b.normal), (in_tri.c.normal), le);

			float eLightNormal = LerpFloat(inLightDotNormal.y, inLightDotNormal.z, le);
			Vector3 ePosition = LerpVector3(GetVector3FromMat3x3(interpWorldVertexPositions, 1), GetVector3FromMat3x3(interpWorldVertexPositions, 2), le);

			// make two triangles := { a, b, d} & {d, b, e}
			Triangle clippedTriangleABD;
			clippedTriangleABD.a = in_tri.a;
			clippedTriangleABD.b = in_tri.b;
			clippedTriangleABD.c = d;

			Mat3x3 outWorldPositions1;
			SetVector3InMat3x3(outWorldPositions1, 0, GetVector3FromMat3x3(interpWorldVertexPositions, 0));
			SetVector3InMat3x3(outWorldPositions1, 1, GetVector3FromMat3x3(interpWorldVertexPositions, 1));
			SetVector3InMat3x3(outWorldPositions1, 2, dPosition);

			outLightDotNormal.push_back({ inLightDotNormal.x, inLightDotNormal.y, dLightNormal });
			outWorldVertexPositions.push_back(outWorldPositions1);

			Triangle clippedTriangleDBE;
			clippedTriangleDBE.a = d;
			clippedTriangleDBE.b = in_tri.b;
			clippedTriangleDBE.c = e;

			Mat3x3 outWorldPositions2;
			SetVector3InMat3x3(outWorldPositions2, 0, dPosition);
			SetVector3InMat3x3(outWorldPositions2, 1, GetVector3FromMat3x3(interpWorldVertexPositions, 1));
			SetVector3InMat3x3(outWorldPositions2, 2, ePosition);

			outLightDotNormal.push_back({ dLightNormal, inLightDotNormal.y, eLightNormal });
			outWorldVertexPositions.push_back(outWorldPositions2);

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

			float dLightNormal = LerpFloat(inLightDotNormal.x, inLightDotNormal.y, ld);
			Vector3 dPosition = LerpVector3(GetVector3FromMat3x3(interpWorldVertexPositions, 0), GetVector3FromMat3x3(interpWorldVertexPositions, 1), ld);

			Point e;
			LinePlaneIntersection(in_tri.c.position, in_tri.b.position, plane, e.position);
			float le = glm::length(e.position - in_tri.c.position) / glm::length(in_tri.b.position - in_tri.c.position);
			e.texCoord = LerpVector3(in_tri.c.texCoord, in_tri.b.texCoord, le);
			//e.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.c.colour), ColourToVector4(in_tri.b.colour), le));
			e.colour = (LerpVector4((in_tri.c.colour), (in_tri.b.colour), le));
			e.normal = (LerpVector3((in_tri.c.normal), (in_tri.b.normal), le));

			float eLightNormal = LerpFloat(inLightDotNormal.z, inLightDotNormal.y, le);
			Vector3 ePosition = LerpVector3(GetVector3FromMat3x3(interpWorldVertexPositions, 2), GetVector3FromMat3x3(interpWorldVertexPositions, 1), le);

			// make two triangles := {a, c, d} & {d, c, e}
			Triangle clippedTriangleACD;
			clippedTriangleACD.a = in_tri.a;
			clippedTriangleACD.b = in_tri.c;
			clippedTriangleACD.c = d;

			Mat3x3 outWorldPositions1;
			SetVector3InMat3x3(outWorldPositions1, 0, GetVector3FromMat3x3(interpWorldVertexPositions, 0));
			SetVector3InMat3x3(outWorldPositions1, 1, GetVector3FromMat3x3(interpWorldVertexPositions, 2));
			SetVector3InMat3x3(outWorldPositions1, 2, dPosition);

			outLightDotNormal.push_back({ inLightDotNormal.x, inLightDotNormal.z, dLightNormal });
			outWorldVertexPositions.push_back(outWorldPositions1);

			Triangle clippedTriangleDCE;
			clippedTriangleDCE.a = d;
			clippedTriangleDCE.b = in_tri.c;
			clippedTriangleDCE.c = e;

			Mat3x3 outWorldPositions2;
			SetVector3InMat3x3(outWorldPositions2, 0, dPosition);
			SetVector3InMat3x3(outWorldPositions2, 1, GetVector3FromMat3x3(interpWorldVertexPositions, 2));
			SetVector3InMat3x3(outWorldPositions2, 2, ePosition);

			outLightDotNormal.push_back({ dLightNormal, inLightDotNormal.z, eLightNormal });
			outWorldVertexPositions.push_back(outWorldPositions2);

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

			float dLightNormal = LerpFloat(inLightDotNormal.x, inLightDotNormal.y, ld);
			Vector3 dPosition = LerpVector3(GetVector3FromMat3x3(interpWorldVertexPositions, 0), GetVector3FromMat3x3(interpWorldVertexPositions, 1), ld);

			Point e;
			LinePlaneIntersection(in_tri.a.position, in_tri.c.position, plane, e.position);
			float le = glm::length(e.position - in_tri.a.position) / glm::length(in_tri.c.position - in_tri.a.position);
			e.texCoord = LerpVector3(in_tri.a.texCoord, in_tri.c.texCoord, le);
			//e.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.a.colour), ColourToVector4(in_tri.c.colour), le));
			e.colour = (LerpVector4((in_tri.a.colour), (in_tri.c.colour), le));
			e.normal = (LerpVector3((in_tri.a.normal), (in_tri.c.normal), le));

			float eLightNormal = LerpFloat(inLightDotNormal.x, inLightDotNormal.z, le);
			Vector3 ePosition = LerpVector3(GetVector3FromMat3x3(interpWorldVertexPositions, 0), GetVector3FromMat3x3(interpWorldVertexPositions, 2), le);

			// make one triangle := {a, d, e}
			Triangle clippedTriangleADE;
			clippedTriangleADE.a = in_tri.a;
			clippedTriangleADE.b = d;
			clippedTriangleADE.c = e;

			Mat3x3 outWorldPositions1;
			SetVector3InMat3x3(outWorldPositions1, 0, GetVector3FromMat3x3(interpWorldVertexPositions, 0));
			SetVector3InMat3x3(outWorldPositions1, 1, dPosition);
			SetVector3InMat3x3(outWorldPositions1, 2, ePosition);

			outLightDotNormal.push_back({ inLightDotNormal.x, dLightNormal, eLightNormal });
			outWorldVertexPositions.push_back(outWorldPositions1);

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

			float dLightNormal = LerpFloat(inLightDotNormal.y, inLightDotNormal.x, ld);
			Vector3 dPosition = LerpVector3(GetVector3FromMat3x3(interpWorldVertexPositions, 1), GetVector3FromMat3x3(interpWorldVertexPositions, 0), ld);


			Point e;
			LinePlaneIntersection(in_tri.c.position, in_tri.a.position, plane, e.position);
			float le = glm::length(e.position - in_tri.c.position) / glm::length(in_tri.a.position - in_tri.c.position);
			e.texCoord = LerpVector3(in_tri.c.texCoord, in_tri.a.texCoord, le);
			//e.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.c.colour), ColourToVector4(in_tri.a.colour), le));
			e.colour = (LerpVector4((in_tri.c.colour), (in_tri.a.colour), le));
			e.normal = (LerpVector3((in_tri.c.normal), (in_tri.a.normal), le));

			float eLightNormal = LerpFloat(inLightDotNormal.z, inLightDotNormal.x, le);
			Vector3 ePosition = LerpVector3(GetVector3FromMat3x3(interpWorldVertexPositions, 2), GetVector3FromMat3x3(interpWorldVertexPositions, 0), le);

			// make two triangles := {b, c, d} & {d, c, e}
			Triangle clippedTriangleBCD;
			clippedTriangleBCD.a = in_tri.b;
			clippedTriangleBCD.b = in_tri.c;
			clippedTriangleBCD.c = d;

			Mat3x3 outWorldPositions1;
			SetVector3InMat3x3(outWorldPositions1, 0, GetVector3FromMat3x3(interpWorldVertexPositions, 2));
			SetVector3InMat3x3(outWorldPositions1, 1, GetVector3FromMat3x3(interpWorldVertexPositions, 2));
			SetVector3InMat3x3(outWorldPositions1, 2, dPosition);

			outLightDotNormal.push_back({ inLightDotNormal.y, inLightDotNormal.z, dLightNormal });
			outWorldVertexPositions.push_back(outWorldPositions1);

			Triangle clippedTriangleDCE;
			clippedTriangleDCE.a = d;
			clippedTriangleDCE.b = in_tri.c;
			clippedTriangleDCE.c = e;

			Mat3x3 outWorldPositions2;
			SetVector3InMat3x3(outWorldPositions2, 0, dPosition);
			SetVector3InMat3x3(outWorldPositions2, 1, GetVector3FromMat3x3(interpWorldVertexPositions, 2));
			SetVector3InMat3x3(outWorldPositions2, 2, ePosition);

			outLightDotNormal.push_back({ dLightNormal, inLightDotNormal.z, eLightNormal });
			outWorldVertexPositions.push_back(outWorldPositions2);

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

			float dLightNormal = LerpFloat(inLightDotNormal.z, inLightDotNormal.y, ld);
			Vector3 dPosition = LerpVector3(GetVector3FromMat3x3(interpWorldVertexPositions, 2), GetVector3FromMat3x3(interpWorldVertexPositions, 1), ld);


			Point e;
			LinePlaneIntersection(in_tri.a.position, in_tri.b.position, plane, e.position);
			float le = glm::length(e.position - in_tri.a.position) / glm::length(in_tri.b.position - in_tri.a.position);
			e.texCoord = LerpVector3(in_tri.a.texCoord, in_tri.b.texCoord, le);
			//e.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.a.colour), ColourToVector4(in_tri.b.colour), le));
			e.colour = (LerpVector4((in_tri.a.colour), (in_tri.b.colour), le));
			e.normal = (LerpVector3((in_tri.a.normal), (in_tri.b.normal), le));

			float eLightNormal = LerpFloat(inLightDotNormal.x, inLightDotNormal.y, le);
			Vector3 ePosition = LerpVector3(GetVector3FromMat3x3(interpWorldVertexPositions, 0), GetVector3FromMat3x3(interpWorldVertexPositions, 1), le);

			// make two triangles := {b, d, e}
			Triangle clippedTriangleBDE;
			clippedTriangleBDE.a = in_tri.b;
			clippedTriangleBDE.b = d;
			clippedTriangleBDE.c = e;

			Mat3x3 outWorldPositions1;
			SetVector3InMat3x3(outWorldPositions1, 0, GetVector3FromMat3x3(interpWorldVertexPositions, 1));
			SetVector3InMat3x3(outWorldPositions1, 1, dPosition);
			SetVector3InMat3x3(outWorldPositions1, 2, ePosition);

			outLightDotNormal.push_back({ inLightDotNormal.y, dLightNormal, eLightNormal });
			outWorldVertexPositions.push_back(outWorldPositions1);

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

			float dLightNormal = LerpFloat(inLightDotNormal.x, inLightDotNormal.z, ld);
			Vector3 dPosition = LerpVector3(GetVector3FromMat3x3(interpWorldVertexPositions, 0), GetVector3FromMat3x3(interpWorldVertexPositions, 2), ld);


			Point e;
			LinePlaneIntersection(in_tri.b.position, in_tri.c.position, plane, e.position);
			float le = glm::length(e.position - in_tri.b.position) / glm::length(in_tri.c.position - in_tri.b.position);
			e.texCoord = LerpVector3(in_tri.b.texCoord, in_tri.c.texCoord, le);
			//e.colour = Vector4ToColour(LerpVector4(ColourToVector4(in_tri.b.colour), ColourToVector4(in_tri.c.colour), le));
			e.colour = (LerpVector4((in_tri.b.colour), (in_tri.c.colour), le));
			e.normal = (LerpVector3((in_tri.b.normal), (in_tri.c.normal), le));

			float eLightNormal = LerpFloat(inLightDotNormal.y, inLightDotNormal.z, le);
			Vector3 ePosition = LerpVector3(GetVector3FromMat3x3(interpWorldVertexPositions, 1), GetVector3FromMat3x3(interpWorldVertexPositions, 2), le);


			// make one triangles := {c, d, e}
			Triangle clippedTriangleCDE;
			clippedTriangleCDE.a = in_tri.c;
			clippedTriangleCDE.b = d;
			clippedTriangleCDE.c = e;

			Mat3x3 outWorldPositions1;
			SetVector3InMat3x3(outWorldPositions1, 0, GetVector3FromMat3x3(interpWorldVertexPositions, 2));
			SetVector3InMat3x3(outWorldPositions1, 1, dPosition);
			SetVector3InMat3x3(outWorldPositions1, 2, ePosition);

			outLightDotNormal.push_back({ inLightDotNormal.z, dLightNormal, eLightNormal });
			outWorldVertexPositions.push_back(outWorldPositions1);

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
	, int lineThickness, Colour lineColour, int& totalTrianglesRendered, bool debugDraw = false) {

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

	//PrintMat4x4Pos0(normalTransformMatrix);

	//std::cout << modelTriangle.a.normal.x << ", " << modelTriangle.a.normal.y << ", " << modelTriangle.a.normal.z << std::endl;
	//std::cout << transformedTriangle.a.normal.x << ", " << transformedTriangle.a.normal.y << ", " << transformedTriangle.a.normal.z << std::endl;

	ApplyTransformToTriangleNormals(transformedTriangle, normalTransformMatrix);
	transformedTriangle.a.normal.y *= -1.0f;
	transformedTriangle.b.normal.y *= -1.0f;
	transformedTriangle.c.normal.y *= -1.0f;
	//std::cout << transformedTriangle.a.normal.x << ", " << transformedTriangle.a.normal.y << ", " << transformedTriangle.a.normal.z << std::endl;

	//std::cout << modelTriangle.a.normal.x << ", " << modelTriangle.a.normal.y << ", " << modelTriangle.a.normal.z << std::endl;

	//transformedTriangle.a.texCoord.y *= -1.0f;
	//transformedTriangle.b.texCoord.y *= -1.0f;
	//transformedTriangle.c.texCoord.y *= -1.0f;

	Vector3 trianglePos = 0.33f * transformedTriangle.a.position + 0.33f * transformedTriangle.b.position + 0.33f * transformedTriangle.c.position;
	//Vector3 lightPos = { 150.0f, 50.0f, 150.0f };
	Vector3 lightPos = { 5.0f, -10.0f, -5.0f };
	Vector3 lightDirFromTriangleA = glm::normalize(lightPos - transformedTriangle.a.position);
	Vector3 lightDirFromTriangleB = glm::normalize(lightPos - transformedTriangle.b.position);
	Vector3 lightDirFromTriangleC = glm::normalize(lightPos - transformedTriangle.c.position);

	//Vector3 lightDirFromTriangleA = glm::normalize(lightPos - trianglePos);
	//Vector3 lightDirFromTriangleB = glm::normalize(lightPos - trianglePos);
	//Vector3 lightDirFromTriangleC = glm::normalize(lightPos - trianglePos);

	Vector3 triangleNorm = 0.33f * transformedTriangle.a.normal + 0.33f * transformedTriangle.b.normal + 0.33f * transformedTriangle.c.normal;

	//Vector3 trianglePosRelativeToCamera = cameraPosition - transformedTriangle.a.position;
	Vector3 trianglePosRelativeToCamera = transformedTriangle.a.position - cameraPosition;
	trianglePosRelativeToCamera = glm::normalize(trianglePosRelativeToCamera);

	//bool curTriangleIsVisibleFromCamera = glm::dot(normal, trianglePosRelativeToCamera) > 0.0f;
	bool curTriangleIsVisibleFromCamera = glm::dot(triangleNorm, trianglePosRelativeToCamera) < 0.0f;
	bool cameraCouldSeeTriangle = glm::dot(trianglePosRelativeToCamera, cameraDirection) > 0.0f;

	//std::cout << transformedTriangle.a.normal.x << ", " << transformedTriangle.a.normal.y << ", " << transformedTriangle.a.normal.z << std::endl;


	//if (cameraCouldSeeTriangle && curTriangleIsVisibleFromCamera)
	if (cameraCouldSeeTriangle && curTriangleIsVisibleFromCamera)
	{
		Mat3x3 worldVertexPositions = { transformedTriangle.a.position, transformedTriangle.b.position, transformedTriangle.c.position };

		//Mat4x4 projectionViewMatrix = projectionMatrix * viewMatrix;

		//float lightDotTriangleNormalA = glm::max(glm::dot(lightDirFromTriangleA, triangleNorm), 0.1f);
		//float lightDotTriangleNormalB = glm::max(glm::dot(lightDirFromTriangleB, triangleNorm), 0.1f);
		//float lightDotTriangleNormalC = glm::max(glm::dot(lightDirFromTriangleC, triangleNorm), 0.1f);
		float lightDotTriangleNormalA = glm::max(glm::dot(lightDirFromTriangleA, transformedTriangle.a.normal), 0.1f);
		float lightDotTriangleNormalB = glm::max(glm::dot(lightDirFromTriangleB, transformedTriangle.b.normal), 0.1f);
		float lightDotTriangleNormalC = glm::max(glm::dot(lightDirFromTriangleC, transformedTriangle.c.normal), 0.1f);

		Vector3 lightDotTriangleVertexNormal = { lightDotTriangleNormalA, lightDotTriangleNormalB, lightDotTriangleNormalC };
		//float lightDotTriangleNormal = glm::max(glm::dot(lightDirFromTriangle, normal), 0.1f);
		//std::cout << lightDotTriangleVertexNormal.x << ", " << lightDotTriangleVertexNormal.y << ", " << lightDotTriangleVertexNormal.z << std::endl;

		// Transform into view space.
		Vector4 viewTransformedA = { viewMatrix * Vector4{transformedTriangle.a.position, 1.0f} };
		Vector4 viewTransformedB = { viewMatrix * Vector4{transformedTriangle.b.position, 1.0f} };
		Vector4 viewTransformedC = { viewMatrix * Vector4{transformedTriangle.c.position, 1.0f} };

		// Clip Viewed Triangle against near plane, this could form two additional
		// additional triangles.
		int nClippedTriangles = 0;
		Triangle curLargeTriangle = Triangle{ {viewTransformedA, modelTriangle.a.texCoord, modelTriangle.a.colour, transformedTriangle.a.normal}, {viewTransformedB, modelTriangle.b.texCoord, modelTriangle.b.colour, transformedTriangle.b.normal}, {viewTransformedC, modelTriangle.c.texCoord, modelTriangle.c.colour, transformedTriangle.c.normal }, colour_white };

		std::vector<Triangle> zClippingOutputTriangles;
		std::vector<Vector3> zClippingLightDotTriangleVertexNormal;
		std::vector<Mat3x3> zClippingWorldVertexPositions;
		nClippedTriangles = TriangleClipAgainstPlane(planeNear, curLargeTriangle, lightDotTriangleVertexNormal, worldVertexPositions, zClippingOutputTriangles, zClippingLightDotTriangleVertexNormal, zClippingWorldVertexPositions);

		//if (nClippedTriangles != zClippingLightDotTriangleVertexNormal.size()) {
		//	std::cout << "NOT EQUAL!!!" << std::endl;
		//}

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

				zClippingLightDotTriangleVertexNormal[n].x *= invW.x;
				zClippingLightDotTriangleVertexNormal[n].y *= invW.y;
				zClippingLightDotTriangleVertexNormal[n].z *= invW.z;

				zClippingOutputTriangles[n].a.colour *= invW.x;
				zClippingOutputTriangles[n].b.colour *= invW.y;
				zClippingOutputTriangles[n].c.colour *= invW.z;

				zClippingOutputTriangles[n].a.normal *= invW.x;
				zClippingOutputTriangles[n].b.normal *= invW.y;
				zClippingOutputTriangles[n].c.normal *= invW.z;

				MultiplyFloatToVectorInMat3x3(zClippingWorldVertexPositions[n], 0, invW.x);
				MultiplyFloatToVectorInMat3x3(zClippingWorldVertexPositions[n], 1, invW.y);
				MultiplyFloatToVectorInMat3x3(zClippingWorldVertexPositions[n], 2, invW.z);

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
				std::vector<Vector3> bottomClippingLightDotTriangleNormal;
				std::vector<Mat3x3> bottomClippingWorldPositions;
				TriangleClipAgainstPlane(planeBottomScreenSpace, curLargeScreenSpaceTriangle, zClippingLightDotTriangleVertexNormal[n], zClippingWorldVertexPositions[n], bottomScreenPlaneClippingResult, bottomClippingLightDotTriangleNormal, bottomClippingWorldPositions);
				
				std::vector<Triangle> topScreenPlaneClippingResult;
				std::vector<Vector3> topClippingLightDotTriangleNormal;
				std::vector<Mat3x3> topClippingWorldPositions;
				for (int i = 0; i < bottomScreenPlaneClippingResult.size(); i++)
				{
					TriangleClipAgainstPlane(planeTopScreenSpace, bottomScreenPlaneClippingResult[i], bottomClippingLightDotTriangleNormal[i], bottomClippingWorldPositions[i], topScreenPlaneClippingResult, topClippingLightDotTriangleNormal, topClippingWorldPositions);
				}

				std::vector<Triangle> leftScreenPlaneClippingResult;
				std::vector<Vector3> leftClippingLightDotTriangleNormal;
				std::vector<Mat3x3> leftClippingWorldPositions;
				for (int i = 0; i < topScreenPlaneClippingResult.size(); i++)
				{
					TriangleClipAgainstPlane(planeLeftScreenSpace, topScreenPlaneClippingResult[i], topClippingLightDotTriangleNormal[i], topClippingWorldPositions[i], leftScreenPlaneClippingResult, leftClippingLightDotTriangleNormal, leftClippingWorldPositions);
				}

				std::vector<Triangle> rightScreenPlaneClippingResult;
				std::vector<Vector3> rightClippingLightDotTriangleNormal;
				std::vector<Mat3x3> rightClippingWorldPositions;
				for (int i = 0; i < leftScreenPlaneClippingResult.size(); i++)
				{
					TriangleClipAgainstPlane(planeRightScreenSpace, leftScreenPlaneClippingResult[i], leftClippingLightDotTriangleNormal[i], leftClippingWorldPositions[i], rightScreenPlaneClippingResult, rightClippingLightDotTriangleNormal, rightClippingWorldPositions);
				}
				//std::cout << "Drawing something." << std::endl;
				//DrawTriangleOnScreenFromScreenSpaceBoundingBoxMethod(imageData, imageDepthData, imageWidth, imageHeight, curTriangleIndex, currentTextureIndex, curLargeScreenSpaceTriangle, normal, invDepth, invW, lineThickness);
				for (int i = 0; i < rightScreenPlaneClippingResult.size(); i++)
				{
					totalTrianglesRendered++;
					DrawTriangleOnScreenFromScreenSpaceBresenhamMethod(imageData, imageDepthData, imageWidth, imageHeight, curTriangleIndex, currentTextureIndex, rightScreenPlaneClippingResult[i], rightClippingLightDotTriangleNormal[i],rightClippingWorldPositions[i], invDepth, invW, lineThickness);
				}
			}
		}
	}
}

void DrawMeshOnScreenFromWorldWithTransform(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight, Mesh& currentMesh, Mat4x4& modelMatrix, Vector3 cameraPosition, Vector3 cameraDirection, Mat4x4& viewMatrix, Mat4x4& projectionMatrix, int lineThickness, Colour lineColour, int& totalTrianglesRendered, bool debugDraw = false) {

	PROFILE_FUNCTION();

	for (int i = 0; i < currentMesh.triangles.size(); i++)
	{
		//std::cout << "READING MESH TEXTURE INDEX 0 : " << currentMesh.textureIndex << std::endl;
		DrawTriangleOnScreenFromWorldTriangleWithClipping(imageData, imageDepthData, imageWidth, imageHeight, i, currentMesh.textureIndex, currentMesh.triangles[i], modelMatrix, cameraPosition, cameraDirection, viewMatrix, projectionMatrix, lineThickness, lineColour, totalTrianglesRendered, debugDraw);
	}
}