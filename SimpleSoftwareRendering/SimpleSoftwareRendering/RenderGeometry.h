#pragma once

#include <queue>

#include "Instrumentor.h"

#include "Colour.h"
#include "Geometry.h"
#include "Model.h"

float LerpFloat(const float& a, const float& b, const float& t) {
    return ((1 - t) * a) + b * t;
}

Vector2 LerpVector2(const Vector2& a, const Vector2& b, const float& t) {
    return ((1 - t) * a) + b * t;
}

Vector4 LerpVector4(const Vector4& a, const Vector4& b, const float& t) {
    return ((1 - t) * a) + b * t;
}

int GetRedFlattenedImageDataSlotForPixel(Vector2Int pixelPos, int imageWidth) {
    return (pixelPos.x + pixelPos.y * imageWidth) * 4;
}

int GetFlattenedImageDataSlotForDepthData(Vector2Int pixelPos, int imageWidth) {
    return (pixelPos.x + pixelPos.y * imageWidth);
}

void FillSubPixels(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, Vector2Int pixelCentre, const float& depth, int halfSizeMinusOne, Colour colourToFillWith) {
    for (int x = -halfSizeMinusOne; x <= halfSizeMinusOne; x++)
    {
        for (int y = -halfSizeMinusOne; y <= halfSizeMinusOne; y++)
        {
            int depthIndex = GetFlattenedImageDataSlotForDepthData(Vector2Int{ pixelCentre.x + x, pixelCentre.y + y }, imageWidth);

            if (true || depthIndex >= 0 && depthIndex < imageDepthData.size() && imageDepthData[depthIndex] > depth) {
                int index = GetRedFlattenedImageDataSlotForPixel(Vector2Int{ pixelCentre.x + x, pixelCentre.y + y }, imageWidth);
                if (index >= 0 && (index + 3) < imageData.size()) {

                    imageData[index + 0] = colourToFillWith.r;
                    imageData[index + 1] = colourToFillWith.g;
                    imageData[index + 2] = colourToFillWith.b;
                    imageData[index + 3] = colourToFillWith.a;

                    //imageDepthData[depthIndex] = depth;
                }
            }

        }
    }
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

void DrawLineSegmentOnScreenWithDepth(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, Vector2Int a, Vector2Int b, Vector2 depthBounds, int lineThickness, Colour lineColour) {

    PROFILE_FUNCTION();

    //std::cout << "Drawing triangle by using scanline." << std::endl;

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

            float curDepth = LerpFloat(depthBounds.x, depthBounds.y, t);

            //std::cout << "Stuck here in first section." << std::endl;

            FillSubPixels(imageData, imageDepthData, imageWidth, Vector2Int{ y, x }, curDepth, lineThickness, lineColour);
        }
    }
    else
    {
        for (int x = x0; x <= x1; x++) {

            float t = (x - x0) / dx;
            int y = y0 + dy * t;

            float curDepth = LerpFloat(depthBounds.x, depthBounds.y, t);

            //std::cout << "Stuck here in second section." << ", dx := " << dx << ", x0 := " << x0 << ", x : = " << x << ", x1 : = " << x1 << std::endl;

            FillSubPixels(imageData, imageDepthData, imageWidth, Vector2Int{ x, y }, curDepth, lineThickness, lineColour);
        }
    }
}

