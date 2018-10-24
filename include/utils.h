#ifndef _BACKER_UTILS_H_
#define _BACKER_UTILS_H_
    
#include "sys_common.h"
#include "net_common.h"
#include <string>

namespace utils 
{
    namespace net 
    {
	
	/*
	   @parameters:address's correct format,eg.:localhost:1986
	   @returns: 0 indicates address is valid,1 otherwise
	*/ 
	int checkAddress (const std::string & address);
	inline std::string getRemoteIPv4 (const int &sockfd) 
	{
	    char ip[INET_ADDRSTRLEN];
	    struct sockaddr_in peerAddr;
	    socklen_t len;
	    getpeername (sockfd, (struct sockaddr *) &peerAddr, &len);
	    inet_ntop (AF_INET, &peerAddr.sin_addr, ip, INET_ADDRSTRLEN);
	    return std::string (ip);
	}
	ssize_t readBuffer (int fd, void *, size_t);
	ssize_t writeBuffer (int fd, void *, size_t);
    } 

    namespace sys 
    {
	inline long int getSysProcessors () 
	{
	    return sysconf (_SC_NPROCESSORS_ONLN);
	}

	inline int setnonblock (int fd) 
	{
	    int option = fcntl (fd, F_GETFL);
	    int option_ = option | O_NONBLOCK;
	    fcntl (fd, F_SETFL, option_);
	    return option;
	}
    }
};


#endif
