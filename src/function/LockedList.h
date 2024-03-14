//LockedList.h 带锁定的list
//
// Copyright (c) ct  All rights reserved.
// 版权所有 ct 保留所有权利
//

#pragma once

#include "config.h"
#include <list>
#include <mutex>
#include <condition_variable>
using namespace ns_my_std;

template<typename T>
class LockedList
{
private:
	list<T> m_data;
	mutex m_mutex;//互斥锁
public:
	bool locked_TryGetBegin(T& data)
	{
		bool ret;
		m_mutex.lock();
		typename list<T>::iterator it = m_data.begin();
		if (it != m_data.end())
		{
			data = *it;
			ret = true;
		}
		else
		{
			ret = false;
		}
		m_mutex.unlock();
		return ret;
	}
	void lock() { return m_mutex.lock(); }
	list<T>& getForForeach() { return m_data; }
	void unlock() { return m_mutex.unlock(); }
	size_t locked_size()
	{
		m_mutex.lock();
		size_t ret = m_data.size();
		m_mutex.unlock();
		return ret;
	}
	void locked_push_back(T const& data)
	{
		m_mutex.lock();
		m_data.push_back(data);
		m_mutex.unlock();
	}
	void locked_push_front(T const& data)
	{
		m_mutex.lock();
		m_data.push_front(data);
		m_mutex.unlock();
	}
	void locked_pop_front()
	{
		m_mutex.lock();
		m_data.pop_front();
		m_mutex.unlock();
	}
	void locked_clear()
	{
		m_mutex.lock();
		m_data.clear();
		m_mutex.unlock();
	}
};
