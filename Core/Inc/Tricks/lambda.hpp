
template <typename CLASS_T, typename RET_T=void, typename ... ARGS_T>
using as_member_function_const = RET_T (CLASS_T::*)(ARGS_T...) const;

template <typename LAMBDA_T, typename ... ARGS_T>
auto make_fn_ptr_of_lambda_mem_fn (as_member_function_const<LAMBDA_T> mem_fn) {
  typedef void(*fn_ptr_t)(LAMBDA_T *, ARGS_T && ...);
  fn_ptr_t as_fn_ptr = (fn_ptr_t)(mem_fn);
  return as_fn_ptr;
}

