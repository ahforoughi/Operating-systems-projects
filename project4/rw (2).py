from threading import Lock, Thread, current_thread
from typing import Callable
from queue import Queue

class FairLock:
    _q: Queue = None
    def __init__(self):
        self._q = Queue()
    
    def acquire(self) -> None:
        l = Lock()
        l.acquire()
        self._q.put(l)
    
    def release(self):
        l = self._q.get()
        l.release()

v = 0  # shared resource
f = FairLock()  # service access lock (queued)
x = Lock()  # exculsive lock (resouce access)
r = Lock()  # shared lock (reader)
rc = 0  # running readers thread count

def write() -> None:
    global v

    f.acquire()
    with x:
        f.release()
        # critical section
        v = current_thread().ident
        print("writed", v)


def read() -> None:
    global rc

    f.acquire()
    with r:
        rc += 1
        if rc == 1:
            x.acquire()
        f.release()

    print("read", v) # critical section

    with r:
        rc -= 1
        if rc == 0:
            x.release()


def run_concurrent(f: Callable, *args, **kwargs) -> Thread:
    # # for random sleep uncomment this code
    # def w():        
    #     import random
    #     import time
    #     time.sleep(random.randint(0,500)/1000)
    #     f(*args, **kwargs)
    # th = Thread(target=w)

    th = Thread(target=f, args=args, kwargs=kwargs)
    th.start()
    return th


for _ in range(2):
    run_concurrent(write)

for _ in range(4):
    run_concurrent(read)
