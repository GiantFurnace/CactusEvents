/*
@filename:lavender.cpp
@author:chenzhengqiang
@date:2018-10-02
@email:642346572@qq.com
*/

#include "utils.h"
#include "error.h"
#include <cctype>
#include <cstdlib>
#include <errno.h>
extern int errno;


namespace utils
{
    namespace net
    {
	/*
	 *  @parameters:address's correct format,eg.:localhost:1986
	 *  @returns: 0 indicates address is valid,1 otherwise
	 */
	int checkAddress (const std::string & address)
	{
	    std::size_t len = address.length ();
	    if (len < 9)
	      {
		  return error::code::INVALID_FORMAT;
	      }
	    std::size_t found = address.find_first_of (":");
	    if (found == std::string::npos)
	      {
		  return error::code::INVALID_FORMAT;
	      }
	    std::string ip = address.substr (0, found);
	    if (ip != "localhost")
	      {
		  int last_character = ip[0];
		  if (last_character == '.' || ip[ip.length () - 1] == '.')
		    {
			return error::code::INVALID_FORMAT;
		    }

		  std::size_t count = 0;
		  std::string::const_iterator iter = ip.begin () + 1;
		  while (iter != ip.end ())
		    {
			if (!isdigit (*iter) && (*iter) != '.')
			  {
			      return error::code::INVALID_FORMAT;
			  }
			if (*iter == '.')
			  {
			      ++count;
			  }
			if (last_character == '.' && (*iter) == '.')
			  {
			      return error::code::INVALID_FORMAT;
			  }
			last_character = *iter;
			++iter;
		    }
		  if (count != 3)
		    {
			return error::code::INVALID_FORMAT;
		    }
	      }

	    std::string portstr = address.substr (found + 1, address.length () - found);
	    std::string::const_iterator iter = portstr.begin ();
	    while (iter != portstr.end ())
	      {
		  if (!isdigit (*iter))
		    {
			return error::code::INVALID_FORMAT;
		    }
		  ++iter;
	      }

	    int portint = atoi (portstr.c_str ());
	    if (portint <= 0 || portint > 65535)
		return error::code::INVALID_FORMAT;
	    return error::code::SUCCESS;
	}

	ssize_t readBuffer (int fd, void *buffer, size_t bufferSize)
	{
	    ssize_t _left;
	    ssize_t _received;
	    uint8_t *_buffer;
	    _buffer = (uint8_t *) buffer;
	    _left = static_cast<int>(bufferSize);
	    while (true)
	      {
		  if ((_received = read (fd, _buffer, _left)) <= 0)
		    {
			if (_received < 0)
			  {
			      if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
				{
				    continue;
				}
			      else
				{
				    return -1;
				}
			  }
			else
			  {
			      return 0;
			  }
		    }
		  _left -= _received;
		  if (_left == 0)
		      break;
		  _buffer += _received;
	      }
	    return static_cast<ssize_t>(bufferSize - _left);
	}

	ssize_t writeBuffer (int fd, void *buffer, size_t bufferSize)
	{
	    ssize_t _left;
	    ssize_t _sent;
	    const uint8_t *_buffer = (const uint8_t *) buffer;
	    _left = static_cast<int> (bufferSize);
	    while (true)
	      {
		  if ((_sent = write (fd, _buffer, _left)) <= 0)
		    {
			if (_sent < 0)
			  {
			      if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
				{
				    continue;
				}
			      else
				{
				    return -1;
				}
			  }
			else
			  {
			      break;
			  }
		    }
		  _left -= _sent;
		  if (_left == 0)
		      break;
		  _buffer += _sent;
	      }
	    return static_cast<ssize_t> (bufferSize - _left);
	}
    }
};
