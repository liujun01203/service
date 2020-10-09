
CBuffMng::CBuffMng()
{
    m_pBuffItem = NULL;
    m_pBuffMngHead = NULL;
    m_pIdxObjMng = NULL;
    m_iInitType = emInit;
}

CBuffMng::~CBuffMng()
{
}

ssize_t CBuffMng::FreeObjList(ssize_t iObjIdx)
{
    if (!m_pIdxObjMng)
        return -1;

    const ssize_t BLOCKNUM = m_pIdxObjMng->GetObjNum();
    if (iObjIdx<0 || iObjIdx>=BLOCKNUM)
        return 0;

    ssize_t iCount = 0;
    ssize_t iFreeIdx = iObjIdx;
    while (iFreeIdx >= 0)
    {
        ssize_t iNextFreeIdx = m_pIdxObjMng->GetDsIdx(iFreeIdx,m_pBuffMngHead->m_iDSSuffix);
        m_pIdxObjMng->DestroyObject(iFreeIdx);
        iFreeIdx = iNextFreeIdx;
        iCount++;
    }
    return iCount;
}

ssize_t CBuffMng::AttachMem(char *aBuf, ssize_t MEMSIZE, ssize_t aBuffNum, ssize_t aInitType=emInit)
{

	if(NULL==aBuf)
	{
		return -1;
	}
	
    if(emInit==aInitType)
    {
        if(MEMSIZE < CountMemSize(aBuffNum))
	        return -1;
	        
        m_pBuffMngHead = (TBuffMngHead*)aBuf;
	    m_pBuffItem = (TBuffItem*)(aBuf+sizeof(TBuffMngHead));
	    m_pBuffMngHead->m_iBuffNum = aBuffNum;
	    m_pBuffMngHead->m_iFreeBuffNum = aBuffNum;
	    m_pBuffMngHead->m_iDSSuffix = -1;

	    memset(m_pBuffItem, -1, sizeof(TBuffItem)*aBuffNum);	   
    }
    else
    {
		m_pBuffMngHead = (TBuffMngHead*)aBuf;
		if(m_pBuffMngHead->m_iBuffNum != aBuffNum)
			return -2;
    }
	m_iInitType = aInitType;
	return  CountMemSize(aBuffNum);
}

ssize_t CBuffMng::AttachIdxObjMng(TIdxObjMng *aIdxObjMng)
{
    if(NULL == m_pBuffMngHead||NULL == aIdxObjMng)
	    return -1;

	m_pIdxObjMng = aIdxObjMng;

	if (emInit == m_iInitType)
	{
	    ssize_t iDSSuffix  = m_pIdxObjMng->GetOneFreeDS();
	    if(iDSSuffix<0)
	    {
	        m_pIdxObjMng = NULL:
	    	return -1;
	    }

	    m_pBuffMngHead->m_iDSSuffix = iDSSuffix;
	}
	else
	{
		;
	}

	return 0;

	
}


ssize_t CBuffMng::CountMemSize(ssize_t aBuffNum)
{
    if(aBuffNum<=0)
	    return -1;
	    
    return sizeof(TBuffMngHead)+sizeof(TBuffItem)*aBuffNum;
}


