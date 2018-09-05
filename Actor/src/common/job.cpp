#include "job.h"

Job::Job()
{
}

Job::~Job()
{
}

void Job::Execute(Float32 deltaTime)
{
	function_(deltaTime);
}