#pragma once

#include <queue>

#include "Instrumentor.h"

#include "Colour.h"
#include "Geometry.h"

int GetRedFlattenedImageDataSlotForPixel(Vector2Int pixelPos, int imageWidth) {
    return (pixelPos.x + pixelPos.y * imageWidth) * 4;
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

void DrawLineSegmentOnScreen(std::vector<unsigned char>& imageData, int imageWidth, Vector3Int a, Vector3Int b, int lineThickness, Colour lineColour) {

    PROFILE_FUNCTION();

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

void DrawLineSegmentOnScreenFromWorldLineSegment(std::vector<unsigned char>& imageData, int imageWidth, int imageHeight, LineSegment& worldLineSegment, Mat4x4& projectionMatrix, int lineThickness, Colour lineColour) {

    Vector4 projectedPointA = { projectionMatrix * Vector4(worldLineSegment.a.position, 1.0f) };
    Vector4 projectedPointB = { projectionMatrix * Vector4(worldLineSegment.b.position, 1.0f) };

    projectedPointA = projectedPointA / projectedPointA.w;
    projectedPointB = projectedPointB / projectedPointB.w;

    projectedPointA.x += 1.0f; projectedPointA.y += 1.0f;
    projectedPointB.x += 1.0f; projectedPointB.y += 1.0f;

    projectedPointA.x *= (0.5 * imageWidth); projectedPointA.y *= (0.5 * imageHeight);
    projectedPointB.x *= (0.5 * imageWidth); projectedPointB.y *= (0.5 * imageHeight);

    Vector3Int remappedPointA = projectedPointA;
    Vector3Int remappedPointB = projectedPointB;

    DrawLineSegmentOnScreen(imageData, imageWidth, remappedPointA, remappedPointB, lineThickness, lineColour);
}

bool IsTopOrLeft(Vector2Int start, Vector2Int end) {
    Vector2Int edge = end - start;

    bool isTopEdge = edge.y == 0 && edge.x > 0;
    bool isLeftEdge = edge.y < 0;

    return isTopEdge || isLeftEdge;
}

void DrawTriangleOnScreenFromPointsOfHomogeneousTriangle(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight
                                            , HomogeneousTriangle& homogeneousTriangle
                                            , Vector3 triangleNormal
                                            , Mat4x4& projectionMatrix
                                            , int lineThickness, Colour triangleColour
                                            , bool debugDraw = false)
{
    Vector4 projectedPointA = homogeneousTriangle.a;
    Vector4 projectedPointB = homogeneousTriangle.b;
    Vector4 projectedPointC = homogeneousTriangle.c;

    if (debugDraw) {
        std::cout << "Projected points values := " << std::endl;
        Mat4x4 projectedPointsMat = Mat4x4{ projectedPointA, projectedPointB, projectedPointC, {0.0f, 0.0f, 0.0f, 1.0f} };
        PrintMat4x4(projectedPointsMat);
    }

    projectedPointA = projectedPointA / projectedPointA.w;
    projectedPointB = projectedPointB / projectedPointB.w;
    projectedPointC = projectedPointC / projectedPointC.w;

    //Vector4 projectedPointA = Vector4{ worldTriangle.a.position, 1.0f };
    //Vector4 projectedPointB = Vector4{ worldTriangle.b.position, 1.0f };
    //Vector4 projectedPointC = Vector4{ worldTriangle.c.position, 1.0f };

    float depthA = projectedPointA.z;
    float depthB = projectedPointB.z;
    float depthC = projectedPointC.z;

    projectedPointA.x += 1.0f; projectedPointA.y += 1.0f;
    projectedPointB.x += 1.0f; projectedPointB.y += 1.0f;
    projectedPointC.x += 1.0f; projectedPointC.y += 1.0f;

    projectedPointA.x *= (0.5 * imageWidth); projectedPointA.y *= (0.5 * imageHeight);
    projectedPointB.x *= (0.5 * imageWidth); projectedPointB.y *= (0.5 * imageHeight);
    projectedPointC.x *= (0.5 * imageWidth); projectedPointC.y *= (0.5 * imageHeight);

    Vector2Int boundingBoxMin = Vector2Int{ (int)std::min(projectedPointA.x, projectedPointB.x), (int)std::min(projectedPointA.y, projectedPointB.y) };
    Vector2Int boundingBoxMax = Vector2Int{ (int)std::max(projectedPointA.x, projectedPointB.x), (int)std::max(projectedPointA.y, projectedPointB.y) };

    //Vector2 boundingBoxMinFloat = Vector2{ std::floor(std::min(projectedPointA.x, projectedPointB.x)), std::floor(std::min(projectedPointA.y, projectedPointB.y)) };
    //Vector2 boundingBoxMaxFloat = Vector2{ std::floor(std::max(projectedPointA.x, projectedPointB.x)), std::floor(std::max(projectedPointA.y, projectedPointB.y)) };

    boundingBoxMin = Vector2Int{ (int)std::min((float)boundingBoxMin.x, projectedPointC.x), (int)std::min((float)boundingBoxMin.y, projectedPointC.y) };
    boundingBoxMax = Vector2Int{ (int)std::max((float)boundingBoxMax.x, projectedPointC.x), (int)std::max((float)boundingBoxMax.y, projectedPointC.y) };

    //boundingBoxMinFloat = Vector2{ std::ceil(std::min((float)boundingBoxMinFloat.x, projectedPointC.x)), std::ceil(std::min((float)boundingBoxMinFloat.y, projectedPointC.y)) };
    //boundingBoxMaxFloat = Vector2{ std::ceil(std::max((float)boundingBoxMaxFloat.x, projectedPointC.x)), std::ceil(std::max((float)boundingBoxMaxFloat.y, projectedPointC.y)) };

    //boundingBoxMinFloat = Vector2{ std::ceil(std::min((float)boundingBoxMin.x, projectedPointC.x)), std::ceil(std::min((float)boundingBoxMin.y, projectedPointC.y)) };
    //boundingBoxMaxFloat = Vector2{ std::ceil(std::max((float)boundingBoxMax.x, projectedPointC.x)), std::ceil(std::max((float)boundingBoxMax.y, projectedPointC.y)) };

    //Vector2Int a = projectedPointB - projectedPointA;
    //Vector2Int b = projectedPointC - projectedPointB;
    //Vector2Int c = projectedPointA - projectedPointC;

    Vector2 aFloat = projectedPointB - projectedPointA;
    Vector2 bFloat = projectedPointC - projectedPointB;
    Vector2 cFloat = projectedPointA - projectedPointC;

    //float areaOfTriangle = (aFloat.x * cFloat.y - cFloat.x * aFloat.y);
    float areaOfTriangle = (cFloat.x * aFloat.y - aFloat.x * cFloat.y);
    //std::cout << areaOfTriangle << std::endl;

    Vector3 lightDir = Vector3{ 1.0f, 1.0f, 1.0f };
    lightDir = glm::normalize(lightDir);
    float normDotLightDirMax = glm::max(glm::dot(triangleNormal, lightDir), 0.0f);

    Colour modifiedColour = triangleColour;
    modifiedColour.r = modifiedColour.r * normDotLightDirMax;
    modifiedColour.g = modifiedColour.g * normDotLightDirMax;
    modifiedColour.b = modifiedColour.b * normDotLightDirMax;
    modifiedColour.a = modifiedColour.a;

    //int biasEdgeg0 = IsTopOrLeft(projectedPointB, projectedPointA) ? 0 : -1;
    //int biasEdgeg1 = IsTopOrLeft(projectedPointC, projectedPointB) ? 0 : -1;
    //int biasEdgeg2 = IsTopOrLeft(projectedPointA, projectedPointC) ? 0 : -1;

    float biasEdgeValueFloat = -0.0001f;
    float biasEdgeg0Float = IsTopOrLeft(projectedPointB, projectedPointA) ? 0.0f : biasEdgeValueFloat;
    float biasEdgeg1Float = IsTopOrLeft(projectedPointC, projectedPointB) ? 0.0f : biasEdgeValueFloat;
    float biasEdgeg2Float = IsTopOrLeft(projectedPointA, projectedPointC) ? 0.0f : biasEdgeValueFloat;

    //Vector2Int triangleCentre = (projectedPointA + projectedPointB + projectedPointC) / 3.0f;
    //int depthIndex = GetFlattenedImageDataSlotForDepthData(triangleCentre, imageWidth);

    //bool completelyCulled = true;

    for (int y = boundingBoxMin.y; y <= boundingBoxMax.y; y++)
        //for (int y = boundingBoxMinFloat.y; y <= boundingBoxMaxFloat.y; y++)
    {
        for (int x = boundingBoxMin.x; x <= boundingBoxMax.x; x++)
            //for (int x = boundingBoxMinFloat.x; x <= boundingBoxMaxFloat.x; x++)
        {
            //std::cout << "Here?" << std::endl;

            Vector2Int curPoint = Vector2Int{ x, y };
            //Vector2Int ap = curPoint - Vector2Int(projectedPointA);
            //Vector2Int bp = curPoint - Vector2Int(projectedPointB);
            //Vector2Int cp = curPoint - Vector2Int(projectedPointC);

            Vector2 curPointFloat = Vector2{ x + 0.5f, y + 0.5f };
            Vector2 apFloat = curPointFloat - Vector2(projectedPointA);
            Vector2 bpFloat = curPointFloat - Vector2(projectedPointB);
            Vector2 cpFloat = curPointFloat - Vector2(projectedPointC);

            //float crossA = (a.x * ap.y) - (ap.x * a.y) + biasEdgeg0;
            //float crossB = (b.x * bp.y) - (bp.x * b.y) + biasEdgeg1;
            //float crossC = (c.x * cp.y) - (cp.x * c.y) + biasEdgeg2;

            float crossAFloat = (aFloat.x * apFloat.y) - (apFloat.x * aFloat.y) + biasEdgeg0Float;
            float crossBFloat = (bFloat.x * bpFloat.y) - (bpFloat.x * bFloat.y) + biasEdgeg1Float;
            float crossCFloat = (cFloat.x * cpFloat.y) - (cpFloat.x * cFloat.y) + biasEdgeg2Float;

            float alpha = crossAFloat / areaOfTriangle;
            float beta = crossBFloat / areaOfTriangle;
            float gamma = crossCFloat / areaOfTriangle;

            float depth = ((alpha * depthA) + (beta * depthB) + (gamma * depthC));
            int depthDataIndex = GetFlattenedImageDataSlotForDepthData(curPoint, imageWidth);

            if (depthDataIndex >= 0 && depthDataIndex < imageDepthData.size() && imageDepthData[depthDataIndex] > depth)
            {
                //completelyCulled = false;
                //float cutOffValue = -80.0f;
                float cutOffValueFloat = 0.0f;
                //if (crossA >= cutOffValue && crossB >= cutOffValue && crossC >= cutOffValue) {
                if (crossAFloat >= cutOffValueFloat && crossBFloat >= cutOffValueFloat && crossCFloat >= cutOffValueFloat)
                {
                    int index = GetRedFlattenedImageDataSlotForPixel(curPoint, imageWidth);
                    if (index >= 0 && (index + 3) < imageData.size())
                    {
                        imageData[index + 0] = modifiedColour.r;
                        imageData[index + 1] = modifiedColour.g;
                        imageData[index + 2] = modifiedColour.b;
                        imageData[index + 3] = modifiedColour.a;
                    }

                    imageDepthData[depthDataIndex] = depth;
                }
            }
        }
    }

    //int missingPixels = 2;

    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointA, projectedPointB, missingPixels, modifiedColour);
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointB, projectedPointC, missingPixels, modifiedColour);
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointC, projectedPointA, missingPixels, modifiedColour);

    //if (completelyCulled) {
    //    lineThickness = 0;
    //    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointA, projectedPointB, lineThickness, Colour{ 0, 0, 255, 255 });
    //    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointB, projectedPointC, lineThickness, Colour{ 0, 0, 255, 255 });
    //    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointC, projectedPointA, lineThickness, Colour{ 0, 0, 255, 255 });
    //}

    //lineThickness = 0;
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointA, projectedPointB, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointB, projectedPointC, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointC, projectedPointA, lineThickness, Colour{ 0, 0, 255, 255 });

    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMin.x, boundingBoxMin.y, 0.0f }, Vector3Int{ boundingBoxMax.x, boundingBoxMin.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMax.x, boundingBoxMin.y, 0.0f }, Vector3Int{ boundingBoxMax.x, boundingBoxMax.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMax.x, boundingBoxMax.y, 0.0f }, Vector3Int{ boundingBoxMin.x, boundingBoxMax.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMin.x, boundingBoxMax.y, 0.0f }, Vector3Int{ boundingBoxMin.x, boundingBoxMin.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
}

