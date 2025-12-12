#include "../pch.h"

constexpr float KINDA_SMALL_NUMBER = 1.e-4f;

inline FVector2D GetSafeNormal(FVector2D v)
{
	double sizeSq = v.X * v.X + v.Y * v.Y;
	if (sizeSq > KINDA_SMALL_NUMBER)
	{
		double inv = 1. / sqrt(sizeSq);
		return FVector2D(v.X * inv, v.Y * inv);
	}
	return FVector2D(0.f, 0.f);
}

inline FVector GetSafeNormal(FVector v)
{
	double sizeSq = v.X * v.X + v.Y * v.Y + v.Z * v.Z;
	if (sizeSq > KINDA_SMALL_NUMBER)
	{
		double inv = 1. / sqrt(sizeSq);
		return FVector(v.X * inv, v.Y * inv, v.Z * inv);
	}
	return FVector(0.f, 0.f, 0.f);
}

inline bool IsNearlyZero(FVector2D v)
{
	return (v.X * v.X + v.Y * v.Y) < KINDA_SMALL_NUMBER * KINDA_SMALL_NUMBER;
}

FVector ClampToPlayableBounds(const FVector& Candidate, float Radius, const FBoxSphereBounds& Bounds)
{
	FVector Clamped = Candidate;

	Clamped.X = std::clamp(Clamped.X, Bounds.Origin.X - Bounds.BoxExtent.X + Radius, Bounds.Origin.X + Bounds.BoxExtent.X - Radius);
	Clamped.Y = std::clamp(Clamped.Y, Bounds.Origin.Y - Bounds.BoxExtent.Y + Radius, Bounds.Origin.Y + Bounds.BoxExtent.Y - Radius);

	return Clamped;
}

#define INV_PI			(0.31830988618f)
#define HALF_PI			(1.57079632679f)
#define PI              (3.1415926535897932f)

float RadiansToDegrees(float Radians)
{
	return Radians * (180.0f / PI);
}
inline void SinCos(float* ScalarSin, float* ScalarCos, float  Value)
{
	// Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
	float quotient = (INV_PI * 0.5f) * Value;
	if (Value >= 0.0f)
	{
		quotient = (float)((int)(quotient + 0.5f));
	}
	else
	{
		quotient = (float)((int)(quotient - 0.5f));
	}
	float y = Value - (2.0f * PI) * quotient;

	// Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
	float sign;
	if (y > HALF_PI)
	{
		y = PI - y;
		sign = -1.0f;
	}
	else if (y < -HALF_PI)
	{
		y = -PI - y;
		sign = -1.0f;
	}
	else
	{
		sign = +1.0f;
	}

	float y2 = y * y;

	// 11-degree minimax approximation
	*ScalarSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

	// 10-degree minimax approximation
	float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
	*ScalarCos = sign * p;
}