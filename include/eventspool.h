/*
 * events pool API header
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



#ifndef _BACKER_CACTUS_EVENTLOOP_H_
#define  _BACKER_CACTUS_EVENTLOOP_H_


#include "event.h"
#include <vector>
#include <map>
#include <sys/epoll.h>
#include <pthread.h>



namespace cactus
{

    class EventsPool
    {
      public:
	enum
	{
	    MAX_EVENTS_SIZE = 65535
	};

	  EventsPool () throw ();
	 ~EventsPool () throw ();

	inline void add (cactus::Event * observer) throw ()
	{
	    observers_.push_back (observer);
	}
	inline pthread_t gettid () const
	{
	    return tid_;
	}
	void run (int status = 0) throw ();


      private:

	EventsPool (const EventsPool &)
	{;
	}
	EventsPool & operator = (const EventsPool &)
	{;
	}

	void _initialize () throw ();
	inline void _register (int fd, types::events::Events event) throw ();
	inline void _remove (int fd) throw ();

	void _clear () throw ();
	void _notify () throw ();

      private:

	std::vector < cactus::Event * >observers_;
	std::map < int, int >ifds_;
	std::map < int, int >ofds_;
	std::map < int, cactus::Event * >iobservers_;
	std::map < int, cactus::Event * >oobservers_;

	int epollfd_;
	epoll_event epoll_events_[MAX_EVENTS_SIZE];
	int status_;
	int event_trigger_;
	pthread_t tid_;


    };

}
#endif
