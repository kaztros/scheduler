#include <memory>
#include <type_traits>

#include "Tricks/tuple.hpp"
#include "CTM/Bottler.hpp"
#include "BitField.hpp"
#include "StackRange.hpp"


const size_t DEFAULT_GREENLET_BYTE_SIZE = 256;

struct Scheduler {
  static const size_t NUM_OF_CTX = 16;
  using ctx_handle_t = size_t;
  
  template <typename T>
  static void __attribute__ ((noinline)) greenlet_terminus [[noreturn]] (Scheduler* scheduler, Scheduler::ctx_handle_t handle, void (*fn_ptr)(T*), T* fn_arg) {
    fn_ptr (fn_arg);
    SetIndex (scheduler->caps_needs_cleanup, handle);
    while (true) { scheduler->sleep(); }
  }
  
  template <auto & CALLABLE, typename CALLABLE_T = std::remove_reference_t<decltype(CALLABLE)>, typename ... ARGS_T>
  static std::enable_if_t <std::is_function_v<CALLABLE_T>, void> __attribute__ ((noinline)) greenlet_terminus [[noreturn]] (Scheduler* scheduler, Scheduler::ctx_handle_t handle, std::tuple <ARGS_T...> & args_as_tuple) {
    
    std::apply (CALLABLE, std::move (args_as_tuple));
    // Intentionally destructive "move" because we don't return to calling scope.
    SetIndex (scheduler->caps_needs_cleanup, handle);
    while (true) { scheduler->sleep(); }
  }
  
  template <typename LAMBDA_T>
  static void __attribute__ ((noinline)) greenlet_terminus [[noreturn]] (Scheduler* scheduler, Scheduler::ctx_handle_t handle, LAMBDA_T & op) {
    /* We violate C++ convention by invoking op's destructor early.
       Because the end of this function, the }, is never executed. */
    op();
    op.~LAMBDA_T();
    SetIndex (scheduler->caps_needs_cleanup, handle);
    while (true) { scheduler->sleep (); }
  }
  
  Bottling::Cap scheduling_cap;
  Bottling::Cap caps [NUM_OF_CTX];
  ctx_handle_t last_cap_idx;
  StackRange stack_ranges [NUM_OF_CTX];
  BitSet <NUM_OF_CTX> caps_is_used;
  BitSet <NUM_OF_CTX> caps_needs_cleanup;
  BitSet <NUM_OF_CTX> caps_is_autoscheduled;

  Scheduler();

  std::optional <ctx_handle_t> true_allocate_greenlet (size_t stack_size_in_bytes);
  
  //std::optional <ctx_handle_t> AllocateGreenlet (void (*fn_ptr)());
  template <typename T>
  std::optional <ctx_handle_t> allocate_greenlet (void (*fn_ptr)(T*), T* fn_arg) {
    auto handle = true_allocate_greenlet (DEFAULT_GREENLET_BYTE_SIZE);
    if (! handle) { return handle; }

    void (*terminus)(decltype(this), ctx_handle_t, void(*)(T*), T*) = &(Scheduler::greenlet_terminus <T>);

    void** start_of_stack = stack_ranges [*handle].start_of_stack ();
    
    this->caps [*handle] = Bottling::make_cap_of
      ( start_of_stack
      , terminus
      , this
      , *handle
      , fn_ptr
      , fn_arg
      );

    SetIndex (this->caps_is_autoscheduled, *handle);
    return handle;
  }
  
  template <typename LAMBDA_T>
  std::optional <ctx_handle_t> allocate_greenlet (LAMBDA_T && op) {
    using LAMBDA_REFLESS_T = typename std::remove_reference<LAMBDA_T>::type;
    auto handle = true_allocate_greenlet (DEFAULT_GREENLET_BYTE_SIZE);
    if (! handle) { return handle; }

    void (*terminus) (decltype(this), ctx_handle_t, LAMBDA_T&) = &(Scheduler::greenlet_terminus <LAMBDA_REFLESS_T>);

    void** start_of_stack =
      emplace_descending_upon
        (stack_ranges [*handle].start_of_stack(), std::forward<LAMBDA_REFLESS_T>(op));
    
    this->caps [*handle] = Bottling::make_cap_of
      ( start_of_stack
      , terminus
      , this
      , *handle
      , reinterpret_cast <LAMBDA_REFLESS_T *> (start_of_stack)
      );

    SetIndex (this->caps_is_autoscheduled, *handle);
    return handle;
  }

  template <auto & CALLABLE, typename ... TRUE_ARGS_T, typename ... ARGS_T>
  std::optional <ctx_handle_t> allocate_greenlet (ARGS_T && ... args) {
    using TRUE_TUPLE_T = TupleTypeFromFunction_t <CALLABLE>;
    auto handle = true_allocate_greenlet (DEFAULT_GREENLET_BYTE_SIZE);
    if (! handle) { return handle; }

    void (*terminus) (decltype (this), ctx_handle_t, TRUE_TUPLE_T &) = & Scheduler::greenlet_terminus <CALLABLE>;

    void** start_of_stack =
      emplace_descending_upon <TRUE_TUPLE_T>
        (stack_ranges [*handle].start_of_stack (), std::forward_as_tuple (args...));
    
    this->caps [*handle] = Bottling::make_cap_of
      ( start_of_stack
      , terminus
      , this
      , *handle
      , reinterpret_cast<TRUE_TUPLE_T*>(start_of_stack)
      );

    SetIndex (this->caps_is_autoscheduled, *handle);
    return handle;
  }

  void run_until_all_joined();

  void yield ();  // Yield control.
  void sleep ();  // Yield control and remove from auto-schedule.
  void wake_other (ctx_handle_t other);	// Add back to an auto-schedule.

protected:
  void free_greenlet (ctx_handle_t);

};

