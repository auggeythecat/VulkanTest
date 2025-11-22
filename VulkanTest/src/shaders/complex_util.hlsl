struct doublefloat
{
    float high;
    float low;
};

struct ComplexDD
{
    doublefloat Re;
    float _pad0;
    float _pad1;
    doublefloat Im;
};

inline float df_to_float(doublefloat A)
{
    return A.high + A.low;
}

inline doublefloat two_prod_slow(float a, float b)
{
    float a_high = mad(a, 8388609.0, 0); // 2^23 + 1
    float a_low = a - a_high;
    float b_high = mad(b, 8388609.0, 0);
    float b_low = b - b_high;
    
    doublefloat res;
    res.high = a * b;
    res.low = (a_high * b_high - res.high) + (a_high * b_low) + (a_low * b_high) + (a_low * b_low);
    return res;
}

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

inline doublefloat two_prod(float a, float b)
{
    doublefloat result;
    result.high = a * b;
    
    result.low = mad(a, b, -result.high);
    return result;
}

inline doublefloat quick_two_sum(float a, float b)
{
    doublefloat result;
    result.high = a + b;
    result.low = b - (result.high - a);
    return result;
}

inline doublefloat df_neg(doublefloat A)
{
    A.high = -A.high;
    A.low = -A.low;
    return A;
}

inline doublefloat df_add(doublefloat Re, doublefloat Im)
{
    doublefloat S = two_sum(Re.high, Im.high);
    
    doublefloat T = two_sum(Re.low, Im.low);

    doublefloat C = quick_two_sum(S.high, S.low + T.high);
    doublefloat result;
    result.high = C.high;
    result.low = T.low + C.low;
    
    return quick_two_sum(result.high, result.low);
}

inline doublefloat df_sub(doublefloat A, doublefloat B)
{
    return df_add(A, df_neg(B));
}

inline doublefloat df_abs(doublefloat A)
{
    if (A.high < 0.0f)
    {
        return df_neg(A);
    }
    return A;
}

inline doublefloat df_mul(doublefloat Re, doublefloat Im)
{
    doublefloat P = two_prod(Re.high, Im.high);
    
    float cross_prod = mad(Re.high, Im.low, P.low);
    cross_prod = cross_prod + (Re.low * Im.high);
    
    return quick_two_sum(P.high, cross_prod);
}

inline doublefloat df_from_float(float val)
{
    doublefloat result;
    result.high = val;
    result.low = 0.0;
    return result;
}

inline bool df_gt_float(doublefloat A, float b) // A > b
{
    if (A.high > b)
        return true;
    if (A.high < b)
        return false;
    return A.low > 0.0f;
}

inline bool df_lt_float(doublefloat A, float b) // A < b
{
    if (A.high < b)
        return true;
    if (A.high > b)
        return false;
    return A.low < 0.0f;
}

inline bool df_eq_float(doublefloat A, float b) // A == b
{
    return A.high == b && A.low == 0.0f;
}

inline doublefloat df_div(doublefloat A, doublefloat B)
{
    float q0_h = A.high / B.high;
    doublefloat q0 = df_from_float(q0_h);
    
    doublefloat r = df_sub(A, df_mul(B, q0));
    
    float q1_h = r.high / B.high;
    doublefloat q1 = df_from_float(q1_h);

    return df_add(q0, q1); // q0 + q1
}

inline doublefloat df_sqrt(doublefloat A)
{
    if (A.high <= 0.0f)
        return df_from_float(0.0f); // Sqrt of <= 0 is 0
    
    float guess_f = sqrt(A.high);
    doublefloat X = df_from_float(guess_f);
    
    doublefloat onehalf = df_from_float(0.5f);
    
    X = df_mul(onehalf, df_add(X, df_div(A, X)));
    X = df_mul(onehalf, df_add(X, df_div(A, X)));
    X = df_mul(onehalf, df_add(X, df_div(A, X)));
    
    return X;
}

