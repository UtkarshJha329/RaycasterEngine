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

class Mesh {

public:

	std::vector<Triangle> triangles;
};

Triangle ApplyTransformToTriangle(Triangle curTriangle, Mat4x4& transformMatrix) {

	curTriangle.a.position = transformMatrix * Vector4(curTriangle.a.position, 1.0f);
	curTriangle.b.position = transformMatrix * Vector4(curTriangle.b.position, 1.0f);
	curTriangle.c.position = transformMatrix * Vector4(curTriangle.c.position, 1.0f);

	Triangle transformedTriangle = curTriangle;

	return transformedTriangle;
}