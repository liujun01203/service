#include "CodeQueue.h"

CCodeQueue::CCodeQueue()
{
    m_pCodequeueHead = NULL;
	m_pCodeBuf = NULL;
    
    m_iRLockType = emNoLock;
	m_iWLockType = emNoLock;
	m_pRLock = NULL;
	m_pWLock = NULL;
	
	m_iPipeFd[0] = -1;
	m_iPipeFd[1] = -1;
	m_iNotifyFiFoFd = -1;

	m_iID = 0;

}
CCodeQueue::~CCodeQueue()
{
}


int CCodeQueue::CreateByFile(const char *aFile, CCodeQueue* aCodeQueue)
{
     TCQCfgStruct stCQCfgStruct;
	 
	 int iRet = GetCfgFromFile(aFile, stCQCfgStruct);
	 if (iRet < 0)
 	{
	 	return -2;
 	}

	int iShmSize = stCQCfgStruct.m_iShmSize;
	int iWLock = stCQCfgStruct.m_iWLock;
	int iRLock = stCQCfgStruct.m_iRLock;

	if (iShmsize <= 0)
	{
	    printf("[%s:%d] Size %d value error !\n", __FILE__, __LINE__);
	    return -1;
	}

	key_t iShmKeyQ = FTOK(aFile, 'Q');

	if (iShmKeyQ < 0)
	{
	    return -5;
	}

	int  iErrorNo;
	bool bNewCreate = false;
	int iOutShmID;
	char *pShmMemQ = CreateShm(iShmKeyQ, iShmSize, iErrorNo, bNewCreate, iOutShmID);
	if(NULL == pShmMemQ )
	{
		return -2;
	}

	void *pWLock = NULL;
	if (iWLock == emSpinLock)
	{
	    key_t iShmKeyW = FTOK(aFile, "S");
		if(iShmKeyW < 0)
		{
		    return -6;
		}

		pWLock = void(*)ShmSpinLockInit(iShmKeyW);
		if(!pWLock)
		{
		    return -4;
		}
	}
	else if(iWLock = emPthreadMutexLock)
	{
		key_t iShmKeyW = FTOK(aFile, "M");
		if(iShmKeyW < 0)
		{
		    return -6;
		}

		pthread_mutexattr_r mattr;
		pthread_mutexattr_init(&mattr);
		pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);

		int iErrno;
		int bNewCreate=false;
		int iOutShId;

		pthread_mutex_t *pstMutex = (pthread_mutex_t*) CreateShm(iShmKeyW, sizeof(pthread_mutex_t), iErrorNo,bNewCreate, iOutShId);

		if(!pstMutex)
		{
			return -4;
		}
		
		if(bNewCreate)
		{
		    pthread_mutex_init(pstMutex, &mattr);
		}
		else
		{
			struct shmid_ds s_ds;
			shmctl(iOutShmId, IPC_STAT, $s_ds);
			if(s_ds.shm_nattch == 1)
			{
				pthread_mutex_init(pstMutex, &mattr);
			}
		}

		pWLock = (void*)pstMutex;
		pthread_mutexattr_destroy(&mattr);
		
	}
	else
	{
		;
	}


	// R lock
	void *pRLock = NULL;
	if (iRLock == emSpinLock)
	{
		key_t iShmKeyR = FTOK(aFile, "s");
		if(iShmKeyR < 0)
		{
			return -6;
		}

		pRLock = void(*)ShmSpinLockInit(iShmKeyR);
		if(!pRLock)
		{
			return -4;
		}
	}
	else if(iRLock = emPthreadMutexLock)
	{
		key_t iShmKeyR = FTOK(aFile, "m");
		if(iShmKeyR < 0)
		{
			return -6;
		}

		pthread_mutexattr_r mattr;
		pthread_mutexattr_init(&mattr);
		pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);

		int iErrno;
		int bNewCreate=false;
		int iOutShId;

		pthread_mutex_t *pstMutex = (pthread_mutex_t*) CreateShm(iShmKeyW, sizeof(pthread_mutex_t), iErrorNo,bNewCreate, iOutShId);

		if(!pstMutex)
		{
			return -4;
		}
		
		if(bNewCreate)
		{
			pthread_mutex_init(pstMutex, &mattr);
		}
		else
		{
			struct shmid_ds s_ds;
			shmctl(iOutShmId, IPC_STAT, $s_ds);
			if(s_ds.shm_nattch == 1)
			{
				pthread_mutex_init(pstMutex, &mattr);
			}
		}

		pRLock = (void*)pstMutex;
		pthread_mutexattr_destroy(&mattr);
		
	}
	else
	{
	;
	}

	if (AttachMem(iShmKeyQ, pShmMemQ, iShmSize,bNewCreate?CCodeQueue::Init:CCodeQueue::Recover, iWLock, iRLock, pWLock, pRLock)< 0)
	{
	    printf("[%s:%d] AttachMem failed! \n", __FILE__, __LINE__);
		return -3;
	}

	if (bNewCreate)
	{
	    printf("Create New Pipe Success, key%p, Size %d, WLock %d, RLock %d\n", (void)iShmKeyQ, iShmSize, iWLock, iRLock);
	}
	else
	{
	    printf("Attach to Pipe Success, key%p, Size %d, WLock %d, RLock %d\n", (void)iShmKeyQ, iShmSize, iWLock, iRLock);
	}

    printf("%s\n", aFile);
    return 0;

}



