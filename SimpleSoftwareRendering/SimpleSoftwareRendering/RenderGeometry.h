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
                                            , int lineThickness
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

    float depthA = projectedPointA.z;
    float depthB = projectedPointB.z;
    float depthC = projectedPointC.z;

    projectedPointA.x += 1.0f; projectedPointA.y += 1.0f;
    projectedPointB.x += 1.0f; projectedPointB.y += 1.0f;
    projectedPointC.x += 1.0f; projectedPointC.y += 1.0f;

    projectedPointA.x *= (0.5 * imageWidth); projectedPointA.y *= (0.5 * imageHeight);
    projectedPointB.x *= (0.5 * imageWidth); projectedPointB.y *= (0.5 * imageHeight);
    projectedPointC.x *= (0.5 * imageWidth); projectedPointC.y *= (0.5 * imageHeight);

    //projectedPointA.x = glm::max(projectedPointA.x, 0.0f);
    //projectedPointB.x = glm::max(projectedPointB.x, 0.0f);
    //projectedPointC.x = glm::max(projectedPointC.x, 0.0f);

    //projectedPointA.x = glm::min(projectedPointA.x, (float)imageWidth);
    //projectedPointB.x = glm::min(projectedPointB.x, (float)imageWidth);
    //projectedPointC.x = glm::min(projectedPointC.x, (float)imageWidth);

    //projectedPointA.y = glm::max(projectedPointA.y, 0.0f);
    //projectedPointB.y = glm::max(projectedPointB.y, 0.0f);
    //projectedPointC.y = glm::max(projectedPointC.y, 0.0f);

    //projectedPointA.y = glm::min(projectedPointA.y, (float)imageWidth);
    //projectedPointB.y = glm::min(projectedPointB.y, (float)imageWidth);
    //projectedPointC.y = glm::min(projectedPointC.y, (float)imageWidth);


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

    Colour modifiedColour = homogeneousTriangle.colour;
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
                    //std::cout << "Drawing this triangle." << std::endl;
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

void DrawTriangleOnScreenFromNDCSpace(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight
                                            , Triangle& triangle, Vector3 triangleNormal
                                            , int lineThickness)
{

    Vector3 projectedPointA = triangle.a.position;
    Vector3 projectedPointB = triangle.b.position;
    Vector3 projectedPointC = triangle.c.position;

    float depthA = projectedPointA.z;
    float depthB = projectedPointB.z;
    float depthC = projectedPointC.z;

    projectedPointA.x += 1.0f; projectedPointA.y += 1.0f;
    projectedPointB.x += 1.0f; projectedPointB.y += 1.0f;
    projectedPointC.x += 1.0f; projectedPointC.y += 1.0f;

    projectedPointA.x *= (0.5 * imageWidth); projectedPointA.y *= (0.5 * imageHeight);
    projectedPointB.x *= (0.5 * imageWidth); projectedPointB.y *= (0.5 * imageHeight);
    projectedPointC.x *= (0.5 * imageWidth); projectedPointC.y *= (0.5 * imageHeight);

    //projectedPointA.x = glm::max(projectedPointA.x, 0.0f);
    //projectedPointB.x = glm::max(projectedPointB.x, 0.0f);
    //projectedPointC.x = glm::max(projectedPointC.x, 0.0f);

    //projectedPointA.x = glm::min(projectedPointA.x, (float)imageWidth);
    //projectedPointB.x = glm::min(projectedPointB.x, (float)imageWidth);
    //projectedPointC.x = glm::min(projectedPointC.x, (float)imageWidth);

    //projectedPointA.y = glm::max(projectedPointA.y, 0.0f);
    //projectedPointB.y = glm::max(projectedPointB.y, 0.0f);
    //projectedPointC.y = glm::max(projectedPointC.y, 0.0f);

    //projectedPointA.y = glm::min(projectedPointA.y, (float)imageWidth);
    //projectedPointB.y = glm::min(projectedPointB.y, (float)imageWidth);
    //projectedPointC.y = glm::min(projectedPointC.y, (float)imageWidth);


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

    Colour modifiedColour = triangle.colour;
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
                    //std::cout << "Drawing this triangle." << std::endl;
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

    lineThickness = 0;
    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointA, projectedPointB, lineThickness, Colour{ 0, 0, 255, 255 });
    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointB, projectedPointC, lineThickness, Colour{ 0, 0, 255, 255 });
    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointC, projectedPointA, lineThickness, Colour{ 0, 0, 255, 255 });

    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMin.x, boundingBoxMin.y, 0.0f }, Vector3Int{ boundingBoxMax.x, boundingBoxMin.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMax.x, boundingBoxMin.y, 0.0f }, Vector3Int{ boundingBoxMax.x, boundingBoxMax.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMax.x, boundingBoxMax.y, 0.0f }, Vector3Int{ boundingBoxMin.x, boundingBoxMax.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMin.x, boundingBoxMax.y, 0.0f }, Vector3Int{ boundingBoxMin.x, boundingBoxMin.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
}

