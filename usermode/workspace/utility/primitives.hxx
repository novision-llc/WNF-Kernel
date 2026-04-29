#pragma once

template <typename T>
struct Vector2
{
	T x, y;

	Vector2() : x(0), y(0) {}
	Vector2(T _x, T _y) : x(_x), y(_y) {}
	~Vector2() = default;

	// Basic vector arithmetic
	[[nodiscard]] Vector2<T> operator+(const Vector2<T>& v) const noexcept
	{
		return Vector2<T>(x + v.x, y + v.y);
	}

	[[nodiscard]] Vector2<T> operator-(const Vector2<T>& v) const noexcept
	{
		return Vector2<T>(x - v.x, y - v.y);
	}

	[[nodiscard]] Vector2<T> operator*(const Vector2<T>& v) const noexcept
	{
		return Vector2<T>(x * v.x, y * v.y);
	}

	[[nodiscard]] Vector2<T> operator/(const Vector2<T>& v) const noexcept
	{
		return Vector2<T>(x / v.x, y / v.y);
	}

	// Scalar multiplication and division
	[[nodiscard]] Vector2<T> operator*(T scalar) const noexcept
	{
		return Vector2<T>(x * scalar, y * scalar);
	}

	[[nodiscard]] Vector2<T> operator/(T scalar) const noexcept
	{
		return Vector2<T>(x / scalar, y / scalar);
	}

	// Length (magnitude) of the vector
	[[nodiscard]] T Length() const noexcept
	{
		return std::sqrt(x * x + y * y);
	}

	// Squared length (avoids square root for comparisons)
	[[nodiscard]] T LengthSquared() const noexcept
	{
		return x * x + y * y;
	}
};

template <typename T>
struct Vector3
{
	T x, y, z;

	Vector3() : x(0), y(0), z(0) {}
	Vector3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
	~Vector3() = default;

	[[nodiscard]] T Dot(Vector3<T> v) const noexcept
	{
		return x * v.x + y * v.y + z * v.z;
	}

	[[nodiscard]] T SizeSquared() const noexcept
	{
		return x * x + y * y + z * z;
	}

	[[nodiscard]] T Distance(Vector3<T> v) const noexcept
	{
		Vector3<T> delta{
			x - v.x,
			y - v.y,
			z - v.z
		};

		return std::sqrt(delta.SizeSquared());
	}

	[[nodiscard]] Vector3<T> Normalize() const noexcept
	{
		double length = std::sqrt(static_cast<double>(SizeSquared()));
		if (length <= 1.0e-6)
			return Vector3<T>{ 0, 0, 0 };

		return Vector3<T>{
			x / static_cast<T>(length),
				y / static_cast<T>(length),
				z / static_cast<T>(length)
		};
	}

	bool operator==(const Vector3<T>& rhs) const noexcept
	{
		return x == rhs.x && y == rhs.y && z == rhs.z;
	}

	[[nodiscard]] Vector3<T> operator+(const Vector3<T>& v) const noexcept
	{
		return Vector3<T>(x + v.x, y + v.y, z + v.z);
	}

	[[nodiscard]] Vector3<T> operator-(const Vector3<T>& v) const noexcept
	{
		return Vector3<T>(x - v.x, y - v.y, z - v.z);
	}

	[[nodiscard]] Vector3<T> operator*(const Vector3<T>& v) const noexcept
	{
		return Vector3<T>(x * v.x, y * v.y, z * v.z);
	}

	[[nodiscard]] Vector3<T> operator/(const Vector3<T>& v) const noexcept
	{
		return Vector3<T>(x / v.x, y / v.y, z / v.z);
	}

	[[nodiscard]] T& operator[](int i) noexcept
	{
		return (&x)[i];
	}

	[[nodiscard]] const T& operator[](int i) const noexcept
	{
		return (&x)[i];
	}
};

template <typename T>
struct Vector4
{
	T x, y, z, w;

	Vector4() : x(0), y(0), z(0), w(0) {}
	Vector4(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}
	~Vector4() = default;

	[[nodiscard]] Vector4<T> operator+(const Vector4<T>& v) const noexcept
	{
		return Vector4<T>(x + v.x, y + v.y, z + v.z, w + v.w);
	}

	[[nodiscard]] Vector4<T> operator-(const Vector4<T>& v) const noexcept
	{
		return Vector4<T>(x - v.x, y - v.y, z - v.z, w - v.w);
	}

