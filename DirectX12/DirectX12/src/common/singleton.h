#pragma once

template<class T>
class Singleton
{
public:
	static T& GetInstance()
	{
		static T instance;
		return instance;
	}

protected:
	Singleton() {}
	virtual ~Singleton(){}
private:
	Singleton(const T&) = delete;
	Singleton& operator=(const Singleton&) = delete;
};