ssize_t CBuffMng::SetBufferSpace(ssize_t aBuffItemNo, ssize_t aBuffLen)
{
    ssize_t iRet = 0;
    if(aBuffItemNo>m_pBuffMngHead->m_iBuffNum)
	    return -1;

	if(m_pBuffItem[aBuffItemNo].m_iBuffIdx > 0)
	{
	    iRet = FreeBuffer(aBuffItemNo);
	    if(iRet < 0)
		    return iRet;
	}

	if(!HaveFreeSpace(aBuffLen))
		return -2;

    ssize_t iFirstIdx = m_pIdxObjMng->CreateObject();
    if(iFirstIdx < 0)
	    return iFirstIdx;

	m_pBuffItem[aBuffItemNo].m_iBuffIdx = iFirstIdx;	
    m_pBuffMngHead->m_iFreeBuffNum--;  // should be place here 


	ssize_t iObjSize = m_pIdxObjMng->GetObjSize();
	ssize_t iObjNum = aBuffLen/iObjSize;
	if(aBuffLen%iObjSize)
		iObjNum++;

    for(ssize_t i=1; i<iObjNum;i++)
    {
        ssize_t iNextIdx = m_pIdxObjMng->CreateObject();
        if(iNextIdx < 0)
        {
    		FreeBuffer(aBuffItemNo);
		    return iNextIdx;
        }

        iRet = m_pIdxObjMng->SetDsIdx(iFirstIdx, m_pBuffMngHead->m_iDSSuffix, iNextIdx);

        if (iRet < 0)
        {
            FreeBuffer(aBuffItemNo);
		    return iRet;
        }
        iFirstIdx = iNextIdx;
    }

    m_pBuffItem[aBuffItemNo].m_iBuffSize = aBuffLen;
    //m_pBuffMngHead->m_iFreeBuffNum--;  // should not be place here 

 #ifdef
     m_pBuffItem[aBuffItemNo].m_iBuffOffset = 0;
     m_pBuffItem[aBuffItemNo].m_iLastBuffIdx = iFirstIdx;
 #endif

	return 0;
	
}


//拷贝数据到指定的缓冲区， 缓冲区之前已经分配好
ssize_t CBuffMng::SetDataToBufferSpace(ssize_t aBuffItemNo, char* aBuf,ssize_t aBuffLen)
{
    ssize_t iRet = 0;
    if(aBuffItemNo>m_pBuffMngHead->m_iBuffNum)
	    return -1;

	if(m_pBuffItem[aBuffItemNo].m_iBuffSize != aBuffLen)
	    return -2;

     if(m_pBuffItem[aBuffItemNo].m_iBuffIdx < 0)
	    return -3;
	  
	 ssize_t iBlockSize = m_pIdxObjMng->GetObjSize();

	 ssize_t iIdx = m_pBuffItem[aBuffItemNo].m_iBuffIdx;
	 ssize_t iCopyedLen = 0;

	 while(iCopyedLen < aBuffLen)
	 {
	     if(iIdx<0)
		     assert(false);
	     ssize_t iCopyLen = (aBuffLen-iCopyedLen)<iBlockSize?(aBuffLen-iCopyedLen):iBlockSize;
	     m_pIdxObjMng->SetAttachObj(iIdx, 0, aBuf+iCopyedLen, iCopyLen);
	     iCopyedLen+=iCopyLen;
	     iIdx = m_pIdxObjMng->GetDsIdx(iIdx, m_pBuffMngHead->m_iDSSuffix);
	 }

	 return 0;
	  
}


ssize_t CBuffMng::SetBuffer(ssize_t aBuffNo, char* aBuf, ssize_t aBufLen)
{
}


