#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Colour.h"

typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;
typedef glm::ivec2 Vector2Int;
typedef glm::ivec3 Vector3Int;
typedef glm::ivec4 Vector4Int;
typedef glm::mat3x3 Mat3x3;
typedef glm::mat4x4 Mat4x4;

class Point {

public:
	Vector3 position;
	Vector3 texCoord;
	Vector4 colour;
	//Colour colour;
	Vector3 normal;
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

	Colour colour;
};

class Plane {

public:
	Vector3 normal;
	Point pointOnPlane;
};

Vector4 ColourToVector4(const Colour& curColour) {
	return Vector4{ curColour.r, curColour.g, curColour.b, curColour.a };
}

Colour Vector4ToColour(const Vector4& curColour) {
	return Colour{ (unsigned char)curColour.x, (unsigned char)curColour.y, (unsigned char)curColour.z, (unsigned char)curColour.w };
}

float EdgeFunction(const Vector3& a, const Vector3& b, const Vector3& c)
{
	return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
	//return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

bool LinePlaneIntersection(const LineSegment& lineSegmentToCheck, const Plane& planeToCheckAgainst, Point& intersectionPoint, float intersectionDistance)
{
	Vector3 vectorAlongLineSegment = lineSegmentToCheck.b.position - lineSegmentToCheck.a.position;
	Vector3 vectorFromLineSegmentToPlane = planeToCheckAgainst.pointOnPlane.position - lineSegmentToCheck.a.position;

	intersectionDistance = glm::dot(vectorFromLineSegmentToPlane, planeToCheckAgainst.normal) / glm::dot(vectorAlongLineSegment, planeToCheckAgainst.normal);

	intersectionPoint = { lineSegmentToCheck.a.position * intersectionDistance };

	return intersectionDistance >= 0 && intersectionDistance <= 1;
}

bool LinePlaneIntersection(const Vector3& a, const Vector3& b, const Plane& planeToCheckAgainst, Vector3& intersectionPoint)
{
	// if a line is described in its parametric form L(t) = a + t * (b - a) and a plane as n . (x - p) = 0
	// then t = (n . (p - a)) / (n . (b - a))
	// if n . (b - a) = 0 line is parallel
	// if t is not in range [0, 1] the line and plane intersect but not withint the range of its start and end points.

	Vector3 lineSegment = b - a;
	float denominator = glm::dot(planeToCheckAgainst.normal, lineSegment);

	if (denominator != 0) {
		Vector3 vectorFromLineSegmentToPlane = planeToCheckAgainst.pointOnPlane.position - a;
		float numerator = glm::dot(planeToCheckAgainst.normal, vectorFromLineSegmentToPlane);

		float t = numerator / denominator;
		intersectionPoint = a + t * (lineSegment);

		return true;
	}
	else {
		return false;
	}
}

float DistanceFromPointToPlane(const Point& point, const Plane& plane) {
	return glm::dot(plane.normal, point.position - plane.pointOnPlane.position);
}

Vector3 VectorIntersectPlane(const LineSegment& lineSegmentToCheck, Plane& plane)
{
	plane.normal = glm::normalize(plane.normal);

	float plane_d = -glm::dot(plane.normal, plane.pointOnPlane.position);
	float ad = glm::dot(lineSegmentToCheck.a.position, plane.normal);
	float bd = glm::dot(lineSegmentToCheck.b.position, plane.normal);
	float t = (-plane_d - ad) / (bd - ad);
	Vector3 lineStartToEnd = lineSegmentToCheck.b.position - lineSegmentToCheck.a.position;
	Vector3 lineToIntersect = lineStartToEnd * t;
	return lineSegmentToCheck.a.position + lineToIntersect;
}

Vector3 VectorIntersectPlane(const LineSegment& lineSegmentToCheck, Plane& plane, float& outPercecntage)
{
	plane.normal = glm::normalize(plane.normal);

	float plane_d = -glm::dot(plane.normal, plane.pointOnPlane.position);
	float ad = glm::dot(lineSegmentToCheck.a.position, plane.normal);
	float bd = glm::dot(lineSegmentToCheck.b.position, plane.normal);
	float t = (-plane_d - ad) / (bd - ad);
	Vector3 lineStartToEnd = lineSegmentToCheck.b.position - lineSegmentToCheck.a.position;
	Vector3 lineToIntersect = lineStartToEnd * t;

	outPercecntage = t;
	return lineSegmentToCheck.a.position + lineToIntersect;
}

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

void ApplyTransformToTriangleNormals(Triangle& curTriangle, Mat4x4& transformMatrix) {

	Vector4 normalA = transformMatrix * Vector4(curTriangle.a.normal, 1.0f);
	Vector4 normalB = transformMatrix * Vector4(curTriangle.b.normal, 1.0f);
	Vector4 normalC = transformMatrix * Vector4(curTriangle.c.normal, 1.0f);

	//normalA = normalA / normalA.w;
	//normalB = normalB / normalB.w;
	//normalC = normalC / normalC.w;

	curTriangle.a.normal = normalA;
	curTriangle.b.normal = normalB;
	curTriangle.c.normal = normalC;
}