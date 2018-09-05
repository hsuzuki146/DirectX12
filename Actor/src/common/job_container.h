#pragma once

#include <vector>
#include "common.h"

class Job;

class JobContainer final
{
public:
	JobContainer();
	~JobContainer();

	// ��ԊǗ��ϐ����N���A����.
	void MakeReady();

	// �W���u��o�^����.
	void Register(Job * job);

	// �W���u�̓o�^����������.
	void UnRegister(Job * job);

	// ���s����W���u���擾����.
	Job* Pop();

private:
	JobContainer(const JobContainer&) = delete;
	JobContainer& operator=(const JobContainer&) = delete;

private:
	static const UInt32 JOB_LIST_RESERVE = 128;

private:
	UInt32 current_index_ = { 0 };	// �R���e�i���̃W���u�̎��s�Ǘ��p.

	std::vector<Job*> job_list_;	// �W���u�Ǘ��R���e�i.
};