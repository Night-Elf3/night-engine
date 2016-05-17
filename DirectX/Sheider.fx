Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0) {
	matrix World;
	matrix View;
	matrix Projection;
}
cbuffer LightConstantBuffer : register(b1) {
	float4 vLightColor[3];
	float4 vLightPos[3];
	int n;
	int temp[3];
};


struct VS_OUTPUT {
	float4 Pos : SV_POSITION;
	float4 Color : COLOR0;
	float2 Tex : TEXCOORD0;
};


VS_OUTPUT MapVertexShader(float4 Pos : POSITION, float4 Normal : NORMAL, float2 Tex : TEXTCOORD) {
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Pos = mul(Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.Tex = Tex;

	for (int i = 0; i < n; ++i)
		output.Color += saturate(dot(normalize(Normal), normalize((float3)vLightPos[i] - (float3)Pos))*vLightColor[i]);
	output.Color.a = 1;

	return output;
}
float4 MapPixelShader(VS_OUTPUT input) : SV_Target{
	return  txDiffuse.Sample(samLinear, input.Tex)*input.Color;
}

VS_OUTPUT CircleVertexShader(float4 Pos : POSITION, float4 Normal : NORMAL, float2 Tex : TEXTCOORD) {
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Pos = mul(Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.Tex = Tex;

	return output;
}
float4 CirclePixelShader(VS_OUTPUT input) : SV_Target{
	input.Color.r = 0;
	input.Color.g = .1;
	input.Color.b = 0;
	input.Color.a = 0;

	return  input.Color;
}