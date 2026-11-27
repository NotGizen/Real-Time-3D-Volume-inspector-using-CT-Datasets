RWTexture2D<float4> OutColor : register(u0);

cbuffer Params : register(b1)
{
    float3 camPos;
    float fovTan;
    float3 camFwd;
    float aspect;
    float3 camRight;
    float pad0;
    float3 camUp;
    float pad1;
    uint2 outDim;
    uint2 pad2;
};

bool IntersectBox(float3 ro, float3 rd, float3 bmin, float3 bmax, out float tN, out float tF)
{
    float3 inv = 1.0 / rd;
    float3 t0 = (bmin - ro) * inv;
    float3 t1 = (bmax - ro) * inv;
    float3 tmin = min(t0, t1), tmax = max(t0, t1);
    tN = max(max(tmin.x, tmin.y), tmin.z);
    tF = min(min(tmax.x, tmax.y), tmax.z);
    return tF > max(tN, 0.0);
}

[numthreads(8, 8, 1)]
void main( uint2 DTid : SV_DispatchThreadID )
{
    if (any(DTid.xy >= outDim))
        return;

    float2 ndc = (DTid.xy + 0.5) / outDim * 2.0 - 1.0;
    ndc.y = -ndc.y;
    float3 rd = normalize(camFwd + ndc.x * aspect * fovTan * camRight + ndc.y * fovTan * camUp);

    float3 bg = float3(0.05, 0.05, 0.1);
    float tN, tF;
    float3 color = bg;
    if (IntersectBox(camPos, rd, float3(-0.5, -0.5, -0.5), float3(0.5, 0.5, 0.5), tN, tF))
    {
        const float density = 1.0;
        const float sigma = 3.0;
        float dist = tF - max(tN, 0.0);
        float transmittance = exp(-density * sigma * dist);
        color = lerp(float3(1.0, 0.5, 0.2), bg, transmittance);
		//OutColor[DTid.xy] = float4(1.f, 0.f, 0.f, 1.0);
        
    }
    OutColor[DTid.xy] = float4(color, 1.0f);
}

