#include "job_container.h"
#include "job.h"

JobContainer::JobContainer()
{
	job_list_.reserve(JOB_LIST_RESERVE);
}

JobContainer::~JobContainer()
{
}

void JobContainer::MakeReady()
{
	current_index_ = 0;
}

void JobContainer::Register(Job* job)
{
	job_list_.push_back(job);
	job->SetContainer(this);
}

void JobContainer::UnRegister(Job* job)
{
	std::vector<Job*>::const_iterator end = job_list_.cend();
	for (std::vector<Job*>::iterator it = job_list_.begin(); it != end; ++it)
	{
		if (*it == job)
		{
			// “o˜^‰ðœ.
			job->SetContainer(nullptr);
			job_list_.erase(it);
			break;
		}
	}
}

Job* JobContainer::Pop()
{
	const UInt32 index = current_index_++;
	if (index >= job_list_.size())
	{
		return nullptr;
	}
	return job_list_[index];
}