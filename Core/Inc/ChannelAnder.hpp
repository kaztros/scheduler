#include <utility>

#include "Channel.hpp"
#include "OptionalTuple.hpp"

template <typename...T>
struct ChannelAnder
: public SourceDelegate <std::tuple <T...>>
{
	using TUPLE_T = std::tuple <T...>;
  using OPT_TUPLE_T = OptionalTuple <T...>;

	template <size_t IDX>
	using TUPLE_SUB_T = std::tuple_element_t <IDX, TUPLE_T>;
	
	
	OPT_TUPLE_T values_maybe;


	ChannelAnder (SourceDelegate <T> & ... args)
	: SourceDelegate <std::tuple <T...>> ()
	, values_maybe () /* Unlike std::optional, this can't contain default-unconstructible types */
	{
		redirect_sinks_helper (std::index_sequence_for <T...> {}, args...);
	}
	
	template <size_t IDX>
	void sink (TUPLE_SUB_T <IDX> x) {
		if ( ! GetIndex (values_maybe.is_values, IDX) ) {
			std::get <IDX> (values_maybe.values) = x;
			SetIndex (values_maybe.is_values, IDX);	
			if (all (values_maybe.is_values)) {
				this->source (values_maybe.values);
			}
		}
	}
	
protected:
	template <class INT_T, INT_T ... INTS>
	void redirect_sinks_helper (std::integer_sequence<INT_T, INTS...>, SourceDelegate<T> & ... args) {
		auto args_as_lref_tuple = std::tie (args...);
		(	( std::get <INTS> (args_as_lref_tuple).source
			  = [this] (TUPLE_SUB_T <INTS> x) { this->sink <INTS> (x); }
			)
		,
		...);
	}
};

