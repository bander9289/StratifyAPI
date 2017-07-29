/*! \file */ //Copyright 2011-2016 Tyler Gilbert; All Rights Reserved

#ifndef THREAD_HPP_
#define THREAD_HPP_

#ifndef __link

#include <mcu/types.h>
#include <pthread.h>
#include "Sched.hpp"

namespace sys {

/*! \brief Thread Class
 * \details This class creates and manages new threads using POSIX calls.
 *
 */
class Thread {
public:

	enum {
		ID_ERROR = -2,
		ID_UNINITIALIZED = -1,
		JOINABLE = PTHREAD_CREATE_JOINABLE,
		DETACHED = PTHREAD_CREATE_DETACHED
	};

	/*! \details This constructs a new thread object
	 *
	 * @param stack_size  The stack size of the new thread (default is 4096)
	 * @param detached Whether to create as a detached thread.  If this value is false,
	 * another thread must use join() in order for the thread to terminate correctly.
	 */
	Thread(int stack_size = 4096, bool detached = true);

	~Thread();

	/*! \details Sets the stacksize (no effect after create) */
	int set_stacksize(int size);

	/*! \details Gets the stacksize */
	int get_stacksize() const;

	/*! \details Get the detach state (Thread::JOINABLE or Thread::DETACHED) */
	int get_detachstate() const;

	/*! \details Sets the thread priority */
	int set_priority(int prio, enum Sched::policy policy = Sched::RR);

	/*! \details Gets the thread priority */
	int get_priority() const;

	/*! \details Get the thread policy */
	int get_policy() const;

	/*! \details Gets the ID of the thread */
	int id() const { return m_id; }

	/*! \details Start the thread */
	int create(void * (*func)(void*), void * args = NULL, int prio = 0, enum Sched::policy policy = Sched::OTHER);

	/*! \details Check if the thread is running */
	bool is_running() const;

	/*! \details Wait for the thread to complete (joins thread if it is not detached) */
	int wait(void**ret = 0, int interval = 1000);

	/*! \details Yield the processor to another thread */
	static void yield();

	/*! \details Join the current thread to the specified thread
	 *
	 * @param ident the ID of the target thread
	 * @param value_ptr A pointer to the return value of the target thread
	 * @return Zero on success
	 */
	static int join(int ident, void ** value_ptr);

	/*! \details This method returns true if the thread is joinable */
	bool is_joinable() const;

	/*! \details The calling thread will joing the thread with id Thread::id()
	 *
	 * @return 0 if joined, -1 if couldn't join (doesn't exist or is detached)
	 */
	int join(void ** value_ptr) const;

	/*! \details Reset the object (thread must not be running) */
	void reset();

	/*! \details Set the scheduler
	 *
	 * @param pid The process ID
	 * @param value The policy (such as Sched::FIFO)
	 * @param priority The priority (higher is higher priority)
	 * @return Zero on success of -1 with errno set
	 */
	static int set_scheduler(int id, enum Sched::policy value, int priority);

	/*! \details Allows read only access to the thread attributes */
	const pthread_attr_t & attr() const { return m_pthread_attr; }

private:
	pthread_attr_t m_pthread_attr;
	pthread_t m_id;

	int init(int stack_size, bool detached);

	void set_id_default(){ m_id = ID_UNINITIALIZED; }
	void set_id_error(){ m_id = ID_ERROR; }
};

};

#endif

#endif /* THREAD_HPP_ */
