
const int MAX_MSG_LEN	= 16*1024*1024;
const int MQ_NAME_LEN	= 8;
const int  MQ_ECHO_DATA_LEN	= 12;

#pragma pack(1)
typedef struct
{
    u_int8_t m_uCmd;
    enum
    {    
		//ͨ��
		CMD_DATA_TRANS = 0, 		//���ݴ����

		//CCSʹ��
		CMD_CCS_NOTIFY_DISCONN, 	//֪ͨ�Ͽ�����

		//SCCʹ�ã�S->C����
		CMD_REQ_SCC_CONN,			//��������
		CMD_REQ_SCC_CLOSE,			//����ر�
		
		//SCCʹ�ã�C->S����
		CMD_SCC_RSP_CONNSUCC,		//֪ͨ���ӳɹ�
		CMD_SCC_RSP_CONNFAIL,		//֪ͨ����ʧ��
		CMD_SCC_RSP_DISCONN,		//֪ͨ���Ӷ���	

		//�ڲ�ʹ��
		CMD_NEW_TCP_SOCKET,
		CMD_NEW_UDP_SOCKET,
    };
    u_int8_t m_ucDataType;    

    enum
    {
        DATA_TYPE_TC =0,
        DATA_TYPE_UDP =1,
    };
    
	// SCC/UDP ʹ�ö�λ���� 
	u_int16_t m_usClientPort;		/*�ͻ��˶˿�,�����ֽ���*/	
	u_int32_t m_unClientIP; 		/*�ͻ���IP��ַ,�����ֽ���*/

	//CCSʹ�ö�λ����
	int32_t m_iSuffix;				/*Ψһ���*/
	char m_szEchoData[MQ_ECHO_DATA_LEN];	/*��������,ʹ���߱���ش�*/

	char m_szSrcMQ[MQ_NAME_LEN];	/*������ԴMQ,�ִ�����,0��β*/
	u_int16_t m_usSrcListenPort;	/*������Դ�ļ����˿�*/
	u_int16_t m_usFromCQID; 		/*��ԴCQ�ܵ�*/

	char m_szDstMQ[MQ_NAME_LEN];	/*����Ŀ��MQ,�ִ�����,0��β*/	
	u_int16_t m_usRserve2;			/*����*/
	u_int16_t m_usRserve3;			/*����*/
	
	//�ܵ�ʱ�����,������֪��Ϣ�ڹܵ��е�����ʱ��
	u_int32_t  m_tTimeStampSec;
	u_int32_t  m_tTimeStampuSec;    
}TMQHeadInfo;
#pragma pack();
