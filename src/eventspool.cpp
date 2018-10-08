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
#include <sys/epoll.h>
#include <errno.h>
extern int errno;


namespace cactus
{


    EventsPool::EventsPool () throw ():epollfd_ (-1), status_ (0),
	event_trigger_ (EPOLLET)
    {
	epollfd_ = epoll_create (1);
	tid_ = pthread_self ();

    }


    EventsPool::~EventsPool () throw ()
    {
	_clear ();
    }



    void EventsPool::_initialize () throw ()
    {
	std::vector < cactus::Event * >::reverse_iterator oiter = observers_.rbegin ();
	while (oiter != observers_.rend ())
	  {

	      std::map < int, int >ifds = (*oiter)->_getifds ();
	      std::map < int, int >::iterator ifd = ifds.begin ();

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
			  iobservers_.insert (std::make_pair (ifd->first, *oiter));
		      }

		    _register (ifd->first, types::events::READ);
		    ++ifd;
		}

	      std::map < int, int >ofds = (*oiter)->_getofds ();
	      std::map < int, int >::iterator ofd = ofds.begin ();

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
			  oobservers_.insert (std::make_pair (ofd->first, *oiter));
		      }

		    _register (ofd->first, types::events::WRITE);
		    ++ofd;
		}

	      ++oiter;
	  }
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



    void EventsPool::run (int status) throw ()
    {
	_initialize ();
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
			  if (epoll_events_[index].events & EPOLLIN)
			    {
				std::map < int, int >::const_iterator iiter = ifds_.find (fd);
				if (iiter != ifds_.end ())
				  {
				      std::map < int,cactus::Event *>::const_iterator iter = iobservers_.find (fd);
				      iter->second->_execute (fd,types::events::READ, tid_);
				  }
			    }
			  else if (epoll_events_[index].events & EPOLLOUT)
			    {
				std::map < int, int >::const_iterator oiter = ofds_.find (fd);
				if (oiter != ofds_.end ())
				  {
				      std::map < int,cactus::Event *>::const_iterator iter = oobservers_.find (fd);
				      iter->second->_execute (fd, types::events::WRITE, tid_);
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
	std::map < int, cactus::Event * >itmp;
	iobservers_.swap (itmp);
	iobservers_.clear ();

	std::map < int, cactus::Event * >otmp;
	oobservers_.swap (otmp);
	oobservers_.clear ();

    }

}
