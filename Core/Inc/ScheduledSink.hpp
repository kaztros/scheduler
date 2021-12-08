#include "Channel.hpp"
#include "Scheduler.hpp"

template <typename T>
struct ScheduledSink
: SinkProtocol <T>
{
	Scheduler * scheduler;
	Scheduler::ctx_handle_t greenlet_handle;
	std::atomic <T*> stack_location;	//Naive implementation probably.

	ScheduledSink (Scheduler * s, Scheduler::ctx_handle_t g)
	: scheduler (s)
	, greenlet_handle (g)
	, stack_location (nullptr)
	{ }

	T attach_and_wait (SourceDelegate <T> & sourcer) {
		T value;
		stack_location = &value;
		sourcer.source = [this] (T x) { this->sink (x); };
		if (stack_location) {		// Use an atomic pointer in case stack_location was nulled immediately.
			scheduler->sleep ();	// That way, we don't sleep forever waiting for a message already here.
			/* Execution stops here. */
			/* Execution probably resumes here. */
		}
		return value;
	}
	
	void sink (T x) override {
		if (stack_location) {
			*stack_location = x;
			stack_location = nullptr;
		}
		scheduler->wake_other (greenlet_handle);
	}
};

