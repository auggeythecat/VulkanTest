#include "complex_util.hlsl"

[[vk::push_constant]]
PushConstants params;

struct PSInput
{
    float4 position : SV_POSITION;
    float3 color    : COLOR;
};


float4 main(PSInput input) : SV_TARGET
{
    
    float mapped_x = ((input.position.x / params.ScreenSize.x) * 2.0) - 1.0;
    float mapped_y = ((input.position.y / params.ScreenSize.y) * 2.0) - 1.0;
    
    Complex Varying_Param;
    
    Varying_Param.Re = mapped_x * params.ZoomLevel * (params.ScreenSize.x / params.ScreenSize.y) + params.ScreenCenter.x;
    Varying_Param.Im = mapped_y * params.ZoomLevel + params.ScreenCenter.y;

    
    Complex Zn;
    Complex Cp;
    Complex Xp;
    

    if (params.PlaneMode == 0)
    {
        Zn.Re = params.Z0_Const.x;
        Zn.Im = params.Z0_Const.y;
        Cp = Varying_Param;
        Xp.Re = params.X_Const.x;
        Xp.Im = params.X_Const.y;
    }
    else if (params.PlaneMode == 1)
    {
        Zn = Varying_Param;
        Cp.Re = params.C_Const.x;
        Cp.Im = params.C_Const.y;
        Xp.Re = params.X_Const.x;
        Xp.Im = params.X_Const.y;
    }
    else if (params.PlaneMode == 2)
    {
        Zn.Re = params.Z0_Const.x;
        Zn.Im = params.Z0_Const.y;
        Cp.Re = params.C_Const.x;
        Cp.Im = params.C_Const.y;
        Xp = Varying_Param;
    }
    else
    {
        return float4(1.0, 1.0, 1.0, 1.0);
    }
    
    if (Zn.Re == 0.0 && Zn.Im == 0.0)
    {
        Zn.Re = 0.000001f;
    }
    
    uint iterations = params.MaxIterations;
    for (uint i = 0; i < params.MaxIterations; i++)
    {
        float magnitude_sqr = Zn.Re * Zn.Re + Zn.Im * Zn.Im;
        if (magnitude_sqr > 20.0)
        {
            iterations = i;
            break;
        }
        if (!params.PlaneMode == 2)
        {
        if (Xp.Re == 2 && Xp.Im == 0)
            {
                if (params.fractalType == 0)
                {
                    Zn = complex_mul(Zn, Zn);
                }
                else if (params.fractalType == 1)
                {
                    Zn.Im = -Zn.Im;
                    Zn = complex_mul(Zn, Zn);
                }
                else if (params.fractalType == 2)
                {
                    Complex Zn_abs;
                    Zn_abs.Re = abs(Zn.Re);
                    Zn_abs.Im = abs(Zn.Im);
                    Zn = complex_mul(Zn_abs, Zn_abs);
                }
            }
        }
        else
        {
            if (params.fractalType == 0)
            {
                Zn = complex_exp(Zn, Xp);
            }
            else if (params.fractalType == 1)
            {
                Zn.Im = -Zn.Im;
                Zn = complex_exp(Zn, Xp);
            }
            else if (params.fractalType == 2)
            {
                Complex Zn_abs;
                Zn_abs.Re = abs(Zn.Re);
                Zn_abs.Im = abs(Zn.Im);
                Zn = complex_exp(Zn_abs, Xp);
            }
        }
       
        Zn.Re += Cp.Re;
        Zn.Im += Cp.Im;
        
    }

    if (iterations != params.MaxIterations)
    {
        float mag_Z = sqrt(Zn.Re * Zn.Re + Zn.Im * Zn.Im);
        float mag_X_sqr = Xp.Re * Xp.Re + Xp.Im * Xp.Im;
		
        float mu = float(iterations) + 1.0 - log(log(mag_Z)) / log(sqrt(max(0.000001f, mag_X_sqr)));
        
        
        if (params.colorMode == 0)
        {
        
            float r = 0.5 + 0.5 * cos(mu * params.colorScaler + 0.0);
            float g = 0.5 + 0.5 * cos(mu * params.colorScaler + 2.0);
            float b = 0.5 + 0.5 * cos(mu * params.colorScaler + 4.0);

            return float4(r, g, b, 1.0);
        }
        else if (params.colorMode == 1)
        {
            float hue = frac(mu * params.colorScaler);
            float sat = 1.0;
            float val = 1.0;

            float3 rgb = hsv_to_rgb(float3(hue, sat, val));
            return float4(rgb, 1.0);
        }
        else if (params.colorMode == 2)
        {
            float t = frac(mu * params.colorScaler);
            float3 rgb = palette_lerp(t);
            return float4(rgb, 1.0);
        }
        else
        {
            return float4(1.0, 1.0, 1.0, 1.0);
        }

    }
    return float4(0.0, 0.0, 0.0, 1.0);
}
