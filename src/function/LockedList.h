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
class LockedList :private list<T>
{
private:
	mutex m_mutex;//互斥锁
public:
	using list<T>::iterator;
	typename list<T>::iterator locked_begin()
	{
		m_mutex.lock();
		typename list<T>::iterator it = this->begin();
		m_mutex.unlock();
		return it;
	}
	typename list<T>::iterator locked_end()
	{
		m_mutex.lock();
		typename list<T>::iterator it = this->end();
		m_mutex.unlock();
		return it;
	}
	size_t locked_size()
	{
		m_mutex.lock();
		size_t ret = this->size();
		m_mutex.unlock();
		return ret;
	}
	void locked_push_back(T const& data)
	{
		m_mutex.lock();
		this->push_back(data);
		m_mutex.unlock();
	}
	void locked_push_front(T const& data)
	{
		m_mutex.lock();
		this->push_front(data);
		m_mutex.unlock();
	}
	void locked_pop_front()
	{
		m_mutex.lock();
		this->pop_front();
		m_mutex.unlock();
	}
	void locked_clear()
	{
		m_mutex.lock();
		this->clear();
		m_mutex.unlock();
	}
};
