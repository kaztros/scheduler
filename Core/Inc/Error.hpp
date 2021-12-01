#pragma once

#include <ostream>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#define xstr(s) str(s)
#define str(s) #s

#define CONTRACT(RULE, ...) do { \
  if (!(RULE)) { \
    return MakeContractViolation (xstr(RULE), std::make_tuple (__VA_ARGS__) ); \
  } \
} while (0);


struct ErrorProtocol {
  virtual ~ErrorProtocol () = default;
  virtual std::ostream& PrintVia (std::ostream& printer) const = 0;
};

using Error = std::shared_ptr<ErrorProtocol>;

template <typename...T>
struct ContractViolation
: public ErrorProtocol
{
	std::string_view rule;
	std::tuple<T...> violatingValues;

	ContractViolation(std::string_view const & rule, std::tuple<T...> violatingValues)
	: rule (rule)
	, violatingValues (violatingValues)
	{ }

	virtual std::ostream& PrintVia (std::ostream& printer) const {
		auto const & printMany = [&printer] (auto const &...x) -> std::ostream& {
			return printer << (x << ...);
		};

		printer << rule << ", variables: (";
		return std::apply(printMany, violatingValues) << ")";
	}
};


template <typename...T>
std::shared_ptr <ContractViolation <T...>> MakeContractViolation(std::string_view const & rule, std::tuple<T...> violatingValues) {
	return std::make_shared <ContractViolation <T...>> (rule, violatingValues);
}


struct NoError
: public ErrorProtocol {
	virtual std::ostream& PrintVia (std::ostream& printer) const override;
};


extern Error NO_ERROR;
