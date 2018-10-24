# Cactus:World Fades Away,But You Stay
---
## 1. Abstract
What is Cactus?<br/>
Firstly you must have a know about the select and epoll model, which are known as io MUX interface in unix.<br/> 
Cactus is actually a high performance event library based on epoll，which similar to [libev https://fossies.org/dox/libev-4.24/](https://fossies.org/dox/libev-4.24/),<br/>
but more lightweight.Also you can find another similary library, the classical [libevent http://libevent.org/](http://libevent.org/).<br/>

## 2. Why I write Cactus
Cactus is a pure c++ library, but more lightweight as said.<br/> 
The main reason is that I used to write code with c++, so I make up my mind to make a difference.<br/>
Make a difference in this world, to be a better man!<br/> 
<br/>

## 3. Is it necessary to use smart pointer?
The answer is no!<br/>
The client is more responsible for memory's management instead of library!<br/>

## 4. Detailed Sample Code
-----------
``` 
#include "utils.h"
#include "eventspool.h"
#include "async.h"
#include "timer.h"
#include <iostream>

/*
@note:
1.all file descriptors are nonblocking, you can call utils::net::readBuffer or 
  utils::net::writeBuffer to handle read or write
2.the main event object are cactus:IO,cactus:Async,cactus:Timer, all are template class 
   defined in namespace cactus
*/

namespace cactus
{
    // defines callback class for callback in namespace cactus
    class Callback
    {
    public:
        Callback():count_(0) {;}
        /* 
        * cactus supports functor as callback 
        * EventSon is a class defined in namespace cactus
        */
        void operator  () (const cactus::EventSon & son)
        {
            int data;
            utils::net::readBuffer (son.fd, &data, sizeof (data));
            // std::cout<<"functor callback has been triggered"<<std::endl;
        }
	// cactus also supports member function as callback 
       void asyncReadCallback (const cactus::EventSon & son)
       {
           int data;
           utils::net::readBuffer (son.fd, &data, sizeof (data));
           // std::cout<<"member callback has been triggered"<<std::endl;
       }

       void newTimerCallback (const cactus::EventSon & son)
       {
           (void) son;
           std::cout << "this is a new timer " << std::endl;
       }

       void oldTimerCallback (const cactus::EventSon & son)
       {
           ++count_;
           std::cout << "this is a timer " << std::endl;
           if (count_ > 10)
           {
	       (son.pool)->kill (son);
	       timer_.set (2000, this, &Callback::newTimerCallback);
	       timer_.join (*(son.pool));
           }
       }

    private:
       cactus::Timer < Callback > timer_;
       int count_;
    };
}

/*
@note:Async is a event object that sent message to another event loop
*/
cactus::Async < Callback > async;
void * async_entry (void *arg)
{
  (void) arg;
  int data = 201314;
  while (true)
    {
      if (!async.pending ())
	{
            async.send (&data, sizeof (data));
	}
    }

  return 0;
}

int main (int argc, char **argv)
{
  (void) argc;
  (void) argv;

  pthread_t tid;
  pthread_create (&tid, 0, async_entry, 0);
  cactus::Callback callback;
  cactus::EventsPool eventPool;

  async.set (&callback);
  async.set (&callback, &cactus::Callback::asyncReadCallback);
  async.join (eventPool);

  cactus::Timer < cactus::Callback > timer;
  timer.set (800, &callback, &cactus::Callback::oldTimerCallback);
  timer.join (eventPool);
  eventPool.run (0);
  return 0;
}
  
```

## 5. Contact

|Author          | Email            | Wechat      |
| ---------------|:----------------:| -----------:|
| chenzhengqiang | 642346572@qq.com | 18819373904 |

**Notice:  Any comments and suggestions are welcomed**

## 6. License
[Apache License 2.0](./LICENSE)
