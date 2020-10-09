
const MAX_MSG_LEN = 16*1024*1024;
typedef int (*check_complete)(const void *aData, unsigned int aDataLen, int &aPkgTheoryLen);
typedef int (*msg_header_len)();


const int MAX_BIND_PORT_NUM =16;

typedef struct
{
	unsigned int m_unIP;
	unsigned short m_usPort;
}TIpPortStu;


typedef struct
{
	long long m_llTcpCSPkgNum;
	long long m_llTcpCSPkgLen;
	long long m_llTcpSCPkgNum;
	long long m_llTcpSCPkgLen;

	long long m_llUdpCSPkgNum;
	long long m_llUdpCSPkgLen;
	long long m_llUdpSCSuccPkgNum;
	long long m_llUdpSCFailedPkgNum;
	long long m_llUdpSCPkgLen;	
}TStatStu;

typedef struct  
{
	typedef struct
	{
		unsigned int m_unIP;
		unsigned short m_usPort;
	}TIpPortStu;

	TIpPortStu m_stTcpIPPortStu[MAX_BIND_PORT_NUM];
	TIpPortStu m_stUdpIPPortStu[MAX_BIND_PORT_NUM];

	//基本
	char	m_szSvrName[256];
	char	m_szCfgFileName[256];

	char	m_szNetCompleteSo[256];

	char	m_szMeToSvrMQ[128];
	char	m_szSvrToMeMQ[128];

	char	m_szAdminIp[32];
	int		m_iAdminPort;

	int 	m_iStatTimeGap;
	int 	m_iDisconnectNotify;

	int		m_iBindCpu;

	int 	m_iIpAccessOpen;
	int 	m_iIpAccessDefault;
	char	m_szIpAccessTable[128];

	int 	m_iTimeOutCheckSecs;
	int 	m_iTimeOutSecs;

	int 	m_iMaxQueueWaitus;

	int 	m_iLoadCheckOpen;
	int 	m_iLoadCheckAllSpan;
	int 	m_iLoadCheckEachSpan;
	int 	m_iLoadCheckMaxPkgNum;

	int		SOCKET_RCVBUF;
	int 	SOCKET_SNDBUF;

	int		MAX_SOCKET_NUM;
	
	int 	RCV_BLOCK_SIZE;
	int 	RCV_BLOCK_NUM;
	
	int 	SND_BLOCK_SIZE;
	int 	SND_BLOCK_NUM;

	int		LISTEN_BACK_LOG;
	void Print()
	{
		TLib_Log_LogMsg("[config]\n");
		TLib_Log_LogMsg("m_szSvrName %s\n",m_szSvrName);
		TLib_Log_LogMsg("m_szCfgFileName %s\n",m_szCfgFileName);
		TLib_Log_LogMsg("m_szNetCompleteSo %s\n",m_szNetCompleteSo);
		TLib_Log_LogMsg("m_szMeToSvrMQ %s\n",m_szMeToSvrMQ);
		TLib_Log_LogMsg("m_szSvrToMeMQ %s\n",m_szSvrToMeMQ);
		TLib_Log_LogMsg("m_szAdminIp %s\n",m_szAdminIp);
		TLib_Log_LogMsg("m_iAdminPort %d\n",m_iAdminPort);
		TLib_Log_LogMsg("m_iStatTimeGap %d\n",m_iStatTimeGap);
		TLib_Log_LogMsg("m_iDisconnectNotify %d\n",m_iDisconnectNotify);
		TLib_Log_LogMsg("m_iBindCpu %d\n",m_iBindCpu);
		TLib_Log_LogMsg("m_iIpAccessOpen %d\n",m_iIpAccessOpen);
		TLib_Log_LogMsg("m_iIpAccessDefault %d\n",m_iIpAccessDefault);
		TLib_Log_LogMsg("m_szIpAccessTable %s\n",m_szIpAccessTable);
		TLib_Log_LogMsg("m_iTimeOutCheckSecs %d\n",m_iTimeOutCheckSecs);
		TLib_Log_LogMsg("m_iTimeOutSecs %d\n",m_iTimeOutSecs);
		TLib_Log_LogMsg("m_iMaxQueueWaitus %d\n",m_iMaxQueueWaitus);
		TLib_Log_LogMsg("m_iLoadCheckOpen %d\n",m_iLoadCheckOpen);
		TLib_Log_LogMsg("m_iLoadCheckAllSpan %d\n",m_iLoadCheckAllSpan);
		TLib_Log_LogMsg("m_iLoadCheckEachSpan %d\n",m_iLoadCheckEachSpan);
		TLib_Log_LogMsg("m_iLoadCheckMaxPkgNum %d\n",m_iLoadCheckMaxPkgNum);
		TLib_Log_LogMsg("SOCKET_RCVBUF %d\n",SOCKET_RCVBUF);
		TLib_Log_LogMsg("SOCKET_SNDBUF %d\n",SOCKET_SNDBUF);
		TLib_Log_LogMsg("RCV_BLOCK_SIZE %d\n",RCV_BLOCK_SIZE);
		TLib_Log_LogMsg("SND_BLOCK_SIZE %d\n",SND_BLOCK_SIZE);
		TLib_Log_LogMsg("RCV_BLOCK_NUM %d\n",RCV_BLOCK_NUM);
		TLib_Log_LogMsg("SND_BLOCK_NUM %d\n",SND_BLOCK_NUM);
		TLib_Log_LogMsg("MAX_SOCKET_NUM %d\n",MAX_SOCKET_NUM);
		TLib_Log_LogMsg("LISTEN_BACK_LOG %d\n",LISTEN_BACK_LOG);
		TLib_Log_LogMsg("\n");
	}	
}TConfig;

