/*
@filename:lavender.h
@author:chenzhengqiang
@date:2018-10-02
@email:642346572@qq.com
*/  
    
#ifndef _BACKER_ERRORNO_H_
#define _BACKER_ERRORNO_H_
namespace error 
{
    namespace code 
    {
	enum 
	{ 
            SUCCESS = 0, 
            INVALID_FORMAT = 1, 
	};
    } 
    const char *getErrorMessage (const int errorno);
};


#endif
