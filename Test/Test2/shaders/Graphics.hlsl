struct VertexOut{
	float4 svPosition: SV_POSITION;
	float2 texCoord: TEXCOORD;
};
VertexOut vsMain(float3 position:POSITION, float2 texCoord:TEXCOORD){
	VertexOut vout;
	vout.svPosition = float4(position,1.0f);
	vout.texCoord   = texCoord;
	return vout;
}
float4 psMain(VertexOut pin): SV_TARGET  {
	return float4(pin.texCoord,1.0f-(pin.texCoord.x+pin.texCoord.y)/2.0f,1.0f);
};