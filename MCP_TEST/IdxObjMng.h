#ifndef __TIDXOBJMNG_H__
#define __TIDXOBJMNG_H__


//本块索引定义
const int MAX_DSINFO_NUM = 16;


#ifndef _IDX64
    #define idx_t int32_t
#else
    #define idx_t int64_t
#endif

//块/对象管理器
class TIdxObjMng
{
public:
/*
在内部管理时，用0存储free指针链，分配后由外部使用，本类就不再管理这个节点了

*/

#pragma pack(1)
    typedef struct
    {
        idx_t m_piDsInfo[0];
    }TIdx;
#pragma pack()

#define GETIDX(iIdx) ((TIdx*)((char*)m_pIdx + m_pIdxObjHead->m_iIdxSize*(iIdx)))

    TIdxObjMng();
    ~TIdxObjMng(){};
    static ssize_t CountMemSize(ssize_t aObjSize, ssize_t aIdxObjNum, ssize_t aDSNum=1);
    static ssize_t CountIdxObjNum(const ssize_t MEMSIZE,  ssize_t aObjSize, ssize_t  iDSNum);

    ssize_t GetOneFreeDS();
    ssize_t AttachMem(char* aBuf, ssize_t MEMSIZE, ssize_t aObjSize, ssize aDSNum);

    void FormatIdx();

    ssize_t CreateObject();
    ssize_t DestroyObject(ssize_t aIdx);
    ssize_t CopyAttachObj(ssize_t aIdx, ssize_t aOffset, char* aBuf, const ssize_t COPYSIZE);
    char* GetAttachObj(ssize_t aIdx);
    
    ssize_t SetAttachObj(ssize_t aIdx, ssize_t aOffset, char* aBuf, ssize_t aLen);
    ssize_t GetObjNum(){return m_pIdxObjHead->m_iIdxObjNum};
    ssize_t GetUsedObjNum(){return m_pIdxObjHead->m_iUsedCount};
    ssize_t GetObjSize(){return m_pIdxObjHead->m_iObjSize};

    ssize_t GetDsIdx(ssize_t aIdx, ssize_t aDsSufix);    
    ssize_t SetDsIdx(ssize_t aIdx, ssize_t aDsSufix, ssize_t aNextIdx);

public:
    typedef struct
    {
        ssize_t m_iFreeHead;
        ssize_t m_iUsedCount;

        ssize_t m_iObjSize;
        ssize_t m_iIdxObjNum;
        ssize_t m_iIdxSize;
    }TIdxObjHead;


protected:

    sszie_t m_iDSNum;
    ssize_t m_piDsUseFlag[MAX_DSINFO_NUM];

    TIdxObjHead* m_pIdxObjHead;
    TIdx* m_pIdx;  //索引区

private: 
    char* m_pObjMem;    //数据区
    
};

#endif
