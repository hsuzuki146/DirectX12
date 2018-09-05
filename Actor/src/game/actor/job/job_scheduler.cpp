#include "job_scheduler.h"
#include "../../../common/job.h"

JobScheduler::JobScheduler()
{
}

JobScheduler::~JobScheduler()
{
}

bool JobScheduler::Create()
{
	//ASSERT(JobScheduler::Containers::End < containers_.size());
	return true;
}

void JobScheduler::Destroy()
{
}

void JobScheduler::Register(Job* job, Containers container)
{
	ASSERT(job); 
	ASSERT(container < Containers::End);
	containers_[container].Register(job);
}

void JobScheduler::Unregister(Job* job)
{
}

void JobScheduler::Execute(Float32 deltaTime)
{
	// ジョブコンテナの順番をリセット.
	for (JobContainer& container : containers_)
	{
		container.MakeReady();
	}

	// 実行.
	for (JobContainer& container : containers_)
	{
		Job* job = container.Pop();
		if (!job)
		{
			continue;
		}
		job->Execute(deltaTime);
	}
}
