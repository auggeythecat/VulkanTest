#include "complex_util.hlsl"

cbuffer params : register(b0, space0)
{
    uint MaxIterations;
    uint PlaneMode;
    uint ColorMode;
    uint FractalType;
		
    float ZoomLevel;
    float colorScaler;
    float ScreenSizeX;
    float ScreenSizeY;

    ComplexDD C_Const;
    ComplexDD Z0_Const;
    ComplexDD X_Const;
    ComplexDD ScreenCenter;
	
};

struct PSInput
{
    float4 position : SV_POSITION;
};


float4 main(PSInput input) : SV_TARGET
{        
    doublefloat mapped_x = df_from_float(((input.position.x / ScreenSizeX) * 2.0) - 1.0);
    doublefloat mapped_y = df_from_float(((input.position.y / ScreenSizeY) * 2.0) - 1.0);
    
    float aspectRatio = (ScreenSizeX / ScreenSizeY);
    
    ComplexDD Varying_Param;
    
    Varying_Param.Re = df_add(df_mul(df_mul(mapped_x, df_from_float(ZoomLevel)), df_from_float(aspectRatio)), ScreenCenter.Re);
    Varying_Param.Im = df_add(df_mul(mapped_y, df_from_float(ZoomLevel)), ScreenCenter.Im);

    
    ComplexDD Zn;
    ComplexDD Cp;
    ComplexDD Xp;
    

    if (PlaneMode == 0)
    {
        Zn.Re = Z0_Const.Re;
        Zn.Im = Z0_Const.Im;
        Cp = Varying_Param;
        Xp.Re = X_Const.Re;
        Xp.Im = X_Const.Im;
    }
    else if (PlaneMode == 1)
    {
        Zn = Varying_Param;
        Cp.Re = C_Const.Re;
        Cp.Im = C_Const.Im;
        Xp.Re = X_Const.Re;
        Xp.Im = X_Const.Im;
    }
    else if (PlaneMode == 2)
    {
        Zn.Re = Z0_Const.Re;
        Zn.Im = Z0_Const.Im;
        Cp.Re = C_Const.Re;
        Cp.Im = C_Const.Im;
        Xp = Varying_Param;
    }
    else
    {
        return float4(1.0, 1.0, 1.0, 1.0);
    }
    
    if (df_eq_float(Zn.Re, 0.0) && df_eq_float(Zn.Im, 0.0))
    {
        Zn.Re = df_from_float(0.000001f);
    }
    
    uint iterations = MaxIterations;
    for (uint i = 0; i < MaxIterations; i++)
    {
        
        if (df_gt_float(complex_df_mag_sqr(Zn), 250.0))
        {
            iterations = i;
            break;
        }
        if (PlaneMode != 2)
        {
            if (FractalType == 0)
            {
                Zn = complex_df_exp(Zn, Xp);
            }
            else if (FractalType == 1)
            {
                Zn.Im = df_neg(Zn.Im);
                Zn = complex_df_exp(Zn, Xp);
            }
            else if (FractalType == 2)
            {
                ComplexDD Zn_abs;
                Zn_abs.Re = df_abs(Zn.Re);
                Zn_abs.Im = df_abs(Zn.Im);
                Zn = complex_df_exp(Zn_abs, Xp);
            }
        }
        else
        {
            if (df_eq_float(Xp.Re, 2.0) && df_eq_float(Xp.Im, 0.0))
            {
                if (FractalType == 0)
                {
                    Zn = complex_df_mul(Zn, Zn);
                }
                else if (FractalType == 1)
                {
                    Zn.Im = df_neg(Zn.Im);
                    Zn = complex_df_mul(Zn, Zn);
                }
                else if (FractalType == 2)
                {
                    ComplexDD Zn_abs;
                    Zn_abs.Re = df_abs(Zn.Re);
                    Zn_abs.Im = df_abs(Zn.Im);
                    Zn = complex_df_mul(Zn_abs, Zn_abs);
                }
            }
            else
            {
                if (FractalType == 0)
                {
                    Zn = complex_df_exp(Zn, Xp);
                }
                else if (FractalType == 1)
                {
                    Zn.Im = df_neg(Zn.Im);
                    Zn = complex_df_exp(Zn, Xp);
                }
                else if (FractalType == 2)
                {
                    ComplexDD Zn_abs;
                    Zn_abs.Re = df_abs(Zn.Re);
                    Zn_abs.Im = df_abs(Zn.Im);
                    Zn = complex_df_exp(Zn, Xp);

                }
            }
        }
       
        Zn = complex_df_add(Zn, Cp);
        
    }

    if (iterations != MaxIterations)
    {
        float mag_Z = sqrt(df_to_float(df_mul(Zn.Re, Zn.Re)) + df_to_float(df_mul(Zn.Im, Zn.Im)));
        float mag_X_sqr = df_to_float(df_mul(Xp.Re, Xp.Re)) + df_to_float(df_mul(Xp.Im, Xp.Im));
		
        float mu = float(iterations) + 1.0 - log(log(mag_Z)) / log(sqrt(max(0.000001f, mag_X_sqr)));
        
        
        if (ColorMode == 0)
        {
        
            float r = 0.5 + 0.5 * cos(mu * colorScaler + 0.0);
            float g = 0.5 + 0.5 * cos(mu * colorScaler + 2.0);
            float b = 0.5 + 0.5 * cos(mu * colorScaler + 4.0);

            return float4(r, g, b, 1.0);
        }
        else if (ColorMode == 1)
        {
            float hue = frac(mu * colorScaler);
            float sat = 1.0;
            float val = 1.0;

            float3 rgb = hsv_to_rgb(float3(hue, sat, val));
            return float4(rgb, 1.0);
        }
        else if (ColorMode == 2)
        {
            float t = frac(mu * colorScaler);
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
