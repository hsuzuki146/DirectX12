#pragma once

#include <vector>
#include "common.h"

class Job;

class JobContainer final
{
public:
	JobContainer();
	~JobContainer();

	// 状態管理変数をクリアする.
	void MakeReady();

	// ジョブを登録する.
	void Register(Job * job);

	// ジョブの登録を解除する.
	void UnRegister(Job * job);

	// 実行するジョブを取得する.
	Job* Pop();

private:
	JobContainer(const JobContainer&) = delete;
	JobContainer& operator=(const JobContainer&) = delete;

private:
	static const UInt32 JOB_LIST_RESERVE = 128;

private:
	UInt32 current_index_ = { 0 };	// コンテナ内のジョブの実行管理用.

	std::vector<Job*> job_list_;	// ジョブ管理コンテナ.
};