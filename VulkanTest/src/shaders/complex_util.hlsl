// [[vk::push_constant]]

struct PushConstants
{
    float ZoomLevel;
    uint MaxIterations;
    uint PlaneMode;
    float _padding0;
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


