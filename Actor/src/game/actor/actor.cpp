#include "actor.h"
#include "job/job_scheduler.h"

bool Actor::Create()
{
	// アクターの更新関数を登録.
	job_.SetFunction(this, &Actor::Update);
	JobScheduler::GetInstance().Register(&job_,
		JobScheduler::Containers::ActorUpdate);

	return true; 
}
