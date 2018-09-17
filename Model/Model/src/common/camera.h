#pragma once

#include "common.h"

class Camera : public Singleton<Camera>
{
public:
	void Initialize();
	void Update();
	//void AddPosition( const FXMVECTOR& pos );
	//void SetRotateLookAt( const XMVECTOR& pos, Float32 length, Float32 add_deg ):
	void AddRotateLookAt(Float32 v, Float32 h);
	const XMFLOAT4X4& GetMatrix() const { return mat_; }

private:
	
	XMVECTOR eye_;
	XMVECTOR at_;
	XMVECTOR up_;

	XMMATRIX view_; 
	XMMATRIX projection_;
	XMFLOAT4X4 mat_;
};

#define CAMERA() (Camera::GetInstance())