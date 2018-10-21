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

#ifndef _BACKER_CACTUS_TIMER_H_
#define  _BACKER_CACTUS_TIMER_H_

#include "sys_common.h"
#include "net_common.h"
#include "event.h"
#include "types.h"
#include "utils.h"
#include "eventspool.h"
#include <map>
#include <utility>
#include <pthread.h>
#include <iostream>

namespace cactus
{
    template < typename K >
    class Timer:virtual public Event
    {
      public:
	/*
	* @desc:define the class member function as callback
	* you can find detailed description about eventson in evnet.h
	*/
	typedef void (K::*ClassMethodCallback) (const EventSon &);

	Timer () {; }
	Timer (EventsPool & pool) { pool.add (this, types::events::TIMER); }

	/*
	 * @parameters:
	 * client: function object of template class 
	 * @return:void
	 * @desc:set the class's function object as callback for async read event 
	 */
	inline void set (size_t took, K * client) throw ()
	{
	    ifds_.insert (std::make_pair (took, types::events::TIMER));
	    ikcbs_.erase (took);
	    iccbs_.erase (took);
	    iccbs_.insert (std::make_pair (took, client));
	}

	/*
	 * @parameters:
	 * client: function object of template class
	 * @return:void
	 * @desc:set the class's member function as callback for async read event
	 */
	inline void set (size_t took, K * client, ClassMethodCallback cb) throw ()
	{
	    ifds_.insert (std::make_pair (took, types::events::TIMER));
	    iccbs_.erase (took);
	    ikcbs_.erase (took);
	    ikcbs_.insert (std::make_pair (took, cb));
	    client_ = client;
	}

	/*
	 * @parameters:
	 * pool: instance of events pool which was defined in file eventspool.h
	 * @return:void
	 * @desc: join the events pool, the events pool monitor all of the events before startup the event loop
	 */
	inline void join (EventsPool & pool)
	{
	    pool.add (this, types::events::TIMER);
	}

    private:
	Timer (const Timer &)
	{;
	}

	Timer & operator = (const Timer &)
	{;
	}

	virtual std::map < size_t, size_t >_getifds () const
	{
	    return ifds_;
	}
	
	/*
	 * @desc::execute the callback function registered on file descriptior when io event has been triggered
	 */
	virtual void _execute (const EventSon &son) throw ()
	{
	    if ( son.type == types::events::READ )
	    {
		if (iccbs_.size () > 0)
		{
		    typename std::map < size_t, K * >::iterator iter = iccbs_.begin();
		    if (iter != iccbs_.end ())
		    {
			(*(iter->second)) (son);
			++iter;
		    }
		}
		if (ikcbs_.size () > 0)
		{
		    typename std::map < size_t, ClassMethodCallback >::iterator iter = ikcbs_.begin();
		    if (iter != ikcbs_.end ())
		    {
		        (client_->*(iter->second)) (son);
			++iter;
		    }
		}
	    }
	}

    private:
	std::map < size_t, size_t > ifds_;
	std::map < size_t, K * >iccbs_;
	std::map < size_t, ClassMethodCallback > ikcbs_;
	K * client_;
    };
}

#endif
