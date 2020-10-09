#include "IdxObjMng.h"

TIdxObjMng::TIdxObjMng()
{
    m_iDSNum = 1;
    
    m_pIdxObjHead = NULL;
    m_pIdx = NULL;
    m_pObjMem = NULL;

    memset(m_piDsUseFlag, 0, sizeof(m_piDsUseFlag));
}

ssize_t TIdxObjMng::GetOneFreeDS()
{
    for(ssize_t i=0; i<m_iDSNum; i++)
    {
        if(m_piDsUseFlag[i] == 0)
        {
            m_piDsUseFlag[i] = 1;
            return i;
        }
    }
    return -1;
}

void TIdxObjMng::FormatIdx()
{
    m_pIdxObjHead->m_iFreeHead = 0;
    m_pIdxObjHead->m_iUsedCount = 0;

    idx_t *ipTmp = m_pIdx;
    while( ipTmp < m_pObjMem)
    {
        *ipTmp = -1;
        ipTmp++;        
    }

    for(ssize_t i=0; i<m_pIdxObjHead->m_iIdxObjNum; i++)
    {
        GETIDX(i)->m_piDsInfo[0] = i+1;
    }

    GETIDX(m_pIdxObjHead->m_iIdxObjNum-1)->m_piDsInfo[0] = -1;
	return ;
}


