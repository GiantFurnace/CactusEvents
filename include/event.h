/*
 * event API header
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

#ifndef _BACKER_CACTUS_EVENT_H_
#define  _BACKER_CACTUS_EVENT_H_

#include "types.h"
#include "eventspool.h"
#include <map>
#include <pthread.h>

namespace cactus
{
    class EventSon;
    using cactus::EventsPool;
    /*
     * Event class is the base class of async, io, timer
     */
    class Event
    {
	friend class EventsPool;
    protected:
	Event () { pthread_mutex_init (&mutex_, 0); }
	~Event () {  pthread_mutex_destroy (&mutex_); }
	inline void _lock () { pthread_mutex_lock (&mutex_); }
	inline void _unlock () { pthread_mutex_unlock (&mutex_); }
    private:
	Event (const Event &){;}
	Event & operator = (const Event &){ return *this; }
    private:
	virtual std::map < int, size_t >_getifds () const { return std::map < int, size_t >(); }
	virtual std::map < int, size_t >_getofds () const { return std::map < int, size_t >(); }
	virtual void _execute ( const EventSon &){;}
    private:
	pthread_mutex_t mutex_;
    };

    /*
    * @desc:event son is defined as the callback function's parameter,which store the file descriptor and 
      the events type such as read or write, the event object such as IO, TIMER.
    */
    class EventSon
    {
        friend class EventsPool;
    public:
        int fd;
        types::events::Events  type;
        types::events::Objects object;
        pthread_t tid;
        EventsPool * pool;
        int error;
    private:
        Event * _event;
    };
}

#endif
