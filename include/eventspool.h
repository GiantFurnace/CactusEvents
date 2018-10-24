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



#ifndef _BACKER_CACTUS_EVENTSPOOL_H_
#define  _BACKER_CACTUS_EVENTSPOOL_H_

#include "sys_common.h"
#include "types.h"
#include <map>
#include <vector>
#include <utility>
#include <sys/epoll.h>

namespace cactus
{
    class Event;
    class EventSon;

    class EventsPool
    {
	friend class Event;
     public:
	enum
	{
	    MAX_EVENTS_SIZE = 65535
	};

	struct Item
	{
	    cactus::Event * observer;
	    types::events::Objects object;
	};

	 EventsPool () throw ();
	~EventsPool () throw ();

	inline void add (cactus::Event * observer, types::events::Objects object=types::events::ANY) throw ()
	{
	    if ( events_.count(observer) > 0 )
	    {
		return;
	    }
            events_.insert( std::make_pair( observer, object));
	    Item item;
	    item.observer = observer;
	    item.object = object;
	    observers_.push_back(item);
	}

	inline pthread_t gettid () const { return tid_; }
	void run (int status = 0) throw ();
	void kill( const EventSon & son) throw();
	   
      private:
	EventsPool (const EventsPool &) {;}
	EventsPool & operator = (const EventsPool &) { return *this; }
	bool _prepare () throw ();
	inline void _register (int fd, types::events::Events event) throw ();
	inline void _remove (int fd) throw ();
	void _clear () throw ();
	void _notify () throw ();
	bool _initialize_timer(size_t took, bool once=false) throw();
	static void _timer_entry(int);
	    
      private:
	    
	std::vector <Item> observers_;
	std::map<cactus::Event *, types::events::Objects> events_;
	std::map < int, size_t >ifds_;
	std::map < int, size_t >ofds_;
	std::map < int, cactus::Event * >iobservers_;
	std::map < int, cactus::Event * >oobservers_;
	int epollfd_;
	epoll_event epoll_events_[MAX_EVENTS_SIZE];
	int status_;
	int event_trigger_;
	pthread_t tid_;
	bool timer_;
	size_t prevSize_;
	size_t curSize_;
	size_t timerTook_;
    };

}

#endif
