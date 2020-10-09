


class CLoadGrid
{
public:
	enum LoadRet
	{
		LR_ERROR = -1,
		LR_NORMAL = 0,
		LR_FULL = 1,
	};
	CLoadGrid(int aTimeAllSpan, int aEachGridSpan, int aMaxNumInAllSpan = 0);
	~CLoadGrid();
    int CheckLoad(timeval aTimeVal);
    void FetchLoad(int& aTimeAllSpanMs, int& aNumInAllSpan);

private:
    void UpdateLoad(timeval *aTimeVal=NULL);
    

private:
    int m_iTimeAllSpanMs;
    int m_iEachGridSpanMs;
    int m_iMaxNumInAllSpan;
    
    int m_iAllGridCount;
    int *m_iGridArray;
    int m_iNumInAllSpan;
    timeval m_tLastGridTime;

    int m_iCurrGrid;
    bool m_bBlock;
    
    spinlock_t m_spinlock;	
};