ssize_t CBuffMng::GetBuffer(ssize_t aBuffNo, char* aBuf, const ssize_t aBuffLen)
{
     if(aBuffNo < 0 || aBuffNo > m_pBuffMngHead->m_iBuffNum || NULL == aBuf || aBuffLen <= 0 ||m_pBuffMngHead = NULL)
	     return 0;

     const ssize_t iObjSize = m_pIdxObjMng->GetObjSize();
	 ssize_t iCurrIdx = m_pBuffItem[aBuffNo].m_iBuffIdx;

	 if(iCurrIdx < 0)
		 return 0;


#ifdef _BUFFMNG_APPEND_SKIP
    ssize_t iBuffOffset = m_pBuffItem[aBuffNo].m_iBuffOffset;
#else
	ssize_t iBuffOffset = 0; 
#endif

	ssize_t iCopyLen = m_pBuffItem[aBuffNo].m_iBuffSize < (iObjSize - iBuffOffset) ? m_pBuffItem[m_pBuffItem].m_iBuffSize:(iObjSize - iBuffOffset);

	if( aBuffLen <= iCopyLen)
	{
	    return m_pIdxObjMng.CopyAttachObj(iCurrIdx, iBuffOffset,aBuf, aBuffLen);
	}


	m_pIdxObjMng.CopyAttachObj(iCurrIdx, iBuffOffset,aBuf, iCopyLen);

	ssize_t iCopiedLen = iCopyLen;
	ssize_t iLeftLen = m_pBuffItem[aBuffNo].m_iBuffSize - iCopiedLen;

    iLeftLen = iLeftLen < (aBuffLen -iCopiedLen)? iLeftLen:(aBuffLen -iCopiedLen);

    while(iLeftLen > 0)
    {
        iCurrIdx = m_pIdxObjMng->GetDsIdx(iCurrIdx, m_pBuffMngHead->m_iDSSuffix);

        if(iCurrIdx < 0)
            break;

        iCopyLen = iLeftLen > iObjSize? iObjSize: iLeftLen;
        m_pIdxObjMng.CopyAttachObj(iCurrIdx, 0, aBuf+iCopiedLen , iCopyLen);

        iLeftLen -= iCopyLen;
        iCopiedLen += iCopyLen;        
    }

    return iCopiedLen;	 
}


ssize_t CBuffMng::GetBufferSize(ssize_t aBuffSuffix)
{
    if(NULL == m_pBuffMngHead||aBuffSuffix > m_pBuffMngHead->m_iBuffNum)
	    return -1;

	if( m_pBuffItem[aBuffSuffix].m_iBuffIdx < 0)
		return 0;

	return m_pBuffItem[aBuffSuffix].m_iBuffSize;
}

bool CBuffMng::HaveFreeSpace(ssize_t aLen)
{
    if(NULL == m_pIdxObjMng)
	    return false;

	ssize_t iLen = (m_pIdxObjMng->GetObjNum() - m_pIdxObjMng->GetUsedObjNum()) * m_pIdxObjMng->GetObjSize();

	return aLen <= iLen? true:false;    
}

ssize_t CBuffMng::FreeBuffer(ssize_t aBuffItemNo)
{
    if(NULL == m_pBuffMngHead || aBuffItemNo > m_pBuffMngHead->m_iBuffNum || aBuffItemNo<0)
	    return -1;

	ssize_t iFreeIdx = m_pBuffItem[aBuffItemNo];
	if(iFreeIdx < 0)
		return -2;

    while(iFreeIdx>=0)
    {
        ssize_t iNextFreeIdx = m_pIdxObjMng->GetDsIdx(iFreeIdx, m_pBuffMngHead->m_iDSSuffix);
        m_pIdxObjMng->DestroyObject(iFreeIdx);
        iFreeIdx = iNextFreeIdx;
    }

	m_pBuffItem[aBuffItemNo].m_iBuffIdx = -1;
	m_pBuffItem[aBuffItemNo].m_iBuffSize = 0;

#ifdef _BUFFMNG_APPEND_SKIP
	m_pBuffItem[aBuffItemNo].m_iLastBuffIdx = -1;   
    m_pBuffItem[aBuffItemNo].m_iBuffOffset = 0;
#endif
    m_pBuffMngHead->m_iFreeBuffNum++;
    return 0;
    
}

