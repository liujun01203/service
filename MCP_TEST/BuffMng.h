



class CBuffMng
{
public:

#pragma pack(1)
typedef struct
{
    idx_t m_iBuffIdx;     //��һ��λ��
    idx_t m_iBuffSize;    //�����ӳ���

#ifdef _BUFFMNG_APPEND_SKIP
    idx_t m_iLastBuffIdx;   //���һ��λ��
    idx_t m_iBuffOffset;    //���ƫ��
#endif
}TBuffItem;
#pragma pack()

    CBuffMng();
    ~CBuffMng();
    static ssize_t CountMemSize(ssize_t aBuffNum);
    ssize_t AttachMem(char *aBuf, ssize_t MEMSIZE, ssize_t aBuffNum, ssize_t aInitType=emInit);
    ssize_t AttachIdxObjMng(TIdxObjMng *aIdxObjMng);

    ssize_t SetBufferSpace(ssize_t aBuffItemNo, ssize_t aBuffLen);
    ssize_t SetDataToBufferSpace(ssize_t aBuffItemNo, char* aBuf,ssize_t aBuffLen);

    ssize_t SetBuffer(ssize_t aBuffSuffix, char* aBuf, ssize_t aBufLen);
    ssize_t GetBuffer(ssize_t aBuffSuffix, char* aBuf, const ssize_t aBufLen);
    ssize_t GetBufferSize(ssize_t aBuffSuffix);

    bool HaveFreeSpace(ssize_t aLen);
    ssize_t FreeObjList(ssize_t iObjIdx);

#ifdef _BUFFMNG_APPEND_SKIP
    ssize_t AppendBuffer(ssize_t aBuffSuffix,char* aBuffer, ssize_t aBuffSize);
    ssize_t Skip(ssize_t aBuffSuffix, ssize_t aSkipLen);
    ssize_t SendBufferToSocket(ssize_t aSocketFD,ssize_t aiBuffSuffix);
#endif


    ssize_t FreeBuffer(ssize_t aBuffItemNo);

public:
typedef struct
{
    ssize_t m_iBuffNum;      //�������
    sszie_t m_iFreeBuffNum;  //���л������
    ssize_t m_iDSSuffix;    //�ڶ����������ʹ�õ���
}TBuffMngHead;
private:
	TBuffMngHead* m_pBuffMngHead;
	TBuffItem * m_pBuffItem;
	TIdxObjMng * m_pIdxObjMng;
	ssize_t m_iInitType;
};
