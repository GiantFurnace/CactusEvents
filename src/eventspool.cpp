/*
 * events pool api implementations
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



#include "eventspool.h"
#include "event.h"
#include "types.h"
#include "utils.h"
#include <utility>
#include <stdexcept>
#include <sys/epoll.h>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <errno.h>
extern int errno;


namespace cactus
{
    /*
    * @desc:the global SOCKFDS_TIMER created by socketpair is used to transform signal event to
    * io event
    */
    int SOCKFDS_TIMER[2];

    EventsPool::EventsPool () throw ():epollfd_ (-1), status_ (0),
	event_trigger_ (EPOLLET), timer_(false), prevSize_(0), curSize_(0),timerTook_(0)
    {
	epollfd_ = epoll_create (1);
	tid_ = pthread_self ();
    }

    EventsPool::~EventsPool () throw ()
    {
	_clear ();
    }

    bool EventsPool::_prepare () throw ()
    {
	bool ok = true;
	curSize_ = observers_.size();
	size_t count = 0;
	std::vector <Item>::reverse_iterator oiter = observers_.rbegin ();		
	while (oiter != observers_.rend () && ( curSize_- prevSize_) > 0 )
	{
	    if ( (*oiter).object == types::events::TIMER )
	    {
		if ( timer_ )
		{
		    ++oiter;
		    continue;
	        }
				
		int ret = socketpair (AF_UNIX, SOCK_STREAM, 0, SOCKFDS_TIMER);
		if (ret < 0)
		{
		    return false;
		}

		std::map < int, size_t >ifds_timer = ((*oiter).observer)->_getifds ();
		if ( ifds_timer.size() > 0 )
		{
					
		    timerTook_ = (ifds_timer.begin())->first;
		    if ( timerTook_ > 0 )
		    {
			timer_ = true;
			_register(SOCKFDS_TIMER[0],  types::events::READ);
			_register(SOCKFDS_TIMER[1],  types::events::WRITE);
			ifds_.insert (std::make_pair (SOCKFDS_TIMER[0], types::events::TIMER));
			iobservers_.insert (std::make_pair (SOCKFDS_TIMER[0],  ((*oiter).observer)));
		    }
					
		}
				
		++oiter;
		continue;
		}

		std::map < int, size_t >ifds = ((*oiter).observer)->_getifds ();
		std::map < int, size_t >::iterator ifd = ifds.begin ();

		while (ifd != ifds.end ())
		{
		    if (iobservers_.count (ifd->first) > 0)
		    {
			++ifd;
			continue;
		    }
		    else
		    {
		        ifds_.insert (std::make_pair (ifd->first, ifd->second));
			iobservers_.insert (std::make_pair (ifd->first, (*oiter).observer));
		    }

		    _register (ifd->first, types::events::READ);
		    ++ifd;
		}

	      std::map < int, size_t >ofds =  ((*oiter).observer)->_getofds ();
	      std::map < int, size_t >::iterator ofd = ofds.begin ();

	      while (ofd != ofds.end ())
	      {
		   if (oobservers_.count (ofd->first) > 0)
		    {
			++ofd;
			continue;
		    }
		    else
		    {
			ofds_.insert (std::make_pair (ofd->first, ofd->second));
			oobservers_.insert (std::make_pair (ofd->first,  ((*oiter).observer)));
		    }
		    _register (ofd->first, types::events::WRITE);
		    ++ofd;
	      }
		
	      ++oiter;
	      ++count;
	      if ( count == (curSize_ - prevSize_) )
	      {
	          break;
	      }
	   }

	   prevSize_ = curSize_;
	   if ( timer_ )
	   {
	        ok = _initialize_timer(timerTook_, false);
	   }
		
	   return ok;
    }

    void EventsPool::_timer_entry( int signo )
    {
        switch( signo )
        {
            case SIGALRM:
	        char val = 0x01;
	        utils::net::writeBuffer(SOCKFDS_TIMER[1], &val, sizeof(val));
	        break;
        }
    }

    bool EventsPool::_initialize_timer(size_t took, bool once) throw()
    {
        struct sigaction sa;
        memset(&sa, '\0', sizeof(sa));
        sa.sa_handler =  _timer_entry;
        sa.sa_flags |= SA_RESTART;
        sigfillset( &sa.sa_mask);
	if ( sigaction(SIGALRM, &sa, 0) < 0 || sigaction(SIGTERM, &sa, 0) < 0)
	{
	    return false;
	}
	    
	struct itimerval newval, oldval;
	newval.it_value.tv_usec = 0;
	if ( took / 1000 > 0 )
	{
	    newval.it_value.tv_sec = took / 1000;
	    newval.it_value.tv_usec = (took % 1000) * 1000;
	}
	else
	{
	    newval.it_value.tv_sec = 1;
	}

	newval.it_interval.tv_sec = 0;
	newval.it_interval.tv_usec = 0;

	if ( ! once )
	{
	    newval.it_interval.tv_sec = took / 1000;
	    newval.it_interval.tv_usec = (took % 1000) * 1000;
	}

	if (setitimer( ITIMER_REAL, &newval, &oldval ) < 0)
	{
	    return false;
	}
	timer_ = false;
	return true;
    }

    inline void EventsPool::_register (int fd, types::events::Events event) throw ()
    {
	utils::sys::setnonblock (fd);
	epoll_event event_;
	event_.data.fd = fd;
	if (event == types::events::READ)
	{
	    event_.events = event_trigger_ | EPOLLIN;
	}
	else
	{
	    event_.events = event_trigger_ | EPOLLOUT;
	}
	epoll_ctl (epollfd_, EPOLL_CTL_ADD, fd, &event_);
     }


     void EventsPool::kill (const EventSon & son) throw ()
     {
	int fd = son.fd;
	if ( son.type == types::events::READ )
	{
	    epoll_event event_;
	    event_.data.fd = fd;
	    event_.events = event_trigger_ | EPOLLIN;
	    epoll_ctl (epollfd_, EPOLL_CTL_DEL, son.fd, &event_);

	    ifds_.erase(fd);
	    iobservers_.erase(fd);
	    if ( oobservers_.count(fd) <=0 )
	    {
		close(son.fd);
	    }

	    if ( son.object == types::events::TIMER )
	    {
		timer_ = false;
		struct itimerval val;  
		val.it_value.tv_sec = 0;  
		val.it_value.tv_usec = 0;  
		val.it_interval = val.it_value;  
		setitimer(ITIMER_REAL, &val, 0);

		event_.data.fd = SOCKFDS_TIMER[1];
		event_.events = event_trigger_ | EPOLLOUT;
		epoll_ctl (epollfd_, EPOLL_CTL_DEL, SOCKFDS_TIMER[1], &event_);

	    }
	}
	else
	{
	    epoll_event event_;
	    event_.data.fd = fd;
	    event_.events = event_trigger_ | EPOLLOUT;
	    epoll_ctl (epollfd_, EPOLL_CTL_DEL, fd, &event_);

	    ofds_.erase(fd);
	    oobservers_.erase(fd);
	    if ( iobservers_.count(fd) <=0 )
	    {
		close(fd);
	    }
        }

    }

    void EventsPool::run (int status) throw ()
    {
        if (status !=0 )
        return;

	if ( !_prepare () )
	{
	    return;
	}

	tid_ = pthread_self ();

	if (epollfd_ == -1)
	{
	    return;
	}

	while (status_ == 0)
	{
	    int ret = epoll_wait (epollfd_, epoll_events_, MAX_EVENTS_SIZE, -1);
	    int events_size = ret;
	    if (events_size > 0)
	    {
		for (int index = 0; index < events_size; ++index)
		{
		    int fd = epoll_events_[index].data.fd;
		    EventSon son;
		    son.fd = fd;
		    son.tid = tid_;
		    son.pool = this;
		    son.object = types::events::ANY;

		    if (epoll_events_[index].events & EPOLLIN)
		    {
			std::map < int, size_t >::const_iterator iiter = ifds_.find (fd);
			if (iiter != ifds_.end ())
			{
			    if ( fd == SOCKFDS_TIMER[0] )
			    {
			        son.object = types::events::TIMER;
				char data;
				utils::net::readBuffer(fd, &data, sizeof(data));
			    }
			    std::map < int, cactus::Event *>::const_iterator iter = iobservers_.find (fd);
			    son.type = types::events::READ;
			    son._event = iter->second;
			    iter->second->_execute (son);
			}
		    }
		    else if (epoll_events_[index].events & EPOLLOUT)
		    {
			std::map < int, size_t >::const_iterator oiter = ofds_.find (fd);
			if (oiter != ofds_.end ())
			{
			    std::map < int, cactus::Event *>::const_iterator iter = oobservers_.find (fd);
			    son.type = types::events::WRITE;
			    son._event = iter->second;
			    iter->second->_execute (son);
			}
		    }
		    else if (epoll_events_[index].events & EPOLLERR || epoll_events_[index].events & EPOLLHUP)
		    {
			_remove (fd);
		    }
		    else
		    {
			;
		    }
		}
	    }
	    else if (errno == EINTR)
	    {
	        continue;
	    }
	    else
	    {
	        status_ = -1;
	    }
	    if ( ! _prepare() )
	    {
	        status_ = -1;
	    }
	}	
    }

    void EventsPool::_remove (int fd) throw ()
    {
	close (fd);
	ifds_.erase (fd);
	ofds_.erase (fd);
	iobservers_.erase (fd);
	oobservers_.erase (fd);
	epoll_ctl (epollfd_, EPOLL_CTL_DEL, fd, 0);
    }

    void EventsPool::_clear () throw ()
    {
	std::map < int, cactus::Event * >::iterator iter = iobservers_.begin ();
	while (iter != iobservers_.end ())
	{
	    int fd = iter->first;
	    close (fd);
	    iobservers_.erase (fd);
	    epoll_ctl (epollfd_, EPOLL_CTL_DEL, fd, 0);
	    ++iter;
	}

	iter = oobservers_.begin ();
	while (iter != oobservers_.end ())
	{
	    int fd = iter->first;
	    close (fd);
	    oobservers_.erase (fd);
	    epoll_ctl (epollfd_, EPOLL_CTL_DEL, fd, 0);
	    ++iter;
	}

	ifds_.clear ();
	ofds_.clear ();
	iobservers_.clear ();
	oobservers_.clear ();
    }

}
