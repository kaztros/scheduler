#include "Scheduler.hpp"

Scheduler::Scheduler () {
}


std::optional <Scheduler::ctx_handle_t> Scheduler::true_allocate_greenlet (size_t stack_size_in_bytes) {
  auto handle = GetFirstFalseIndex (this->caps_is_used);
  if (! handle) { return handle; }  // Out of handles?

  SetIndex (this->caps_is_used, *handle);
  auto & stack = this->stack_ranges [*handle];
  stack = StackRange (stack_size_in_bytes);
  
  if (nullptr == stack.base_ptr) {
    // out of memory D: ?
    ClearIndex (this->caps_is_used, *handle);
    handle = std::nullopt;
  }
  
  return handle;
}

void Scheduler::yield() {
  Bottling::make_vfpless_cap_and_unbottle
    ( &(caps [last_cap_idx])
    , &scheduling_cap
    );
}

void Scheduler::sleep() {
  ClearIndex (caps_is_autoscheduled, last_cap_idx);
  yield();
}

void Scheduler::wake_other(Scheduler::ctx_handle_t other) {
  //Contract (GetIndex(this->caps_is_used, other) == true)
  if (GetIndex(this->caps_is_used, other)) {
    SetIndex (this->caps_is_autoscheduled, other);
  }
}

void Scheduler::run_until_all_joined () {
  while (any (caps_is_used)) {
    caps_needs_cleanup.on_every_true (
      [this] (ctx_handle_t idx) { free_greenlet (idx); }
    );

    if (! any (caps_is_autoscheduled)) {
      /* sleep; */
      continue;
    } else {
      //cap ourselves; and assume the destination cap will ::yield() to us.
      caps_is_autoscheduled.on_every_true (
        [this] (ctx_handle_t idx) {
          last_cap_idx = idx;
          // TODO: Virtual memory - Allow_RW_Access_To_substack (stack_ranges[idx]);
          Bottling::make_vfpless_cap_and_unbottle (&(scheduling_cap), &caps [idx]);
          // TODO: Virtual memory - Deny_RW_Access_To_substack (stack_ranges[idx])
        }
      );
    }
  }
}

void Scheduler::free_greenlet (Scheduler::ctx_handle_t idx) {
  stack_ranges [idx] = StackRange();
  // ^ Deallocate stack.
  ClearIndex (caps_is_used, idx);
}
