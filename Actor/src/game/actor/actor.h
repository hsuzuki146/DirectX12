#pragma once

#include "../../common/common.h"
#include <DirectXMath.h>

class Actor
{
public:
	Actor() {}
	virtual ~Actor() {}
	virtual bool Create() { return true; }
	virtual void Destroy() {}
	virtual void Update(Float32 deltaTime) {}

public:
	void SetId(UInt64 id) { id_ = id; }
	UInt64 GetId() const { return id_; }
	
	void SetPosition( const DirectX::XMVECTOR& position) { position_ = position; }
	const DirectX::XMVECTOR& GetPosition() const { return position_; }

	void SetRotation(const DirectX::XMVECTOR& rotation) { rotation_ = rotation; }
	const DirectX::XMVECTOR& GetRotation() const { return rotation_; }

private:
	UInt64				id_;											// 識別ID.
	DirectX::XMVECTOR	position_ = DirectX::XMVectorZero();			// 座標.
	DirectX::XMVECTOR	rotation_ = DirectX::XMQuaternionIdentity();	// 回転.
};
