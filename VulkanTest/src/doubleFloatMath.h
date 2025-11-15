#pragma once

#include <cmath> 

struct doublefloat {
    float high;
    float low;
};

struct ComplexDD {
    doublefloat Re;
    doublefloat Im;
};

inline doublefloat two_sum(float a, float b)
{
    doublefloat result;
    result.high = a + b;
    float b_virtual = result.high - a;
    float a_virtual = result.high - b_virtual;
    float b_err = b - b_virtual;
    float a_err = a - a_virtual;
    result.low = a_err + b_err;
    return result;
}

// Performs 'a * b' and returns the product AND the error.
inline doublefloat two_prod(float a, float b)
{
    doublefloat result;
    result.high = a * b;
    // std::fmaf is the C++ version of HLSL's fma() for floats
    result.low = std::fmaf(a, b, -result.high);
    return result;
}

// Helper to quickly add two 'doublefloat' numbers
inline doublefloat quick_two_sum(float a, float b)
{
    doublefloat result;
    result.high = a + b;
    result.low = b - (result.high - a);
    return result;
}

// 3. Double-Float Arithmetic

// Adds two doublefloat numbers
inline doublefloat df_add(doublefloat A, doublefloat B)
{
    doublefloat S = two_sum(A.high, B.high);
    doublefloat T = two_sum(A.low, B.low);
    doublefloat C = quick_two_sum(S.high, S.low + T.high);
    doublefloat result;
    result.high = C.high;
    result.low = T.low + C.low;
    return quick_two_sum(result.high, result.low);
}

// Multiplies two doublefloat numbers
inline doublefloat df_mul(doublefloat A, doublefloat B)
{
    doublefloat P = two_prod(A.high, B.high);
    float cross_prod = std::fmaf(A.high, B.low, P.low);
    cross_prod = cross_prod + (A.low * B.high);
    return quick_two_sum(P.high, cross_prod);
}

// Creates a doublefloat from a single float
inline doublefloat df_from_float(float val)
{
    doublefloat result;
    result.high = val;
    result.low = 0.0f;
    return result;
}

// 4. Complex Double-Float Arithmetic

// Adds two ComplexDD numbers
inline ComplexDD complex_df_add(ComplexDD A, ComplexDD B)
{
    ComplexDD result;
    result.Re = df_add(A.Re, B.Re);
    result.Im = df_add(A.Im, B.Im);
    return result;
}

// Multiplies two ComplexDD numbers
inline ComplexDD complex_df_mul(ComplexDD A, ComplexDD B)
{
    doublefloat Re_Re = df_mul(A.Re, B.Re);
    doublefloat Im_Im = df_mul(A.Im, B.Im);
    doublefloat Re_Im = df_mul(A.Re, B.Im);
    doublefloat Im_Re = df_mul(A.Im, B.Re);

    ComplexDD result;

    // Subtraction is just adding the negative
    Im_Im.high = -Im_Im.high;
    Im_Im.low = -Im_Im.low;
    result.Re = df_add(Re_Re, Im_Im);

    result.Im = df_add(Re_Im, Im_Re);

    return result;
}

// Gets the magnitude squared of a ComplexDD
inline doublefloat complex_df_mag_sqr(ComplexDD A)
{
    doublefloat Re_sqr = df_mul(A.Re, A.Re);
    doublefloat Im_sqr = df_mul(A.Im, A.Im);
    return df_add(Re_sqr, Im_sqr);
}