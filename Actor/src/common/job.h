#pragma once

#include <functional>
#include "common.h"

class JobContainer;

class Job final
{
public:
	Job();
	~Job();

	// ���s����.
	void Execute(Float32 deltaTime);
	
	// �W���u�Ƃ��Ď��s����t�@���N�V������ݒ�.
	template<typename T>
	void SetFunction(T* obj, void(T::* func)(Float32));

	// �W���u�R���e�i�̐ݒ�.
	void SetContainer(JobContainer* container) { container_ = container; }

private:
	Job(const Job &) = delete;
	Job &operator=(const Job &) = delete;

private:
	std::function<void(Float32)> function_;	// �W���u�Ƃ��Ď��s����֐��I�u�W�F�N�g.
	
	JobContainer* container_ = { nullptr };	// �o�^�����R���e�i�̃|�C���^.

};

template<typename T>
void Job::SetFunction(T* obj, void(T::* func)(Float32 deltaTime))
{
	function_ = std::bind(std::mem_fn(func), obj, std::placeholders::_1);
}