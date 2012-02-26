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

#pragma once

#include "base/Hash.hpp"
#include "base/DLLImports.hpp"

#ifdef FW_QT
#include <QThread>
#include <QMutex>
#include <QSemaphore>

class ThreadWrapper;
#endif

namespace FW
{
//------------------------------------------------------------------------

class Spinlock
{
public:
                        Spinlock        (void);
                        ~Spinlock       (void);

    void                enter           (void);
    void                leave           (void);

private:
                        Spinlock        (const Spinlock&); // forbidden
    Spinlock&           operator=       (const Spinlock&); // forbidden

private:
#ifdef FW_QT
    QMutex              m_mutex;
#else
    CRITICAL_SECTION    m_critSect;
#endif
};

//------------------------------------------------------------------------

class Semaphore
{
public:
                        Semaphore       (int initCount = 1, int maxCount = 1);
                        ~Semaphore      (void);

    bool                acquire         (int millis = -1);
    void                release         (void);

private:
                        Semaphore       (const Semaphore&); // forbidden
    Semaphore&          operator=       (const Semaphore&); // forbidden

private:
#ifdef FW_QT
    QSemaphore          m_semaphore;
#else
    HANDLE              m_handle;
#endif
};

//------------------------------------------------------------------------

class Monitor
{
public:
                        Monitor         (void);
                        ~Monitor        (void);

    void                enter           (void);
    void                leave           (void);
    void                wait            (void);
    void                notify          (void);
    void                notifyAll       (void);

private:
                        Monitor         (const Monitor&); // forbidden
    Monitor&            operator=       (const Monitor&); // forbidden

private:
    Spinlock            m_lock;
    Semaphore           m_ownerSem;
    Semaphore           m_waitSem;
    Semaphore           m_notifySem;
    volatile U32        m_ownerThread;
    volatile S32        m_enterCount;
    volatile S32        m_waitCount;
};

//------------------------------------------------------------------------

class Thread
{
public:
    typedef void        (*ThreadFunc)   (void* param);
    typedef void (*DeinitFunc)(void* data);

    enum
    {
        Priority_Min    = -15,
        Priority_Normal = 0,
        Priority_Max    = 15
    };

private:
    struct StartParams
    {
        Thread*         thread;
        ThreadFunc      userFunc;
        void*           userParam;
        Semaphore       ready;
    };

    struct UserData
    {
        void*           data;
        DeinitFunc      deinitFunc;
    };

public:
                        Thread          (void);
                        ~Thread         (void);

    void                start           (ThreadFunc func, void* param);

    static Thread*      getCurrent      (void);
    static Thread*      getMain         (void);
    static bool         isMain          (void);
    static U32          getID           (void);
    static void         sleep           (int millis);
    static void         yield           (void);

    int                 getPriority     (void);
    void                setPriority     (int priority);
    bool                isAlive         (void);
    void                join            (void);

    void*               getUserData     (const String& id);
    void                setUserData     (const String& id, void* data, DeinitFunc deinitFunc = NULL);

    static void         suspendAll      (void); // Called by fail().

private:
    void                refer           (void);
    void                unrefer         (void); // Deferred call to exited() once m_exited == true and refCount == 0.

    void                started         (void);
    void                exited          (void);

#ifndef FW_QT
    static DWORD WINAPI threadProc      (LPVOID lpParameter);
#endif

private:
                        Thread          (const Thread&); // forbidden
    Thread&             operator=       (const Thread&); // forbidden

private:
    static Spinlock     s_lock;
    static Thread*      s_mainThread;
    static Hash<U32, Thread*> s_threads;

    Spinlock            m_lock;
    S32                 m_refCount;
    bool                m_exited;

    Spinlock            m_startLock;
    U32                 m_id;
#ifdef FW_QT
    friend class ::ThreadWrapper;
    QThread*            m_handle;
#else
    HANDLE              m_handle;
#endif
    S32                 m_priority;

    Hash<String, UserData> m_userData;
};

}
