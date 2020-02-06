from threading import Lock, Thread, current_thread
from typing import Callable
from queue import Queue


def write():
    global vertices

    func.acquire()
    with x:
        func.release()

        print("write", current_thread().ident)


def read():
    global r_count

    func.acquire()
    with r:
        r_count += 1
        if r_count == 1:
            x.acquire()
        func.release()

    print("read", current_thread().ident)

    with r:
        r_count -= 1
        if r_count == 0:
            x.release()


class rwLock:
    Queue = None

    def __init__(self):
        self.q = Queue()

    def acquire(self):
        lck = Lock()
        lck.acquire()
        self.q.put(lck)

    def release(self):
        lck = self.q.get()
        lck.release()


def run(f: Callable, *args, **kwargs):
    th = Thread(target=f, args=args, kwargs=kwargs)
    th.start()
    return th


if __name__ == '__main__':
    vertices = 0
    func = rwLock()
    x = Lock()
    r = Lock()
    r_count = 0
    for i in range(4):
        run(read)


    for i in range(2):
        run(write)

