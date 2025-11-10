struct VSInput
{
    uint vertexID : SV_VertexID;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput main(VSInput input)
{
    float2 positions[3] =
    {
        float2(-1.0f, -1.0f),
        float2(3.0f, -1.0f),
        float2(-1.0f, 3.0f),
    };
    
    VSOutput output;
    
    output.position = float4(positions[input.vertexID], 0.0f, 1.0f);
    return output;
}
