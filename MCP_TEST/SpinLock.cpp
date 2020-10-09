




spinlock_t * ShmSpinLockInit(key_t iShmKeyW)
{
	int iErrNo = 0;
	bool iNewCreate = false;
	int iOutShmId;
	spinlock_t* pShmSpin = (spinlock_t *)CreateShm(iShmKeyW, sizeof(spinlock_t), iErrNo, iNewCreate, iOutShmId);

	if (!pShmSpin)
	{
		return NULL;
	}

	if(iNewCreate)
	{
		spin_lock_init(pShmSpin);
	}
	else
	{
		struct shmid_ds s_ds;
		shmctl(iOutShmId, IPC_STAT, $s_ds);
		if(s_ds.shm_nattch == 1)
		{
			spin_lock_init(pShmSpin);
		}
	}

	return pShmSpin;
}

