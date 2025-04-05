#pragma once

#include "Instrumentor.h"

#include "Colour.h"
#include "Geometry.h"

int GetRedFlattenedImageDataSlotForPixel(Vector2Int pixelPos, int imageWidth) {
    return (pixelPos.x + pixelPos.y * imageWidth) * 4;
}

void FillSubPixels(std::vector<unsigned char>& imageData, int imageWidth, Vector2Int pixelCentre, int halfSizeMinusOne, Colour colourToFillWith) {
    for (int x = -halfSizeMinusOne; x < halfSizeMinusOne; x++)
    {
        for (int y = -halfSizeMinusOne; y < halfSizeMinusOne; y++)
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

void DrawLineSegmentOnScreenFromWorldLineSegment(std::vector<unsigned char>& imageData, int imageWidth, LineSegment& worldLineSegment, Mat4x4& projectionMatrix, int lineThickness, Colour lineColour) {

    Vector4 projectedPointA = { projectionMatrix * Vector4(worldLineSegment.a.position, 1.0f) };
    Vector4 projectedPointB = { projectionMatrix * Vector4(worldLineSegment.b.position, 1.0f) };

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

void DrawTriangleOnScreenFromWorldTriangle(std::vector<unsigned char>& imageData, int imageWidth, int imageHeight, Triangle worldTriangle, Vector3 triangleNormal, Mat4x4& projectionMatrix, int lineThickness, Colour triangleColour) {

    worldTriangle.a.position.y *= -1.0f;
    worldTriangle.b.position.y *= -1.0f;
    worldTriangle.c.position.y *= -1.0f;

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

    Vector2Int boundingBoxMin = Vector2Int{ (int)std::min(projectedPointA.x, projectedPointB.x), (int)std::min(projectedPointA.y, projectedPointB.y) };
    Vector2Int boundingBoxMax = Vector2Int{ (int)std::max(projectedPointA.x, projectedPointB.x), (int)std::max(projectedPointA.y, projectedPointB.y) };

    boundingBoxMin = Vector2Int{ (int)std::min((float)boundingBoxMin.x, projectedPointC.x), (int)std::min((float)boundingBoxMin.y, projectedPointC.y) };
    boundingBoxMax = Vector2Int{ (int)std::max((float)boundingBoxMax.x, projectedPointC.x), (int)std::max((float)boundingBoxMax.y, projectedPointC.y) };

    Vector2Int a = projectedPointB - projectedPointA;
    Vector2Int b = projectedPointC - projectedPointB;
    Vector2Int c = projectedPointA - projectedPointC;

    Vector3 lightDir = Vector3{ -1.0f, 1.0f, -1.0f };
    lightDir = glm::normalize(lightDir);
    float normDotLightDirMax = glm::max(glm::dot(triangleNormal, lightDir), 0.0f);

    Colour modifiedColour = triangleColour;
    modifiedColour.r = modifiedColour.r * normDotLightDirMax;
    modifiedColour.g = modifiedColour.g * normDotLightDirMax;
    modifiedColour.b = modifiedColour.b * normDotLightDirMax;
    modifiedColour.a = modifiedColour.a;

    int biasEdgeg0 = IsTopOrLeft(projectedPointB, projectedPointA) ? 0 : -1;
    int biasEdgeg1 = IsTopOrLeft(projectedPointC, projectedPointB) ? 0 : -1;
    int biasEdgeg2 = IsTopOrLeft(projectedPointA, projectedPointC) ? 0 : -1;

    for (int y = boundingBoxMin.y; y <= boundingBoxMax.y; y++)
    {
        for (int x = boundingBoxMin.x; x <= boundingBoxMax.x; x++)
        {
            Vector2Int curPoint = Vector2Int{ x, y };
            Vector2Int ap = curPoint - Vector2Int(projectedPointA);
            Vector2Int bp = curPoint - Vector2Int(projectedPointB);
            Vector2Int cp = curPoint - Vector2Int(projectedPointC);

            float crossA = (a.x * ap.y) - (ap.x * a.y) + biasEdgeg0;
            float crossB = (b.x * bp.y) - (bp.x * b.y) + biasEdgeg1;
            float crossC = (c.x * cp.y) - (cp.x * c.y) + biasEdgeg2;

            float cutOffValue = -80.0f;
            if (crossA >= cutOffValue && crossB >= cutOffValue && crossC >= cutOffValue) {

                //std::cout << "Filling triangle." << std::endl;

                int index = GetRedFlattenedImageDataSlotForPixel(curPoint, imageWidth);
                if (index >= 0 && (index + 3) < imageData.size()) {

                    imageData[index + 0] = modifiedColour.r;
                    imageData[index + 1] = modifiedColour.g;
                    imageData[index + 2] = modifiedColour.b;
                    imageData[index + 3] = modifiedColour.a;
                }

            }
        }
    }

    //int missingPixels = 2;

    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointA, projectedPointB, missingPixels, modifiedColour);
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointB, projectedPointC, missingPixels, modifiedColour);
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointC, projectedPointA, missingPixels, modifiedColour);


    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointA, projectedPointB, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointB, projectedPointC, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, projectedPointC, projectedPointA, lineThickness, Colour{ 0, 0, 255, 255 });

    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMin.x, boundingBoxMin.y, 0.0f }, Vector3Int{ boundingBoxMax.x, boundingBoxMin.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMax.x, boundingBoxMin.y, 0.0f }, Vector3Int{ boundingBoxMax.x, boundingBoxMax.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMax.x, boundingBoxMax.y, 0.0f }, Vector3Int{ boundingBoxMin.x, boundingBoxMax.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
    //DrawLineSegmentOnScreen(imageData, imageWidth, Vector3Int{ boundingBoxMin.x, boundingBoxMax.y, 0.0f }, Vector3Int{ boundingBoxMin.x, boundingBoxMin.y, 0.0f }, lineThickness, Colour{ 0, 0, 255, 255 });
}