bool ClipCullTriangle(const Vector4& projectedPointA, const Vector4& projectedPointB, const Vector4& projectedPointC)
{
    // cull tests
    if (projectedPointA.x > projectedPointA.w &&
        projectedPointB.x > projectedPointC.w &&
        projectedPointC.x > projectedPointC.w)
    {
        //std::cout << "Culled when +x = " << projectedPointA.x << ", " << projectedPointB.x << ", " << projectedPointC.x << ", w := " << projectedPointA.w << ", " << projectedPointB.w << ", " << projectedPointC.w << std::endl;
        return true;
    }
    if (projectedPointA.x < -projectedPointA.w &&
        projectedPointB.x < -projectedPointB.w &&
        projectedPointC.x < -projectedPointC.w)
    {
        //std::cout << "Culled when -x = " << projectedPointA.x << ", " << projectedPointB.x << ", " << projectedPointC.x << ", w := " << projectedPointA.w << ", " << projectedPointB.w << ", " << projectedPointC.w << std::endl;
        return true;
    }
    if (projectedPointA.y > projectedPointA.w &&
        projectedPointB.y > projectedPointB.w &&
        projectedPointC.y > projectedPointC.w)
    {
        //std::cout << "Culled when +y = " << projectedPointA.y << ", " << projectedPointB.y << ", " << projectedPointC.y << ", w := " << projectedPointA.w << ", " << projectedPointB.w << ", " << projectedPointC.w << std::endl;
        return true;
    }
    if (projectedPointA.y < -projectedPointA.w &&
        projectedPointB.y < -projectedPointB.w &&
        projectedPointC.y < -projectedPointC.w)
    {
        //std::cout << "Culled when -y = " << projectedPointA.y << ", " << projectedPointB.y << ", " << projectedPointC.y << ", w := " << projectedPointA.w << ", " << projectedPointB.w << ", " << projectedPointC.w << std::endl;
        return true;
    }
    if (projectedPointA.z > projectedPointA.w &&
        projectedPointB.z > projectedPointB.w &&
        projectedPointC.z > projectedPointC.w)
    {
        //std::cout << "Culled when +z = " << projectedPointA.z << ", " << projectedPointB.z << ", " << projectedPointC.z << ", w := " << projectedPointA.w << ", " << projectedPointB.w << ", " << projectedPointC.w << std::endl;
        return true;
    }
    if (projectedPointA.z < 0.0f &&
        projectedPointB.z < 0.0f &&
        projectedPointC.z < 0.0f)
    {
        //std::cout << "Culled when -z = " << projectedPointA.z << ", " << projectedPointB.z << ", " << projectedPointC.z << ", w := " << projectedPointA.w << ", " << projectedPointB.w << ", " << projectedPointC.w << std::endl;
        return true;
    }

    return false;
}

