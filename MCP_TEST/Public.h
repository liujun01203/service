#include <sys/shm.h>
char* CreateShm(key_t aShmKey,  const int aShmSize, int& aErrorNo, bool& aNewCreate, int& aOutShmID)
{
	int iShmId = shmget(aShmKey, aShmSize, IPC_CREAT|IPC_EXCL|0666);

	if (iShmId < 0 )
	{
		aNewCreate = false;
		if (errno != EEXIST)
		{
			aErrorNo = errno;
			return NULL;
		}

		iShmId = shmget(aShmKey, aShmSize, 0666);

		if (iShmId < 0 )
		{
			iShmId = shmget(aShmKey,0, 0666);
			if(iShmId < 0)
			{
				iErrNo = errno;
				return NULL;
			}
			else
			{
				struct shmid_ds s_ds;
				shmctl(iShmId, IPC_STAT, *s_ds);
				if(s_ds.shm_nattch > 0)
				{
					aErrorNo = EEXIST;
					return NULL;
				}
				else
				{
					if (shmctl(iShmId, IPC_RMID, NULL) < 0)
					{
						aErrorNo = errno;
						return NULL;
					}
						
					iShmId = shmget(aShmKey, aShmSize,  IPC_CREAT|IPC_EXCL|0666);
					if(iShmId < 0)
					{
						iErrNo = errno;
						return NULL;
					}

					aNewCreate = true;
				}
			}
		}
	}
	else
	{
		aNewCreate = true;
	}
	aErrorNo = 0;
	aOutShmID = iShmId;
	return (char *)shmat(iShmId, NULL, 0);
	
}



typedef enum
{
	emTCP = 0,
	emUDP = 1,
}emSocketType;
int CreateListenSocket(u_int32 aListenIP, u_int16_t aListenPort, int aRecvBufLen, int aSndBufLen, int aProtocol, int aListenBackLog)
{
}


void GetNameFromPath(char * aPath, char *aName)
{
}
