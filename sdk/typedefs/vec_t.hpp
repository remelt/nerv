#pragma once

#include <cmath>

class vec3_t {
public:
	float x, y, z;

	vec3_t(float _x = 0, float _y = 0, float _z = 0) {
		x = _x;
		y = _y;
		z = _z;
	}

	vec3_t operator+(const vec3_t& vec) const {
		return vec3_t(x + vec.x, y + vec.y, z + vec.z);
	}

	vec3_t operator-(const vec3_t& vec) const {
		return vec3_t(x - vec.x, y - vec.y, z - vec.z);
	}

	vec3_t operator*(float n) const {
		return vec3_t(x * n, y * n, z * n);
	}

	vec3_t operator/(float n) const {
		return vec3_t(x / n, y / n, z / n);
	}

	vec3_t& operator+=(const vec3_t& vec) {
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	vec3_t& operator-=(const vec3_t& vec) {
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}

	vec3_t& operator*=(float n) {
		x *= n;
		y *= n;
		z *= n;
		return *this;
	}

	vec3_t& operator/=(float n) {
		x /= n;
		y /= n;
		z /= n;
		return *this;
	}

	bool operator==(const vec3_t& vec) const {
		return x == vec.x && y == vec.y && z == vec.z;
	}

	bool operator!=(const vec3_t& vec) const {
		return !(*this == vec);
	}

	float length() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	float length_sqr() const {
		return x * x + y * y + z * z;
	}

	float length_2d() const {
		return std::sqrt(x * x + y * y);
	}

	float dist(const vec3_t& vec) const {
		return (*this - vec).length();
	}

	float dot(const vec3_t& vec) const {
		return x * vec.x + y * vec.y + z * vec.z;
	}

	vec3_t cross(const vec3_t& vec) const {
		return vec3_t(
			y * vec.z - z * vec.y,
			z * vec.x - x * vec.z,
			x * vec.y - y * vec.x
		);
	}

	vec3_t normalize() const {
		float len = length();
		if (len > 0.0f)
			return vec3_t(x / len, y / len, z / len);
		return vec3_t();
	}

	vec3_t normalize_in_place()
	{
		const float flLength = this->length();
		const float flRadius = 1.0f / (flLength + std::numeric_limits<float>::epsilon());

		this->x *= flRadius;
		this->y *= flRadius;
		this->z *= flRadius;

		return flLength;
	}

	bool is_valid() const {
		return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
	}

	bool is_zero() const {
		return x == 0.0f && y == 0.0f && z == 0.0f;
	}
};

class vec4_t {
public:
	float x, y, z, w;

	vec4_t(float _x = 0, float _y = 0, float _z = 0, float _w = 0) {
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
};

struct matrix3x4_t {
	float m[3][4];

	float* operator[](int i) { return m[i]; }
	const float* operator[](int i) const { return m[i]; }

	vec3_t transform(const vec3_t& vec) const {
		return vec3_t(
			m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z + m[0][3],
			m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z + m[1][3],
			m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z + m[2][3]
		);
	}
};

using matrix2x4_t = matrix3x4_t;

struct c_transform {
	vec4_t m_position;
	vec4_t m_rotation;

	void to_matrix(matrix3x4_t& out) const {
		float x = m_rotation.x;
		float y = m_rotation.y;
		float z = m_rotation.z;
		float w = m_rotation.w;

		out[0][0] = 1.0f - 2.0f * (y * y + z * z);
		out[0][1] = 2.0f * (x * y - z * w);
		out[0][2] = 2.0f * (x * z + y * w);
		out[0][3] = m_position.x;

		out[1][0] = 2.0f * (x * y + z * w);
		out[1][1] = 1.0f - 2.0f * (x * x + z * z);
		out[1][2] = 2.0f * (y * z - x * w);
		out[1][3] = m_position.y;

		out[2][0] = 2.0f * (x * z - y * w);
		out[2][1] = 2.0f * (y * z + x * w);
		out[2][2] = 1.0f - 2.0f * (x * x + y * y);
		out[2][3] = m_position.z;
	}
};

struct alignas(16) c_bone_data {
	vec3_t m_pos;
	float m_scale;
	vec4_t m_rot;
};