void DrawTriangleOnScreenFromWorldTriangleWithTransform(std::vector<unsigned char>& imageData, int imageWidth, int imageHeight, Triangle modelTriangle, Mat4x4& modelMatrix, Vector3 cameraPosition, Mat4x4& projectionMatrix, int lineThickness, Colour lineColour) {

    Triangle transformedTriangle = ApplyTransformToTriangle(modelTriangle, modelMatrix);

    Vector3 normal, lineA, lineB;
    lineA = transformedTriangle.b.position - transformedTriangle.a.position;
    lineB = transformedTriangle.c.position - transformedTriangle.a.position;

    normal = glm::cross(lineA, lineB);
    normal = glm::normalize(normal);

    Vector3 trianglePosRelativeToCamera = transformedTriangle.a.position - cameraPosition;
    trianglePosRelativeToCamera = glm::normalize(trianglePosRelativeToCamera);

    if (glm::dot(normal, trianglePosRelativeToCamera) < 0.0f) {
        DrawTriangleOnScreenFromWorldTriangle(imageData, imageWidth, imageHeight, transformedTriangle, normal, projectionMatrix, lineThickness, lineColour);
    }

}

void DrawMeshOnScreenFromWorldWithTransform(std::vector<unsigned char>& imageData, int imageWidth, int imageHeight, Mesh& currentMesh, Mat4x4& modelMatrix, Vector3 cameraPosition, Mat4x4& projectionMatrix, int lineThickness, Colour lineColour) {

    for (int i = 0; i < currentMesh.triangles.size(); i++)
    {
        DrawTriangleOnScreenFromWorldTriangleWithTransform(imageData, imageWidth, imageHeight, currentMesh.triangles[i], modelMatrix, cameraPosition, projectionMatrix, lineThickness, lineColour);
    }
}