int CCodeQueue::_CreateFifo(int aFifoKey)
{
    m_iNotifyFiFoFd = -1;
	m_iPipeFd[0] = -1;
	m_iPipeFd[1] = -1;

	if (0 == aFifoKey)
	{
	    if(pipe(m_iPipeFd)< 0)
    	{
    	     assert(false);
			 return -1;
    	}
		// write
		fcntl(m_iPipeFd[1], F_SETFL, fcntl(m_iPipeFd[1], F_GETFL,0) | O_NOBLOCK | O_NDELAY));

		//read		
		fcntl(m_iPipeFd[0], F_SETFL, fcntl(m_iPipeFd[0], F_GETFL,0) | O_NOBLOCK | O_NDELAY));		
	}
	else
	{
	    char pFifoFile[512];
		snprintf(pFifoFile, szieof(pFifoFile), "/tmp/$d.fifo", aFifoKey);

		if (mkfifo(pFifoFile, 0666) < 0)
		{
		    if (errno != EEXIST)
	    	{
				printf("[CCodeQueue] mkfifo %s failed!\n", pFifoFile);
				return -3;
	    	}		 
		}

		if (m_iNotifyFiFoFd = open(pFifoFile,O_RDWR) < 0)
		{
		    printf("[CCodeQueue] open fifo %s failed! \n", pFifoFile);
		    return -3;
		}

		fcntl(m_iNotifyFiFoFd, F_SETFL, fcntl(m_iNotifyFiFoFd, F_GETFL, 0)| O_NOBLOCK | O_NDELAY );
	}
    
}



