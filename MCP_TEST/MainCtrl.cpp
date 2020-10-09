#include <sched.h>

CMainCtrl::CMainCtrl()
{
}
CMainCtrl::~CMainCtrl()
{
}

int CMainCtrl::Initialize(char* aProName, char*aConfigFile))
{
    strncpy(m_stConfig.m_szSvrName, aProName, sizeof(m_stConfig.m_szSvrName));
	int iRet = ReadCfgFile(aConfigFile);
	if(iRet < 0)
	{
	    return iRet;
	}

	if(m_stConfig.m_iBindCpu >= 0)
	{
	    cpu_set_t mask;
	    int iCpuId = m_stConfig.m_iBindCpu;
	    CPU_ZERO(&mask);
	    CPU_SET(iCpuId, &mask);
	    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) < 0)
	    {
	        printf("[%s:%d] Bind CPU failed!\n", __FILE__, __LINE__);
	        return -1;
	    }
	}

	if(m_stConfig.m_stTcpIPPortStu[0].m_usPort ! = 0)
	{
	    void *pHandle = dlopen();
	    if(NULL == pHandle)
	    {
	        printf("[%s:%d]dlopen failed!\n", __FILE__, __LINE__);
	        return -2;
	    }

	    dlerror();
	    net_complete_func = (check_complete)dlsym(pHandle, "net_complete_func");
	    if(NULL != dlerror())
	    {
	        printf("[%s:%d] dlsym failed!\n", __FILE__, __LINE__);
	        return -2;
	    }

	    msg_header_len_func = (msg_header_len)dlsym(pHandle, "msg_header_len_func");
	    if(NULL != dlerror())
	    {
	        printf("[%s:%d] dlsym failed!\n", __FILE__, __LINE__);
	        return -2;
	    }

	    m_iMsgHeaderLen = msg_header_len_func();	    
	}

	long long llMemCost = 0;

	m_pSocketNode = new TSocketNode[m_stConfig.MAX_SOCKET_NUM];
	if(NULL == m_pSocketNode)
	{
	    printf("[%s:%d]Out of Memory!", __FILE__, __LINE__);
	    return -3;
	}

	for(int i =0; i < m_stConfig.MAX_SOCKET_NUM; i++)
	{
	    memset((void*)&m_pSocketNode[i], 0, sizeof(TSocketNode));
	    m_pSocketNode[i].m_iSocket = -1;
	}

  	llMemCost += sizeof(TSocketNode) * m_stConfig.MAX_SOCKET_NUM;
	
	//连接资源限制,root用户下有效
	rlimit rlim;
	rlim.rlim_cur = m_stConfig.MAX_SOCKET_NUM+10240;
	rlim.rlim_max = m_stConfig.MAX_SOCKET_NUM+10240;
	setrlimit(RLIMIT_NOFILE, &rlim);

    //创建管道
	if(CCodeQueue::CreateByFile(m_stConfig.m_szMeToSvrMQ, &m_Me2SvrPipe) < 0)
	{
	    printf("[%s:%d] Create CodeQueue failed!\n", __FILE__, __LINE__);
	    return -1;
	}

	if(CCodeQueue::CreateByFile(m_stConfig.m_szSvrToMeMQ, &m_Svr2MePipe) < 0)
	{
		printf("[%s:%d] Create CodeQueue failed!\n", __FILE__, __LINE__);
	    return -1;
	}

    //创建cs消息缓冲区
    int iMemSize = TIdxObjMng::CountMemSize(m_stConfig.RCV_BLOCK_SIZE, m_stConfig.RCV_BLOCK_NUM,1);
    char *pMem = new char[iMemSize];
    m_stIdxObjMngRecv.AttachMem(pMem, iMemSize, m_stConfig.RCV_BLOCK_SIZE,  m_stConfig.RCV_BLOCK_NUM, emInit,1);

    llMemCost += iMemSize;

    iMemSize = CBuffMng::CountMemSize(m_stConfig.MAX_SOCKET_NUM);
    pMem = new char[iMemSize];
    m_stBuffMngRecv.AttachMem(pMem,iMemSize,m_stConfig.MAX_SOCKET_NUM,emInit);
    llMemCost += iMemSize;

    if(m_stBuffMngRecv.AttachIdxObjMng(&m_stIdxObjMngRecv)<0)
    {
    	printf("BuffMngRecv AttachIdxObjMng failed!\n");
    	return -1;
    }

    //创建sc消息缓冲区
    int iMemSize = TIdxObjMng::CountMemSize(m_stConfig.SND_BLOCK_SIZE, m_stConfig.SND_BLOCK_NUM,1);
    char *pMem = new char[iMemSize];
    m_stIdxObjMngSnd.AttachMem(pMem, iMemSize, m_stConfig.SND_BLOCK_SIZE,  m_stConfig.SND_BLOCK_NUM, emInit,1);

    llMemCost += iMemSize;

    iMemSize = CBuffMng::CountMemSize(m_stConfig.MAX_SOCKET_NUM);
    pMem = new char[iMemSize];
    m_stBuffMngSnd.AttachMem(pMem,iMemSize,m_stConfig.MAX_SOCKET_NUM,emInit);
    llMemCost += iMemSize;

    if(m_stBuffMngSnd.AttachIdxObjMng(&m_stIdxObjMngSnd)<0)
    {
    	printf("BuffMngSnd AttachIdxObjMng failed!\n");
    	return -1;
    }

    //初始化端口
    if(InitSocket())
    {
        printf("[%s:%d] socket initialize failed!\n", __FILE__, __LINE__);
        return -4;
    }

    
	printf("Server Init Success!\n");
	return 0;

	
}