ssize_t CBuffMng::AppendBuffer(ssize_t aBuffSuffix,char* aBuffer, ssize_t aBuffSize)
{
    if(NULL==m_pBuffMngHead||aBuffSuffix<0||NULL==m_pIdxObjMng||aBuffSuffix>m_pBuffMngHead->m_iBuffNum||aBuffSize<0)
        return -1;

    if(0==aBuffSize)
        return 0;
        
    if(!HaveFreeSpace(aBuffSize))
        return -2;
        
    ssize_t iObjSize = m_pIdxObjMng.GetObjSize();
    ssize_t iLeftSize = aBuffSize;
    if(m_pBuffItem[aBuffSuffix].m_iBuffIdx<0)
    {
        ssize_t iCurrIdx = m_pIdxObjMng.CreateObject();
        if(iCurrIdx < 0)
            return -3;

        m_pBuffItem[aBuffSuffix].m_iBuffIdx = iCurrIdx;
        m_pBuffMngHead.m_iFreeBuffNum--;  
        m_pBuffItem[aBuffSuffix].m_iLastBuffIdx = iCurrIdx;
        m_pBuffItem[aBuffSuffix].m_iBuffOffset = 0;
        
        ssize_t iCopyLen = iObjSize>iLeftSize?iLeftSize:iObjSize;
        ssize iRet = m_pIdxObjMng.SetAttachObj(iCurrIdx , 0, aBuffer+iAppendSize, iCopyLen);
        if(iRet < 0)
        {
           FreeBuffer(aBuffSuffix);
           return -4;
        }
        
        m_pBuffItem[aBuffSuffix].m_iBuffSize = iCopyLen;
        iLeftSize -= iCopyLen;
        while(m_pBuffItem[aBuffSuffix].m_iBuffSize < aBuffSize)
        {
             ssize_t iNextIdx = m_pIdxObjMng.CreateObject();
             if(iNextIdx < 0)
             {
                FreeBuffer( aBuffSuffix);
                return -3;
             }
             
             iCopyLen = iObjSize>iLeftSize?iLeftSize:iObjSize;
             m_pIdxObjMng.SetDsIdx(iCurrIdx, m_pBuffMngHead->m_iDSSuffix,  iNextIdx);
             m_pIdxObjMng.SetAttachObj(iCurrIdx , 0, aBuffer+m_pBuffItem[aBuffSuffix].m_iBuffSize, iCopyLen);
             m_pBuffItem[aBuffSuffix].m_iLastBuffIdx = iNextIdx;
             m_pBuffItem[aBuffSuffix].m_iBuffSize += iCopyLen;
             iLeftSize -= iCopyLen;
        }              
    }
    else
    {
         ssize_t iLastIdx = m_pBuffItem[aBuffSuffix].m_iLastBuffIdx;
         if(iLastIdx < 0)
         {
             return -1;
         }

         ssize_t iLastDataSize =(m_pBuffItem[aBuffSuffix].m_iBuffOffset+m_pBuffItem[aBuffSuffix].m_iBuffSize)%iObjSize;
         ssize_t iLastLeftSize = iObjSize-iLastDataSize;

         ssize_t iCopyLen = iLastLeftSize<aBuffSize? iLastDataSize:aBuffSize;         

         m_pIdxObjMng.SetAttachObj(iLastIdx, iLastDataSize, aBuffer, iCopyLen);
         ssize_t iLeftSize = aBuffSize - iCopyLen;

         ssize_t iCopyedLen = iCopyLen;
         while(iLeftSize >0)
         {
             iCopyLen = iLeftSize<iObjSize? iLeftSize:iObjSize;
             ssize_t iNextIdx = m_pIdxObjMng->CreateObject();
             
             if(iNextIdx < 0)
             {
                 FreeObjList(iLastIdx);
                 return -4;                 
             }

             m_pIdxObjMng.SetDsIdx(m_pBuffItem[aBuffSuffix].m_iLastBuffIdx , m_pBuffMngHead->m_iDSSuffix, iNextIdx);
             m_pIdxObjMng.SetAttachObj(iNextIdx ,  0, aBuffer+iCopyedLen, iCopyLen);
             iCopyedLen += iCopyLen;
             iLeftSize -= iCopyLen;
             m_pBuffItem[aBuffSuffix].m_iLastBuffIdx = iNextIdx;
         }

         m_pBuffItem[aBuffSuffix].m_iBuffSize += aBuffSize;
    }

    return 0;
}
