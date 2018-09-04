#include "move_component.h"
#include "../actor.h"

MoveComponent::MoveComponent(Actor* owner)
: Component(owner)
{

}

MoveComponent::~MoveComponent()
{

}
bool MoveComponent::Create()
{
	// �W���u�𐶐�.

	// �W���u�X�P�W�����ɓo�^.

	return true;
}
void MoveComponent::Destroy()
{
}

void MoveComponent::Update(Float32 deltaTime)
{
	DirectX::XMVECTOR vector = DirectX::XMVectorScale(velocity_, speed_);
	owner_->SetPosition(DirectX::XMVectorAdd(owner_->GetPosition(), vector));
}