int CMainCtrl::InitSocket()
{
   if( m_stEPollFlow.create(m_stConfig.MAX_SOCKET_NUM)<0)
	   return -1;

	//tcp 监听端口

	for(int i = 0; i< MAX_BIND_PORT_NUM; i++)
	{
	    unsigned short usListenPort = m_stConfig.m_stTcpIPPortStu[i].m_usPort;
	    unsigned int unListenIP = m_stConfig.m_stTcpIPPortStu[i].m_unIP; 

	    if(0==usListenPort)
	    {
	        break;
	    }

	    int iLisetenSocket = CreateListenSocket(unListenIP, usListenPort, m_stConfig.SOCKET_RCVBUF, m_stConfig.SOCKET_SNDBUF, emTCP, m_stConfig.LISTEN_BACK_LOG);
	    if(iLisetenSocket<0)
	    {
	        printf("[%s:%d]CreateListenSocket %u:%u failed!\n", unListenIP, usListenPort);
	        return -2;
	    }

	    int iIdx = CreateSocketNode(iLisetenSocket, TSocketNode::TCP_LISTEN_SOCKET, unListenIP, usListenPort);
	    if(iIdx < 0)
	    {
	        
	        close(iLisetenSocket);
	        printf("[%s:%d] add to socket array failed!\n", __FILE__, __LINE__);
	        return -3;
	    }
	    
	}


	//udp 监听端口

    for(int i = 0; i< MAX_BIND_PORT_NUM; i++)
	{
	 	unsigned short usListenPort = m_stConfig.m_stUdpIPPortStu[i].m_usPort;
	    unsigned int unListenIP = m_stConfig.m_stUdpIPPortStu[i].m_unIP; 

	    if(0==usListenPort)
	    {
	        break;
	    }

	    int iLisetenSocket = CreateListenSocket(unListenIP, usListenPort, m_stConfig.SOCKET_RCVBUF, m_stConfig.SOCKET_SNDBUF, emUDP);
	    if(iLisetenSocket<0)
	    {
	        printf("[%s:%d]CreateListenSocket %u:%u failed!\n", unListenIP, usListenPort);
	        return -2;
	    }

	    int iIdx = CreateSocketNode(iLisetenSocket, TSocketNode::UDP_SOCKET, unListenIP, usListenPort);
	    if(iIdx < 0)
	    {
	        
	        close(iLisetenSocket);
	        printf("[%s:%d] add to socket array failed!\n", __FILE__, __LINE__);
	        return -3;
	    }	    
	    
	}

	//管理端口
	if(m_stConfig.m_iAdminPort > 0)
	{
	    unsigned short usListenPort = m_stConfig.m_iAdminPort;
	    unsigned int unListenIP = m_stConfig.m_szAdminIP; 

	    if(0==usListenPort)
	    {
	        break;
	    }

	    int iLisetenSocket = CreateListenSocket(unListenIP, usListenPort, m_stConfig.SOCKET_RCVBUF, m_stConfig.SOCKET_SNDBUF, emUDP);
	    if(iLisetenSocket<0)
	    {
	        printf("[%s:%d]CreateListenSocket %u:%u failed!\n", unListenIP, usListenPort);
	        return -2;
	    }

	    int iIdx = CreateSocketNode(iLisetenSocket, TSocketNode::ADMIN_LISTEN_SOCKET, unListenIP, usListenPort);
	    if(iIdx < 0)
	    {
	        
	        close(iLisetenSocket);
	        printf("[%s:%d] add to socket array failed!\n", __FILE__, __LINE__);
	        return -3;
	    }
	    
	}

	if(m_Svr2MePipe.GetReadNotifyFd()>=0)
	{
	    int iIdx = CreateSocketNode(m_Svr2MePipe.GetReadNotifyFd(), TSocketNode::MQ_PIPE_SOCKET, 0,0);
	    if(iIdx < 0)
	    {
	        printf("[%s:%d] add MQ_PIPE_SOCKET to socket array failed!\n", __FILE__, __LINE__);
	        return -3;
	    }
	}

	return 0;

}


