cbuffer cbTransMatrix : register(b0)
{
	float4x4 WVP;	// ワールドビュープロジェクション行列.
};

struct VS_INPUT
{
	float3 pos : POSITION;		// 座標(まだレンダリングパイプラインで加工されていない).
	float3 normal : NORMAL;
	float4 color : COLOR;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;	// 座標(レンダリングパイプライン上では処理済みであるため、システム上で扱われる座標情報).
	float4 normal : NORMAL;
	float4 color : COLOR;
};

PS_INPUT VSMain(VS_INPUT input)
{
	PS_INPUT output;

	float4 pos = float4(input.pos, 1.0f);
	float4 normal = float4(input.normal, 1.0f);
	output.pos = mul(pos, WVP);
	output.normal = mul(normal, WVP);
	output.color = input.color;

	return output;
}

float4 PSMain(PS_INPUT input ) : SV_TARGET // ピクセルの色.
{
	return input.color;
}