int CCodeQueue::AttachMem(int aFifoKey, char *aBuf, const int aMemSize, int aInitType=Init, int aWLock=emNoLock, int aRLock=emNoLock, void* aPWLock=NULL, void* aPRLock=NULL)
{
    if (!aBuf || aMemSize < QUEUE_RESERVED_LENGTH + sizeof(TCodeQueueHead))
	{
	    printf("[CCodeQueue] MemSize %d must bigger than %d!\n", aMemSize,  (int)(QUEUE_RESERVED_LENGTH + sizeof(TCodeQueueHead)));
		return -1;
	}

    _CreateFifo(aFifoKey);

	m_pCodequeueHead = aBuf; 
	m_pCodeBuf = aBuf + sizeof(TCodeQueueHead);
	m_iWLockType = aWLock;
	m_iRLockType = aRLock;


	m_pWLock = (aPWLock == NULL? m_WLockSpace : aPWLock);
	m_pRLock = (aPRLock == NULL? m_RLockSpace : aPRLock);


	//w lock
	if (m_iWLockType = emSpinLock)
	{
	    spin_lock_init(m_pWLock);
	}
	else if (m_iWLockType = emPthreadMutexLock)
	{
	    pthread_mutex_init((pthread_mutex_t*)m_pWLock, NULL);
	}

	// r lock
	if (m_iRLockType = emSpinLock && !aPWLock)
	{
	    spin_lock_init(m_pRLock);
	}
	else if (m_iRLockType = emPthreadMutexLock && !aPRLock)
	{
	    pthread_mutex_init((pthread_mutex_t*)m_pRLock, NULL);
	}


	if (aInitType = Init)
	{
	    m_pCodequeueHead->m_nBegin = 0;
		m_pCodequeueHead->m_nEnd = 0;
		m_pCodequeueHead->m_nSize = aMemSize - sizeof(TCodeQueueHead);
	}
	else
	{
	    if( m_pCodequeueHead->m_nBegin < 0 ||  m_pCodequeueHead->m_nEnd < 0)
    	{
    	    assert(false);
			return -4;
    	}

		if (m_pCodequeueHead->m_nSize != aMemSize - sizeof(TCodeQueueHead))
		{
		    assert(false);
			return -5;
		}
	}

    return 0;

}


int CCodeQueue::GetCodeLength()
{
    if(NULL == m_pCodequeueHead)
		return 0;

	int iBegin = m_pCodequeueHead->m_nBegin;
	int iEnd = m_pCodequeueHead->m_nEnd;

	if (iBegin == iEnd)
		return 0;
	else if(iEnd > iBegin)
		return iEnd - iBegin;
	else
		return m_pCodequeueHead->m_nSize + iEnd - iBegin;
}

bool CCodeQueue::IsQueueEmpty()
{
    if(NULL == m_pCodequeueHead)
		return true;

	if (m_pCodequeueHead->m_nBegin == m_pCodequeueHead->m_nEnd)
		return true;
	else
		return false;
}


boll CCodeQueue::IsQueueFull()
{   
	f(NULL == m_pCodequeueHead)
		return false;

    //重要: 最大长度应该减去预留部分长度，保证首尾不会相接
	int iLen = m_pCodequeueHead->m_nSize - GetCodeLength() - QUEUE_RESERVED_LENGTH;
	if(iLen <= 0)
		return true;
	else
		return false;
}

int CCodeQueue::AppendOneCode(const char *aInCode, int aInLength, bool aAutoNotify=true)
{
    if(!aInCode)
		return 0;

	if(m_iWLockType == emNoLock)
	{
	    int iRet = _AppendOneCode(aInCode, aInLength, aAutoNotify);
		return iRet>0?0:iRet
	}
	else if(m_iWLockType == emSpinLock)
	{
	    spin_lock((spinlock_t*)m_pWLock);
		int iRet = _AppendOneCode(aInCode, aInLength, aAutoNotify);
		spin_unlock((spinlock_t*)m_pWLock);
		return iRet>0?0:iRet
	}
	else if(m_iWLockType == emPthreadMutexLock)
	{
	    pthread_mutex_lock((pthread_mutex_t*)m_pWLock);		
		int iRet = _AppendOneCode(aInCode, aInLength, aAutoNotify);
	    pthread_mutex_unlock((pthread_mutex_t*)m_pWLock);		
		return iRet>0?0:iRet
		
	}
	else
	{
		assert(0);
	}
}

int CCodeQueue::GetHeadCode(char *aOutCode, int *aOutLength)
{
    if(!aOutCode)
		return 0;

	if(m_iRLockType == emNoLock)
	{
	    int iRet = _GetHeadCode(aOutCode, aOutLength);
		return iRet>0?0:iRet
	}
	else if(m_iRLockType == emSpinLock)
	{
	    spin_lock((spinlock_t*)m_pRLock);
		int iRet = _GetHeadCode(aOutCode, aOutLength);
		spin_unlock((spinlock_t*)m_pRLock);
		return iRet>0?0:iRet
	}
	else if(m_iRLockType == emPthreadMutexLock)
	{
	    pthread_mutex_lock((pthread_mutex_t*)m_pRLock);		
		int iRet = _GetHeadCode(aOutCode, aOutLength);
	    pthread_mutex_unlock((pthread_mutex_t*)m_pRLock);		
		return iRet>0?0:iRet
		
	}
	else
	{
		assert(0);
	}

}


