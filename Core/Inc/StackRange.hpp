#include <memory>

struct StackRange {
  StackRange ();
  StackRange (size_t minimum_size_in_bytes);
  ~StackRange ();
  
  StackRange (StackRange&&) = default;
  StackRange& operator= (StackRange&&) = default;

  void** start_of_stack() const;
  
  /* base_ptr needs to be strict in alignment (e.g. 4-byte boundaries) */
  /* TODO: StackRange's handling of float64 needs to be tested. */
  std::unique_ptr <void*[]> base_ptr;
  size_t size_in_words;
};


constexpr size_t word_count_from_bytes (size_t num_of_bytes) {
  if (0 < num_of_bytes)
  { return ((num_of_bytes - 1) / sizeof(void*)) + 1; }
  else
  { return size_t (0); }
}


template <typename T>
void** emplace_descending_upon (void** sp, T && t) {
  auto word_size = word_count_from_bytes (sizeof(T));
  auto address = & sp [-word_size];
  new (reinterpret_cast <T*> (address)) T (std::forward <T> (t));
  return address;
}


