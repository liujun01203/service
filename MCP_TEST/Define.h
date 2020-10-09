#ifndef __DEFINE_H__
#define __DEFINE_H__


#ifndef _IDX64
    #define idx_t int32_t
#else
    #define idx_t int64_t
#endif


typedef enum
{
	emInit = 0,
	emRecover = 1,
}emInitType;


#endif 