void DrawTriangleScreenSpaceScanLine(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, Triangle triangleToDraw, const Vector3& normal, const Vector3& depth, int lineThickness) {

    // Set 'a' to be top vertex, followed by 'b' and then 'c'.
    if (triangleToDraw.a.position.y > triangleToDraw.b.position.y) {
        std::swap(triangleToDraw.a, triangleToDraw.b);
        if (triangleToDraw.a.position.y > triangleToDraw.c.position.y) {
            std::swap(triangleToDraw.c, triangleToDraw.a);
        }
    }
    else if (triangleToDraw.a.position.y > triangleToDraw.c.position.y) {
        std::swap(triangleToDraw.c, triangleToDraw.a);
    }

    // Set 'c' to be left vertex and 'b' to be the right vertex.
    if (triangleToDraw.c.position.x > triangleToDraw.b.position.x) {
        std::swap(triangleToDraw.c, triangleToDraw.b);
    }

    Vector3 lightDir = Vector3{ 1.0f, 1.0f, 1.0f };
    lightDir = glm::normalize(lightDir);
    float normDotLightDirMax = glm::max(glm::dot(normal, lightDir), 0.0f);

    Colour modifiedColour = triangleToDraw.colour;
    modifiedColour.r = modifiedColour.r * normDotLightDirMax;
    modifiedColour.g = modifiedColour.g * normDotLightDirMax;
    modifiedColour.b = modifiedColour.b * normDotLightDirMax;
    modifiedColour.a = modifiedColour.a;


    int ySeparatingPoint = triangleToDraw.b.position.y;
    float cutOffDeltaDenomMargin = 0.001f;

    // ---------------------- Flat bottom triangle -----------------------------
    float deltaXAB1 = triangleToDraw.b.position.x - triangleToDraw.a.position.x;
    float deltaXAC1 = triangleToDraw.c.position.x - triangleToDraw.a.position.x;

    float deltaYAB1 = triangleToDraw.b.position.y - triangleToDraw.a.position.y;
    float deltaYAC1 = triangleToDraw.c.position.y - triangleToDraw.a.position.y;

    if (deltaYAC1 > cutOffDeltaDenomMargin && deltaYAB1 > cutOffDeltaDenomMargin) {

        float deltaDepthAB1 = depth.y - depth.x;
        float deltaDepthAC1 = depth.z - depth.x;

        float deltaXByDeltaYAB1 = deltaXAB1 / deltaYAB1;
        float deltaXByDeltaYAC1 = deltaXAC1 / deltaYAC1;

        float deltaDepthByDeltaYAB1 = deltaXAB1 / deltaYAB1;
        float deltaDepthByDeltaYAC1 = deltaXAC1 / deltaYAC1;

        float curXStart = triangleToDraw.a.position.x;
        float curXEnd = triangleToDraw.a.position.x;

        float startDepth = depth.x;
        float endDepth = depth.x;

        int xSeparatingPoint = triangleToDraw.a.position.x + ((deltaYAB1 / deltaYAC1) * (triangleToDraw.c.position.x - triangleToDraw.a.position.x));

        for (int scanlineY = triangleToDraw.a.position.y; scanlineY <= ySeparatingPoint; scanlineY++)
        {
            //std::cout << "Drawing up triangle. \n\tCur X end:= " << curXEnd
            //    << "\n\t deltaXByDeltaYAC1 := " << deltaDepthByDeltaYAC1
            //    << "\n\t deltaYAC1 := " << deltaYAC1 << std::endl;
            DrawLineSegmentOnScreenWithDepth(imageData, imageDepthData, imageWidth, Vector2Int{ curXStart, scanlineY }, Vector2Int{ curXEnd, scanlineY }, Vector2{ startDepth, endDepth }, lineThickness, modifiedColour);

            curXStart += deltaXByDeltaYAB1;
            curXEnd += deltaXByDeltaYAC1;

            startDepth += deltaDepthByDeltaYAB1;
            endDepth += deltaDepthByDeltaYAC1;
        }
    }

    // ---------------------- Flat top triangle -----------------------------
    float deltaXAB2 = triangleToDraw.c.position.x - triangleToDraw.a.position.x;
    float deltaXAC2 = triangleToDraw.c.position.x - triangleToDraw.b.position.x;

    float deltaYAB2 = triangleToDraw.c.position.y - triangleToDraw.a.position.y;
    float deltaYAC2 = triangleToDraw.c.position.y - triangleToDraw.b.position.y;

    if (deltaYAC2 > cutOffDeltaDenomMargin && deltaYAB2 > cutOffDeltaDenomMargin) {

        float deltaDepthAB2 = depth.y - depth.x;
        float deltaDepthAC2 = depth.y - depth.z;

        float deltaXByDeltaYAB2 = deltaXAB2 / deltaYAB2;
        float deltaXByDeltaYAC2 = deltaXAC2 / deltaYAC2;

        float deltaDepthByDeltaYAB2 = deltaXAB2 / deltaYAB2;
        float deltaDepthByDeltaYAC2 = deltaXAC2 / deltaYAC2;

        float curXStart = triangleToDraw.c.position.x;
        float curXEnd = triangleToDraw.c.position.x;

        float startDepth = depth.z;
        float endDepth = depth.z;

        for (int scanlineY = triangleToDraw.c.position.y; scanlineY > ySeparatingPoint; scanlineY--)
        {
            //std::cout << "Drawing down triangle := " << scanlineY << std::endl;
            DrawLineSegmentOnScreenWithDepth(imageData, imageDepthData, imageWidth, Vector2Int{ curXStart, scanlineY }, Vector2Int{ curXEnd, scanlineY }, Vector2{ startDepth, endDepth }, lineThickness, modifiedColour);
            curXStart -= deltaXByDeltaYAB2;
            curXEnd -= deltaXByDeltaYAC2;

            startDepth -= deltaDepthByDeltaYAB2;
            endDepth -= deltaDepthByDeltaYAC2;

        }
    }
    //std::cout << "Finished drawing triangle." << std::endl;
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

int printRateCounter = 0;
int printRate = 5000;
std::string printString = "";

void DrawTriangleOnScreenFromScreenSpace(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight
                                            , int curTriangleIndex, int currentTextureIndex, const Triangle& drawTriangle, Vector3 triangleNormal, Vector3 depth
                                            , int lineThickness)
{

    Triangle triangle = drawTriangle;
    triangle.a.colour = Colours::blue;
    triangle.b.colour = Colours::red;
    triangle.c.colour = Colours::green;

    if (triangle.a.position.y > triangle.b.position.y) {
        //std::cout << "SORTING IS WRONG!!! A > B := " << triangle.a.position.y << ", " << triangle.b.position.y << std::endl;
        std::swap(triangle.a, triangle.b);
    }
    if (triangle.a.position.y > triangle.c.position.y) {
        //std::cout << "SORTING IS WRONG!!! A > C := " << triangle.a.position.y << ", " << triangle.c.position.y << std::endl;
        std::swap(triangle.a, triangle.c);
    }
    if (triangle.b.position.y > triangle.c.position.y) {
        //std::cout << "SORTING IS WRONG!!! B > C := " << triangle.b.position.y << ", " << triangle.c.position.y << std::endl;
        std::swap(triangle.b, triangle.c);
    }

    if (triangle.a.position.y > triangle.b.position.y) {
        std::cout << "SORTING IS WRONG!!! A > B := " << triangle.a.position.y << ", " << triangle.b.position.y << std::endl;
        //std::swap(triangle.a, triangle.b);
    }
    if (triangle.a.position.y > triangle.c.position.y) {
        std::cout << "SORTING IS WRONG!!! A > C := " << triangle.a.position.y << ", " << triangle.c.position.y << std::endl;
        //std::swap(triangle.a, triangle.c);
    }
    if (triangle.b.position.y > triangle.c.position.y) {
        std::cout << "SORTING IS WRONG!!! B > C := " << triangle.b.position.y << ", " << triangle.c.position.y << std::endl;
        //std::swap(triangle.b, triangle.c);
    }

    Point sortedPointXA = triangle.a;
    Point sortedPointXB = triangle.b;
    Point sortedPointXC = triangle.c;

    if (sortedPointXA.position.x > sortedPointXB.position.x) {
        //std::cout << "SORTING IS WRONG!!! A > B := " << triangle.a.position.y << ", " << triangle.b.position.y << std::endl;
        std::swap(sortedPointXA, sortedPointXB);
    }
    if (sortedPointXA.position.x > sortedPointXC.position.x) {
        //std::cout << "SORTING IS WRONG!!! A > C := " << triangle.a.position.y << ", " << triangle.c.position.y << std::endl;
        std::swap(sortedPointXA, sortedPointXC);
    }
    if (sortedPointXB.position.x > sortedPointXC.position.x) {
        //std::cout << "SORTING IS WRONG!!! B > C := " << triangle.b.position.y << ", " << triangle.c.position.y << std::endl;
        std::swap(sortedPointXB, sortedPointXC);
    }

    if (sortedPointXA.position.x > sortedPointXB.position.x) {
        std::cout << "SORTING IS WRONG!!! Ax > Bx := " << sortedPointXA.position.x << ", " << sortedPointXB.position.x << std::endl;
        //std::swap(triangle.a, triangle.b);
    }
    if (sortedPointXA.position.x > sortedPointXC.position.x) {
        std::cout << "SORTING IS WRONG!!! Ax > Cx := " << sortedPointXA.position.x << ", " << sortedPointXC.position.x << std::endl;
        //std::swap(triangle.a, triangle.c);
    }
    if (sortedPointXB.position.x > sortedPointXC.position.x) {
        std::cout << "SORTING IS WRONG!!! Bx > Cx := " << sortedPointXB.position.x << ", " << sortedPointXC.position.x << std::endl;
        //std::swap(triangle.b, triangle.c);
    }

    Vector4 colourC0 = { triangle.a.colour.r, triangle.a.colour.g, triangle.a.colour.b, triangle.a.colour.a };
    Vector4 colourC1 = { triangle.b.colour.r, triangle.b.colour.g, triangle.b.colour.b, triangle.b.colour.a };
    Vector4 colourC2 = { triangle.c.colour.r, triangle.c.colour.g, triangle.c.colour.b, triangle.c.colour.a };

    Vector3 projectedPointA = triangle.a.position;
    Vector3 projectedPointB = triangle.b.position;
    Vector3 projectedPointC = triangle.c.position;

    float depthA = depth.z;
    float depthB = depth.z;
    float depthC = depth.z;


    Vector2 aFloat = projectedPointB - projectedPointA;
    Vector2 bFloat = projectedPointC - projectedPointB;
    Vector2 cFloat = projectedPointA - projectedPointC;

    float areaOfTriangle = (cFloat.x * aFloat.y - aFloat.x * cFloat.y) * 0.5f;

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

    float dXdYABTexCoord = (triangle.b.texCoord.x - triangle.a.texCoord.x) / (triangle.b.texCoord.y - triangle.a.texCoord.y);
    float dXdYACTexCoord = (triangle.c.texCoord.x - triangle.a.texCoord.x) / (triangle.c.texCoord.y - triangle.a.texCoord.y);



    int divisionPointY = triangle.b.position.y;

    float x1 = triangle.b.position.x;
    float x2 = (((triangle.a.position.x - triangle.c.position.x) / (triangle.a.position.y - triangle.c.position.y)) * (triangle.b.position.y - triangle.c.position.y)) + triangle.c.position.x;

    float divisionYLerpParam = ((triangle.b.position.y - triangle.a.position.y) / ((triangle.c.position.y - triangle.a.position.y) + 0.001f));
    Vector4 colourDivisionPoint = LerpVector4(colourC0, colourC2, divisionYLerpParam);
    Vector2 texCoordsAtDivisionPoint = LerpVector2(triangle.a.texCoord, triangle.c.texCoord, divisionYLerpParam);

    float minX = std::min(x1, x2);
    float maxX = std::max(x1, x2);

    //colourC1 = (minX == x1) ? colourC1 : colourC2;
    //colourC2 = (maxX == x2) ? colourC2 : colourC1;

    Texture* curTex = &Model::textures[currentTextureIndex];
    //float colourTextureMixFactor = 1.0f;
    float colourTextureMixFactor = 1.0f;

    //if (printRateCounter % printRate == 0) {
    //    std::cout << "Before drawing anything := " << std::endl;
    //    int randIndex = printRateCounter;
    //    PrintThisTriangleInfo(triangle, randIndex);
    //}

    int smallerTriangle = -1;
    float lengthDiffCB = (triangle.c.position.y - triangle.b.position.y);
    float lengthDiffBA = (triangle.a.position.y - triangle.b.position.y);
    if (abs(lengthDiffBA) > abs(lengthDiffCB)) {
        smallerTriangle = 0;//top triangle is smaller.
    }
    else if (abs(lengthDiffBA) < abs(lengthDiffCB)) {
        smallerTriangle = 1; //bottom triangle is smaller.
    }

    // DRAW TOP TRIANGLE.
    {
        float startScanlineX = minX;
        float endScanlineX = maxX;

        float slopeX1 = (x1 - triangle.c.position.x) / (triangle.b.position.y - triangle.c.position.y);
        float slopeX2 = (x2 - triangle.c.position.x) / (triangle.b.position.y - triangle.c.position.y);

        float appropriateSlope1 = (minX == x1) ? slopeX1 : slopeX2;
        float appropriateSlope2 = (maxX == x2) ? slopeX2 : slopeX1;

        for (int y = divisionPointY; y <= triangle.c.position.y; y++)
        {
            //float tY = ((y - triangle.a.position.y) / ((triangle.c.position.y - triangle.a.position.y) + 0.001f));
            //float tY = ((y - divisionPointY) / ((triangle.c.position.y - divisionPointY) + 0.001f));
            float tY2 = ((y - triangle.a.position.y) / ((triangle.c.position.y - triangle.a.position.y) + 0.001f));

            Vector4 startColourForThisScanLine = LerpVector4(colourC1, colourC2, tY2);
            Vector4 endColourForThisScanLine = LerpVector4(colourC0, colourC2, tY2);

            Vector2 startTexCoordForThisScanLine = LerpVector2(triangle.b.texCoord, triangle.c.texCoord, tY2);
            Vector2 endTexCoordForThisScanLine = LerpVector2(texCoordsAtDivisionPoint, triangle.c.texCoord, tY2);
            

            //for (int x = boundingBoxMin.x; x <= boundingBoxMax.x; x++)
            for (int x = startScanlineX; x <= endScanlineX; x++)
            {
                Vector2Int curPoint = Vector2Int{ x, y };

                Vector2 curPointFloat = Vector2{ x + 0.5f, y + 0.5f };
                Vector2 apFloat = curPointFloat - Vector2(projectedPointA);
                Vector2 bpFloat = curPointFloat - Vector2(projectedPointB);
                Vector2 cpFloat = curPointFloat - Vector2(projectedPointC);

                float crossAFloat = (aFloat.x * apFloat.y) - (apFloat.x * aFloat.y);
                float crossBFloat = (bFloat.x * bpFloat.y) - (bpFloat.x * bFloat.y);
                float crossCFloat = (cFloat.x * cpFloat.y) - (cpFloat.x * cFloat.y);

                float crossAFloatBias = crossAFloat + biasEdgeg0Float;
                float crossBFloatBias = crossBFloat + biasEdgeg1Float;
                float crossCFloatBias = crossCFloat + biasEdgeg2Float;

                float alpha = crossAFloat / areaOfTriangle;
                float beta = crossBFloat / areaOfTriangle;
                float gamma = crossCFloat / areaOfTriangle;

                //std::cout << "alpha := " << alpha << ", beta := " << beta << ", gamma := " << gamma << std::endl;

                float depth = ((alpha * depthA) + (beta * depthB) + (gamma * depthC));
                int depthDataIndex = GetFlattenedImageDataSlotForDepthData(curPoint, imageWidth);

                if (depthDataIndex >= 0 && depthDataIndex < imageDepthData.size() && imageDepthData[depthDataIndex] < depth)
                {
                    {
                        int index = GetRedFlattenedImageDataSlotForPixel(curPoint, imageWidth);
                        if (index >= 0 && (index + 3) < imageData.size())
                        {
                            float tX = (x - startScanlineX) / ((endScanlineX - startScanlineX) + 0.001f);


                            Vector4 curColour = LerpVector4(startColourForThisScanLine, endColourForThisScanLine, tX);
                            Vector2 curTexCoord = LerpVector2(startTexCoordForThisScanLine, endTexCoordForThisScanLine, tX);

                            //curColour = { 255, 255, 255, 255 };


                            Colour texelColour = GetColourFromTexCoord(*curTex, curTexCoord);
                            float r = ((1.0f - colourTextureMixFactor) * texelColour.r + (colourTextureMixFactor) * curColour.r);
                            float g = ((1.0f - colourTextureMixFactor) * texelColour.g + (colourTextureMixFactor) * curColour.g);
                            float b = ((1.0f - colourTextureMixFactor) * texelColour.b + (colourTextureMixFactor) * curColour.b);

                            //if (/*smallerTriangle == 0 || */tX < 0.0f) {
                            //    r = 255;
                            //    g = 255;
                            //    b = 0;
                            //}

                            //r = tX * 255;
                            //g = tY2 * 255;
                            //b = 0;

                            imageData[index + 0] = r;
                            imageData[index + 1] = g;
                            imageData[index + 2] = b;
                            imageData[index + 3] = 255;


                        }

                        imageDepthData[depthDataIndex] = depth;
                    }
                }

            }
            startScanlineX += appropriateSlope1;
            endScanlineX += appropriateSlope2;
        }
    }

    //if (printRateCounter % printRate == 0) {
    //    std::cout << "After drawing top triangle := " << std::endl;
    //    int randIndex = printRateCounter;
    //    PrintThisTriangleInfo(triangle, randIndex);
    //}

    // DRAW BOTTOM TRIANGLE.
    {
        float startScanlineX = minX;
        float endScanlineX = maxX;

        float slopeX1 = (triangle.a.position.x - triangle.b.position.x) / (triangle.a.position.y - triangle.b.position.y);
        float slopeX2 = (triangle.a.position.x - x2) / (triangle.a.position.y - triangle.b.position.y);

        float appropriateSlope1 = (minX == x1) ? slopeX1 : slopeX2;
        float appropriateSlope2 = (maxX == x2) ? slopeX2 : slopeX1;

        for (int y = divisionPointY; y > triangle.a.position.y; y--)
        {
            //float tY = ((y - triangle.a.position.y) / ((triangle.c.position.y - triangle.a.position.y) + 0.001f));
            float tY = ((divisionPointY - y) / ((divisionPointY - triangle.a.position.y) + 0.001f));
            float tY2 = ((y - triangle.a.position.y) / ((triangle.c.position.y - triangle.a.position.y) + 0.001f));

            Vector4 startColourForThisScanLine = LerpVector4(colourC1, colourC2, tY2);
            Vector4 endColourForThisScanLine = LerpVector4(colourC0, colourC2, tY2);

            Vector2 startTexCoordForThisScanLine = LerpVector2(triangle.b.texCoord, triangle.a.texCoord, tY2);
            Vector2 endTexCoordForThisScanLine = LerpVector2(texCoordsAtDivisionPoint, triangle.a.texCoord, tY2);

            //for (int x = boundingBoxMin.x; x <= boundingBoxMax.x; x++)
            for (int x = startScanlineX; x <= endScanlineX; x++)
            {
                Vector2Int curPoint = Vector2Int{ x, y };

                Vector2 curPointFloat = Vector2{ x + 0.5f, y + 0.5f };
                Vector2 apFloat = curPointFloat - Vector2(projectedPointA);
                Vector2 bpFloat = curPointFloat - Vector2(projectedPointB);
                Vector2 cpFloat = curPointFloat - Vector2(projectedPointC);

                float crossAFloat = (aFloat.x * apFloat.y) - (apFloat.x * aFloat.y);
                float crossBFloat = (bFloat.x * bpFloat.y) - (bpFloat.x * bFloat.y);
                float crossCFloat = (cFloat.x * cpFloat.y) - (cpFloat.x * cFloat.y);

                float crossAFloatBias = crossAFloat + biasEdgeg0Float;
                float crossBFloatBias = crossBFloat + biasEdgeg1Float;
                float crossCFloatBias = crossCFloat + biasEdgeg2Float;

                float alpha = crossAFloat / areaOfTriangle;
                float beta = crossBFloat / areaOfTriangle;
                float gamma = crossCFloat / areaOfTriangle;

                //std::cout << "alpha := " << alpha << ", beta := " << beta << ", gamma := " << gamma << std::endl;

                float depth = ((alpha * depthA) + (beta * depthB) + (gamma * depthC));
                int depthDataIndex = GetFlattenedImageDataSlotForDepthData(curPoint, imageWidth);

                if (depthDataIndex >= 0 && depthDataIndex < imageDepthData.size() && imageDepthData[depthDataIndex] < depth)
                {
                    {
                        int index = GetRedFlattenedImageDataSlotForPixel(curPoint, imageWidth);
                        if (index >= 0 && (index + 3) < imageData.size())
                        {
                            //float tX = (x) / ((boundingBoxMax.x - boundingBoxMin.x) + 0.001f);;
                            float tX = (x - startScanlineX) / ((endScanlineX - startScanlineX) + 0.001f);

                            Vector4 curColour = LerpVector4(startColourForThisScanLine, endColourForThisScanLine, tX);
                            Vector2 curTexCoord = LerpVector2(startTexCoordForThisScanLine, endTexCoordForThisScanLine, tX);

                            //curColour = { 255, 255, 255, 255 };

                            Colour texelColour = GetColourFromTexCoord(*curTex, curTexCoord);
                            float r = ((1.0f - colourTextureMixFactor) * texelColour.r + (colourTextureMixFactor)*curColour.r);
                            float g = ((1.0f - colourTextureMixFactor) * texelColour.g + (colourTextureMixFactor)*curColour.g);
                            float b = ((1.0f - colourTextureMixFactor) * texelColour.b + (colourTextureMixFactor)*curColour.b);

                            //if (/*smallerTriangle == 1 ||*/ tX < 0.0f) {
                            //    r = 255;
                            //    g = 255;
                            //    b = 0;
                            //}

                            //r = tX * 255;
                            //g = tY2 * 255;
                            //b = 0;

                            imageData[index + 0] = r;
                            imageData[index + 1] = g;
                            imageData[index + 2] = b;
                            imageData[index + 3] = 255;
                        }

                        imageDepthData[depthDataIndex] = depth;
                    }
                }

            }
            startScanlineX -= appropriateSlope1;
            endScanlineX -= appropriateSlope2;
        }
    }

    //if (printRateCounter % printRate == 0) {
    //    std::cout << "After drawing bottom triangle := " << std::endl;
    //    int randIndex = printRateCounter;
    //    PrintThisTriangleInfo(triangle, randIndex);
    //}

    //if (printRateCounter % printRate == 0) {
    //    std::cout << printString << std::endl;
    //}

    //printRateCounter++;
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

    lineThickness = 1;
    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointA, projectedPointB, lineThickness, Colours::black);
    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointB, projectedPointC, lineThickness, Colours::black);
    DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointC, projectedPointA, lineThickness, Colours::black);


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

