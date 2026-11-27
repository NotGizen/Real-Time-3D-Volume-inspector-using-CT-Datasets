RWTexture2D<float4> OutColor : register(u0);

[numthreads(8, 8, 1)]
void main( uint2 DTid : SV_DispatchThreadID )
{
    OutColor[DTid.xy] = float4(1.0f, 0.0f, 0.0f, 1.0f);
}

