#include "camera.h"

namespace 
{
	//const XMVECTOR INIT_POS = { 5.0f, 5.0f,-10.0f };
	const XMVECTOR INIT_POS = { 0, -30.0f, 300.0f };
}

void Camera::Initialize()
{
	eye_ = INIT_POS;
	at_ = XMVECTOR{ 0.0f, 0.0f, 0.0f };
	up_ = XMVECTOR{ 0.0f, 1.0f, 0.0f };
}

void Camera::Update()
{
	{
		static Float32 rad = 60.0f;
		static Float32 range = 300.0f;
		rad = 0.01f;
#if 0
		eye_.m128_f32[0] = range * sin(rad);
		eye_.m128_f32[1] = eye_.m128_f32[1];
		eye_.m128_f32[2] = range * cos(rad);
#else
		//AddRotateLookAt(0.0f, rad);
#endif
	}
	{
		view_ = XMMatrixLookAtLH(eye_, at_, up_);
		XMMATRIX m;
		static Float32 rotX = 0.0f; 
		static Float32 rotY = 0.0f;
		static Float32 rotZ = 0.0f; 
		m = XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationX(rotX) * XMMatrixRotationY(rotY) * XMMatrixRotationZ(rotZ);
		view_ = m * view_;
		//rotX += 0.01f; 
		rotY += 0.01f;
		rotZ += 0.00f;
	}

	projection_ = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60.0f),
		static_cast<float>(SetupParam::GetInstance().GetParam().windowSize_.cx) / static_cast<float>(SetupParam::GetInstance().GetParam().windowSize_.cy),
		1.0f,
		10000.0f);

	// ビュープロジェクション行列.
	XMStoreFloat4x4(&mat_, XMMatrixTranspose(view_ * projection_));

}

#if 0
void Camera::AddPosition(const FXMVECTOR& pos)
{
	eye_.m128_f32[0] += pos.m128_f32[0];
	eye_.m128_f32[1] += pos.m128_f32[1];
	eye_.m128_f32[2] += pos.m128_f32[2];
}
#else
void Camera::AddRotateLookAt(Float32 v, Float32 h)
{
	XMVECTOR rot = {};
	rot.m128_f32[0] = cos(h) * cos(v);
	rot.m128_f32[1] = sin(v);
	rot.m128_f32[2] = sin(h) * cos(v);
	//rot.m128_f32[0] = 300.0f * sinf(h);
	//rot.m128_f32[1] = 0.0f;
	//rot.m128_f32[2] = 300.0f * cosf(h);

	eye_ += rot;
}
#endif