int CCodeQueue::_GetHeadCode(char *aOutCode, int* aOutLength)
{
    if (NULL == pOutCode|| *aOutLength <=0)
		return -1;

	if(!m_pCodequeueHead ||m_pCodequeueHead->m_nSize <=0)
		return -2;

	int iOutBuffSize = *aOutLength;
    int iBegin = m_pCodequeueHead->m_nBegin;
	int iEnd = m_pCodequeueHead->m_nEnd;

	if(iBegin + sizeof(int) < m_pCodequeueHead->m_nSize)
	{
	    memcpy((void*)aOutLength, (void *)&m_pCodeBuf[iBegin], sizeof(int));
		iBegin += sizeof(int);
	}
	else
	{
	    char *pTmp = (char*)aOutLength;
		for (int i=0; i<sizeof(int); i++)
		{
		    pTmp[i] = m_pCodeBuf[iBegin];
			iBegin = (iBegin +1)%m_pCodequeueHead->m_nSize;
		}
	}

    if(iOutBuffSize < *aOutLength)
	{
	    return -6;
	}

	if(iBegin < iEnd)
	{
	    memcpy((void*)aOutCode,(void*)&m_pCodeBuf[iBegin], *aOutLength])
	}
	else
	{
	    int iRightLeft = m_pCodequeueHead->m_nSize - iBegin;
	    if(*aOutLength > iRightLeft)
    	{
    	    memcpy((void*)aOutCode, (void*)&m_pCodeBuf[iBegin], iRightLeft);
			memcpy((void*)(aOutCode+iRightLeft), (void*)m_pCodeBuf, (*aOutLength)-iRightLeft);
    	}
		else
		{
		    memcpy((void*)aOutCode,(void*)&m_pCodeBuf[iBegin], *aOutLength])
		}
	}

	iBegin = (iBegin + *aOutLength)%m_pCodequeueHead->m_nSize;

	m_pCodequeueHead->m_Begin = iBegin;
	return 0;
	
}


