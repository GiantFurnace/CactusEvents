/*
 * cactus sample code
 *
 * Copyright (c) 2018-2028 chenzhengqiang 642346572@qq.com 
 * All rights reserved since 2018-10-04
 *
 * Redistribution and use in source and binary forms, with or without modifica-
 * tIOn, are permitted provided that the following conditions are met:
 *
 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *   2.  RedistributIOns in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
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

#include "utils.h"
#include "eventspool.h"
#include "async.h"
#include "timer.h"
#include <iostream>

int count = 0;

class Callback
{
public:

  void operator  () (const cactus::EventSon & son)
  {
    int data;
    int bytes = utils::net::readBuffer (son.fd, &data, sizeof (data));
    // std::cout<<"functor callback has been triggered"<<std::endl;
  }

  void asyncReadCallback (const cactus::EventSon & son)
  {
    int data;
    int bytes = utils::net::readBuffer (son.fd, &data, sizeof (data));
    //std::cout<<"member callback has been triggered"<<std::endl;
  }

  void newTimerCallback (const cactus::EventSon & son)
  {
    std::cout << "this is a new timer " << std::endl;
  }

  void oldTimerCallback (const cactus::EventSon & son)
  {
    ++count;
    std::cout << "this is a timer " << std::endl;
    if (count > 10)
      {
	(son.pool)->kill (son);
	timer.set (2000, this, &Callback::newTimerCallback);
	timer.join (*(son.pool));
      }
  }

private:
  cactus::Timer < Callback > timer;

};

cactus::Async < Callback > async;

void * async_entry (void *arg)
{
  int data = 201314;
  while (true)
    {
      if (!async.pending ())
	{
	  int bytes = async.send (&data, sizeof (data));
	}
    }
}


int main (int argc, char **argv)
{
  pthread_t tid;
  pthread_create (&tid, 0, async_entry, 0);
  Callback callback;
  cactus::EventsPool eventPool;

  async.set (&callback);
  async.set (&callback, &Callback::asyncReadCallback);
  async.join (eventPool);

  cactus::Timer < Callback > timer;
  timer.set (800, &callback, &Callback::oldTimerCallback);
  timer.join (eventPool);
  timer.join (eventPool);
  timer.join (eventPool);
  timer.join (eventPool);
  timer.join (eventPool);
  eventPool.run (0);
}