const float nearPlaneDistance = 0.1f;
const float screenWidth = 800.0f;
const float screenHeight = 600.0f;

Plane planeNear = { {0.0f, 0.0f, 1.0f}, {{0.0f, 0.0f, nearPlaneDistance}} };

void DrawTriangleOnScreenFromWorldTriangle(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight
                                            , Triangle& worldTriangle, Vector3 triangleNormal
                                            , Mat4x4& projectionMatrix
                                            , int lineThickness, Colour triangleColour
                                            , bool debugDraw = false)
{

    Vector4 projectedPointA = { projectionMatrix * Vector4{worldTriangle.a.position, 1.0f} };
    Vector4 projectedPointB = { projectionMatrix * Vector4{worldTriangle.b.position, 1.0f} };
    Vector4 projectedPointC = { projectionMatrix * Vector4{worldTriangle.c.position, 1.0f} };

    if (debugDraw) {
        std::cout << "Projected points values := " << std::endl;
        Mat4x4 projectedPointsMat = Mat4x4{ projectedPointA, projectedPointB, projectedPointC, {0.0f, 0.0f, 0.0f, 1.0f} };
        PrintMat4x4(projectedPointsMat);
    }

    projectedPointA = projectedPointA / projectedPointA.w;
    projectedPointB = projectedPointB / projectedPointB.w;
    projectedPointC = projectedPointC / projectedPointC.w;

    //Vector4 projectedPointA = Vector4{ worldTriangle.a.position, 1.0f };
    //Vector4 projectedPointB = Vector4{ worldTriangle.b.position, 1.0f };
    //Vector4 projectedPointC = Vector4{ worldTriangle.c.position, 1.0f };

    float depthA = projectedPointA.z;
    float depthB = projectedPointB.z;
    float depthC = projectedPointC.z;

    projectedPointA.x += 1.0f; projectedPointA.y += 1.0f;
    projectedPointB.x += 1.0f; projectedPointB.y += 1.0f;
    projectedPointC.x += 1.0f; projectedPointC.y += 1.0f;

    projectedPointA.x *= (0.5 * imageWidth); projectedPointA.y *= (0.5 * imageHeight);
    projectedPointB.x *= (0.5 * imageWidth); projectedPointB.y *= (0.5 * imageHeight);
    projectedPointC.x *= (0.5 * imageWidth); projectedPointC.y *= (0.5 * imageHeight);

    Vector2Int boundingBoxMin = Vector2Int{ (int)std::min(projectedPointA.x, projectedPointB.x), (int)std::min(projectedPointA.y, projectedPointB.y) };
    Vector2Int boundingBoxMax = Vector2Int{ (int)std::max(projectedPointA.x, projectedPointB.x), (int)std::max(projectedPointA.y, projectedPointB.y) };

    //Vector2 boundingBoxMinFloat = Vector2{ std::floor(std::min(projectedPointA.x, projectedPointB.x)), std::floor(std::min(projectedPointA.y, projectedPointB.y)) };
    //Vector2 boundingBoxMaxFloat = Vector2{ std::floor(std::max(projectedPointA.x, projectedPointB.x)), std::floor(std::max(projectedPointA.y, projectedPointB.y)) };

    boundingBoxMin = Vector2Int{ (int)std::min((float)boundingBoxMin.x, projectedPointC.x), (int)std::min((float)boundingBoxMin.y, projectedPointC.y) };
    boundingBoxMax = Vector2Int{ (int)std::max((float)boundingBoxMax.x, projectedPointC.x), (int)std::max((float)boundingBoxMax.y, projectedPointC.y) };

    //boundingBoxMinFloat = Vector2{ std::ceil(std::min((float)boundingBoxMinFloat.x, projectedPointC.x)), std::ceil(std::min((float)boundingBoxMinFloat.y, projectedPointC.y)) };
    //boundingBoxMaxFloat = Vector2{ std::ceil(std::max((float)boundingBoxMaxFloat.x, projectedPointC.x)), std::ceil(std::max((float)boundingBoxMaxFloat.y, projectedPointC.y)) };

    //boundingBoxMinFloat = Vector2{ std::ceil(std::min((float)boundingBoxMin.x, projectedPointC.x)), std::ceil(std::min((float)boundingBoxMin.y, projectedPointC.y)) };
    //boundingBoxMaxFloat = Vector2{ std::ceil(std::max((float)boundingBoxMax.x, projectedPointC.x)), std::ceil(std::max((float)boundingBoxMax.y, projectedPointC.y)) };

    //Vector2Int a = projectedPointB - projectedPointA;
    //Vector2Int b = projectedPointC - projectedPointB;
    //Vector2Int c = projectedPointA - projectedPointC;

    Vector2 aFloat = projectedPointB - projectedPointA;
    Vector2 bFloat = projectedPointC - projectedPointB;
    Vector2 cFloat = projectedPointA - projectedPointC;

    //float areaOfTriangle = (aFloat.x * cFloat.y - cFloat.x * aFloat.y);
    float areaOfTriangle = (cFloat.x * aFloat.y - aFloat.x * cFloat.y);
    //std::cout << areaOfTriangle << std::endl;

    Vector3 lightDir = Vector3{ 1.0f, 1.0f, 1.0f };
    lightDir = glm::normalize(lightDir);
    float normDotLightDirMax = glm::max(glm::dot(triangleNormal, lightDir), 0.0f);

    Colour modifiedColour = triangleColour;
    modifiedColour.r = modifiedColour.r * normDotLightDirMax;
    modifiedColour.g = modifiedColour.g * normDotLightDirMax;
    modifiedColour.b = modifiedColour.b * normDotLightDirMax;
    modifiedColour.a = modifiedColour.a;

    //int biasEdgeg0 = IsTopOrLeft(projectedPointB, projectedPointA) ? 0 : -1;
    //int biasEdgeg1 = IsTopOrLeft(projectedPointC, projectedPointB) ? 0 : -1;
    //int biasEdgeg2 = IsTopOrLeft(projectedPointA, projectedPointC) ? 0 : -1;

    float biasEdgeValueFloat = -0.0001f;
    float biasEdgeg0Float = IsTopOrLeft(projectedPointB, projectedPointA) ? 0.0f : biasEdgeValueFloat;
    float biasEdgeg1Float = IsTopOrLeft(projectedPointC, projectedPointB) ? 0.0f : biasEdgeValueFloat;
    float biasEdgeg2Float = IsTopOrLeft(projectedPointA, projectedPointC) ? 0.0f : biasEdgeValueFloat;

    //Vector2Int triangleCentre = (projectedPointA + projectedPointB + projectedPointC) / 3.0f;
    //int depthIndex = GetFlattenedImageDataSlotForDepthData(triangleCentre, imageWidth);

    //bool completelyCulled = true;

    for (int y = boundingBoxMin.y; y <= boundingBoxMax.y; y++)
        //for (int y = boundingBoxMinFloat.y; y <= boundingBoxMaxFloat.y; y++)
    {
        for (int x = boundingBoxMin.x; x <= boundingBoxMax.x; x++)
            //for (int x = boundingBoxMinFloat.x; x <= boundingBoxMaxFloat.x; x++)
        {
            //std::cout << "Here?" << std::endl;

            Vector2Int curPoint = Vector2Int{ x, y };
            //Vector2Int ap = curPoint - Vector2Int(projectedPointA);
            //Vector2Int bp = curPoint - Vector2Int(projectedPointB);
            //Vector2Int cp = curPoint - Vector2Int(projectedPointC);

            Vector2 curPointFloat = Vector2{ x + 0.5f, y + 0.5f };
            Vector2 apFloat = curPointFloat - Vector2(projectedPointA);
            Vector2 bpFloat = curPointFloat - Vector2(projectedPointB);
            Vector2 cpFloat = curPointFloat - Vector2(projectedPointC);

            //float crossA = (a.x * ap.y) - (ap.x * a.y) + biasEdgeg0;
            //float crossB = (b.x * bp.y) - (bp.x * b.y) + biasEdgeg1;
            //float crossC = (c.x * cp.y) - (cp.x * c.y) + biasEdgeg2;

            float crossAFloat = (aFloat.x * apFloat.y) - (apFloat.x * aFloat.y) + biasEdgeg0Float;
            float crossBFloat = (bFloat.x * bpFloat.y) - (bpFloat.x * bFloat.y) + biasEdgeg1Float;
            float crossCFloat = (cFloat.x * cpFloat.y) - (cpFloat.x * cFloat.y) + biasEdgeg2Float;

            float alpha = crossAFloat / areaOfTriangle;
            float beta = crossBFloat / areaOfTriangle;
            float gamma = crossCFloat / areaOfTriangle;

            float depth = ((alpha * depthA) + (beta * depthB) + (gamma * depthC));
            int depthDataIndex = GetFlattenedImageDataSlotForDepthData(curPoint, imageWidth);

            if (depthDataIndex >= 0 && depthDataIndex < imageDepthData.size() && imageDepthData[depthDataIndex] > depth)
            {
                //completelyCulled = false;
                //float cutOffValue = -80.0f;
                float cutOffValueFloat = 0.0f;
                //if (crossA >= cutOffValue && crossB >= cutOffValue && crossC >= cutOffValue) {
                if (crossAFloat >= cutOffValueFloat && crossBFloat >= cutOffValueFloat && crossCFloat >= cutOffValueFloat)
                {
                    int index = GetRedFlattenedImageDataSlotForPixel(curPoint, imageWidth);
                    if (index >= 0 && (index + 3) < imageData.size())
                    {
                        imageData[index + 0] = modifiedColour.r;
                        imageData[index + 1] = modifiedColour.g;
                        imageData[index + 2] = modifiedColour.b;
                        imageData[index + 3] = modifiedColour.a;
                    }

                    imageDepthData[depthDataIndex] = depth;
                }
            }
        }
    }

    //int missingPixels = 2;

    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointA, projectedPointB, missingPixels, modifiedColour);
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointB, projectedPointC, missingPixels, modifiedColour);
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointC, projectedPointA, missingPixels, modifiedColour);

    //if (completelyCulled) {
    //    lineThickness = 0;
    //    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointA, projectedPointB, lineThickness, Colour{ 0, 0, 255, 255 });
    //    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointB, projectedPointC, lineThickness, Colour{ 0, 0, 255, 255 });
    //    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointC, projectedPointA, lineThickness, Colour{ 0, 0, 255, 255 });
    //}

    //lineThickness = 0;
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointA, projectedPointB, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointB, projectedPointC, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointC, projectedPointA, lineThickness, Colour{ 0, 0, 255, 255 });

    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMin.x, boundingBoxMin.y, 0.0f }, Vector3Int{ boundingBoxMax.x, boundingBoxMin.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMax.x, boundingBoxMin.y, 0.0f }, Vector3Int{ boundingBoxMax.x, boundingBoxMax.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMax.x, boundingBoxMax.y, 0.0f }, Vector3Int{ boundingBoxMin.x, boundingBoxMax.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMin.x, boundingBoxMax.y, 0.0f }, Vector3Int{ boundingBoxMin.x, boundingBoxMin.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
}

