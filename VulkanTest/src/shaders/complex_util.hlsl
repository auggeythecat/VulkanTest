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

float3 hsv_to_rgb(float3 c)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float3 palette_lerp(float t)
{
    const float3 colors[3] =
    {
        float3(0.9882352941, 0.6901960784, 0.9529411765),
        float3(0.2392156863, 0.0196078431, 0.8666666667),
        float3(0.2784313725, 0.0627450980, 0.4117647059)
    };
    
    float scaled_t = t * (5.0 - 1.0);
    
    int index_a = (int) scaled_t;
    int index_b = min(index_a + 1, 2);
    
    float blend_t = frac(scaled_t);
    
    return lerp(colors[index_a], colors[index_b], blend_t);
}