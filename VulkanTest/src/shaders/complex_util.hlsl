// [[vk::push_constant]]

struct PushConstants
{
    uint MaxIterations;
    uint PlaneMode;
    uint colorMode;
    uint fractalType;
    float ZoomLevel;
    float colorScaler;
    float2 C_Const;
    float2 Z0_Const;
    float2 X_Const;
    float2 ScreenCenter;
    float2 ScreenSize;
};
    

struct Complex
{
    float Re;
    float Im;
};

Complex complex_mul(Complex a, Complex b)
{
    Complex result;
    result.Re = a.Re * b.Re - a.Im * b.Im;
    result.Im = a.Re * b.Im + a.Im * b.Re;
    return result;
}

Complex complex_exp(Complex z, Complex x)
{
    float r_sqr = z.Re * z.Re + z.Im * z.Im;
    float mag = sqrt(r_sqr);
    float arg = atan2(z.Im, z.Re);
    
    Complex logZ;
    logZ.Re = log(mag);
    logZ.Im = arg;

    Complex w = complex_mul(x, logZ);
    
    float exp_w_re = exp(w.Re);
    
    Complex result;
    result.Re = exp_w_re * cos(w.Im);
    result.Im = exp_w_re * sin(w.Im);
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