void DrawWireFrame(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight, Triangle worldTriangle, Vector3 triangleNormal, Mat4x4& projectionMatrix, int lineThickness, Colour triangleColour) {

    Vector4 projectedPointA = { projectionMatrix * Vector4(worldTriangle.a.position, 1.0f) };
    Vector4 projectedPointB = { projectionMatrix * Vector4(worldTriangle.b.position, 1.0f) };
    Vector4 projectedPointC = { projectionMatrix * Vector4(worldTriangle.c.position, 1.0f) };

    projectedPointA = projectedPointA / projectedPointA.w;
    projectedPointB = projectedPointB / projectedPointB.w;
    projectedPointC = projectedPointC / projectedPointC.w;

    projectedPointA.x += 1.0f; projectedPointA.y += 1.0f;
    projectedPointB.x += 1.0f; projectedPointB.y += 1.0f;
    projectedPointC.x += 1.0f; projectedPointC.y += 1.0f;

    projectedPointA.x *= (0.5 * imageWidth); projectedPointA.y *= (0.5 * imageHeight);
    projectedPointB.x *= (0.5 * imageWidth); projectedPointB.y *= (0.5 * imageHeight);
    projectedPointC.x *= (0.5 * imageWidth); projectedPointC.y *= (0.5 * imageHeight);

    lineThickness = 0;
    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointA, projectedPointB, lineThickness, Colour{ 0, 0, 255, 255 });
    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointB, projectedPointC, lineThickness, Colour{ 0, 0, 255, 255 });
    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointC, projectedPointA, lineThickness, Colour{ 0, 0, 255, 255 });
}

