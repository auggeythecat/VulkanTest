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

    uint iterations = 0;
    for (uint i = 0; i < params.MaxIterations; i++)
    {
        float magnitude_sqr = Zn.Re * Zn.Re + Zn.Im * Zn.Im;
        if (magnitude_sqr > 4.0)
        {
            iterations = i;
            break;
        }   
        
        if (Xp.Re == 2 && Xp.Im == 0)
        {
            Zn = complex_mul(Zn, Zn);
        }
        else
        {
            Zn = complex_exp(Zn, Xp);
        }
       
        Zn.Re += Cp.Re;
        Zn.Im += Cp.Im;
        
    }

    if (iterations != params.MaxIterations)
    {
        
        float mu = float(iterations) + 1.0 - log(log(sqrt(Zn.Re * Zn.Re + Zn.Im * Zn.Im))) / log(Xp.Re * Xp.Re + Xp.Im * Xp.Im);
        float r = 0.5 + 0.5 * sin(3.0 + mu * 0.15);
        float g = 0.5 + 0.5 * cos(3.0 + mu * 0.15 + 2.0);
        float b = 0.5 + 0.5 * cos(3.0 + mu * 0.15 + 4.0);

        return float4(r, g, b, 1.0);
    }
    return float4(0.0, 0.0, 0.0, 0.0);
}
