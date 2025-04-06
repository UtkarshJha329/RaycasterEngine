#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;
typedef glm::ivec2 Vector2Int;
typedef glm::ivec3 Vector3Int;
typedef glm::ivec4 Vector4Int;
typedef glm::mat4x4 Mat4x4;

class PointScreen {

public:
	Vector3Int position;
};

class LineSegmentScreen {

public:

	PointScreen a;
	PointScreen b;
};

class TriangleScreen {

public:

	PointScreen a;
	PointScreen b;
	PointScreen c;
};

class Point {

public:
	Vector3 position;
};

class LineSegment {

public:

	Point a;
	Point b;
};

class Triangle {

public:

	Point a;
	Point b;
	Point c;
};

class HomogeneousTriangle {

public:

	Vector4 a;
	Vector4 b;
	Vector4 c;
};


class Plane {

public:
	Vector3 normal;
	Point pointOnPlane;
};

bool LinePlaneIntersection(const LineSegment& lineSegmentToCheck, const Plane& planeToCheckAgainst, Point& intersectionPoint, float intersectionDistance)
{
	Vector3 vectorAlongLineSegment = lineSegmentToCheck.b.position - lineSegmentToCheck.a.position;
	Vector3 vectorFromLineSegmentToPlane = planeToCheckAgainst.pointOnPlane.position - lineSegmentToCheck.a.position;

	intersectionDistance = glm::dot(vectorFromLineSegmentToPlane, planeToCheckAgainst.normal) / glm::dot(vectorAlongLineSegment, planeToCheckAgainst.normal);

	intersectionPoint = { lineSegmentToCheck.a.position * intersectionDistance };

	return intersectionDistance >= 0 && intersectionDistance <= 1;
}

class Mesh {

public:

	std::vector<Triangle> triangles;
};

Triangle ApplyTransformToTriangle(Triangle curTriangle, Mat4x4& transformMatrix) {

	Vector4 transformedPositionA = transformMatrix * Vector4(curTriangle.a.position, 1.0f);
	Vector4 transformedPositionB = transformMatrix * Vector4(curTriangle.b.position, 1.0f);
	Vector4 transformedPositionC = transformMatrix * Vector4(curTriangle.c.position, 1.0f);

	transformedPositionA = transformedPositionA / transformedPositionA.w;
	transformedPositionB = transformedPositionB / transformedPositionB.w;
	transformedPositionC = transformedPositionC / transformedPositionC.w;

	curTriangle.a.position = transformedPositionA;
	curTriangle.b.position = transformedPositionB;
	curTriangle.c.position = transformedPositionC;

	Triangle transformedTriangle = curTriangle;

	return transformedTriangle;
}

void PrintMat4x4(const Mat4x4& matrixToPrint) {

	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			std::cout << matrixToPrint[x][y] << ", ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
}