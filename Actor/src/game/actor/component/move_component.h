#pragma once
#include "../../../common/common.h"
#include "component.h"

class Actor;

//�ړ������p�̃R���|�[�l���g.
class MoveComponent : public Component
{
public:
	static constexpr UInt16 ID() { return ComponentID::Move; }

public:
	MoveComponent(Actor* owner);
	virtual ~MoveComponent();
	virtual bool Create() override;
	virtual void Destroy() override;
	void Update(Float32 deltaTime);
	
	void SetVelocity(const DirectX::XMVECTOR& velocity) { velocity_ = velocity; }
	const DirectX::XMVECTOR& GetVelocity() const { return velocity_; }
	
	void SetSpeed(Float32 speed) { speed_ = speed; }
	Float32 GetSpeed() const { return speed_; }

private:
	DirectX::XMVECTOR velocity_ = DirectX::XMVectorZero();	// �ړ�����.
	Float32 speed_ = 0.0f;
};