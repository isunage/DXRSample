struct Vertex{
    float3 position;
    float2 texCoord;
};
struct MissConstants{
    float3 bgColor;
};
/*********************************GLOBAL RESOURCES*****************************************/
GlobalRootSignature globalRootSignature = {
    "DescriptorTable(UAV(u0),SRV(t0))"
};
RaytracingAccelerationStructure gRtScene: register(t0);
RWTexture2D<float4>             gOutput : register(u0);
/*********************************RAYGEN RESOURCES*****************************************/
/*LocalRootSignature            rayGenRootSignature  = {};*/
/*SubobjectToExportsAssociation rayGenAssociation    = {"rayGenRootSignature", "rayGen"};*/
/***********************************MISS RESOURCES*****************************************/
LocalRootSignature              missRootSignature = {
    "CBV(b0)"   //For Miss Color
};
SubobjectToExportsAssociation missAssociation      = {"missRootSignature"  ,    "miss"};
ConstantBuffer<MissConstants>   missConstants : register(b0);
/*******************************HITGROUP RESOURCES*****************************************/
TriangleHitGroup hitGroup =
{
    "","chs"
};
LocalRootSignature hitGroupRootSignature = {
    "SRV(t1)", //For Vertex Buffer
    "SRV(t2)"  //For  Index Buffer
};
SubobjectToExportsAssociation hitGroupAssociation = {"hitGroupRootSignature", "hitGroup"};
StructuredBuffer<Vertex>        vertices: register(t1);//register(8)
ByteAddressBuffer               indices : register(t2);//register(8)
uint3 GetIndices(uint triangleIndex) 
{
    uint address = triangleIndex * (4 * 3);
    return indices.Load3(address);
}
Vertex GetVertex(uint triangleIndex, float3 barycentrices)
{
    uint3 indices = GetIndices(triangleIndex);
    Vertex v = (Vertex)0;
    [unroll]
    for (uint i=0;i<3;++i)
    {
        uint index = indices[i];
        v.position += vertices[index].position * barycentrices[i];
        v.texCoord += vertices[index].texCoord * barycentrices[i];
    }
    return v;
}
/*********************************CONFIG RESOURCES*****************************************/
RaytracingShaderConfig          shaderConfig      = 
{
    16, 8 
};
RaytracingPipelineConfig        pipelineConfig    =
{
    1
};
StateObjectConfig               stateObjectConfig = 
{
    STATE_OBJECT_FLAGS_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS
};
float3 linear2Srgb(float3 c)
{
    float3 sq1 = sqrt(c);
    float3 sq2 = sqrt(sq1);
    float3 sq3 = sqrt(sq2);
    float3 srgb= 0.662002687 * sq1 + 0.684122060 * sq2 - 0.323583601 * sq3 - 0.0225411470 * c;
    return srgb;
}

struct RayPayload{
    float3 color;
};

[shader("raygeneration")]
void rayGen() {
    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim   = DispatchRaysDimensions();

    float2 crd = float2(launchIndex.xy);
    float2 dims = float2(launchDim.xy);

    float2 d = ((crd/dims) * 2.f - 1.f);
    float aspectRatio = dims.x / dims.y;

    RayDesc ray;
    ray.Origin = float3(0, 0, -2);
    ray.Direction = normalize(float3(d.x * aspectRatio, -d.y, 1));

    ray.TMin = 0;
    ray.TMax = 100000;

    RayPayload payload;
    TraceRay( gRtScene, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, ray, payload );
    float3 col = linear2Srgb(payload.color);
    gOutput[launchIndex.xy] = float4(col,1.0f);
}

[shader("miss")]
void miss(inout RayPayload payload) {
    payload.color = missConstants.bgColor;
}


[shader("closesthit")]
void chs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes  attribs) {
    
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
    
    uint triangleIndex = PrimitiveIndex();

    Vertex v = GetVertex(triangleIndex,barycentrics);
    
    payload.color = float3(v.texCoord,0.0f);
}