void DrawTriangleOnScreenFromScreenSpace(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight
                                            , Triangle& triangle, Vector3 triangleNormal, Vector3 depth
                                            , int lineThickness)
{

    Vector3 projectedPointA = triangle.a.position;
    Vector3 projectedPointB = triangle.b.position;
    Vector3 projectedPointC = triangle.c.position;

    float depthA = depth.z;
    float depthB = depth.z;
    float depthC = depth.z;

    Vector2Int boundingBoxMin = Vector2Int{ (int)std::min(projectedPointA.x, projectedPointB.x), (int)std::min(projectedPointA.y, projectedPointB.y) };
    Vector2Int boundingBoxMax = Vector2Int{ (int)std::max(projectedPointA.x, projectedPointB.x), (int)std::max(projectedPointA.y, projectedPointB.y) };

    boundingBoxMin = Vector2Int{ (int)std::min((float)boundingBoxMin.x, projectedPointC.x), (int)std::min((float)boundingBoxMin.y, projectedPointC.y) };
    boundingBoxMax = Vector2Int{ (int)std::max((float)boundingBoxMax.x, projectedPointC.x), (int)std::max((float)boundingBoxMax.y, projectedPointC.y) };

    Vector2 aFloat = projectedPointB - projectedPointA;
    Vector2 bFloat = projectedPointC - projectedPointB;
    Vector2 cFloat = projectedPointA - projectedPointC;

    float areaOfTriangle = (cFloat.x * aFloat.y - aFloat.x * cFloat.y);

    Vector3 lightDir = Vector3{ 1.0f, 1.0f, 1.0f };
    lightDir = glm::normalize(lightDir);
    float normDotLightDirMax = glm::max(glm::dot(triangleNormal, lightDir), 0.0f);

    Colour modifiedColour = triangle.colour;
    modifiedColour.r = modifiedColour.r * normDotLightDirMax;
    modifiedColour.g = modifiedColour.g * normDotLightDirMax;
    modifiedColour.b = modifiedColour.b * normDotLightDirMax;
    modifiedColour.a = modifiedColour.a;

    float biasEdgeValueFloat = -0.00001f;
    float biasEdgeg0Float = IsTopOrLeft(projectedPointB, projectedPointA) ? 0.0f : biasEdgeValueFloat;
    float biasEdgeg1Float = IsTopOrLeft(projectedPointC, projectedPointB) ? 0.0f : biasEdgeValueFloat;
    float biasEdgeg2Float = IsTopOrLeft(projectedPointA, projectedPointC) ? 0.0f : biasEdgeValueFloat;

    for (int y = boundingBoxMin.y; y <= boundingBoxMax.y; y++)
    {
        for (int x = boundingBoxMin.x; x <= boundingBoxMax.x; x++)
        {
            Vector2Int curPoint = Vector2Int{ x, y };

            Vector2 curPointFloat = Vector2{ x + 0.5f, y + 0.5f };
            Vector2 apFloat = curPointFloat - Vector2(projectedPointA);
            Vector2 bpFloat = curPointFloat - Vector2(projectedPointB);
            Vector2 cpFloat = curPointFloat - Vector2(projectedPointC);

            float crossAFloat = (aFloat.x * apFloat.y) - (apFloat.x * aFloat.y) + biasEdgeg0Float;
            float crossBFloat = (bFloat.x * bpFloat.y) - (bpFloat.x * bFloat.y) + biasEdgeg1Float;
            float crossCFloat = (cFloat.x * cpFloat.y) - (cpFloat.x * cFloat.y) + biasEdgeg2Float;

            float alpha = crossAFloat / areaOfTriangle;
            float beta = crossBFloat / areaOfTriangle;
            float gamma = crossCFloat / areaOfTriangle;

            float depth = ((alpha * depthA) + (beta * depthB) + (gamma * depthC));
            int depthDataIndex = GetFlattenedImageDataSlotForDepthData(curPoint, imageWidth);

            if (depthDataIndex >= 0 && depthDataIndex < imageDepthData.size() && imageDepthData[depthDataIndex] < depth)
            {
                float cutOffValueFloat = 0.0f;
                if ((crossAFloat >= cutOffValueFloat && crossBFloat >= cutOffValueFloat && crossCFloat >= cutOffValueFloat)
                  || crossAFloat <= cutOffValueFloat && crossBFloat <= cutOffValueFloat && crossCFloat <= cutOffValueFloat)
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

    //if (drawnNothingAfterPassingDepthTest) {
    //    lineThickness = 0;
    //    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointA, projectedPointB, lineThickness, black);
    //    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointB, projectedPointC, lineThickness, black);
    //    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointC, projectedPointA, lineThickness, black);

    //    DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMin.x, boundingBoxMin.y, 0.0f }, Vector3Int{ boundingBoxMax.x, boundingBoxMin.y, 0.0f }, lineThickness, black);
    //    DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMax.x, boundingBoxMin.y, 0.0f }, Vector3Int{ boundingBoxMax.x, boundingBoxMax.y, 0.0f }, lineThickness, black);
    //    DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMax.x, boundingBoxMax.y, 0.0f }, Vector3Int{ boundingBoxMin.x, boundingBoxMax.y, 0.0f }, lineThickness, black);
    //    DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMin.x, boundingBoxMax.y, 0.0f }, Vector3Int{ boundingBoxMin.x, boundingBoxMin.y, 0.0f }, lineThickness, black);

    //    if (!EnsureTriangleVerticesAreInClockwiseOrder(triangle)) {
    //        std::cout << "The not drawn triangle was not in the correct order!" << std::endl;
    //    }
    //}

    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMin.x, boundingBoxMin.y, 0.0f }, Vector3Int{ boundingBoxMax.x, boundingBoxMin.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMax.x, boundingBoxMin.y, 0.0f }, Vector3Int{ boundingBoxMax.x, boundingBoxMax.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMax.x, boundingBoxMax.y, 0.0f }, Vector3Int{ boundingBoxMin.x, boundingBoxMax.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMin.x, boundingBoxMax.y, 0.0f }, Vector3Int{ boundingBoxMin.x, boundingBoxMin.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
}

void DrawTriangleOnScreenFromProjectedTriangle(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight
                                            , HomogeneousTriangle& triangle, Vector3 triangleNormal
                                            , int lineThickness, Colour triangleColour
                                            , bool debugDraw = false)
{

    Vector4 projectedPointA = { triangle.a };
    Vector4 projectedPointB = { triangle.b };
    Vector4 projectedPointC = { triangle.c };

    if (debugDraw) {
        std::cout << "Projected points values := " << std::endl;
        Mat4x4 projectedPointsMat = Mat4x4{ projectedPointA, projectedPointB, projectedPointC, {0.0f, 0.0f, 0.0f, 1.0f} };
        PrintMat4x4(projectedPointsMat);
    }

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

void DrawWireFrameHomogeneousTriangle(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight, HomogeneousTriangle homogeneousTriangle, Vector3 triangleNormal, Mat4x4& projectionMatrix, int lineThickness, Colour triangleColour) {

    Vector4 projectedPointA = { homogeneousTriangle.a };
    Vector4 projectedPointB = { homogeneousTriangle.b };
    Vector4 projectedPointC = { homogeneousTriangle.c };

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

int TriangleClipAgainstPlane(Plane& plane, Triangle& in_tri, Triangle& out_tri1, Triangle& out_tri2, bool test = false)
{
    // Make sure plane normal is indeed normal
    plane.normal = glm::normalize(plane.normal);

    // Return signed shortest distance from point to plane, plane normal must be normalised
    auto dist = [&](Point& point)
    {
        //point = glm::normalize(point);
        return (plane.normal.x * point.position.x + plane.normal.y * point.position.y + plane.normal.z * point.position.z - glm::dot(plane.normal, plane.pointOnPlane.position));
    };

    // Create two temporary storage arrays to classify points either side of plane
    // If distance sign is positive, point lies on "inside" of plane
    Point* inside_points[3];  int nInsidePointCount = 0;
    Point* outside_points[3]; int nOutsidePointCount = 0;

    // Get signed distance of each point in triangle to plane
    float d0 = dist(in_tri.a);
    float d1 = dist(in_tri.b);
    float d2 = dist(in_tri.c);

    if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.a; }
    else { outside_points[nOutsidePointCount++] = &in_tri.a; }
    if (d1 >= 0) { inside_points[nInsidePointCount++] = &in_tri.b; }
    else { outside_points[nOutsidePointCount++] = &in_tri.b; }
    if (d2 >= 0) { inside_points[nInsidePointCount++] = &in_tri.c; }
    else { outside_points[nOutsidePointCount++] = &in_tri.c; }

    // Now classify triangle points, and break the input triangle into 
    // smaller output triangles if required. There are four possible
    // outcomes...

    if (nInsidePointCount == 0)
    {
        // All points lie on the outside of plane, so clip whole triangle
        // It ceases to exist

        // Test
        if (test) {
            out_tri1 = in_tri;
            out_tri1.colour = pink;

            return 1; // Test.
        }
        else {
            return 0; // No returned triangles are valid
        }
    }

    if (nInsidePointCount == 3)
    {
        // All points lie on the inside of plane, so do nothing
        // and allow the triangle to simply pass through
        if(test)
        {
            return 0; // Test
        }
        else {

            out_tri1 = in_tri;

            return 1; // Just the one returned original triangle is valid
        }
    }

    if (nInsidePointCount == 1 && nOutsidePointCount == 2)
    {
        if (test) {
            return 0;
        }
        // Triangle should be clipped. As two points lie outside
        // the plane, the triangle simply becomes a smaller triangle

        // Copy appearance info to new triangle
        out_tri1.colour = blue/*in_tri.colour*/;

        // The inside point is valid, so keep that...
        out_tri1.a = *inside_points[0];

        // but the two new points are at the locations where the 
        // original sides of the triangle (lines) intersect with the plane
        out_tri1.b.position = VectorIntersectPlane({*inside_points[0], *outside_points[0]}, plane);
        out_tri1.c.position = VectorIntersectPlane({*inside_points[0], *outside_points[1]}, plane);

        return 1; // Return the newly formed single triangle
    }

    if (nInsidePointCount == 2 && nOutsidePointCount == 1)
    {
        if (test) {
            return 0;
        }
        // Triangle should be clipped. As two points lie inside the plane,
        // the clipped triangle becomes a "quad". Fortunately, we can
        // represent a quad with two new triangles

        // Copy appearance info to new triangles
        out_tri1.colour = red/*in_tri.colour*/;

        out_tri2.colour = green/*in_tri.colour*/;

        // The first triangle consists of the two inside points and a new
        // point determined by the location where one side of the triangle
        // intersects with the plane
        out_tri1.a = *inside_points[0];
        out_tri1.b = *inside_points[1];
        out_tri1.c.position = VectorIntersectPlane({ *inside_points[0], *outside_points[0] }, plane);

        // The second triangle is composed of one of he inside points, a
        // new point determined by the intersection of the other side of the 
        // triangle and the plane, and the newly created point above

        out_tri2.a = *inside_points[1];
        out_tri2.b.position = VectorIntersectPlane({ *inside_points[1], *outside_points[0] }, plane);
        out_tri2.c = out_tri1.c;

        return 2; // Return two newly formed triangles which form a quad
    }
}

const float nearPlaneDistance = 2.1f;
const float screenWidth = 800.0f;
const float screenHeight = 600.0f;

Plane planeNear     = { {0.0f, 0.0f, 1.0f}, {{0.0f, 0.0f, nearPlaneDistance}} };    // CAMERA SPACE!!!

float bias = 0.0f;
Plane planeBottom   = { {0.0f, 1.0f, 0.0f }, {{ 0.0f, -(1.0f + bias) , 0.0f }} };             // NDC SPACE!!!!!!
Plane planeTop      = { { 0.0f, -1.0f, 0.0f }, {{ 0.0f, 1.0f + bias, 0.0f }} };
Plane planeLeft     = { { 1.0f, 0.0f, 0.0f }, {{ -(1.0f + bias), 0.0f, 0.0f }} };
Plane planeRight    = { { -1.0f, 0.0f, 0.0f }, {{ 1.0f + bias, 0.0f, 0.0f } } };

Plane planeBottomScreenSpace    = { {0.0f, 1.0f, 0.0f }, {{ 0.0f, 0.0f , 0.0f }} };             // SCREEN SPACE!!!!!!
Plane planeTopScreenSpace       = { { 0.0f, -1.0f, 0.0f }, {{ 0.0f, screenHeight - 1, 0.0f }} };
Plane planeLeftScreenSpace      = { { 1.0f, 0.0f, 0.0f }, {{ 0.0f, 0.0f, 0.0f }} };
Plane planeRightScreenSpace     = { { -1.0f, 0.0f, 0.0f }, {{ screenWidth - 1, 0.0f, 0.0f } } };

void DrawTriangleOnScreenFromWorldTriangleWithClipping(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight, Triangle modelTriangle, Mat4x4& modelMatrix, Vector3 cameraPosition, Vector3 cameraDirection, Mat4x4& viewMatrix, Mat4x4& projectionMatrix, int lineThickness, Colour lineColour, bool debugDraw = false) {

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

    if (cameraCouldSeeTriangle && curTriangleIsVisibleFromCamera)
    {
        //Mat4x4 projectionViewMatrix = projectionMatrix * viewMatrix;

        // Transform into view space.
        Vector4 viewTransformedA = { viewMatrix * Vector4{transformedTriangle.a.position, 1.0f} };
        Vector4 viewTransformedB = { viewMatrix * Vector4{transformedTriangle.b.position, 1.0f} };
        Vector4 viewTransformedC = { viewMatrix * Vector4{transformedTriangle.c.position, 1.0f} };

        // Clip Viewed Triangle against near plane, this could form two additional
        // additional triangles. 
        int nClippedTriangles = 0;
        Triangle clipped[2];
        Triangle curTriangle = Triangle{ {viewTransformedA}, {viewTransformedB}, {viewTransformedC }, white };
        nClippedTriangles = TriangleClipAgainstPlane(planeNear, curTriangle, clipped[0], clipped[1]);

        for (int n = 0; n < nClippedTriangles; n++)
        {
            //Transform into Homogeneous space
            Vector4 projectedPointA = { projectionMatrix * Vector4 {clipped[n].a.position, 1.0f} };
            Vector4 projectedPointB = { projectionMatrix * Vector4 {clipped[n].b.position, 1.0f} };
            Vector4 projectedPointC = { projectionMatrix * Vector4 {clipped[n].c.position, 1.0f} };

            //Transform into NDC space.
            projectedPointA = projectedPointA / projectedPointA.w;
            projectedPointB = projectedPointB / projectedPointB.w;
            projectedPointC = projectedPointC / projectedPointC.w;

            // Get depth for Z-Buffer
            Vector3 depth = Vector3{ projectedPointA.z, projectedPointB.z, projectedPointC.z };

            //Transform into Screen Space
            projectedPointA.x += 1.0f; projectedPointA.y += 1.0f;
            projectedPointB.x += 1.0f; projectedPointB.y += 1.0f;
            projectedPointC.x += 1.0f; projectedPointC.y += 1.0f;

            projectedPointA.x *= (0.5 * imageWidth); projectedPointA.y *= (0.5 * imageHeight);
            projectedPointB.x *= (0.5 * imageWidth); projectedPointB.y *= (0.5 * imageHeight);
            projectedPointC.x *= (0.5 * imageWidth); projectedPointC.y *= (0.5 * imageHeight);

            Triangle curLargeScreenSpaceTriangle = { projectedPointA, projectedPointB, projectedPointC, clipped[n].colour };

            std::queue<Triangle> listOfTrianglesToBeCheckeddForClippingAndRendered;

            listOfTrianglesToBeCheckeddForClippingAndRendered.push(curLargeScreenSpaceTriangle);
            int nNewTriangles = 1;

            bool drawOnlyClipped = false;

            for (int p = 0; p < 4; p++)
            {
                int nTrisToAdd = 0;
                while (nNewTriangles > 0)
                {
                    // Take triangle from front of queue
                    Triangle triangleToTest = listOfTrianglesToBeCheckeddForClippingAndRendered.front();
                    listOfTrianglesToBeCheckeddForClippingAndRendered.pop();
                    nNewTriangles--;

                    Triangle clippedSub[2];

                    // Clip it against a plane. We only need to test each 
                    // subsequent plane, against subsequent new triangles
                    // as all triangles after a plane clip are guaranteed
                    // to lie on the inside of the plane. I like how this
                    // comment is almost completely and utterly justified
                    if (p == 0) {
                        nTrisToAdd = TriangleClipAgainstPlane(planeBottomScreenSpace, triangleToTest, clippedSub[0], clippedSub[1], drawOnlyClipped);
                    }
                    else if (p == 1) {
                        nTrisToAdd = TriangleClipAgainstPlane(planeTopScreenSpace, triangleToTest, clippedSub[0], clippedSub[1], drawOnlyClipped);
                    }
                    else if (p == 2) {
                        nTrisToAdd = TriangleClipAgainstPlane(planeLeftScreenSpace, triangleToTest, clippedSub[0], clippedSub[1], drawOnlyClipped);
                    }
                    else if (p == 3) {
                        nTrisToAdd = TriangleClipAgainstPlane(planeRightScreenSpace, triangleToTest, clippedSub[0], clippedSub[1], drawOnlyClipped);
                    }

                    // Clipping may yield a variable number of triangles, so
                    // add these new ones to the back of the queue for subsequent
                    // clipping against next planes
                    for (int w = 0; w < nTrisToAdd; w++)
                        listOfTrianglesToBeCheckeddForClippingAndRendered.push(clippedSub[w]);
                }
                nNewTriangles = listOfTrianglesToBeCheckeddForClippingAndRendered.size();
            }

            while(listOfTrianglesToBeCheckeddForClippingAndRendered.size() > 0)
            {
                //std::cout << "Drawing something? " << std::endl;
                Triangle curTriangle = listOfTrianglesToBeCheckeddForClippingAndRendered.front();
                listOfTrianglesToBeCheckeddForClippingAndRendered.pop();
                DrawTriangleOnScreenFromScreenSpace(imageData, imageDepthData, imageWidth, imageHeight, curTriangle, normal, depth, lineThickness);
            }
        }
    }
    //else {
    //    // Transform into view space.
    //    Vector4 viewTransformedA = { viewMatrix * Vector4{transformedTriangle.a.position, 1.0f} };
    //    Vector4 viewTransformedB = { viewMatrix * Vector4{transformedTriangle.b.position, 1.0f} };
    //    Vector4 viewTransformedC = { viewMatrix * Vector4{transformedTriangle.c.position, 1.0f} };

    //    //Transform into Homogeneous space
    //    Vector4 projectedPointA = { projectionMatrix * viewTransformedA };
    //    Vector4 projectedPointB = { projectionMatrix * viewTransformedB };
    //    Vector4 projectedPointC = { projectionMatrix * viewTransformedC };

    //    //Transform into NDC space.
    //    projectedPointA = projectedPointA / projectedPointA.w;
    //    projectedPointB = projectedPointB / projectedPointB.w;
    //    projectedPointC = projectedPointC / projectedPointC.w;

    //    // Get depth for Z-Buffer
    //    Vector3 depth = Vector3{ projectedPointA.z, projectedPointB.z, projectedPointC.z };

    //    //Transform into Screen Space
    //    projectedPointA.x += 1.0f; projectedPointA.y += 1.0f;
    //    projectedPointB.x += 1.0f; projectedPointB.y += 1.0f;
    //    projectedPointC.x += 1.0f; projectedPointC.y += 1.0f;

    //    projectedPointA.x *= (0.5 * imageWidth); projectedPointA.y *= (0.5 * imageHeight);
    //    projectedPointB.x *= (0.5 * imageWidth); projectedPointB.y *= (0.5 * imageHeight);
    //    projectedPointC.x *= (0.5 * imageWidth); projectedPointC.y *= (0.5 * imageHeight);

    //    Triangle curTriangle = { projectedPointA, projectedPointB, projectedPointC, pink };
    //    DrawTriangleOnScreenFromScreenSpace(imageData, imageDepthData, imageWidth, imageHeight, curTriangle, normal, depth, lineThickness);
    //}
}

void DrawMeshOnScreenFromWorldWithTransform(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight, Mesh& currentMesh, Mat4x4& modelMatrix, Vector3 cameraPosition, Vector3 cameraDirection, Mat4x4& viewMatrix, Mat4x4& projectionMatrix, int lineThickness, Colour lineColour, bool debugDraw = false) {

    for (int i = 0; i < currentMesh.triangles.size(); i++)
    {
        DrawTriangleOnScreenFromWorldTriangleWithClipping(imageData, imageDepthData, imageWidth, imageHeight, currentMesh.triangles[i], modelMatrix, cameraPosition, cameraDirection, viewMatrix, projectionMatrix, lineThickness, lineColour, debugDraw);
    }
}