Vector4 InterpolateZ(const Vector4& a, const Vector4& b, float t) {

    Vector4 newVector = Vector4(0.0f);
    newVector.z = a.z + t * (b.z - a.z);

    return newVector;
}

void ClipHomogeneousTriangleOneVertexInsideZ(const Vector4& insidePoint, const Vector4& outsidePoint1, const Vector4& outsidePoint2, const float clippingPoint, std::vector<HomogeneousTriangle>& trianglesToDraw) {

    // calculate alpha values for getting adjusted vertices
    const float alphaA = (clippingPoint - insidePoint.z) / (outsidePoint1.z - insidePoint.z);
    const float alphaB = (clippingPoint - insidePoint.z) / (outsidePoint2.z - insidePoint.z);
    // interpolate to get v0a and v0b
    const Vector4 intersectionA = InterpolateZ(insidePoint, outsidePoint1, alphaA);
    const Vector4 intersectionB = InterpolateZ(insidePoint, outsidePoint2, alphaB);

    // draw triangles
    HomogeneousTriangle curTriangle = { insidePoint, intersectionA, intersectionB };
    trianglesToDraw.push_back(curTriangle);
}

void ClipHomogeneousTriangleTwoVertexInsideZ(const Vector4& insidePoint1, const Vector4& insidePoint2, const Vector4& outsidePoint, const float clippingPoint, std::vector<HomogeneousTriangle>& trianglesToDraw) {

    // calculate alpha values for getting adjusted vertices
    const float alphaA = (clippingPoint - insidePoint1.z) / (outsidePoint.z - insidePoint1.z);
    const float alphaB = (clippingPoint - insidePoint2.z) / (outsidePoint.z - insidePoint2.z);
    // interpolate to get v0a and v0b
    const Vector4 intersectionA = InterpolateZ(insidePoint1, outsidePoint, alphaA);
    const Vector4 intersectionB = InterpolateZ(insidePoint2, outsidePoint, alphaB);
    // draw triangles
    HomogeneousTriangle curTriangle1 = { insidePoint1, intersectionA, intersectionB };
    HomogeneousTriangle curTriangle2 = { insidePoint1, intersectionB, insidePoint2  };
    trianglesToDraw.push_back(curTriangle1);
    trianglesToDraw.push_back(curTriangle2);

}

