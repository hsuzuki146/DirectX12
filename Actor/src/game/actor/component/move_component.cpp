#include "move_component.h"
#include "../actor.h"
#include "../job/job_scheduler.h"

MoveComponent::MoveComponent(Actor* owner)
: Component(owner)
{

}

MoveComponent::~MoveComponent()
{

}
bool MoveComponent::Create()
{
	// ジョブを生成.
	job_.SetFunction(this, &MoveComponent::Update);
	// ジョブスケジュラに登録.
	JobScheduler& job_scheduler = JobScheduler::GetInstance();
	job_scheduler.Register(&job_, JobScheduler::Containers::MoveUpdate);

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