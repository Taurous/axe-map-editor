#pragma once

#include <tuple> // For std::tie
#include <sstream> // To overload << operator
#include <cmath> // for floor

template <typename T> class vec2;

using vec2f = vec2<float>;
using vec2d = vec2<double>;
using vec2i = vec2<int>;

template <typename T>
class vec2
{
public:
	vec2() : x(T(0)), y(T(0)) {}
	vec2(const T& x, const T& y) : x(x), y(y) {}
	vec2(const vec2& other) : x(other.x), y(other.y) {}
	template <typename O>
	vec2(const vec2<O>& other) : x((T)other.x), y((T)other.y) {}
	vec2(const vec2&& other) noexcept : x(other.x), y(other.y) {}
	vec2<T>& operator=(const vec2<T>& other) noexcept { x = other.x; y = other.y; return *this; }
	//template <typename O>
	//vec2<T>& operator=(const vec2<O>& other) noexcept { x = (T)other.x; y = (T)other.y; return *this; }

	//Negation
	vec2 operator-() const { return vec2(-x, -y); }

	//Compound assignments
	vec2& operator+=(const vec2& rhs) { x += rhs.x; y += rhs.y; return *this; }
	vec2& operator-=(const vec2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	vec2& operator*=(const vec2& rhs) { x *= rhs.x; y *= rhs.y; return *this; }
	vec2& operator/=(const vec2& rhs) { x /= rhs.x; y /= rhs.y; return *this; }

	//Scalar multi/div compound
	vec2& operator*=(const T& rhs) { x *= rhs; y *= rhs; return *this; }
	vec2& operator/=(const T& rhs) { x /= rhs; y /= rhs; return *this; }

	//Comparison
	friend bool operator==(const vec2& lhs, const vec2& rhs) { return std::tie(lhs.x, lhs.y) == std::tie(rhs.x, rhs.y); }
	friend bool operator!=(const vec2& lhs, const vec2& rhs) { return !(lhs == rhs); }

	//Stream
	friend std::ostream& operator<<(std::ostream& os, const vec2& v)
	{
		os << "{" << v.x << ", " << v.y << "}";
		return os;
	}

	bool isInBounds(vec2<T> top_left, vec2<T> bottom_right) { return (x >= top_left.x && x <= bottom_right.x && y >= top_left.y && y <= bottom_right.y); }
	std::string str()
	{
		std::stringstream ss;
		ss << (*this);
		return ss.str();
	}

	//Public data members
	T x, y;
};

//Arithmetic
template <typename T> vec2<T> operator+(const vec2<T>& lhs, const vec2<T>& rhs) { return vec2<T>(lhs) += rhs; }
template <typename T> vec2<T> operator-(const vec2<T>& lhs, const vec2<T>& rhs) { return vec2<T>(lhs) -= rhs; }
template <typename T> vec2<T> operator*(const vec2<T>& lhs, const vec2<T>& rhs) { return vec2<T>(lhs) *= rhs; }
template <typename T, typename O> vec2<T> operator*(const vec2<T>& lhs, const vec2<O>& rhs) { return vec2<T>(lhs) *= rhs; }
template <typename T> vec2<T> operator/(const vec2<T>& lhs, const vec2<T>& rhs) { return vec2<T>(lhs) /= rhs; }

//Scalar multi/div
template <typename T> vec2<T> operator*(const vec2<T>& lhs, const T& rhs) { return vec2<T>(lhs) *= rhs; }
template <typename T, typename V> vec2<T> operator*(const vec2<T>& lhs, const V& rhs) { return vec2<T>(lhs) *= rhs; }
template <typename T> vec2<T> operator*(const T& lhs, const vec2<T>& rhs) { return vec2<T>(rhs) *= lhs; }
template <typename T, typename V> vec2<T> operator*(const V& lhs, const vec2<T>& rhs) { return vec2<T>(rhs) *= lhs; }
template <typename T> vec2<T> operator/(const vec2<T>& lhs, const T& rhs) { return vec2<T>(lhs) /= rhs; }
template <typename T, typename V> vec2<T> operator/(const vec2<T>& lhs, const V& rhs) { return vec2<T>(lhs) /= rhs; }
template <typename T> vec2<T> operator/(const T* lhs, const vec2<T>& rhs) { return vec2<T>(lhs / rhs.x, lhs / rhs.y); }
template <typename T, typename V> vec2<T> operator/(const V* lhs, const vec2<T>& rhs) { return vec2<T>(lhs / rhs.x, lhs / rhs.y); }

//Helper Functions
template <typename T> T dotProduct(const vec2<T>& v1, const vec2<T>& v2) { return v1.x * v2.x + v1.y * v2.y; }
template <typename T> T magnitude(const vec2<T>& v) { return sqrt(v.x * v.x + v.y * v.y); }
template <typename T> T magSquared(const vec2<T>& v) { return v.x * v.x + v.y * v.y; }
template <typename T> vec2<T> normalize(const vec2<T>& v)
{
	T mag = magnitude(v);
	if (mag < 0.00001f) return vec2<T>(0, 0);
	return vec2<T>(v.x / mag, v.y / mag);
}
template <typename T> vec2<T> floor(const vec2<T>& v) { return vec2<T>{ floor(v.x), floor(v.y) }; }
template <typename T> vec2<T> ceil(const vec2<T>& v) { return vec2<T>{ ceil(v.x), ceil(v.y) }; }
template <typename T> vec2<T> absolute(const vec2<T>& v) { return vec2<T>{ abs(v.x), abs(v.y)}; }
template <typename T> vec2<T> inverse(const vec2<T>& v1) { return vec2<T>{ 1.f / v1.x, 1.f / v1.y }; }
template <typename T> vec2<T> vec_lerp(const vec2<T>& v1, const vec2<T>& v2, const double& t)
{
	return { (v1.x * (1.0 - t)) + (v2.x * t), (v1.y * (1.0 - t)) + (v2.y * t) };
}