int CMainCtrl::CreateSocketNode(int aSocket, int aType,unsigned int aClientIP, unsigned short aClientPort)
{
    if(aSocket<0)
	    return -1;

	int iIdx = (aSocket%m_stConfig.MAX_SOCKET_NUM);

	if(m_pSocketNode[iIdx].m_iSocket > 0)
	{
	    printf("[%s:%d] MaxSocketNum %d too small!\n", m_stConfig.MAX_SOCKET_NUM);
	    return -2;
	}

	TSocketNode *stPSocketNode = m_pSocketNode[iIdx];

	stPSocketNode->m_iSocket = aSocket;
	stPSocketNode->m_sSocketType = aType;
	stPSocketNode->m_usClientPort = aClientPort;
	stPSocketNode->m_uiClientIP = aClientIP;
	stPSocketNode->m_iActiveTime = m_tNow.tv_sec;
	stPSocketNode->m_uiID = m_unID++;

	fcntl(aSocket, F_SETFL, fcntl(aSocket,F_GETFL)|O_NONBLOCK|O_NDELAY);
	m_stEPollFlow.Add(aSocket, iIdx, EPOLLIN|EPOLLERR|EPOLLHUP );

	if(aType == TSocketNode::TCP_CLIENT_SOCKET)
	    m_unTcpClientNum++;

	 return iIdx;
	    
}
int CMainCtrl::Run()
{
    while(1)
    {
        if(em_FlagExit == m_iRunFlag)
        {
            return 0;
        }
        
        if(em_FlagReloadCfg == m_iRunFlag)
        {
            ReadCfgFile(m_stConfig.m_szCfgFileName);
            m_iRunFlag = em_FlagNormal;
        }

		//1ms
        if (m_Svr2MePipe.GetCodeLength()==0)
        	m_stEPollFlow.wait(0);
        else
        	m_stEPollFlow.wait(1);


        //
        gettimeofday(&m_tNow, NULL);

        //
        CheckClientMessage();

        //
        CheckSvrMessage();

        //
        CheckTimeOut();

        //
        WriteStat();
        
        
    }
}