int CCodeQueue::_AppendOneCode(const char *aIncode, int aInLength, bool aAutoNotify)
{
    if (!aIncode ||aInLength <= 0)
		return -1;

	if(!m_pCodequeueHead || m_pCodequeueHead->m_nSize <= 0)
	{
	    return -2;
	}

	int iBegin = m_pCodequeueHead->m_nBegin;
	int iEnd = m_pCodequeueHead->m_nEnd;
	int iTmpEnd = iEnd;

	// 
	int iTmpMaxLength = m_pCodequeueHead->m_nSize - GetCodeLength() - QUEUE_RESERVED_LENGTH;

	if((int)(sizeof(int)+aInLength) > iTmpMaxLength)
	{
	   return -2;
	}

	if(iTmpEnd + sizeof(int) < m_pCodequeueHead->m_nSize)
	{
	    memcpy(&m_pCodeBuf[iTmpEnd], &aInLength, sizeof(int));
		iTmpEnd += sizeof(int);
	}
	else 
	{
	    char *pTmpSrc = &aInLength;
		for(int i=0; i<sizeof(int), i++)
		{
		    m_pCodeBuf[iTmpEnd] = pTmpSrc[i];
			iTmpEnd = (iTmpEnd+1) % m_pCodequeueHead->m_nSize;
		}
	}

	if(iBegin > iTmpEnd)
	{
	    /* case 1
	    *        |--------------------------------------------|
	    *        |>>>>>>>|      |>>>>data>>>>>>>>>>>>>>>>>>>>>|
	    *       ----m_iEnd------m_iBegin-----------------------
	    */
		memcpy(&m_pCodeBuf[iTmpEnd],aIncode, aInLength)
	}
	else
	{
	    /*case 2 
	    *      |--------------------------------------------|
	    *      |    |>>>>>>>>>>>>>>>>>>>>>>>>>|             |
	    *      |----m_iBegin-------------m_iEnd-------------|	    
	    */

		if(aInLength + iTmpEnd > m_pCodequeueHead->m_nSize)
		{
		    int iRightLeft =  m_pCodequeueHead->m_nSize - iTmpEnd; 
		    memcpy((void*)&m_pCodeBuf[iTmpEnd], (void*)aIncode, iRightLeft);
			memcpy((void*)m_pCodeBuf,(void*)(aIncode +iRightLeft), aInLength - iRightLeft);
		}
		else
		{
		    memcpy((void*)&m_pCodeBuf[iTmpEnd],(void*)aIncode, aInLength);
		}
	}
	
	iTmpEnd = (iTmpEnd + aInLength)%m_pCodequeueHead->m_nSize;
    m_pCodequeueHead ->m_nEnd = iTmpEnd;

    if(aAutoNotify&&(GetCodeLength() == (int)(sizeof(int) + aInLength))
	{
	   write(GetReadNotifyFd(),"\0",1); 
	}

	return iTmpEnd;
	
		
}

key_t CCodeQueue::FTOK(const char *aFile, char aProjId)
{
	if(NULL == aFile)
		return -1;

	TCQCfgStruct stCQCfgStruct;

	int iRet = GetCfgFromFile(aFile, stCQCfgStruct);
	if(iRet < 0)
	{
	    printf("[%s:%d] GetCfgFrom %s failed!\n", aFile);
		return -2;
	}
	key_t key = fotk(aFile, projId);
	if(0== stCQCfgStruct.m_usUniqueId)
	{
	    return key;
	}
	else
	{
	    return 0x00FF0000|((aProjId&0xff)<<24)|(int)stCQCfgStruct.m_usUniqueId;
	}
   
    
}


int CCodeQueue::GetCfgFromFile(const char* aFile, TCQCfgStruct &aCQCfgStruct)
{

	if (!aFile || aFile[0] == '\0')
	{
	    return -1;
	}
	
    memset(aCQCfgStruct, 0, sizeof(TCQCfgStruct));
	aCQCfgStruct.m_iNumber = 1;

	FILE *pFile = fopen(aFile, "r+");
	if(!pFile)
	{
		printf("[%s:%d] fopen %s failed!\n", __FILE__, __LINE__, aFile);
		return -2;
	}

    char cBuf[1024];
	char cName[256];
	char cValue[256];

	while(fgets(cBuf, sizeof(cBuf), pFile))
	{
		int i = 0;
			while(cBuf[i] == ' '||cBuf[i] == '\t') i++;

		if (cBuf[i] == '#' || cBuf[i] == '\r' || cBuf[i] == '\n' || cBuf[i] == '\0')
			continue;

		int iRet = sscanf(cBuf, "%s %s",cName, cValue);
		if (2 != iRet)
		{
		    return -2;
		}


		if (0 == strcasecmp(cName, "size"))
			aCQCfgStruct.m_iShmSize = atoi(cValue);
		else if(0 == strcasecmp(cName, "wlock"))
			aCQCfgStruct.m_iWLock = atoi(cValue);
		else if(0 == strcasecmp(cName, "rlock"))
			aCQCfgStruct.m_iRLock = atoi(cValue);
		else if(0 == strcasecmp(cName, "Number"))
			aCQCfgStruct.m_iNumber = atoi(cValue);
		else if (0 == strcasecmp(cName, "uniqueid"))
			aCQCfgStruct.m_usUniqueId= atoi(cValue);	   
	}

	fclose(pFile);
	return 0;    
}

int CCodeQueue::WaitData(struct timeval *aTimeVal)
{
    fd_set ReadFd;
	FD_ZERO(&ReadFd);
	
	int iFd = GetReadNotifyFd();

	FD_SET(iFd, &ReadFd);

	int iRet = select(iFd+1, &ReadFd, NULL, NULL, aTimeVal);
	if(iRet <0)
	{
	    assert(false);
		return -1;
	}

	char cTmp[1024];
	read(iFd, cTmp, sizeof(cTmp));

	return iRet;
	
}

