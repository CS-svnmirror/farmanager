#pragma once

/*
synchro.hpp

����������� ������, �������, ������ � �.�.
*/
/*
Copyright � 1996 Eugene Roshal
Copyright � 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "farexcpt.hpp"

class CriticalSection: NonCopyable
{

public:
	CriticalSection() { InitializeCriticalSection(&object); }
	~CriticalSection() { DeleteCriticalSection(&object); }

	void lock() { EnterCriticalSection(&object); }
	void unlock() { LeaveCriticalSection(&object); }

private:
	CRITICAL_SECTION object;
};

class CriticalSectionLock: NonCopyable
{
public:
	CriticalSectionLock(CriticalSection &object): object(object) { object.lock(); }
	~CriticalSectionLock() { object.unlock(); }

private:
	CriticalSection &object;
};

class HandleWrapper: NonCopyable
{
protected:

	HANDLE h;
	string strName;

public:

	HandleWrapper() : h(nullptr) {}

	void SetName(const string& HashPart, const string& TextPart)
	{
		strName = GetNamespace() + std::to_wstring(make_hash(HashPart)) + L"_" + TextPart;
	}

	virtual const wchar_t *GetNamespace() const = 0;

	bool Opened() const { return h != nullptr; }

	bool Close()
	{
		if (!h) return true;
		bool ret = CloseHandle(h) != FALSE;
		h = nullptr;
		return ret;
	}

	bool Wait(DWORD Milliseconds = INFINITE) const{ return WaitForSingleObject(h, Milliseconds) == WAIT_OBJECT_0; }

	bool Signaled() const { return Wait(0); }

	virtual ~HandleWrapper() { Close(); }

private:
	friend class MultiWaiter;

	HANDLE GetHandle() const { return h; }
};

class Thread: public HandleWrapper
{
public:
	Thread() : m_ThreadId(0) {}

	virtual ~Thread() {}

	virtual const wchar_t *GetNamespace() const override { return L""; }

#if defined _MSC_VER && _MSC_VER < 1800
	template<typename T>
	bool Start(T&& Function) { return Starter(std::bind(Function)); }

	template<typename T, typename A1>
	bool Start(T&& Function, A1&& Arg1) { return Starter(std::bind(Function, Arg1)); }

	template<typename T, typename A1, typename A2>
	bool Start(T&& Function, A1&& Arg1, A2&& Arg2) { return Starter(std::bind(Function, Arg1, Arg2)); }

	template<typename T, typename A1, typename A2, typename A3>
	bool Start(T&& Function, A1&& Arg1, A2&& Arg2, A3&& Arg3) { return Starter(std::bind(Function, Arg1, Arg2, Arg3)); }

	template<typename T, typename A1, typename A2, typename A3, typename A4>
	bool Start(T&& Function, A1&& Arg1, A2&& Arg2, A3&& Arg3, A4&& Arg4) { return Starter(std::bind(Function, Arg1, Arg2, Arg3, Arg4)); }

	// and so on...
#else
	template<class T, class... A>
	bool Start(T&& Function, A&&... Args)
	{
		return Starter(std::bind(Function, Args...));
	}
#endif

	unsigned int GetId() const { return m_ThreadId; }

private:
	template<class T>
	bool Starter(T&& f)
	{
		assert(!h);

		auto Param = new T(std::move(f));
		if (!(h = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, Wrapper<T>, Param, 0, &m_ThreadId))))
		{
			delete Param;
			return false;
		}
		return true;
	}

	template<class T>
	static unsigned int WINAPI Wrapper(void* p)
	{
		EnableSeTranslation();

		auto pParam = reinterpret_cast<T*>(p);
		auto Param = std::move(*pParam);
		delete pParam;
		Param();
		return 0;
	}

	unsigned int m_ThreadId;
};

class Mutex: public HandleWrapper
{
public:

	Mutex() {}

	virtual ~Mutex() {}

	virtual const wchar_t *GetNamespace() const override { return L"Far_Manager_Mutex_"; }

	bool Open()
	{
		assert(!h);

		h = CreateMutex(nullptr, FALSE, strName.empty() ? nullptr : strName.data());
		return h != nullptr;
	}

	bool Lock() const { return Wait(); }

	bool Unlock() const { return ReleaseMutex(h) != FALSE; }
};

class AutoMutex: NonCopyable
{
public:

	AutoMutex(const wchar_t *HashPart=nullptr, const wchar_t *TextPart=nullptr)
	{
		m.SetName(HashPart, TextPart);
		m.Open();
		m.Lock();
	}

	~AutoMutex() { m.Unlock(); }

private:

	Mutex m;
};

class Event: public HandleWrapper
{
public:

	Event() {}

	virtual ~Event() {}

	virtual const wchar_t *GetNamespace() const override { return L"Far_Manager_Event_"; }

	bool Open(bool ManualReset=false, bool InitialState=false)
	{
		assert(!h);

		h = CreateEvent(nullptr, ManualReset, InitialState, strName.empty() ? nullptr : strName.data());
		return h != nullptr;
	}

	bool Set() const { return SetEvent(h) != FALSE; }

	bool Reset() const { return ResetEvent(h) != FALSE; }

	void Associate(OVERLAPPED& o) const { o.hEvent = h; }
};

template<class T> class SyncedQueue: NonCopyable {
	std::queue<T> Queue;
	CriticalSection csQueueAccess;

public:
	typedef T value_type;

	SyncedQueue() {}
	~SyncedQueue() {}

	bool Empty()
	{
		SCOPED_ACTION(CriticalSectionLock)(csQueueAccess);
		return Queue.empty();
	}

	void Push(const T& item)
	{
		SCOPED_ACTION(CriticalSectionLock)(csQueueAccess);
		Queue.push(item);
	}

	void Push(T&& item)
	{
		SCOPED_ACTION(CriticalSectionLock)(csQueueAccess);
		Queue.push(std::forward<T>(item));
	}

	bool PopIfNotEmpty(T& To)
	{
		SCOPED_ACTION(CriticalSectionLock)(csQueueAccess);
		if (!Queue.empty())
		{
			To = std::move(Queue.front());
			Queue.pop();
			return true;
		}
		return false;
	}
};

class MultiWaiter: NonCopyable
{
public:
	enum wait_mode
	{
		wait_any,
		wait_all
	};
	MultiWaiter() { Objects.reserve(10); }
	~MultiWaiter() {}
	void Add(const HandleWrapper& Object) { Objects.emplace_back(Object.GetHandle()); }
	void Add(HANDLE handle) { Objects.emplace_back(handle); }
	DWORD Wait(wait_mode Mode = wait_all, DWORD Milliseconds = INFINITE) const { return WaitForMultipleObjects(static_cast<DWORD>(Objects.size()), Objects.data(), Mode == wait_all, Milliseconds); }
	void Clear() {Objects.clear();}

private:
	std::vector<HANDLE> Objects;
};
