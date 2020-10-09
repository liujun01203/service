
const int MAX_MSG_LEN	= 16*1024*1024;
const int MQ_NAME_LEN	= 8;
const int  MQ_ECHO_DATA_LEN	= 12;

#pragma pack(1)
typedef struct
{
    u_int8_t m_uCmd;
    enum
    {    
		//通用
		CMD_DATA_TRANS = 0, 		//数据传输包

		//CCS使用
		CMD_CCS_NOTIFY_DISCONN, 	//通知断开连接

		//SCC使用，S->C方向
		CMD_REQ_SCC_CONN,			//请求连接
		CMD_REQ_SCC_CLOSE,			//请求关闭
		
		//SCC使用，C->S方向
		CMD_SCC_RSP_CONNSUCC,		//通知连接成功
		CMD_SCC_RSP_CONNFAIL,		//通知连接失败
		CMD_SCC_RSP_DISCONN,		//通知连接断裂	

		//内部使用
		CMD_NEW_TCP_SOCKET,
		CMD_NEW_UDP_SOCKET,
    };
    u_int8_t m_ucDataType;    

    enum
    {
        DATA_TYPE_TC =0,
        DATA_TYPE_UDP =1,
    };
    
	// SCC/UDP 使用定位部分 
	u_int16_t m_usClientPort;		/*客户端端口,本地字节序*/	
	u_int32_t m_unClientIP; 		/*客户端IP地址,网络字节序*/

	//CCS使用定位部分
	int32_t m_iSuffix;				/*唯一标记*/
	char m_szEchoData[MQ_ECHO_DATA_LEN];	/*回射数据,使用者必须回带*/

	char m_szSrcMQ[MQ_NAME_LEN];	/*本次来源MQ,字串类型,0结尾*/
	u_int16_t m_usSrcListenPort;	/*本次来源的监听端口*/
	u_int16_t m_usFromCQID; 		/*来源CQ管道*/

	char m_szDstMQ[MQ_NAME_LEN];	/*本次目标MQ,字串类型,0结尾*/	
	u_int16_t m_usRserve2;			/*保留*/
	u_int16_t m_usRserve3;			/*保留*/
	
	//管道时间戳记,用来得知消息在管道中的留存时间
	u_int32_t  m_tTimeStampSec;
	u_int32_t  m_tTimeStampuSec;    
}TMQHeadInfo;
#pragma pack();
