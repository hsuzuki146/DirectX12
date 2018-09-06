cbuffer cbTransMatrix : register(b0)
{
	float4x4 WVP;	// ���[���h�r���[�v���W�F�N�V�����s��.
};

struct VS_INPUT
{
	float3 pos : POSITION;		// ���W(�܂������_�����O�p�C�v���C���ŉ��H����Ă��Ȃ�).
	float3 normal : NORMAL;
	float4 color : COLOR;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;	// ���W(�����_�����O�p�C�v���C����ł͏����ς݂ł��邽�߁A�V�X�e����ň�������W���).
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

float4 PSMain(PS_INPUT input ) : SV_TARGET // �s�N�Z���̐F.
{
	return input.color;
}