int CMainCtrl::ClearSocketNode(int aSocketSuffix, int aCloseNotify = 0)
{
}
int CMainCtrl::CheckClientMessage()
{
    long long  llKey;
    unsigned int unEvents;

    while(m_stEPollFlow.GetEvents(llKey,unEvents))
    {
        int iSocketSuffix = llKey;
        TSocketNode *pSocketNode = &m_pSocketNode[iSocketSuffix];

        if(pSocketNode->m_iSocket < 0)
        {
            continue;
        }

        if(!((EPOLLIN|EPOLLOUT)&unEvents))
        {
            ClearSocketNode(iSocketSuffix, m_stConfig.m_iDisconnectNotify);
            continue;
        }

        if(EPOLLIN&unEvents)
        {
            if(TSocketNode::TCP_LISTEN_SOCKET == pSocketNode->m_sSocketType)
            {
                struct sockaddr_in stSockAddr ;
                int iSockAddrSize = sizeof(stSockAddr);
                int iNewSocket = accept(pSocketNode->m_iSocket, (struct sockaddr*)&stSockAddr, (socklen_t *)&iSockAddrSize);

                if (iNewSocket < 0)
                {
                    //printf();
                    continue;
                }

                if(CheckIPAccess(stSockAddr.sin_addr.s_addr) != emIPAccess)
                {
                    //printf();
                    close(iNewSocket);
                    continue;
                }

               int iNewSuffix = CreateSocketNode(iNewSocket, TSocketNode::TCP_CLIENT_SOCKET, stSockAddr.sin_addr.s_addr, ntohs(stSockAddr.sin_port));
               if(iNewSocket < 0)
               {
                   // printf();
                   close(iNewSocket);
                   continue;
               }                
            }
            else if(TSocketNode::TCP_CLIENT_SOCKET ==  pSocketNode->m_sSocketType)
            {
                RecvClientData(iSocketSuffix);
            }
            else if(TSocketNode::UDP_SOCKET ==  pSocketNode->m_sSocketType)
            {
            }
            else if(TSocketNode::ADMIN_LISTEN_SOCKET == pSocketNode->m_sSocketType)
            {
				struct sockaddr_in stSockAddr ;
				int iSockAddrSize = sizeof(stSockAddr);
				int iNewSocket = accept(pSocketNode->m_iSocket, (struct sockaddr*)&stSockAddr, (socklen_t *)&iSockAddrSize);

				if (iNewSocket < 0)
				{
				    //printf();
				    continue;
				}

				int iNewSuffix = CreateSocketNode(iNewSocket, TSocketNode::TCP_CLIENT_SOCKET, stSockAddr.sin_addr.s_addr, ntohs(stSockAddr.sin_port));
				if(iNewSocket < 0)
				{
				   // printf();
				   close(iNewSocket);
				   continue;
				}                
            }
            else if(TSocketNode::MQ_PIPE_SOCKET == pSocketNode->m_sSocketType)
            {
            }
            else if(TSocketNode::ADMIN_CLIENT_SOCKET== pSocketNode->m_sSocketType)
            {
            }
            else
            {
            }
        }
    }
    
}
int  CMainCtrl::ReadCfgFile(char *aConfigFile)
{
}

int CMainCtrl::FillMQHead(int aSuffix,TMQHeadInfo* aMQHeadInfo,
		unsigned char aCmd,unsigned char aDataType/*=TMQHeadInfo::DATA_TYPE_TCP*/)
{
    TSocketNode* pSocketNode = &m_pSocket[iSuffix];    
	if (pSocketNode->m_iSocket<0)
		return -1;

    memset(aMQHeadInfo,0,sizeof(TMQHeadInfo));
	aMQHeadInfo->m_iSuffix = aSuffix;
	aMQHeadInfo->m_ucCmd = aCmd;
	aMQHeadInfo->m_ucDataType = aDataType;
	aMQHeadInfo->m_usClientPort = pSocketNode->m_usClientPort;		
	aMQHeadInfo->m_unClientIP = pSocketNode->m_unClientIP;
	memcpy(aMQHeadInfo->m_szSrcMQ,CCS_MQ,sizeof(CCS_MQ));

	aMQHeadInfo->m_tTimeStampSec = m_tNow.tv_sec;
	aMQHeadInfo->m_tTimeStampuSec = m_tNow.tv_usec;

	memcpy(aMQHeadInfo->m_szEchoData,&(pSocketNode->m_unID),sizeof(int));
	return 0;
}

