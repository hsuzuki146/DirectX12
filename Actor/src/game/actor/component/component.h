#pragma once

#include "../../../common/common.h"
#include "../../../common/job.h"
#include "component_types.h"

class Actor;

class Component
{
public:
	virtual ~Component(){}
	virtual bool Create() = 0;
	virtual void Destroy() = 0;

protected:
	Component(Actor* owner) : owner_(owner) { }

protected:
	Actor*	owner_;	// �R���|�[�l���g���������Ă���A�N�^�[.
	Job		job_;	// �W���u.
};