#pragma once

class VectorPP
{
public:
    float x = 0;
    float y = 0;

    // --- Constructors ---
    VectorPP();
    VectorPP(float a_x, float a_y);

    // --- Comparison Operators ---
    bool operator==(const VectorPP &a_rVec) const;
    bool operator==(float a_fVal) const;
    bool operator!=(const VectorPP &a_rVec) const;
    bool operator>(const float rhs) const;

    // --- Arithmetic Operators ---
    VectorPP operator+(const VectorPP &a_rVec) const;
    VectorPP operator-(const VectorPP &a_rVec) const;
    VectorPP operator/(const float a_fVal) const;
    VectorPP operator*(const float a_fVal) const;

    // --- Compound Assignment Operators ---
    void operator+=(const VectorPP &a_rVec);
    void operator-=(const VectorPP &a_rVec);
    void operator*=(const float a_fVal);
    void operator/=(const float a_fVal);

    // --- Utility ---
    float dist(const VectorPP &a_rVec) const;
};