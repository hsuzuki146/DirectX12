#include "camera.h"

namespace 
{
	const XMVECTOR INIT_POS = { 20.0f, 30.0f,-30.0f };
	//const XMVECTOR INIT_POS = { 0, 0.0f, -300.0f };
}

void Camera::Initialize()
{
	eye_ = INIT_POS;
	at_ = XMVECTOR{ 0.0f, 0.0f, 0.0f };
	up_ = XMVECTOR{ 0.0f, 1.0f, 0.0f };
}

void Camera::Update()
{
	view_ = XMMatrixLookAtLH(eye_, at_, up_);

	projection_ = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(60.0f),
		static_cast<float>(SetupParam::GetInstance().GetParam().windowSize_.cx) / static_cast<float>(SetupParam::GetInstance().GetParam().windowSize_.cy),
		1.0f,
		10000.0f);

	// ビュープロジェクション行列.
	XMStoreFloat4x4(&mat_, XMMatrixTranspose(view_ * projection_));

}

void Camera::AddPosition(const FXMVECTOR& pos)
{
	eye_.m128_f32[0] += pos.m128_f32[0];
	eye_.m128_f32[1] += pos.m128_f32[1];
	eye_.m128_f32[2] += pos.m128_f32[2];
}