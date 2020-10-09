#ifndef __TLIB_CFG_H__
#define __TLIB_CFG_H__

const int CFG_STRING=1;
const int CFG_INT=2;
const int CFG_INT64=3;
const int CFG_DOUBLE=4;

typedef struct
{
	char m_cName[256];
	char m_cVal[256];
}TNVStu;

int TLibCfgGetConfig(const char *aConfigFilePath, ...);
void TLibCfgGetNVConfig(const char * aConfigFilePath, TNVStu *aNVStu);
#endif
