




class CEPollFollow
{

public:
    CEPollFollow();
    ~CEPollFollow();

    int Add(int aFd, long long aKey,int aFlag);
    int Del(int aFd);
    int Mod(int aFd, long long aKey,int aFlag);

    int GetEvents(long long &aKey, unsigned &aEvent);
    int Wait(int aTimeout);
    int Create(int aMaxFd);
private:
    int ctl(int aFd, long long aKey, int aAction, int aFlag);

private:
    int m_iEPollFd;
    int m_iMaxFd;
    epoll_event *m_pEPollEvents;

    int m_iEventNum;
    int m_iCurrEvtIdx;
    
};
