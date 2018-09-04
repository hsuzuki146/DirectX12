#pragma once

#include "../../common/common.h"
#include <map>

class Component;

class Actor
{
public:
	Actor() {}
	virtual ~Actor() {}
	virtual bool Create() { return true; }
	virtual void Destroy() {}
	virtual void Update(Float32 deltaTime) {}

public:
	void SetId(UInt64 id) { id_ = id; }
	UInt64 GetId() const { return id_; }
	
	void SetPosition( const DirectX::XMVECTOR& position) { position_ = position; }
	const DirectX::XMVECTOR& GetPosition() const { return position_; }

	void SetRotation(const DirectX::XMVECTOR& rotation) { rotation_ = rotation; }
	const DirectX::XMVECTOR& GetRotation() const { return rotation_; }

public:
	// コンポーネントの追加.
	template<typename T>
	T* AddComponent();

	// コンポーネントノの取得.
	template<typename T>
	T* GetComponent();

private:
	UInt64				id_;											// 識別ID.
	std::map<UInt64, Component*> components_;
	DirectX::XMVECTOR	position_ = DirectX::XMVectorZero();			// 座標.
	DirectX::XMVECTOR	rotation_ = DirectX::XMQuaternionIdentity();	// 回転.
};

template<typename T>
T* Actor::AddComponent()
{
	ASSERT(GetComponent<T>() == nullptr);

	T* component = new T(this);
	component->Create();
	components_.insert(std::pair<UInt64, Component*>(T::ID(), component));

	return component;
}

template<typename T>
T* Actor::GetComponent()
{

	std::map<UInt64, Component*>::iterator it = components_.find(T::ID());
	if (it != components_.end())
	{
		it->second->Destroy();
		return static_cast<T*>(it->second);
	}
	return nullptr;
}
