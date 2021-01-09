//
// Created by Administrator on 2018/8/14.
//
#pragma once

#include <stdlib.h>
#include <memory>
#include <mutex>
#include <queue>
#include <list>

namespace Network
{
class CMessage
{
public:
	CMessage(uint8_t* pMsg, uint16_t wSize)
	{
		m_pMsg  = new uint8_t[wSize];
		m_wSize = wSize;
		memcpy(m_pMsg,pMsg,wSize);
	}
	~CMessage()
	{
		delete[](m_pMsg);
		m_pMsg  = NULL;
		m_wSize = 0;
	}
	uint8_t* Data()
	{
		return m_pMsg;
	}
	uint16_t Length()
	{
		return m_wSize;
	}
private:
	CMessage()
	{
	}
protected:
	uint8_t* m_pMsg;    // 消息指针
	uint16_t m_wSize;   // 消息长度
};

// Simple mutex-guarded queue
template<typename _T>
class LockedQueue
{
private:
	std::mutex     mutex;
	std::queue<_T> queue;
public:
	void push(_T value)
	{
		std::unique_lock<std::mutex> lock(mutex);
		queue.push(value);
	};

	// Get top message in the queue
	// Note: not exception-safe (will lose the message)
	_T pop()
	{
		std::unique_lock<std::mutex> lock(mutex);
		_T                           value;
		std::swap(value, queue.front());
		queue.pop();
		return value;
	};

	bool empty()
	{
		std::unique_lock<std::mutex> lock(mutex);
		return queue.empty();
	}
	uint32_t size()
	{
		std::unique_lock<std::mutex> lock(mutex);
		return queue.size();
	}
};

};

