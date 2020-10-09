#include "EpollFollow.h"

CEPollFollow::CEPollFollow()
{
    m_iEPollFd = -1; 
    m_pEPollEvents = NULL;
}

CEPollFollow::~CEPollFollow()
{
    if(m_pEPollEvents)
    {
        delete m_pEPollEvents[];
        m_pEPollEvents = NULL;
    }


    if (m_iEPollFd >= 0)
        close(m_iEPollFd);
}


int CEPollFollow::Create(int aMaxFd)
{
    if(m_iEPollFd>=0)
    {
        close(m_iEPollFd);
        m_iEPollFd = -1;
    }

    m_iEPollFd = epoll_create(aMaxFd);
    if(m_iEPollFd < 0)
    {
        return -1;
    }

    m_pEPollEvents = new epoll_event[aMaxFd];
    if(!m_pEPollEvents)
    {
       close(m_iEPollFd);
       return -2;
    }
    m_iMaxFd = aMaxFd;

    return 0;
}


int CEPollFollow::Add(int aFd, long long aKey, int aFlag)
{
    return ctl(aFd, aKey, EPOLL_CTL_ADD, aFlag);
}

int CEPollFollow::Del(int aFd)
{
    return ctl(aFd,0,EPOLL_CTL_DEL,0);
}

int CEPollFollow::Mod(int aFd, long long aKey,int aFlag)
{
    return ctl(aFd, aKey, EPOLL_CTL_MOD, aFlag);
}

/*
typedef union epoll_data {
    void *ptr;
    int fd;
    __uint32_t u32;
    __uint64_t u64;
} epoll_data_t;

struct epoll_event {
    __uint32_t events;
    epoll_data_t data;
};
*/

int CEPollFollow::ctl(int aFd, long long aKey, int aAction, int aFlag)
{
    epoll_event eVent;

    eVent.events = aFlag;
    eVent.data.u64 = aKey;

    if(aFd < 0)
        return -1;

    int iRet = epoll_ctl(m_iEPollFd, aAction, aFd, &eVent);

    if (iRet<0)
        return -2;
    else 
        return 0;
    
}

int CEPollFollow::Wait(int aTimeout)
{
    m_iEventNum = epoll_wait(m_iEPollFd,m_pEPollEvents, m_iMaxFd, aTimeout);
    m_iCurrEvtIdx = 0;
    return m_iEventNum;
}

int CEPollFollow::GetEvents(long long &aKey, unsigned &aEvent)
{
    if(m_iCurrEvtIdx >= m_iEventNum)
        return 0;

    aKey = m_pEPollEvents[m_iCurrEvtIdx].data.u64;
    aEvent = m_pEPollEvents[m_iCurrEvtIdx].events;

    m_iCurrEvtIdx++;
    return 1;
}
 

