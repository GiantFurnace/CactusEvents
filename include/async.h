/*
 * cactus events native API header
 *
 * Copyright (c) 2018-2028 chenzhengqiang 642346572@qq.com 
 * All rights reserved since 2018-10-04
 *
 * Redistribution and use in source and binary forms, with or without modifica-
 * tion, are permitted provided that the following conditions are met:
 *
 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTH-
 * ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU General Public License ("GPL") version 2 or any later version,
 * in which case the provisions of the GPL are applicable instead of
 * the above. If you wish to allow the use of your version of this file
 * only under the terms of the GPL and not to allow others to use your
 * version of this file under the BSD license, indicate your decision
 * by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL. If you do not delete the
 * provisions above, a recipient may use your version of this file under
 * either the BSD or the GPL.
 */

#ifndef _BACKER_CACTUS_ASYNC_H_
#define  _BACKER_CACTUS_ASYNC_H_

#include "sys_common.h"
#include "net_common.h"
#include "event.h"
#include "types.h"
#include "utils.h"
#include "eventspool.h"
#include <map>
#include <utility>
#include <stdexcept>
#include <string.h>
#include <errno.h>
#include <pthread.h>
extern int errno;


namespace cactus
{


    class Async:virtual public Event
    {
      public:

	Async ():pending_ (true)
	{
	    _initialize ();
	}

	Async (EventsPool & pool)
	{
	    _initialize ();
	    pool.add (this);
	    pooltid_ = pool.gettid ();
	}


	struct AsyncData
	{
	    int fd;
	};


	/*
	   @desc:check if the socket pipe is pending for write
	 */
	inline bool pending () const
	{
	    return pending_;
	}

	/*
	 * @parameters:
	 * pool: instance of events pool which was defined in file eventspool.h
	 * @return:void
	 * @desc: join the events pool, the events pool monitor all of the events before startup the event loop
	 */
	inline void join (EventsPool & pool)
	{
	    pool.add (this);
	    pooltid_ = pool.gettid ();
	}



	void send (void *buffer, size_t bufferSize)
	{
	    tid_ = pthread_self ();
	    if (!pthread_equal (tid_, pooltid_))
	      {
		  Event::_lock ();
	      }

	    if (!pending_)
	      {
		  int ret = utils::net::writeBuffer (sockfds_[1], buffer, bufferSize);
		  pending_ = true;
	      }

	    if (!pthread_equal (tid_, pooltid_))
	      {
		  Event::_unlock ();
	      }
	}



      private:
	Async (const Async &)
	{;
	}
	Async & operator = (const Async &)
	{;
	}
	virtual std::map < int, int >_getifds () const
	{
	    return ifds_;
	}
	virtual std::map < int, int >_getofds () const
	{
	    return ofds_;
	}

	inline void _initialize ()
	{
	    tid_ = pthread_self ();
	    pooltid_ = pthread_self ();
	    int ret = socketpair (AF_UNIX, SOCK_STREAM, 0, sockfds_);
	    if (ret < 0)
	      {
		  throw std::runtime_error (strerror (errno));
	      }

	    ifds_.insert (std::make_pair (sockfds_[0], types::events::ASYNC));
	    ofds_.insert (std::make_pair (sockfds_[1], types::events::ASYNC));

	}



	/*
	   @desc::execute the callback function registered on file descriptior when io event has been triggered
	 */
	virtual void _execute (int fd, types::events::Events event,
			       pthread_t pooltid) throw ()
	{

	    pooltid_ = pooltid;
	    switch (event)
	      {
	      case types::events::READ:
		  if (!pthread_equal (tid_, pooltid))
		    {
			Event::_lock ();
		    }

		  if (!pthread_equal (tid_, pooltid))
		    {
			Event::_unlock ();
		    }
		  break;
	      case types::events::WRITE:
		  if (!pthread_equal (tid_, pooltid))
		    {
			Event::_lock ();
		    }

		  pending_ = false;

		  if (!pthread_equal (tid_, pooltid))
		    {
			Event::_unlock ();
		    }
		  break;
	      }

	}

      private:
	std::map < int, int >ifds_;
	std::map < int, int >ofds_;
	bool pending_;
	int sockfds_[2];
	pthread_t tid_;
	pthread_t pooltid_;
    };

}

#endif
