#ifndef __CCODEQUEUE__H__
#define __CCODEQUEUE__H__


const int QUEUE_RESERVED_LENGTH = 8;
const int MAX_LOCK_STRUCT_SIZE = 1024;

typedef struct
{
	int m_iShmSize;
	int m_iWLock;
	int m_iRLock;
	int m_iNumber;
	unsigned short m_usUniqueId;
}TCQCfgStruct;

class CCodeQueue
{
	public:
		CCodeQueue();
		~CCodeQueue();

		static key_t FTOK(const char *aFile, char aProjId);
		static int CreateByFile(const char *aFile, CCodeQueue* aCodeQueue);
		static int GetCfgFromFile(const char *aFile, TCQCfgStruct &stCQCfgStruct);

		int AttachMem(int aFifoKey,char *aBuf, const int aMemSize, int aInitType=Init, int aWLock=emNoLock, int aRLock=emNoLock, void* aPWLock=NULL, void* aPRLock=NULL);
		int GetCodeLength();
		

		int GetReadNotifyFd();
		int SetId();
		int GetId();
		int GetWLock();
		int GetRLock();
		int WaitData(struct timeval *aTimeVal);
		bool IsQueueEmpty();
		boll IsQueueFull();
		void Print(FILE * aFp= stdout);



		int AppendOneCode(const char *aInCode, int aInLength, bool aAutoNotify=true);
		int GetHeadCode(char *aOutCode, int *aOutLength);

	private:
		int _CreateFifo(int aFifoKey);
		int _AppendOneCode(const char *aIncode, int aInLength, bool aAutoNotify);
		int _GetHeadCode(char *aOutCode, int* aOutLength );
			
  
 
	private:
	    typedef struct 
	    {
			int m_nSize;
			volatile int m_nBegin;
			volatile int m_nEnd;
		}TCodeQueueHead;


		TCodeQueueHead *m_pCodequeueHead;
		char *m_pCodeBuf;


		typedef enum
	    {
			emNoLock = 0,
			emSpinLock = 1,
			emPthreadMutexLock,
		}EMLockType;

	    // 锁类型
		int m_iWLockType;	
		int m_iRLockType;

		//锁
		void* m_pWLock;
		void *m_pRLock;

		char m_WLockSpace[MAX_LOCK_STRUCT_SIZE];
		char m_RLockSpace[MAX_LOCK_STRUCT_SIZE];


        //通知使用
		int m_iNotifyFiFoFd;
		int m_iPipeFd[2];

        // 自身ID
		int m_iID;
		
};



#endif
