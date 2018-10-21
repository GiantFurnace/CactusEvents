/*
 * cactus events native API header
 *
 * Copyright (c) 2018-2028 chenzhengqiang 642346572@qq.com 
 * All rights reserved since 2018-10-04
 *
 * RedistributIOn and use in source and binary forms, with or without modifica-
 * tIOn, are permitted provided that the following conditIOns are met:
 *
 *   1.  RedistributIOns of source code must retain the above copyright notice,
 *       this list of conditIOns and the following disclaimer.
 *
 *   2.  RedistributIOns in binary form must reproduce the above copyright
 *       notice, this list of conditIOns and the following disclaimer in the
 *       documentatIOn and/or other materials provided with the distributIOn.
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
 * the GNU General Public License ("GPL") versIOn 2 or any later versIOn,
 * in which case the provisIOns of the GPL are applicable instead of
 * the above. If you wish to allow the use of your versIOn of this file
 * only under the terms of the GPL and not to allow others to use your
 * versIOn of this file under the BSD license, indicate your decisIOn
 * by deleting the provisIOns above and replace them with the notice
 * and other provisIOns required by the GPL. If you do not delete the
 * provisIOns above, a recipient may use your versIOn of this file under
 * either the BSD or the GPL.
 */

#ifndef _BACKER_CACTUS_IO_H_
#define  _BACKER_CACTUS_IO_H_

#include "event.h"
#include "types.h"
#include "eventspool.h"
#include <map>
#include <utility>

namespace cactus
{

    template < typename K > class IO:virtual public Event
    {
      
	public:
	/*
	* @note:common function callback differs from c++ class's member functIOn
	*/
	typedef void (*CommonCallback) (const EventSon & son);
	
        /*
	* @desc:define the class member function as callback
	* you can find detailed description about eventson in evnet.h
	*/
	typedef void (K::*ClassMethodCallback) (const EventSon & son);

	IO (){;}
	IO (EventsPool & pool) { pool.add (this); }

	/*
	 * @parameters:
	 * fd:file descriptor
	 * client: function object of template class 
	 * @return:void
	 * @desc:set the class's function object as callback with specify fd and event( read or write )
	 */
	inline void set (size_t fd, types::events::Events event, K * client) throw ()
	{
	    if (event == types::events::READ)
	    {
		ikcbs_.erase (fd);
		iccbs_.erase (fd);
		iccbs_.insert (std::make_pair (fd, client));
		ifds_.insert (std::make_pair (fd, types::events::IO));
	    }
	    else if (event == types::events::WRITE)
	    {
		okcbs_.erase (fd);
		occbs_.erase (fd);
		occbs_.insert (std::make_pair (fd, client));
		ofds_.insert (std::make_pair (fd, types::events::IO));
	    }
	}

	/*
	 * @parameters:
	 * fd:file descriptor
	 * event:types is a namespace , event specify IO event of read or write
	 * client: function object of template class
	 * @return:void
	 * @desc:set the class's member function as callback with specify fd and event
	 */
	inline void set (size_t fd, types::events::Events event, K * client, ClassMethodCallback cb) throw ()
	{
	    client_ = client;
	    if (event == types::events::READ)
	    {
		iccbs_.erase (fd);
		ikcbs_.erase (fd);
		ikcbs_.insert (std::make_pair (fd, cb));
		ifds_.insert (std::make_pair (fd, types::events::IO));
	    }
	    else if (event == types::events::WRITE)
	    {
		occbs_.erase (fd);
		okcbs_.erase (fd);
		okcbs_.insert (std::make_pair (fd, cb));
		ofds_.insert (std::make_pair (fd, types::events::IO));
	    }
	}

	/*
	 * @parameters:
	 * pool: instance of events pool which was defined in file eventspool.h
	 * @return:void
	 * @desc: join the events pool, the events pool monitor all of the events before startup the event loop
	 */
	void join (EventsPool & pool)
	{
	    pool.add (this);
	}
	
    private:
	IO (const IO &) {;}
	IO & operator = (const IO &) {;}

	/*
	 * @return:map<k,v> k represent the file descriptor, v represent the event object
	 * @desc: get all of the registered input file descriptor 
	 */
	virtual std::map < size_t, size_t >_getifds () const
	{
	    return ifds_;
	}

	/*
	 * @return:map<k,v> k represent the file descriptor, v represent the event object
	 * @desc: get all of the registered output file descriptor
	 */
	virtual std::map < size_t, size_t >_getofds () const
	{
	    return ofds_;
	}

	/*
	 * @desc::execute the callback function registered on file descriptior when IO event has been triggered
	 */
	virtual void _execute (const EventSon & son) throw ()
	{
	    size_t fd = son.fd;
	    if (son.type == types::events::READ)
	    {
		if (iccbs_.size () > 0)
		{
		    typename std::map < size_t, K * >::iterator iter = iccbs_.find (fd);
		    if (iter != iccbs_.end ())
		    {
			(*(iter->second)) (son);
		    }
		}
		if (ikcbs_.size () > 0)
		{
		    typename std::map < size_t, ClassMethodCallback >::iterator iter = ikcbs_.find (fd);
		    if (iter != ikcbs_.end ())
		    {
			(client_->*(iter->second)) (son);
		    }
		}
	      }
	      else
	      {
		    if (occbs_.size () > 0)
		    {
			typename std::map < size_t, K * >::iterator iter = occbs_.find (fd);
			if (iter != occbs_.end ())
			{
			    (*(iter->second)) (son);
			}
		    }
		    if (okcbs_.size () > 0)
		    {
			typename std::map < size_t, ClassMethodCallback >::iterator iter = okcbs_.find (fd);
			if (iter != okcbs_.end ())
			{
			    (client_->*(iter->second)) (son);
			}
		    }
	      }
	}

      private:

	/*
	   @note:
	   '''
	   c means common
	   i  means input, 
	   o means output 
	   k  means class
	   cb means callback
	   '''
	 */

	std::map < size_t, size_t >ifds_;
	std::map < size_t, size_t >ofds_;
	std::map < size_t, K * >iccbs_;
	std::map < size_t, K * >occbs_;
	std::map < size_t, ClassMethodCallback > ikcbs_;
	std::map < size_t, ClassMethodCallback > okcbs_;
	K *client_;

    };

}

#endif