//int printRateCounter = 0;
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
            out_tri1.colour = Colours::pink;

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

            //std::cout << "Here." << std::endl;
            //printRateCounter++;
            //if (printRateCounter % 1000 == 0) {
            //    std::cout << in_tri.a.texCoord.x << ", " << in_tri.a.texCoord.y << ", " << in_tri.b.texCoord.x << ", " << in_tri.b.texCoord.y << ", " << in_tri.c.texCoord.x << ", " << in_tri.c.texCoord.y << ", " << std::endl;
            //}

            //std::cout << "No tri generated : " << std::endl;
            //std::cout << "\t same tri := " << in_tri.a.texCoord.x << ", " << in_tri.a.texCoord.y << ", " << in_tri.b.texCoord.x << ", " << in_tri.b.texCoord.y << ", " << in_tri.c.texCoord.x << ", " << in_tri.c.texCoord.y << ", " << std::endl;

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
        out_tri1.colour = Colours::blue/*in_tri.colour*/;

        // The inside point is valid, so keep that...
        out_tri1.a = *inside_points[0];

        // but the two new points are at the locations where the 
        // original sides of the triangle (lines) intersect with the plane
        out_tri1.b.position = VectorIntersectPlane({*inside_points[0], *outside_points[0]}, plane);
        out_tri1.b.texCoord = outside_points[0]->texCoord;
        out_tri1.c.position = VectorIntersectPlane({*inside_points[0], *outside_points[1]}, plane);
        out_tri1.c.texCoord = outside_points[1]->texCoord;

        //std::cout << in_tri.a.texCoord.x << ", " << in_tri.a.texCoord.y << ", " << in_tri.b.texCoord.x << ", " << in_tri.b.texCoord.y << ", " << in_tri.c.texCoord.x << ", " << in_tri.c.texCoord.y << ", " << std::endl;

        //std::cout << "Generated 1 tri : " << std::endl;
        //std::cout << "\t tri1 a := " << out_tri1.a.texCoord.x << ", " << out_tri1.a.texCoord.y << ", b := " << out_tri1.b.texCoord.x << ", " << out_tri1.b.texCoord.y << ", c := " << out_tri1.c.texCoord.x << ", " << out_tri1.c.texCoord.y << ", " << std::endl;


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
        out_tri1.colour = Colours::red/*in_tri.colour*/;

        out_tri2.colour = Colours::green/*in_tri.colour*/;

        // The first triangle consists of the two inside points and a new
        // point determined by the location where one side of the triangle
        // intersects with the plane
        out_tri1.a = *inside_points[0];
        out_tri1.b = *inside_points[1];
        out_tri1.c.position = VectorIntersectPlane({ *inside_points[0], *outside_points[0] }, plane);
        out_tri1.c.texCoord = outside_points[0]->texCoord;

        // The second triangle is composed of one of he inside points, a
        // new point determined by the intersection of the other side of the 
        // triangle and the plane, and the newly created point above

        out_tri2.a = *inside_points[1];
        out_tri2.b.position = VectorIntersectPlane({ *inside_points[1], *outside_points[0] }, plane);
        out_tri2.b.texCoord = outside_points[0]->texCoord;
        out_tri2.c = out_tri1.c;

        //std::cout << "Generated 2 tris : " << std::endl;
        //std::cout << "\t tri1 a := " << out_tri1.a.texCoord.x << ", " << out_tri1.a.texCoord.y << ", b := " << out_tri1.b.texCoord.x << ", " << out_tri1.b.texCoord.y << ", c := " << out_tri1.c.texCoord.x << ", " << out_tri1.c.texCoord.y << ", " << std::endl;
        //std::cout << "\t tri2 a := " << out_tri2.a.texCoord.x << ", " << out_tri2.a.texCoord.y << ", b := " << out_tri2.b.texCoord.x << ", " << out_tri2.b.texCoord.y << ", c := " << out_tri2.c.texCoord.x << ", " << out_tri2.c.texCoord.y << ", " << std::endl;

        //std::cout << in_tri.a.texCoord.x << ", " << in_tri.a.texCoord.y << ", " << in_tri.b.texCoord.x << ", " << in_tri.b.texCoord.y << ", " << in_tri.c.texCoord.x << ", " << in_tri.c.texCoord.y << ", " << std::endl;

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

