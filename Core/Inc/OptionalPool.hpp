#include "boost/variant.hpp"
#include "BitField.hpp"
#include "Error.hpp"

template <typename T, size_t SIZE>
struct OptionalPool {
  using handle_t = unsigned int;
  std::array <T, SIZE> values;
  BitSet<SIZE> is_values;
};



//What did I want from a scheduler?

//So a context is a
//  stack pointer
//  program counter
//and eventually
//  stack range

//I wanted to allocate contexts by handles so that
//  Referencing by small integers
//  Mapping additional attributes to a context
//  Taking away context-state-management from the user
//    Because some state transitions aren't user controlled

struct Context {};
struct ConditionVariable {};

struct FiberSwitch {
  static const size_t NUM_CONTEXTS = 16;
  static const size_t NUM_COND_VARS = 8;
  
  using ctx_handle_t = OptionalPool <Context, NUM_CONTEXTS> ::handle_t;
  using cv_handle_t = OptionalPool <ConditionVariable, NUM_COND_VARS> ::handle_t;
  
  OptionalPool <Context, NUM_CONTEXTS> contexts;
  OptionalPool <ConditionVariable, NUM_COND_VARS> condition_variables;
  
  ctx_handle_t current_context;
  
  BitSet <NUM_CONTEXTS> is_auto_scheduled;
  
  std::array <BitSet <NUM_CONTEXTS>, NUM_COND_VARS> contexts_to_schedule_by_cv_handle;
  
public:
  Error run ();
  Error set_is_auto_scheduled (ctx_handle_t, bool);
  
  template <typename LAMBDA_T>
  boost::variant <Error, ctx_handle_t> run_context (ctx_handle_t ctx_handle, LAMBDA_T const &) {

  /* The issue is that I need to synthesize a context from C++-land. */

    set_is_auto_scheduled (ctx_handle, true);
    LAMBDA_T();
    set_is_auto_scheduled (ctx_handle, false);
  }
  
  
};


