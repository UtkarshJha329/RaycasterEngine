#pragma once

#include "Geometry.h"

void SetCameraViewDirection(Mat4x4& viewMatrix, Vector3 position, Vector3 direction, Vector3 up = Vector3{ 0.0f, -1.0f, 0.0f }) {

	const Vector3 w{ glm::normalize(direction) };
	const Vector3 u{ glm::normalize(glm::cross(w, up)) };
	const Vector3 v{ glm::cross(w, u) };

	viewMatrix = Mat4x4{ 1.f };
	viewMatrix[0][0] = u.x;
	viewMatrix[1][0] = u.y;
	viewMatrix[2][0] = u.z;
	viewMatrix[0][1] = v.x;
	viewMatrix[1][1] = v.y;
	viewMatrix[2][1] = v.z;
	viewMatrix[0][2] = w.x;
	viewMatrix[1][2] = w.y;
	viewMatrix[2][2] = w.z;
	viewMatrix[3][0] = -glm::dot(u, position);
	viewMatrix[3][1] = -glm::dot(v, position);
	viewMatrix[3][2] = -glm::dot(w, position);

	//vec<3, T, Q> const f(normalize(center - eye));
	//vec<3, T, Q> const s(normalize(cross(up, f)));
	//vec<3, T, Q> const u(cross(f, s));

	//mat<4, 4, T, Q> Result(1);
	//Result[0][0] = s.x;
	//Result[1][0] = s.y;
	//Result[2][0] = s.z;
	//Result[0][1] = u.x;
	//Result[1][1] = u.y;
	//Result[2][1] = u.z;
	//Result[0][2] = f.x;
	//Result[1][2] = f.y;
	//Result[2][2] = f.z;
	//Result[3][0] = -dot(s, eye);
	//Result[3][1] = -dot(u, eye);
	//Result[3][2] = -dot(f, eye);
	//return Result;

}

void SetCameraViewTarget(Mat4x4& viewMatrix, Vector3 position, Vector3 target, Vector3 up = Vector3{ 0.0f, -1.0f, 0.0f }) {

	SetCameraViewDirection(viewMatrix, position, target - position, up);
}

void SetViewYXZ(Mat4x4& viewMatrix, Vector3 position, Vector3 rotation) {

	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const Vector3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
	const Vector3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
	const Vector3 w{ (c2 * s1), (-s2), (c1 * c2) };
	viewMatrix = Mat4x4{ 1.f };
	viewMatrix[0][0] = u.x;
	viewMatrix[1][0] = u.y;
	viewMatrix[2][0] = u.z;
	viewMatrix[0][1] = v.x;
	viewMatrix[1][1] = v.y;
	viewMatrix[2][1] = v.z;
	viewMatrix[0][2] = w.x;
	viewMatrix[1][2] = w.y;
	viewMatrix[2][2] = w.z;
	viewMatrix[3][0] = -glm::dot(u, position);
	viewMatrix[3][1] = -glm::dot(v, position);
	viewMatrix[3][2] = -glm::dot(w, position);

}