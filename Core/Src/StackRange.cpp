#include "StackRange.hpp"

StackRange::StackRange ()
: base_ptr (nullptr)
, size_in_words (0)
{ }


StackRange::StackRange (size_t minimum_size_in_bytes)
: base_ptr (new void* [word_count_from_bytes (minimum_size_in_bytes)])
, size_in_words (word_count_from_bytes (minimum_size_in_bytes))
{ }


StackRange::~StackRange() {
  size_in_words = 0;
}


void** StackRange::start_of_stack() const {
  return &(base_ptr [size_in_words]);
}