void DrawTriangleOnScreenFromWorldTriangleWithClipping(std::vector<unsigned char>& imageData, std::vector<float>& imageDepthData, int imageWidth, int imageHeight
                                                        , int curTriangleIndex, int currentTextureIndex, Triangle& modelTriangle, Mat4x4& modelMatrix
                                                        , Vector3 cameraPosition, Vector3 cameraDirection
                                                        , const Mat4x4& viewMatrix, const Mat4x4& projectionMatrix
                                                        , int lineThickness, Colour lineColour, bool debugDraw = false) {

    //cameraPosition.y *= -1.0f;

    Triangle transformedTriangle = ApplyTransformToTriangle(modelTriangle, modelMatrix);
    transformedTriangle.a.position.y *= -1.0f;
    transformedTriangle.b.position.y *= -1.0f;
    transformedTriangle.c.position.y *= -1.0f;

    //transformedTriangle.a.texCoord.y *= -1.0f;
    //transformedTriangle.b.texCoord.y *= -1.0f;
    //transformedTriangle.c.texCoord.y *= -1.0f;

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
        Triangle curTriangle = Triangle{ {viewTransformedA, modelTriangle.a.texCoord}, {viewTransformedB, modelTriangle.b.texCoord}, {viewTransformedC, modelTriangle.c.texCoord }, Colours::white };
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

            Triangle curLargeScreenSpaceTriangle = { {projectedPointA, clipped[n].a.texCoord}, {projectedPointB, clipped[n].b.texCoord}, {projectedPointC, clipped[n].c.texCoord}, clipped[n].colour };

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
                DrawTriangleOnScreenFromScreenSpace(imageData, imageDepthData, imageWidth, imageHeight, curTriangleIndex, currentTextureIndex, curTriangle, normal, depth, lineThickness);
                //DrawTriangleScreenSpaceScanLine(imageData, imageDepthData, imageWidth, curTriangle, normal, depth, lineThickness);
                //std::cout << "Finished drawing." << std::endl;
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
        DrawTriangleOnScreenFromWorldTriangleWithClipping(imageData, imageDepthData, imageWidth, imageHeight, i, currentMesh.textureIndex, currentMesh.triangles[i], modelMatrix, cameraPosition, cameraDirection, viewMatrix, projectionMatrix, lineThickness, lineColour, debugDraw);
    }
}