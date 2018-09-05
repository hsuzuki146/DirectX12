#pragma once

#include "../../../common/job_container.h"
#include <array>

class JobScheduler final : public Singleton<JobScheduler>
{
public:
	enum Containers : UInt32 
	{
		ActorUpdate,
		MoveUpdate,
		End,
	};

public:
	JobScheduler();
	~JobScheduler();

	bool Create();
	void Destroy();

	void Register(Job* job, Containers container);
	void Unregister(Job* job);

	void Execute(Float32 deltaTime);

private:
	JobScheduler(const JobScheduler&) = delete;
	JobScheduler& operator=(const JobScheduler&) = delete;

private:
	std::array<JobContainer, Containers::End > containers_;

};