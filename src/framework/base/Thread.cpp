/*
 *  Copyright (c) 2009-2011, NVIDIA Corporation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of NVIDIA Corporation nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "base/Thread.hpp"

#include <cassert>

using namespace FW;

#ifdef FW_QT

namespace
{

const float s_priorityX = float(QThread::TimeCriticalPriority-QThread::IdlePriority)/
        float(Thread::Priority_Max-Thread::Priority_Min);
const float s_priorityY = QThread::IdlePriority-Thread::Priority_Min*s_priorityX;

//------------------------------------------------------------------------

QThread::Priority fwToQt(int priority)
{
    return QThread::Priority(
                qBound<int>(QThread::IdlePriority,
                       qRound(s_priorityX*priority+s_priorityY),
                       QThread::TimeCriticalPriority));
}

//------------------------------------------------------------------------

int qtToFw(QThread::Priority priority)
{
    return qBound<int>(Thread::Priority_Min,
                  qRound((priority-s_priorityY)/s_priorityX),
                  Thread::Priority_Max);
}

}

//------------------------------------------------------------------------

class ThreadWrapper : public QThread
{
public:
    ThreadWrapper(Thread::StartParams& params);

protected:
    virtual void run();

private:
    Thread::StartParams& m_params;
};

//------------------------------------------------------------------------

#endif

//------------------------------------------------------------------------

Spinlock            Thread::s_lock;
Hash<U32, Thread*>  Thread::s_threads;
Thread*             Thread::s_mainThread    = NULL;

//------------------------------------------------------------------------

#ifdef FW_QT

//------------------------------------------------------------------------

Spinlock::Spinlock(void)
{
}

//------------------------------------------------------------------------

Spinlock::~Spinlock(void)
{
}

//------------------------------------------------------------------------

void Spinlock::enter(void)
{
    m_mutex.lock();
}

//------------------------------------------------------------------------

void Spinlock::leave(void)
{
    m_mutex.unlock();
}

//------------------------------------------------------------------------

Semaphore::Semaphore(int initCount, int maxCount)
    : m_semaphore(maxCount)
{
    if (initCount < maxCount)
        m_semaphore.acquire(maxCount - initCount);
}

//------------------------------------------------------------------------

Semaphore::~Semaphore(void)
{
}

//------------------------------------------------------------------------

bool Semaphore::acquire(int millis)
{
    return m_semaphore.tryAcquire(1, millis);
}

//------------------------------------------------------------------------

void Semaphore::release(void)
{
    m_semaphore.release();
}

//------------------------------------------------------------------------

#else

//------------------------------------------------------------------------

Spinlock::Spinlock(void)
{
    InitializeCriticalSection(&m_critSect);
}

//------------------------------------------------------------------------

Spinlock::~Spinlock(void)
{
    DeleteCriticalSection(&m_critSect);
}

//------------------------------------------------------------------------

void Spinlock::enter(void)
{
    EnterCriticalSection(&m_critSect);
}

//------------------------------------------------------------------------

void Spinlock::leave(void)
{
    LeaveCriticalSection(&m_critSect);
}

//------------------------------------------------------------------------

Semaphore::Semaphore(int initCount, int maxCount)
{
    m_handle = CreateSemaphore(NULL, initCount, maxCount, NULL);
    if (!m_handle)
        failWin32Error("CreateSemaphore");
}

//------------------------------------------------------------------------

Semaphore::~Semaphore(void)
{
    CloseHandle(m_handle);
}

//------------------------------------------------------------------------

bool Semaphore::acquire(int millis)
{
    DWORD res = WaitForSingleObject(m_handle, (millis >= 0) ? millis : INFINITE);
    if (res == WAIT_FAILED)
        failWin32Error("WaitForSingleObject");
    return (res == WAIT_OBJECT_0);
}

//------------------------------------------------------------------------

void Semaphore::release(void)
{
    if (!ReleaseSemaphore(m_handle, 1, NULL))
        failWin32Error("ReleaseSemaphore");
}

//------------------------------------------------------------------------

#endif

//------------------------------------------------------------------------

Monitor::Monitor(void)
:   m_ownerSem      (1, 1),
    m_waitSem       (0, 1),
    m_notifySem     (0, 1),
    m_ownerThread   (0),
    m_enterCount    (0),
    m_waitCount     (0)
{
}

//------------------------------------------------------------------------

Monitor::~Monitor(void)
{
}

//------------------------------------------------------------------------

void Monitor::enter(void)
{
    U32 currThread = Thread::getID();

    m_lock.enter();
    if (m_ownerThread != currThread || !m_enterCount)
    {
        m_lock.leave();
        m_ownerSem.acquire();
        m_lock.enter();
    }

    m_ownerThread = currThread;
    m_enterCount++;
    m_lock.leave();
}

//------------------------------------------------------------------------

void Monitor::leave(void)
{
    FW_ASSERT(m_ownerThread == Thread::getID() && m_enterCount);
    m_enterCount--;
    if (!m_enterCount)
        m_ownerSem.release();
}

//------------------------------------------------------------------------

void Monitor::wait(void)
{
    FW_ASSERT(m_ownerThread == Thread::getID() && m_enterCount);
    U32 currThread = m_ownerThread;
    int enterCount = m_enterCount;

    m_waitCount++;
    m_enterCount = 0;
    m_ownerSem.release();

    m_waitSem.acquire();
    m_waitCount--;
    m_notifySem.release();

    m_ownerSem.acquire();
    m_lock.enter();
    m_ownerThread = currThread;
    m_enterCount = enterCount;
    m_lock.leave();
}

//------------------------------------------------------------------------

void Monitor::notify(void)
{
    FW_ASSERT(m_ownerThread == Thread::getID() && m_enterCount);
    if (m_waitCount)
    {
        m_waitSem.release();
        m_notifySem.acquire();
    }
}

//------------------------------------------------------------------------

void Monitor::notifyAll(void)
{
    FW_ASSERT(m_ownerThread == Thread::getID() && m_enterCount);
    while (m_waitCount)
    {
        m_waitSem.release();
        m_notifySem.acquire();
    }
}

//------------------------------------------------------------------------

Thread::Thread(void)
:   m_refCount  (0),
    m_id        (0),
    m_handle    (NULL),
    m_priority  (Priority_Normal)
{
}

//------------------------------------------------------------------------

Thread::~Thread(void)
{
    // Wait and exit.

    if (this != getCurrent())
        join();
    else
    {
        failIfError();
        refer();
        m_exited = true;
        unrefer();
}

    // Deinit user data.

    for (int i = m_userData.firstSlot(); i != -1; i = m_userData.nextSlot(i))
    {
        const UserData& data = m_userData.getSlot(i).value;
        if (data.deinitFunc)
            data.deinitFunc(data.data);
    }
}

//------------------------------------------------------------------------

void Thread::start(ThreadFunc func, void* param)
{
    m_startLock.enter();
    join();

    StartParams params;
    params.thread       = this;
    params.userFunc     = func;
    params.userParam    = param;
    params.ready.acquire();

#ifdef FW_QT
    ThreadWrapper* handle = new ThreadWrapper(params);
    handle->start();
#else
    if (!CreateThread(NULL, 0, threadProc, &params, 0, NULL))
        failWin32Error("CreateThread");
#endif

    params.ready.acquire();
    m_startLock.leave();
}

//------------------------------------------------------------------------

Thread* Thread::getCurrent(void)
{
    s_lock.enter();
    Thread** found = s_threads.search(getID());
    Thread* thread = (found) ? *found : NULL;
    s_lock.leave();

    if (!thread)
    {
        thread = new Thread;
        thread->started();
    }
    return thread;
}

//------------------------------------------------------------------------

Thread* Thread::getMain(void)
{
    getCurrent();
    return s_mainThread;
}

//------------------------------------------------------------------------

bool Thread::isMain(void)
{
    Thread* curr = getCurrent();
    return (curr == s_mainThread);
}

//------------------------------------------------------------------------

#ifdef FW_QT

//------------------------------------------------------------------------

U32 Thread::getID(void)
{
    return QThread::currentThreadId();
}

//------------------------------------------------------------------------

void Thread::sleep(int millis)
{
    usleep(millis * 1000);
}

//------------------------------------------------------------------------

void Thread::yield(void)
{
    QThread::yieldCurrentThread();
}

//------------------------------------------------------------------------

int Thread::getPriority(void)
{
    refer();
    if (m_handle)
        m_priority = qtToFw(m_handle->priority());
    unrefer();

    return m_priority;
}

//------------------------------------------------------------------------

void Thread::setPriority(int priority)
{
    refer();
    m_priority = priority;
    if (m_handle)
        m_handle->setPriority(fwToQt(priority));
    unrefer();
}

//------------------------------------------------------------------------

bool Thread::isAlive(void)
{
    bool alive = false;
    refer();

    if (m_handle)
    {
        if (m_handle->isFinished())
            m_exited = true;
        else
            alive = true;
    }

    unrefer();
    return alive;
}

//------------------------------------------------------------------------

void Thread::join(void)
{
    FW_ASSERT(this != getMain());
    FW_ASSERT(this != getCurrent());

    refer();
    if (m_handle)
        m_handle->wait();
    m_exited = true;
    unrefer();
}

//------------------------------------------------------------------------

#else

//------------------------------------------------------------------------

U32 Thread::getID(void)
{
    return GetCurrentThreadId();
}

//------------------------------------------------------------------------

void Thread::sleep(int millis)
{
    Sleep(millis);
}

//------------------------------------------------------------------------

void Thread::yield(void)
{
    SwitchToThread();
}

//------------------------------------------------------------------------

int Thread::getPriority(void)
{
    refer();
    if (m_handle)
        m_priority = GetThreadPriority(m_handle);
    unrefer();

    if (m_priority == THREAD_PRIORITY_ERROR_RETURN)
        failWin32Error("GetThreadPriority");
    return m_priority;
}

//------------------------------------------------------------------------

void Thread::setPriority(int priority)
{
    refer();
    m_priority = priority;
    if (m_handle && !SetThreadPriority(m_handle, priority))
        failWin32Error("SetThreadPriority");
    unrefer();
}

//------------------------------------------------------------------------

bool Thread::isAlive(void)
{
    bool alive = false;
    refer();

    if (m_handle)
    {
    DWORD exitCode;
        if (!GetExitCodeThread(m_handle, &exitCode))
        failWin32Error("GetExitCodeThread");

    if (exitCode == STILL_ACTIVE)
            alive = true;
        else
            m_exited = true;
    }

    unrefer();
    return alive;
}

//------------------------------------------------------------------------

void Thread::join(void)
{
    FW_ASSERT(this != getMain());
    FW_ASSERT(this != getCurrent());

    refer();
    if (m_handle && WaitForSingleObject(m_handle, INFINITE) == WAIT_FAILED)
        failWin32Error("WaitForSingleObject");
    m_exited = true;
    unrefer();
}

//------------------------------------------------------------------------

#endif

//------------------------------------------------------------------------

void* Thread::getUserData(const String& id)
{
    m_lock.enter();
    UserData* found = m_userData.search(id);
    void* data = (found) ? found->data : NULL;
    m_lock.leave();
    return data;
}

//------------------------------------------------------------------------

void Thread::setUserData(const String& id, void* data, DeinitFunc deinitFunc)
{
    UserData oldData;
    oldData.data = NULL;
    oldData.deinitFunc = NULL;

    UserData newData;
    newData.data = data;
    newData.deinitFunc = deinitFunc;

    // Replace data.

    m_lock.enter();

    UserData* found = m_userData.search(id);
    if (found)
    {
        oldData = *found;
        *found = newData;
    }

    if ((found != NULL) != (data != NULL || deinitFunc != NULL))
    {
        if (found)
            m_userData.remove(id);
        else
            m_userData.add(id, newData);
    }

    m_lock.leave();

    // Deinit old data.

    if (oldData.deinitFunc)
        oldData.deinitFunc(oldData.data);
}

//------------------------------------------------------------------------

void Thread::suspendAll(void)
{
    s_lock.enter();
    for (int i = s_threads.firstSlot(); i != -1; i = s_threads.nextSlot(i))
    {
        Thread* thread = s_threads.getSlot(i).value;
        thread->refer();
        if (thread->m_handle && thread->m_id != getID()) {
#ifdef FW_QT
            setError("Thread::suspendAll isn't implemented with FW_QT");
#else
            SuspendThread(thread->m_handle);
#endif
        }
        thread->unrefer();
    }
    s_lock.leave();
}

//------------------------------------------------------------------------

void Thread::refer(void)
{
    m_lock.enter();
    m_refCount++;
    m_lock.leave();
}

//------------------------------------------------------------------------

void Thread::unrefer(void)
{
    m_lock.enter();
    if (--m_refCount == 0 && m_exited)
    {
        m_exited = false;
        exited();
}
    m_lock.leave();
}

//------------------------------------------------------------------------

void Thread::started(void)
{
    m_id = getID();
#ifdef FW_QT
    m_handle = QThread::currentThread();
    assert(m_handle);
#else
    m_handle = OpenThread(THREAD_ALL_ACCESS, FALSE, m_id);
    if (!m_handle)
        failWin32Error("OpenThread");
#endif

    s_lock.enter();

    if (!s_mainThread)
        s_mainThread = this;

    if (!s_threads.contains(m_id))
        s_threads.add(m_id, this);

    s_lock.leave();
}

//------------------------------------------------------------------------

void Thread::exited(void)
{
    if (!m_handle)
        return;

    s_lock.enter();

    if (this == s_mainThread)
        s_mainThread = NULL;

    if (s_threads.contains(m_id))
        s_threads.remove(m_id);

    if (!s_threads.getSize())
        s_threads.reset();

    s_lock.leave();

    if (m_handle) {
#ifdef FW_QT
        m_handle->deleteLater();
#else
        CloseHandle(m_handle);
#endif
    }
    m_id = 0;
    m_handle = NULL;
}

//------------------------------------------------------------------------

#ifdef FW_QT

//------------------------------------------------------------------------

ThreadWrapper::ThreadWrapper(Thread::StartParams& params)
    : m_params(params) {}

//------------------------------------------------------------------------

void ThreadWrapper::run()
{
    Thread*         thread      = m_params.thread;
    Thread::ThreadFunc userFunc    = m_params.userFunc;
    void*           userParam   = m_params.userParam;

    // Initialize.

    thread->started();
    thread->setPriority(thread->m_priority);
    m_params.ready.release();

    // Execute.

    userFunc(userParam);

    // Check whether the thread object still exists,
    // as userFunc() may have deleted it.

    Thread::s_lock.enter();
    bool exists = Thread::s_threads.contains(Thread::getID());
    Thread::s_lock.leave();

    // Still exists => deinit.

    if (exists)
    {
        failIfError();
        thread->getPriority();

        thread->refer();
        thread->m_exited = true;
        thread->unrefer();
    }
}

//------------------------------------------------------------------------

#else

//------------------------------------------------------------------------

DWORD WINAPI Thread::threadProc(LPVOID lpParameter)
{
    StartParams*    params      = (StartParams*)lpParameter;
    Thread*         thread      = params->thread;
    ThreadFunc      userFunc    = params->userFunc;
    void*           userParam   = params->userParam;

    // Initialize.

    thread->started();
    thread->setPriority(thread->m_priority);
    params->ready.release();

    // Execute.

    userFunc(userParam);

    // Check whether the thread object still exists,
    // as userFunc() may have deleted it.

    s_lock.enter();
    bool exists = s_threads.contains(getID());
    s_lock.leave();

    // Still exists => deinit.

    if (exists)
    {
        failIfError();
        thread->getPriority();

        thread->refer();
        thread->m_exited = true;
        thread->unrefer();
    }
    return 0;
}

#endif

//------------------------------------------------------------------------