	[[nodiscard]] Vector4<T> operator*(const Vector4<T>& v) const noexcept
	{
		return Vector4<T>(x * v.x, y * v.y, z * v.z, w * v.w);
	}

	[[nodiscard]] Vector4<T> operator/(const Vector4<T>& v) const noexcept
	{
		return Vector4<T>(x / v.x, y / v.y, z / v.z, w / v.w);
	}

	[[nodiscard]] T& operator[](int i) const noexcept
	{
		return (&x)[i];
	}
};

using FVector = Vector3<double>;
using FRotator = Vector3<float>;
using FPlane = Vector4<double>;
using FQuat = Vector4<double>;


template <typename T>
struct Matrix4x4
{
	Vector4<T> v1, v2, v3, v4;

	Matrix4x4() : v1(), v2(), v3(), v4() {}
	Matrix4x4(const Vector4<T>& _v1, const Vector4<T>& _v2, const Vector4<T>& _v3, const Vector4<T>& _v4)
		: v1(_v1), v2(_v2), v3(_v3), v4(_v4) {
	}
	~Matrix4x4() = default;

	[[nodiscard]] Matrix4x4<T> operator+(const Matrix4x4<T>& m) const noexcept
	{
		return Matrix4x4<T>(
			v1 + m.v1,
			v2 + m.v2,
			v3 + m.v3,
			v4 + m.v4
		);
	}

	[[nodiscard]] Matrix4x4<T> operator-(const Matrix4x4<T>& m) const noexcept
	{
		return Matrix4x4<T>(
			v1 - m.v1,
			v2 - m.v2,
			v3 - m.v3,
			v4 - m.v4
		);
	}

	[[nodiscard]] Matrix4x4<T> operator*(const Matrix4x4<T>& m) const noexcept
	{
		return Matrix4x4<T>(
			Vector4<T>(
				v1.x * m.v1.x + v1.y * m.v2.x + v1.z * m.v3.x + v1.w * m.v4.x,
				v1.x * m.v1.y + v1.y * m.v2.y + v1.z * m.v3.y + v1.w * m.v4.y,
				v1.x * m.v1.z + v1.y * m.v2.z + v1.z * m.v3.z + v1.w * m.v4.z,
				v1.x * m.v1.w + v1.y * m.v2.w + v1.z * m.v3.w + v1.w * m.v4.w
			),
			Vector4<T>(
				v2.x * m.v1.x + v2.y * m.v2.x + v2.z * m.v3.x + v2.w * m.v4.x,
				v2.x * m.v1.y + v2.y * m.v2.y + v2.z * m.v3.y + v2.w * m.v4.y,
				v2.x * m.v1.z + v2.y * m.v2.z + v2.z * m.v3.z + v2.w * m.v4.z,
				v2.x * m.v1.w + v2.y * m.v2.w + v2.z * m.v3.w + v2.w * m.v4.w
			),
			Vector4<T>(
				v3.x * m.v1.x + v3.y * m.v2.x + v3.z * m.v3.x + v3.w * m.v4.x,
				v3.x * m.v1.y + v3.y * m.v2.y + v3.z * m.v3.y + v3.w * m.v4.y,
				v3.x * m.v1.z + v3.y * m.v2.z + v3.z * m.v3.z + v3.w * m.v4.z,
				v3.x * m.v1.w + v3.y * m.v2.w + v3.z * m.v3.w + v3.w * m.v4.w
			),
			Vector4<T>(
				v4.x * m.v1.x + v4.y * m.v2.x + v4.z * m.v3.x + v4.w * m.v4.x,
				v4.x * m.v1.y + v4.y * m.v2.y + v4.z * m.v3.y + v4.w * m.v4.y,
				v4.x * m.v1.z + v4.y * m.v2.z + v4.z * m.v3.z + v4.w * m.v4.z,
				v4.x * m.v1.w + v4.y * m.v2.w + v4.z * m.v3.w + v4.w * m.v4.w
			)
		);
	}

	[[nodiscard]] Matrix4x4<T> operator/(const Matrix4x4<T>& m) const noexcept
	{
		return Matrix4x4<T>(
			v1 / m.v1,
			v2 / m.v2,
			v3 / m.v3,
			v4 / m.v4
		);
	}
};


// fbox_sphere_bounds
#pragma pack(push, 1)
struct FBoxBounds
{
		
	FVector orgin{};
	FVector boxExtent{};
	double sphereRadius;
};
#pragma pack(pop)
