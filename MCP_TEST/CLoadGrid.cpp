
/*
总时间轴长度aTimeAllSpan毫秒
时间片长度aEachGridSpan 毫秒
总请求容量aMaxNumInAllSpan个
*/
CLoadGrid::CLoadGrid(int aTimeAllSpan, int aEachGridSpan, int aMaxNumInAllSpan = 0)
{
    m_iTimeAllSpanMs = aTimeAllSpan;
    m_iEachGridSpanMs = aEachGridSpan;
    m_iMaxNumInAllSpan = aMaxNumInAllSpan;
    m_iCurrGrid = 0;
    m_iNumInAllSpan = 0;
    
    m_iAllGridCount = (aTimeAllSpan+aEachGridSpan-1)/aEachGridSpan;
    m_iGridArray = new int[m_iAllGridCount];
    memset(m_iGridArray, 0, sizeof(int)*m_iAllGridCount);
    
    gettimeofday(m_tLastGridTime, NULL);

    m_bBlock = false;
    spin_lock_init(&m_spinlock);
}

CLoadGrid::~CLoadGrid()
{
    delete m_iGridArray[];
}

int CLoadGrid::CheckLoad(timeval aTimeVal)
{

    int iRet = 0;
    if(m_bBlock)
        spin_lock(&m_spinlock);
    do
    {
        if(m_iTimeAllSpanMs<=0 ||m_iEachGridSpanMs<=0)
        {
            iRet = LR_NORMAL;
            break;
        }

        //根据当前时间更新时间轴
        UpdateLoad(&aTimeVal);

        //累加本次
        m_iNumInAllSpan++;
        m_iGridArray[m_iCurrGrid]++;

        //到达临界
        if(m_iMaxNumInAllSpan&&(m_iNumInAllSpan >= m_iMaxNumInAllSpan))
            iRet = LR_FULL;
        else
            iRet = LR_NORMAL;
    }while(0);     

    if(m_bBlock)
        spin_unlock(&m_spinlock);

    return iRet;
    
}


/*
获取当前时间轴数据，获取之前要根据当前时间更新，否则
得到的是最后时刻的情况
*/
void CLoadGrid::FetchLoad(int& aTimeAllSpanMs, int& aNumInAllSpan)
{
    if(m_bBlock)
        spin_lock(&m_spinlock);

    UpdateLoad();
    aTimeAllSpanMs = m_iTimeAllSpanMs;
    aNumInAllSpan = m_iNumInAllSpan;
    
    if(m_bBlock)
        spin_unlock(&m_spinlock);
    return ;
    
}

void CLoadGrid::UpdateLoad(timeval *aTimeVal=NULL)
{
    if(m_iTimeAllSpanMs <= 0||m_iEachGridSpanMs <= 0)
        return ;

    timeval iCurrTime;
    if(aTimeVal)
        memcpy((void*)&iCurrTime, (void*)aTimeVal, sizeof(timeval));
    else
        gettimeofday(&iCurrTime, NULL);

    //流逝的时间
    int iTimeGoUs = (iCurrTime.tv_sec - m_tLastGridTime.tv_sec)*1000000 +
                    (iCurrTime.tv_usec - m_tLastGridTime.tv_usec)；

    //时间异常或流逝的时间超过总时间轴而过期
    if(iTimeGoUs<0 || iTimeGoUs > m_iTimeAllSpanMs*1000)
    {
        //全部清理
        memset((void*)m_iGridArray, 0, sizeof(int)*m_iAllGridCount);
        m_iNumInAllSpan = 0;
        m_iCurrGrid = 0;
        gettimeofday(&m_tLastGridTime, NULL);
        return;
    }

    //超过一个时间片
    if(iTimeGoUs > m_iEachGridSpanMs * 1000)
    {
        //流逝掉的时间片数
        int iGridGoNum = iTimeGoUs/(1000*m_iEachGridSpanMs);
        for(int i=0; i<iGridGoNum; i++)
        {
            m_iCurrGrid = (m_iCurrGrid+1)%m_iAllGridCount;
            m_iNumInAllSpan -= m_iGridArray[m_iCurrGrid];
            m_iGridArray[m_iCurrGrid] = 0;
        }

        //更新最后记录时间

        m_tLastGridTime.tv_sec += iTimeGoUs/1000000;
        m_tLastGridTime.tv_usec += (iTimeGoUs%1000000);

        if(m_tLastGridTime.tv_usec >= 1000000)
        {
            m_tLastGridTime.tv_sec++;
            m_tLastGridTime.tv_usec -= 1000000;
        }
    }
    //未超过一个时间片，无需更新
    else
    {
        ;
    }
    
}