// 连接内存，返回占用空间大小
// aIdxObjNum=0 自动计算
// MEMSIZE 内存大小， aBuf 连接的内存
ssize_t TIdxObjMng::AttachMem(char* aBuf, ssize_t MEMSIZE, ssize_t aObjSize, ssize_t aIdxObjNum, ssize_t aInitType = emInit,ssize aDSNum=1)
{
    if(aDSNum < 1)
  		m_iDSNum = 1;
  	else
  	    m_iDSNum = aDSNum;
  		
    ssize_t iIdxSize = sizeof(TIdx) + sizeof(idx_t)* aDSNum;
    if((MEMSIZE <=0 || NULL==aBuf)
	    return -1;

	if(aIdxObjNum<=0)
	{
	    ssize_t iLeftLen = MEMSIZE - sizeof(TIdxObjHead);
	    ssize_t iIdxObjSize = iIdxSize + aObjSize;

	    aIdxObjNum = iLeftLen/iIdxObjSize;
	}

	if (aIdxObjNum <=0)
		return -1;

	m_pIdxObjHead = (TIdxObjHead*)aBuf;
	m_pIdx =(TIdx*)(aBuf+sizeof(TIdxObjHead));
	m_pObjMem = aBuf + sizeof(TIdxObjHead) + aIdxObjNum * iIdxSize;

	if(emInit == aInitType)
	{
	    m_pIdxObjHead->m_iObjSize = aObjSize;
	    m_pIdxObjHead->m_iIdxSize = iIdxSize;
	    m_pIdxObjHead->m_iIdxObjNum = aIdxObjNum;
	    FormatIdx();
	}
	else
	{
	    if( m_pIdxObjHead->m_iObjSize != aObjSize ||
	        m_pIdxObjHead->m_iIdxObjNum != aIdxObjNum||
	        m_pIdxObjHead->m_iIdxSize != iIdxSize )
	    return -3;
	}

	return CountMemSize(aObjSize,aIdxObjNum,m_iDSNum);
	
}

//计算总占用内存量
ssize_t TIdxObjMng::CountMemSize(ssize_t aObjSize, ssize_t aIdxObjNum, ssize_t aDSNum=1)
{
    if(aDSNum < 1)
    	aDSNum = 1;

    ssize_t iIdxSize = sizeof(TIdx) +sizeof(idx_t) * aDSNum;
    return sizeof(TIdxObjHead) + aIdxObjNum * (iIdxSize + aObjSize)
}

//根据内存量计算节点数量
ssize_t TIdxObjMng::CountIdxObjNum(const ssize_t MEMSIZE,  ssize_t aObjSize, ssize_t  aDSNum)
{
	if(aDSNum < 1)
		aDSNum = 1;	

	ssize_t iIdxSize = sizeof(TIdx) +sizeof(idx_t) * aDSNum;

	return (MEMSIZE - sizeof(TIdxObjHead))/(iIdxSize + aObjSize);
}


ssize_t TIdxObjMng::CreateObject()
{
    if(m_pIdxObjHead->m_iFreeHead < 0)
	    return -1;

	ssize_t iTmpIdx = m_pIdxObjHead->m_iFreeHead;
	TIdx * pIdx = GETIDX(iTmpIdx);
	m_pIdxObjHead->m_iFreeHead = pIdx->m_piDsInfo[0];

    for(int i=0; i<m_iDSNum; i++)
    {
        pIdx->m_piDsInfo[i] = -1;
    }
	m_pIdxObjHead->m_iUsedCount++;
	return iTmpIdx;
}

ssize_t TIdxObjMng::DestroyObject(ssize_t iIdx)
{
    if(iIdx<0 || iIdx > m_pIdxObjHead->m_iIdxObjNum)
	    return -1;
	    
	TIdx * pIdx = GETIDX(iIdx);
	pIdx->m_piDsInfo[0] = m_pIdxObjHead->m_iFreeHead;
	m_pIdxObjHead->m_iFreeHead = iIdx;
	m_pIdxObjHead->m_iUsedCount--;
	return iIdx;	
}


ssize_t TIdxObjMng::CopyAttachObj(ssize_t aIdx, ssize_t aOffset, char* aBuf, const ssize_t COPYSIZE)
{
    if(NULL == aBuf)
	    return -1;
    if(aIdx<0 || aIdx>m_pIdxObjHead->m_iIdxObjNum)
    {
        return -1;
    }

    ssize_t iCopyLen = COPYSIZE <(m_pIdxObjHead->m_iObjSize-aOffset)?COPYSIZE:(m_pIdxObjHead->m_iObjSize-aOffset);
    memcpy(aBuf, m_pObjMem+m_pIdxObjHead->m_iObjSize*aIdx+aOffset, iCopyLen);
    return iCopyLen;    
}


char* TIdxObjMng::GetAttachObj(ssize_t aIdx)
{
    if(aIdx<0 || aIdx > m_pIdxObjHead->m_iIdxObjNum)
	    return NULL;	
    if(0==m_pIdxObjHead->m_iObjSize)
	    return NULL;

	return (m_pObjMem + m_pIdxObjHead->m_iObjSize * aIdx);
}
ssize _t  TIdxObjMng::SetAttachObj(ssize_t aIdx, ssize_t aOffset, char* aBuf, ssize_t aLen)
{
    if(NULL == aBuf || aIdx<0 || aIdx>=m_pIdxObjHead->m_iIdxObjNum)
    	return -1;

    if(aLen <=0)
    {
        aLen = m_pIdxObjHead->m_iObjSize;
    }

    if(aOffset + aLen > m_pIdxObjHead->m_iObjSize )
    {
        return -2;
    }

    memcpy(m_pObjMem + m_pIdxObjHead->m_iObjSize*aIdx+aOffset, aBuf, aLen);
    return 0;    
}


ssize_t TIdxObjMng::GetDsIdx(ssize_t aIdx, ssize_t aDsSufix)
{
    if(aIdx<0 || aDsSufix<0 || aIdx > m_pIdxObjHead->m_iIdxObjNum || aDsSufix >m_iDSNum)
	    return -1;

	return GETIDX(aIdx)->m_piDsInfo[aDsSufix];    
}

ssize_t TIdxObjMng::SetDsIdx(ssize_t aIdx, ssize_t aDsSufix, ssize_t aNextIdx)
{
     if(aIdx<0 || aDsSufix<0 || aIdx > m_pIdxObjHead->m_iIdxObjNum || aDsSufix >m_iDSNum)
	    return -1;

	  GETIDX(aIdx)->m_piDsInfo[aDsSufix] = aNextIdx;
	  return 0;
}




