
/*
��ʱ���᳤��aTimeAllSpan����
ʱ��Ƭ����aEachGridSpan ����
����������aMaxNumInAllSpan��
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

        //���ݵ�ǰʱ�����ʱ����
        UpdateLoad(&aTimeVal);

        //�ۼӱ���
        m_iNumInAllSpan++;
        m_iGridArray[m_iCurrGrid]++;

        //�����ٽ�
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
��ȡ��ǰʱ�������ݣ���ȡ֮ǰҪ���ݵ�ǰʱ����£�����
�õ��������ʱ�̵����
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

    //���ŵ�ʱ��
    int iTimeGoUs = (iCurrTime.tv_sec - m_tLastGridTime.tv_sec)*1000000 +
                    (iCurrTime.tv_usec - m_tLastGridTime.tv_usec)��

    //ʱ���쳣�����ŵ�ʱ�䳬����ʱ���������
    if(iTimeGoUs<0 || iTimeGoUs > m_iTimeAllSpanMs*1000)
    {
        //ȫ������
        memset((void*)m_iGridArray, 0, sizeof(int)*m_iAllGridCount);
        m_iNumInAllSpan = 0;
        m_iCurrGrid = 0;
        gettimeofday(&m_tLastGridTime, NULL);
        return;
    }

    //����һ��ʱ��Ƭ
    if(iTimeGoUs > m_iEachGridSpanMs * 1000)
    {
        //���ŵ���ʱ��Ƭ��
        int iGridGoNum = iTimeGoUs/(1000*m_iEachGridSpanMs);
        for(int i=0; i<iGridGoNum; i++)
        {
            m_iCurrGrid = (m_iCurrGrid+1)%m_iAllGridCount;
            m_iNumInAllSpan -= m_iGridArray[m_iCurrGrid];
            m_iGridArray[m_iCurrGrid] = 0;
        }

        //��������¼ʱ��

        m_tLastGridTime.tv_sec += iTimeGoUs/1000000;
        m_tLastGridTime.tv_usec += (iTimeGoUs%1000000);

        if(m_tLastGridTime.tv_usec >= 1000000)
        {
            m_tLastGridTime.tv_sec++;
            m_tLastGridTime.tv_usec -= 1000000;
        }
    }
    //δ����һ��ʱ��Ƭ���������
    else
    {
        ;
    }
    
}

