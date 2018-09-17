#ifndef OPT_SIZE_VECTOR__
#define OPT_SIZE_VECTOR__

#include <climits>
#include <cstdio>
#include <limits>
#include <memory>
#include <vector>

namespace csci4117 {

namespace impl {

// Integer square root implementation runs in O(lg(n)) where n is the input to
// solve for the square root.
// def: sqrt(n)*sqrt(n) <= n
template <class size_type = std::size_t>
size_type sqrt(size_type n) {
  if (n < 2) return 1;

  size_type count = n, step;
  size_type i = 0;

  while (count > 0) {
    step = count / 2;
    if ((step+i)*(step+i) < n) {
      i += step + ((step+i+1)*(step+i+1) <= n);
      count -= step + 1;
    } else count = step;
  }

  return i;
}

} // namespace impl

// Implementation of optimal sized resizeable array. Auxilary memory: O(sqrt(n)).
// - push_back(T e) O(1) amortized
// - operator[](size_type i) O(1).
template <class T>
struct opt_size_vector {
public:
  using size_type = std::size_t;

  opt_size_vector() : size_{0}, inode_cap_{0} {}

  // Insert inodes and blocks as needed. The scheme is as follows:
  //  The ith inode stores up to 1 << i elements. These elements are stored in
  //  contiguous blocks of size O(sqrt(1 << i)).
  //
  // push_back adds inodes and blocks as required to fit size_++ elements in the
  // data structure.
  //
  // Amortized O(1) time.
  void push_back(const T& e) {
    if (size_ == inode_cap_) {
      size_type cap = 1 << inodes_.size();
      inodes_.emplace_back(std::make_unique<block_inode>(impl::sqrt(cap), cap));
      inode_cap_ |= cap;
    }

    const auto ref = get_addr(++size_);
    auto& inode = inodes_[ref.inode];
    if (inode->blocks.size() <= ref.block) {
      inode->blocks.push_back(
        data_block(
          std::min(
            inode->block_size,
            inode->block_max_cap - inode->blocks.size() * inode->block_size
          )
        )
      );
    }

    inode->blocks[ref.block].data[ref.offset] = e;
  }

  // Random access of index i.
  const T& operator[](size_type i) const {
    const auto ref = get_addr(i+1);
    return inodes_[ref.inode]->blocks[ref.block].data[ref.offset];
  }

  // Random access of index i.
  T& operator[](size_type i) {
    const auto ref = get_addr(i+1);
    return inodes_[ref.inode]->blocks[ref.block].data[ref.offset];
  }

  // Number of elements in array.
  size_type size() const { return size; }

private:
  struct address_ref {
    size_type inode;
    size_type block;
    size_type offset;
  };

  // get_addr returns an address_ref that acts as a layered pointer to access
  // the data at index i.
  //
  // O(1) time.
  address_ref get_addr(size_type i) const {
    if (i == 0) {
      return address_ref{0, 0, 0};
    }

    address_ref ref = {};
    ref.inode = CHAR_BIT * sizeof i - __builtin_clzll(i) - 1;
    const auto& inode = inodes_[ref.inode];
    ref.block = (i - (size_type{1} << ref.inode)) / inode->block_size;
    ref.offset = i - (ref.block * inode->block_size + (size_type{1} << ref.inode));

    return ref;
  }

  // Data block that holds size contiguous items in memory.
  struct data_block {
    explicit data_block(size_type size) :
      data{std::make_unique<T[]>(size)},
      size{size} {}

    data_block(data_block&& db) noexcept :
      data{std::move(db.data)}, size{db.size} {}

    ~data_block() = default;

    data_block(data_block& db) = delete;
    data_block& operator=(data_block&& db) = delete;
    data_block& operator=(data_block& db) = delete;

    std::unique_ptr<T[]> data;
    const size_type size;
  };

  // Inodes are used to map indices to powers of two for random access.
  // each inode will contain O(sqrt(n)) blocks of O(sqrt(n)) size when
  // completely utilized.
  //
  // Blocks are allocated dynamically.
  struct block_inode {
    block_inode(size_type block_size, size_type block_max_cap) :
      blocks{},
      block_size{block_size},
      block_max_cap{block_max_cap} {}

    std::vector<data_block> blocks;
    size_type block_size;
    size_type block_max_cap;
  };

  // inode array.
  std::vector<std::unique_ptr<block_inode>> inodes_;
  size_type size_;
  size_type inode_cap_;
};

} // namespace csci4117
#endif // OPT_SIZE_VECTOR__
