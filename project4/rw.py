import threading
import unittest
import threading
import time
import copy

class RWLock:
	
	def __init__(self):
		self.__read_switch = _LightSwitch()
		self.__write_switch = _LightSwitch()
		self.__no_readers = threading.Lock()
		self.__no_writers = threading.Lock()
		self.__readers_queue = threading.Lock()
		"""A lock giving an even higher priority to the writer in certain"""
		
	
	def reader_acquire(self):
		self.__readers_queue.acquire()
		self.__no_readers.acquire()
		self.__read_switch.acquire(self.__no_writers)
		self.__no_readers.release()
		self.__readers_queue.release()
	
	def reader_release(self):
		self.__read_switch.release(self.__no_writers)
	
	def writer_acquire(self):
		self.__write_switch.acquire(self.__no_readers)
		self.__no_writers.acquire()
	
	def writer_release(self):
		self.__no_writers.release()
		self.__write_switch.release(self.__no_readers)
	

class _LightSwitch:
	def __init__(self):
		self.__counter = 0
		self.__mutex = threading.Lock()
	
	def acquire(self, lock):
		self.__mutex.acquire()
		self.__counter += 1
		if self.__counter == 1:
			lock.acquire()
		self.__mutex.release()

	def release(self, lock):
		self.__mutex.acquire()
		self.__counter -= 1
		if self.__counter == 0:
			lock.release()
		self.__mutex.release()



class Writer(threading.Thread):
	def __init__(self, buffer_, rw_lock, init_sleep_time, sleep_time, to_write):
		
		threading.Thread.__init__(self)
		self.__buffer = buffer_
		self.__rw_lock = rw_lock
		self.__init_sleep_time = init_sleep_time
		self.__sleep_time = sleep_time
		self.__to_write = to_write
		self.entry_time = None
		"""Time of entry to the critical section"""
		self.exit_time = None
		"""Time of exit from the critical section"""
		
	def run(self):
		time.sleep(self.__init_sleep_time)
		self.__rw_lock.writer_acquire()
		self.entry_time = time.time()
		time.sleep(self.__sleep_time)
		self.__buffer.append(self.__to_write)
		self.exit_time = time.time()
		self.__rw_lock.writer_release()

class Reader(threading.Thread):
	def __init__(self, buffer_, rw_lock, init_sleep_time, sleep_time):

		threading.Thread.__init__(self)
		self.__buffer = buffer_
		self.__rw_lock = rw_lock
		self.__init_sleep_time = init_sleep_time
		self.__sleep_time = sleep_time
		self.buffer_read = None
		"""a copy of a the buffer read while in critical section"""	
		self.entry_time = None
		"""Time of entry to the critical section"""
		self.exit_time = None
		"""Time of exit from the critical section"""

	def run(self):
		time.sleep(self.__init_sleep_time)
		self.__rw_lock.reader_acquire()
		self.entry_time = time.time()
		time.sleep(self.__sleep_time)
		self.buffer_read = copy.deepcopy(self.__buffer)
		self.exit_time = time.time()
		self.__rw_lock.reader_release()

class RWLockTestCase(unittest.TestCase):
	def test_readers_nonexclusive_access(self):
		(buffer_, rw_lock, threads) = self.__init_variables()

		threads.append(Reader(buffer_, rw_lock, 0, 0))
		threads.append(Writer(buffer_, rw_lock, 0.2, 0.4, 1))
		threads.append(Reader(buffer_, rw_lock, 0.3, 0.3))
		threads.append(Reader(buffer_, rw_lock, 0.5, 0))
		
		self.__start_and_join_threads(threads)
		
		## The third reader should enter after the second one but it should
		## exit before the second one exits 
		## (i.e. the readers should be in the critical section 
		## at the same time)

		self.assertEqual([], threads[0].buffer_read)
		self.assertEqual([1], threads[2].buffer_read)
		self.assertEqual([1], threads[3].buffer_read)
		self.assert_(threads[1].exit_time <= threads[2].entry_time)
		self.assert_(threads[2].entry_time <= threads[3].entry_time)
		self.assert_(threads[3].exit_time < threads[2].exit_time)
	
	def test_writers_exclusive_access(self):
		(buffer_, rw_lock, threads) = self.__init_variables()

		threads.append(Writer(buffer_, rw_lock, 0, 0.4, 1))
		threads.append(Writer(buffer_, rw_lock, 0.1, 0, 2))
		threads.append(Reader(buffer_, rw_lock, 0.2, 0))
		
		self.__start_and_join_threads(threads)
		
		## The second writer should wait for the first one to exit

		self.assertEqual([1, 2], threads[2].buffer_read)
		self.assert_(threads[0].exit_time <= threads[1].entry_time)
		self.assert_(threads[1].exit_time <= threads[2].exit_time)
	
	def test_writer_priority(self):
		(buffer_, rw_lock, threads) = self.__init_variables()
		
		threads.append(Writer(buffer_, rw_lock, 0, 0, 1))
		threads.append(Reader(buffer_, rw_lock, 0.1, 0.4))
		threads.append(Writer(buffer_, rw_lock, 0.2, 0, 2))
		threads.append(Reader(buffer_, rw_lock, 0.3, 0))
		threads.append(Reader(buffer_, rw_lock, 0.3, 0))
		
		self.__start_and_join_threads(threads)
		
		## The second writer should go before the second and the third reader
		
		self.assertEqual([1], threads[1].buffer_read)
		self.assertEqual([1, 2], threads[3].buffer_read)
		self.assertEqual([1, 2], threads[4].buffer_read)		
		self.assert_(threads[0].exit_time < threads[1].entry_time)
		self.assert_(threads[1].exit_time <= threads[2].entry_time)
		self.assert_(threads[2].exit_time <= threads[3].entry_time)
		self.assert_(threads[2].exit_time <= threads[4].entry_time)
	
	def test_many_writers_priority(self):
		(buffer_, rw_lock, threads) = self.__init_variables()
		
		threads.append(Writer(buffer_, rw_lock, 0, 0, 1))
		threads.append(Reader(buffer_, rw_lock, 0.1, 0.6))
		threads.append(Writer(buffer_, rw_lock, 0.2, 0.1, 2))
		threads.append(Reader(buffer_, rw_lock, 0.3, 0))
		threads.append(Reader(buffer_, rw_lock, 0.4, 0))
		threads.append(Writer(buffer_, rw_lock, 0.5, 0.1, 3))
		
		self.__start_and_join_threads(threads)

		## The two last writers should go first -- after the first reader and
		## before the second and the third reader
		
		self.assertEqual([1], threads[1].buffer_read)
		self.assertEqual([1, 2, 3], threads[3].buffer_read)
		self.assertEqual([1, 2, 3], threads[4].buffer_read)		
		self.assert_(threads[0].exit_time < threads[1].entry_time)
		self.assert_(threads[1].exit_time <= threads[2].entry_time)
		self.assert_(threads[1].exit_time <= threads[5].entry_time)
		self.assert_(threads[2].exit_time <= threads[3].entry_time)
		self.assert_(threads[2].exit_time <= threads[4].entry_time)		
		self.assert_(threads[5].exit_time <= threads[3].entry_time)
		self.assert_(threads[5].exit_time <= threads[4].entry_time)	
	
	
    def __init_variables():
		buffer_ = []
		rw_lock = RWLock()
		threads = []
		return (buffer_, rw_lock, threads)

	def __start_and_join_threads(threads):
		for t in threads:
			t.start()
		for t in threads:
			t.join()
print(hello)
a = RWLockTestCase()
print(type(a))