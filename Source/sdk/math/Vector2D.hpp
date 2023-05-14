#pragma once

// 2D Vector
class Vector2D
{
public:
    // Members
    float x, y;

    // Construction/destruction:
    Vector2D(void);
    Vector2D(float X, float Y);
    Vector2D(float* clr);
    Vector2D(const Vector2D& vOther);

    // Initialization
    void Init(float ix = 0.0f, float iy = 0.0f);
    // TODO (Ilya): Should there be an init that takes a single float for consistency?

    // Got any nasty NAN's?
    bool IsValid() const;

    // array access...
    float operator[](int i) const;
    float& operator[](int i);

    // Base address...
    float* Base();
    float const* Base() const;

    // Initialization methods
    void Random(float minVal, float maxVal);

    // equality
    bool operator==(const Vector2D& v) const;
    bool operator!=(const Vector2D& v) const;

    // arithmetic operations
    Vector2D& operator+=(const Vector2D& v)
    {
        x += v.x; y += v.y;
        return *this;
    }

    Vector2D& operator-=(const Vector2D& v)
    {
        x -= v.x; y -= v.y;
        return *this;
    }

    Vector2D& operator*=(float fl)
    {
        x *= fl;
        y *= fl;
        return *this;
    }

    Vector2D& operator*=(const Vector2D& v)
    {
        x *= v.x;
        y *= v.y;
        return *this;
    }

    Vector2D& operator/=(const Vector2D& v)
    {
        x /= v.x;
        y /= v.y;
        return *this;
    }

    // this ought to be an opcode.
    Vector2D& operator+=(float fl)
    {
        x += fl;
        y += fl;
        return *this;
    }

    // this ought to be an opcode.
    Vector2D& operator/=(float fl)
    {
        x /= fl;
        y /= fl;
        return *this;
    }
    Vector2D& operator-=(float fl)
    {
        x -= fl;
        y -= fl;
        return *this;
    }

    // negate the vector components
    void Negate();

    // Get the vector's magnitude.
    float Length() const;

    // Get the vector's magnitude squared.
    float LengthSqr(void) const
    {
        return (x*x + y*y);
    }

    // return true if this vector is (0,0,0) within tolerance
    bool IsZero(float tolerance = 0.01f) const
    {
        return (x > -tolerance && x < tolerance &&
            y > -tolerance && y < tolerance);
    }

    float NormalizeInPlace();
    bool IsLengthGreaterThan(float val) const;
    bool IsLengthLessThan(float val) const;

    // Get the distance from this vector to the other one squared.
    // NJS: note, VC wasn't inlining it correctly in several deeply nested inlines due to being an 'out of line' .  
    // may be able to tidy this up after switching to VC7
    float DistToSqr(const Vector2D &vOther) const
    {
        Vector2D delta;

        delta.x = x - vOther.x;
        delta.y = y - vOther.y;

        return delta.LengthSqr();
    }

    // Copy
    void CopyToArray(float* rgfl) const;

    // Multiply, add, and assign to this (ie: *this = a + b * scalar). This
    // is about 12% faster than the actual vector equation (because it's done per-component
    // rather than per-vector).
    void MulAdd(const Vector2D& a, const Vector2D& b, float scalar);
    void Vector2DLerp(const Vector2D& src1, const Vector2D& src2, float t, Vector2D& dest);

    // Dot product.
    float Dot(const Vector2D& vOther) const;
    float Vector2DNormalize(Vector2D& v);

    // assignment
    Vector2D& operator=(const Vector2D &vOther);

    // 2d
	float LengthDrop() const;
	Vector2D NormalizedDrop() const 
	{
		Vector2D reses = *this;
		float l = reses.LengthDrop();
		if (l != 0.0f)
			reses /= l;
		else 
			reses.x = reses.y = 0.0f;
		return reses;
	}
	float DistToDrop(const Vector2D &vOther) const
	{
		Vector2D deltar;

		deltar.x = x - vOther.x;
		deltar.y = y - vOther.y;

		return deltar.LengthDrop();
	}

    // copy constructors
    void ComputeClosestPoint2D(const Vector2D& vecStart, float flMaxDist, const Vector2D& vecTarget, Vector2D* pResult);
    // Vector2D(const Vector2D &vOther);

    // arithmetic operations
    Vector2D operator-(void) const;

    Vector2D operator+(const Vector2D& v) const;
    Vector2D operator-(const Vector2D& v) const;
    Vector2D operator*(const Vector2D& v) const;
    Vector2D operator/(const Vector2D& v) const;
    Vector2D operator*(float fl) const;
    Vector2D operator/(float fl) const;

    // Returns a vector with the min or max in X, Y, and Z.
    void Vector2DMin(const Vector2D& a, const Vector2D& b, Vector2D& result);
    void Vector2DMax(const Vector2D& a, const Vector2D& b, Vector2D& result);
    Vector2D Min(const Vector2D &vOther) const;
    Vector2D Max(const Vector2D &vOther) const;
};