void DrawTriangleOnScreenFromWorldTriangleWithClipping(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight, Triangle modelTriangle, Mat4x4& modelMatrix, Vector3 cameraPosition, Vector3 cameraDirection, Mat4x4& projectionMatrix, int lineThickness, Colour lineColour, bool debugDraw = false) {

    //cameraPosition.y *= -1.0f;

    Triangle transformedTriangle = ApplyTransformToTriangle(modelTriangle, modelMatrix);
    transformedTriangle.a.position.y *= -1.0f;
    transformedTriangle.b.position.y *= -1.0f;
    transformedTriangle.c.position.y *= -1.0f;

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

    if (/*cameraCouldSeeTriangle && */curTriangleIsVisibleFromCamera)
    {
        Vector4 projectedPointA = { projectionMatrix * Vector4{transformedTriangle.a.position, 1.0f} };
        Vector4 projectedPointB = { projectionMatrix * Vector4{transformedTriangle.b.position, 1.0f} };
        Vector4 projectedPointC = { projectionMatrix * Vector4{transformedTriangle.c.position, 1.0f} };

        if (!ClipCullTriangle(projectedPointA, projectedPointB, projectedPointC)) {
            // near clipping tests
            std::vector<HomogeneousTriangle> trianglesToDrawAfterClippingZ;

            if (projectedPointA.z < nearPlaneDistance)
            {
                if (projectedPointB.z < nearPlaneDistance)
                {
                    ClipHomogeneousTriangleOneVertexInsideZ(projectedPointC, projectedPointA, projectedPointB, nearPlaneDistance, trianglesToDrawAfterClippingZ);
                }
                else if (projectedPointC.z < nearPlaneDistance)
                {
                    ClipHomogeneousTriangleOneVertexInsideZ(projectedPointB, projectedPointA, projectedPointC, nearPlaneDistance, trianglesToDrawAfterClippingZ);
                }
                else
                {
                    ClipHomogeneousTriangleTwoVertexInsideZ(projectedPointB, projectedPointC, projectedPointA, nearPlaneDistance, trianglesToDrawAfterClippingZ);
                }
            }
            else if (projectedPointB.z < nearPlaneDistance)
            {
                if (projectedPointC.z < nearPlaneDistance)
                {
                    ClipHomogeneousTriangleOneVertexInsideZ(projectedPointA, projectedPointB, projectedPointC, nearPlaneDistance, trianglesToDrawAfterClippingZ);
                }
                else
                {
                    ClipHomogeneousTriangleTwoVertexInsideZ(projectedPointA, projectedPointC, projectedPointB, nearPlaneDistance, trianglesToDrawAfterClippingZ);
                }
            }
            else if (projectedPointC.z < nearPlaneDistance)
            {
                ClipHomogeneousTriangleTwoVertexInsideZ(projectedPointA, projectedPointB, projectedPointC, nearPlaneDistance, trianglesToDrawAfterClippingZ);
            }
            else // no near clipping necessary
            {
                //std::cout << "No clipping necessary." << std::endl;
                HomogeneousTriangle curLargeTriangle = { projectedPointA, projectedPointB, projectedPointC };
                trianglesToDrawAfterClippingZ.push_back(curLargeTriangle);
            }

            for (int i = 0; i < trianglesToDrawAfterClippingZ.size(); i++)
            {
                //std::cout << "Drawing triangles." << std::endl;
                DrawTriangleOnScreenFromPointsOfHomogeneousTriangle(imageData, imageDepthData, imageWidth, imageHeight, trianglesToDrawAfterClippingZ[i], normal, projectionMatrix, lineThickness, lineColour, debugDraw);
            }
        }

        //DrawTriangleOnScreenFromWorldTriangle(imageData, imageDepthData, imageWidth, imageHeight, transformedTriangle, normal, projectionMatrix, lineThickness, lineColour);
    }
}

void DrawMeshOnScreenFromWorldWithTransform(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight, Mesh& currentMesh, Mat4x4& modelMatrix, Vector3 cameraPosition, Vector3 cameraDirection, Mat4x4& projectionMatrix, int lineThickness, Colour lineColour, bool debugDraw = false) {

    for (int i = 0; i < currentMesh.triangles.size(); i++)
    {
        DrawTriangleOnScreenFromWorldTriangleWithClipping(imageData, imageDepthData, imageWidth, imageHeight, currentMesh.triangles[i], modelMatrix, cameraPosition, cameraDirection, projectionMatrix, lineThickness, lineColour, debugDraw);
    }
}