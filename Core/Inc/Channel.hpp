#include <optional>
#include "Scheduler.hpp"

template <typename T>
void source_default (T trash) { /* Do nothing.  Or maybe debug? */ }

template <typename T>
struct SinkProtocol {
	virtual void sink (T) = 0;
};

template <typename T>
struct SourceDelegate {
	std::function <void (T)> source;

	SourceDelegate () : source (&source_default <T>) { }

	template <typename U>
	SourceDelegate (U x) : source (std::forward <U> (x)) { }
};


template <typename T>
struct ChannelIdentity
: public SinkProtocol <T>
, public SourceDelegate <T>
{
	void sink (T x) override { this->source(x); }	
};

#include "OptionalTuple.hpp"

template <typename...T>
struct ChannelAnder
: public SourceDelegate <std::tuple <T...>>
{
	using TUPLE_T = typename std::tuple <T...>;
  using OPT_TUPLE_T = typename OptionalTuple <T...>;

	template <size_t IDX>
	using TUPLE_SUB_T = std::tuple_element <IDX, TUPLE_T>;
	
	OPT_TUPLE_T values_maybe;

	ChannelAnder (SourceDelegate <T> & args...)
	: SourceDelegate <std::tuple <T...>> ()
	{
		FillSinksHelper (std::index_sequence_for <args> (), args...);
	}
	
	template <size_t IDX>
	void sink (TUPLE_SUB_T x) {
		if ( ! GetIndex (values_maybe.is_values, IDX) ) {
			values_maybe.values = x;
			SetIndex (values_maybe.is_values, IDX);	
			if (all (values_maybe.is_values) ) {
				this->source (values_maybe.values);
			}
		}
	}
	
protected:
	template <class INT_T, INT_T ... INTS>
	void FillSinksHelper (std::integer_sequence<INT_T, INTS...>, SourceDelegate<T> & args ...) {
		auto args_as_lref_tuple = std::tie (args);		
		(	( std::get <INTS> (args_as_lref_tuple) 
			  = [this] (T x) { this->sink <INTS> (x); }
			)
		,
		...);
	}
	
};

/*

template <typename T>
class NullSource {
	void deferredWake (T&& x) { }
	std::optional<T> source() { return std::nullopt; }
};


template <typename T>
class ScheduledSource {
	Scheduler * scheduler;
	Scheduler::ctx_handle_t handle;
	optional<T> * stack_copy;

protected:
	void deferredWake (T&& x) {
		if (stack_copy) {
			*stack_copy = std::forward<T> (x);
			stack_copy = nullptr
	  	scheduler->wakeOther (handle); 
		}
	}
	
	void waitForWaking() {
		scheduler->sleep ();
  }

public:
	std::optional<T> source() {
		std::optional<T> local_copy;
		stack_copy = &local_copy;
		waitForWaking();
		return local_copy;
	}
};


template <typename T>
struct functor {
	void operator() (T&&);
};
*/

class InterruptTraits {
	using Summary_t = std::none_t;	//but maybe like uint16_t or smth?
	
	static functor<Summary_t> * callback;
};

struct AdcXInterrupt {
	EventResponderListNode <uint16_t> eventHandlers;
};

template <typename T>
struct EventResponderListNode {
	virtual (void)(EventResponderListNode <T>:: *sink) (T) = 0;
	EventResponderListNodeBase<T> * next;
};

template <typename... INT_TRAITS_T>
class SourceSequence
: EventResponderListNode </*  BAD.  Meta-programming is rusty. */> {
	using TUPLE_T = typename std::tuple <INT_TRAITS_T::Summary_t ...>;

	template <size_t IDX>
	using TUPLE_SUB_T = std::tuple_element <IDX, TUPLE_T> 

	Scheduler * scheduler;
	Scheduler::ctx_handle_t handle;
	TUPLE_T collection;
	std::tuple <decltype(INT_TRAITS_T::callback)> functor_callbacks;

	template <typename ... INT_TRAITS_T>
	SourceSequence (Scheduler * _scheduler, Scheduler::ctx_handle_t _handle, INT_TRAITS_T... args)
	: scheduler (_scheduler)
	, handle (_handle)
	, functor_callbacks: make_tuple(INT_TRAITS_T::callback ...)
 	{
 		/* TODO:  The helpers are killing me.  */
	}

	template <size_t TUPLE_IDX>
	as
	
	
	template <size_t TUPLE_IDX>
	void operator() (INNER_T && x) {
		std::get <TUPLE_IDX> (orig->collection) = std::forward <INNER_T> (x);
		/*	Hey, doing this check on a tuple of optionals sucks cycles.
		    Consider an optionalArray made into an optionalTuple to speed it up. */
		if (std::apply(AllOptionalsPresent, orig->collection)) {
			scheduler->wakeOther (handle);
		}
		std::get <TUPLE_IDX> (orig->functor_callbacks) (x);		
	}

	
	TUPLE_T source() {
		return collection;
	}
	
};


template <typename ... T_REST>
bool AllOptionalsPresent (std::optional<T_REST> && ... args) {
	true && (... && args.has_value());
};


template <size_t BOOF_ID, typename... T>
class SourceSequenceAspect {
	SourceSequence <T...> * orig;
	using INNER_T = typename SourceSequence <T...>::TUPLE_SUB_T <BOOF_ID>;
		
	void operator() (INNER_T && x) {
		std::get <BOOF_ID> (orig->collection) = std::forward <INNER_T> (x);
		/*	Hey, doing this check on a tuple of optionals sucks cycles.
		    Consider an optionalArray made into an optionalTuple to speed it up. */
		if (std::apply(AllOptionalsPresent, orig->collection)) {
			scheduler->wakeOther (handle);
		}
		std::get <BOOF_ID> (orig->functor_callbacks) (x);		
	}
};

	