inline doublefloat df_log(doublefloat A)
{
    if (A.high <= 0.0f)
        return df_from_float(-1.#INF);
    
    float log_high = log(A.high);
    doublefloat x = df_div(A, df_from_float(A.high)); // x = (1 + A.l/A.h)
    x = df_sub(x, df_from_float(1.0f)); // x = A.l/A.h
    
    return df_add(df_from_float(log_high), x);
}

inline doublefloat df_exp(doublefloat A)
{
    float exp_high = exp(A.high);
    
    // exp(A.l) ~= 1 + A.l
    doublefloat exp_low = df_add(df_from_float(1.0f), df_from_float(A.low));
    
    return df_mul(df_from_float(exp_high), exp_low);
}

inline doublefloat df_cos(doublefloat A)
{
    float cos_high = cos(A.high);
    float sin_high = sin(A.high);
    
    // cos(A.l) ~= 1
    // sin(A.l) ~= A.l
    
    // result = cos_high * 1 - sin_high * A.l
    doublefloat term1 = df_from_float(cos_high);
    doublefloat term2 = df_mul(df_from_float(sin_high), df_from_float(A.low));
    
    return df_sub(term1, term2);
}

inline doublefloat df_sin(doublefloat A)
{
    float cos_high = cos(A.high);
    float sin_high = sin(A.high);
    
    // cos(A.l) ~= 1
    // sin(A.l) ~= A.l

    // result = sin_high * 1 + cos_high * A.l
    doublefloat term1 = df_from_float(sin_high);
    doublefloat term2 = df_mul(df_from_float(cos_high), df_from_float(A.low));
    
    return df_add(term1, term2);
}

inline doublefloat complex_df_mag_sqr(ComplexDD A)
{
    doublefloat Re_sqr = df_mul(A.Re, A.Re);
    doublefloat Im_sqr = df_mul(A.Im, A.Im);
    return df_add(Re_sqr, Im_sqr);
}

inline ComplexDD complex_df_add(ComplexDD A, ComplexDD B)
{
    ComplexDD result;
    result.Re = df_add(A.Re, B.Re);
    result.Im = df_add(A.Im, B.Im);
    return result;
}

inline ComplexDD complex_df_mul(ComplexDD A, ComplexDD B)
{
    doublefloat Re_Re = df_mul(A.Re, B.Re);
    doublefloat Im_Im = df_mul(A.Im, B.Im);
    doublefloat Re_Im = df_mul(A.Re, B.Im);
    doublefloat Im_Re = df_mul(A.Im, B.Re);
    
    ComplexDD result;
    
    // result.Re = Re_Re - Im_Im
    result.Re = df_sub(Re_Re, Im_Im);
    // result.Im = Re_Im + Im_Re
    result.Im = df_add(Re_Im, Im_Re);
    
    return result;
}

inline ComplexDD complex_df_neg(ComplexDD A)
{
    ComplexDD result;
    result.Re = df_neg(A.Re);
    result.Im = df_neg(A.Im);
    return result;
}

inline ComplexDD complex_df_conjugate(ComplexDD A)
{
    ComplexDD result;
    result.Re = A.Re;
    result.Im = df_neg(A.Im); // Only negate imaginary part
    return result;
}

inline ComplexDD complex_df_exp(ComplexDD X, ComplexDD Z)
{
    // 1. log(Z) = log|Z| + i*arg(Z)
    doublefloat mag_sqr = complex_df_mag_sqr(Z);
    doublefloat mag = df_sqrt(mag_sqr);
    doublefloat log_mag = df_log(mag);
    
    // arg(Z) = atan2(Z.im, Z.re)
    float arg_Z_f = atan2(Z.Im.high, Z.Re.high);
    doublefloat arg_Z = df_from_float(arg_Z_f);
    
    ComplexDD log_Z;
    log_Z.Re = log_mag;
    log_Z.Im = arg_Z;
    
    // 2. X * log(Z)
    ComplexDD W = complex_df_mul(X, log_Z);
    
    // 3. exp(W) = exp(W.re) * (cos(W.im) + i*sin(W.im))
    doublefloat exp_W_re = df_exp(W.Re);
    doublefloat cos_W_im = df_cos(W.Im);
    doublefloat sin_W_im = df_sin(W.Im);
    
    ComplexDD result;
    result.Re = df_mul(exp_W_re, cos_W_im);
    result.Im = df_mul(exp_W_re, sin_W_im);
    
    return result;
}


float3 catmull_rom(float3 p0, float3 p1, float3 p2, float3 p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;
    
    float3 C0 = p1;
    float3 C1 = 0.5 * (p2 - p0);
    float3 C2 = 0.5 * (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3);
    float3 C3 = 0.5 * (-p0 + 3.0 * p1 - 3.0 * p2 + p3);
    
    return C0 + C1 * t + C2 * t2 + C3 * t3;
}

float3 hsv_to_rgb(float3 c)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float3 palette_lerp(float t)
{
    uint colorNum = 4;
    const float3 colors[4] =
    {
        float3(0.5607843137, 0.1372549020, 0.9843137255),
        float3(0.5921568627, 0.2784313725, 0.7607843137),
        float3(0.1882352941, 0.0666666667, 0.2352941176),
        float3(0.0666666667, 0.0196078431, 0.1019607843)
    };
    
    t = t + 0.05;
    
    float scaled_t = t * (colorNum - 1.0);
    uint p1 = floor(scaled_t);
    uint p2 = min(p1 + 1, colorNum - 1);
    uint p0 = max(0, p1 - 1);
    uint p3 = min(p1 + 2, colorNum - 1);
    
    t = frac(scaled_t);
    
    
    float3 interpolatedColor = catmull_rom(colors[p0], colors[p1], colors[p2], colors[p3], scaled_t);
    
    return float4(interpolatedColor, 1.0);
    
}