typedef struct
{
    int m_iSocket;
    short m_sSocketType;
    enum 
    {
        TCP_LISTEN_SOCKET = 0,   //监听
        TCP_CLIENT_SOCKET = 1,   //客户的连接

        UDP_SOCKET = 2,          //UDP 
        MQ_PIPE_SOCKET = 3,      //shm 管道通知
        ADMIN_LISTEN_SOCKET = 4,  // 管理监听
        ADMIN_CLIENT_SOCKET = 5,   // 管理连接
    };

    unsigned short m_usClientPort;  //客户端端口
    unsigned int m_uiClientIP;      //客户端IP
    int m_iActiveTime;              //最后活跃时间

    unsigned short m_usListenPort;  //原生的监听端口
    unsigned int m_uiID;            //唯一ID    
}TSocketNode;

enum
{
    em_FlagReloadCfg =1,
    em_FlagExit =2,
};

enum
{
	emIPAccess = 1,
	emIPDeny = 2,
};

class CMainCtrl
{
	public:

		CMainCtrl();
		~CMainCtrl();

		int Initialize(char* aProName, char*aConfigFile);
		int InitSocket();

		int Run();
		void SetRunFlag(int iRunFlag)
		{
			m_iRunFlag = iRunFlag;
		}   

		int ReadCfgFile(char *aConfigFile);
		int CreateSocketNode(int aSocket, int aType,unsigned int aClientIP, unsigned short aClientPort );
		
        int CheckClientMessage();        
        int CheckSvrMessage();
        int CheckTimeOut();

        
		int ClearSocketNode(int aSocketSuffix, int aCloseNotify = 0);
		int RecvClientData(int aSocketSuffix);
        int FillMQHead(int aSuffix,TMQHeadInfo* aMQHeadInfo,
		unsigned char aCmd,unsigned char aDataType/*=TMQHeadInfo::DATA_TYPE_TCP*/);

	private:
		CCodeQueue  m_Me2SvrPipe;
		CCodeQueue  m_Svr2MePipe;
		char m_cBuffRecv[MAX_MSG_LEN];
		char m_cBuffSend[MAX_MSG_LEN];

        //Recv Buff
        TIdxObjMng m_stIdxObjMngRecv;
        CBuffMng   m_stBuffMngRecv;

        //Send Buff
        TIdxObjMng m_stIdxObjMngSnd;
        CBuffMng   m_stBuffMngSnd;		

		TConfig m_stConfig;
		timeval m_tNow;

		CLoadGrid *m_pCLoadGrid;
		volatile int m_iRunFlag;
		
        CEPollFollow m_stEPollFlow;
		TSocketNode *m_pSocketNode;		
		unsigned int m_unID;

        //判断包函数，返回包长
		check_complete net_complete_func;
		msg_header_len msg_header_len_func;
		int m_iMsgHeaderLen;

		unsigned int m_unTcpClientNum;

        TStatStu	m_stStat;;
		
		
};