int CMainCtrl::RecvClientData(int aSocketSuffix)
{
    if(aSocketSuffix < 0 || aSocketSuffix >= m_stConfig.MAX_SOCKET_NUM)
	    return -1;

	TSocketNode* pSocketNode = &m_pSocketNode[aSocketSuffix];

	if(pSocketNode->m_iSocket < 0)
		return -2;

    int iMissDataLen = 0;
	int iOldDataSize = m_stBuffMngRecv.GetBufferSize(aSocketSuffix);
	if(iOldDataSize < 0)
	{
	    ClearSocketNode(aSocketSuffix, m_stConfig.m_iDisconnectNotify);
	    //printf();
	    return -1;
	}

	ssize_t iGetSize = m_stBuffMngRecv.GetBuffer(aSocketSuffix , m_cBuffRecv + sizeof(TMQHeadInfo), iOldDataSize);
	if(iGetSize != iOldDataSize)
	{
	    // printf();
	    ClearSocketNode(aSocketSuffix, m_stConfig.m_iDisconnectNotify);
	    return -1;
	}

	m_stBuffMngRecv.FreeBuffer(aSocketSuffix);

	char* pCodeDataPtr = m_cBuffRecv + sizeof(TMQHeadInfo)+iOldDataSize;

	ssize_t iRecvDataLen = read(pSocketNode->m_iSocket, pCodeDataPtr, sizeof(m_cBuffRecv)-sizeof(TMQHeadInfo)-iOldDataSize);
	if(0==iRecvDataLen||(iRecvDataLen<0&&(errno !=EAGAIN)))
	{
	    //printf();
	    ClearSocketNode(aSocketSuffix, m_stConfig.m_iDisconnectNotify);
	    return -2;
	}
	else if(iRecvDataLen<0)
	{
	    return -3;
	}

    int  iPkgTheoryLen;
    int iMsgLen =0;
    pCodeDataPtr = m_cBuffRecv + sizeof(TMQHeadInfo);
    int iCodeLeftLen = iOldDataSize+iRecvDataLen;
	while(iMsgLen = net_complete_func(pCodeDataPtr, iCodeLeftLen,iPkgTheoryLen)>0)
	{
	    if(iPkgTheoryLen < 0 || iPkgTheoryLen > MAX_MSG_LEN-sizeof(TMQHeadInfo))
	    {
	        //printf();
	        ClearSocketNode(aSocketSuffix, m_stConfig.m_iDisconnectNotify);
	        return -4;
	    }

	   TMQHeadInfo* pMQHeadInfo = (TMQHeadInfo*)(pCodeDataPtr-sizeof(TMQHeadInfo));
	   FillMQHead(aSocketSuffix,pMQHeadInfo,TMQHeadInfo::CMD_DATA_TRANS);

	   int iRet =  m_Me2SvrPipe.AppendOneCode(pCodeDataPtr-sizeof(TMQHeadInfo), iPkgTheoryLen+sizeof(TMQHeadInfo));
	   if(iRet < 0)
	   {
	        //printf();

	        //不能秘密丢包,关闭连接,使外界感知
	        ClearSocketNode(aSocketSuffix, m_stConfig.m_iDisconnectNotify);
	        return -6;
	   }

	    m_stStat.m_llTcpCSPkgLen += iPkgTheoryLen
	    m_stStat.m_llTcpCSPkgNum ++;
	    pCodeDataPtr += iPkgTheoryLen;
	    iCodeLeftLen -= iPkgTheoryLen;	    
	}
	
	if(iMsgLen < 0)
    {
        //printf();

        ClearSocketNode(aSocketSuffix, m_stConfig.m_iDisconnectNotify);
        return -6;
    }

    if(iCodeLeftLen <=0)
        return  0;


    m_stBuffMngRecv.A


    

    

    

}

int CMainCtrl::CheckSvrMessage()
{
}

int CheckTimeOut()
{
}

