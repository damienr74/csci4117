#ifndef OPT_SIZE_VECTOR__
#define OPT_SIZE_VECTOR__

#include <climits>
#include <cstdio>
#include <limits>
#include <memory>
#include <vector>

namespace csci4117 {

namespace impl {

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

template <class T>
struct opt_size_vector {
public:
  using size_type = std::size_t;

  opt_size_vector() : size_{0}, inode_cap_{0} {}

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

  const T& operator[](size_type i) const {
    const auto ref = get_addr(i+1);
    return inodes_[ref.inode]->blocks[ref.block].data[ref.offset];
  }

  T& operator[](size_type i) {
    const auto ref = get_addr(i+1);
    return inodes_[ref.inode]->blocks[ref.block].data[ref.offset];
  }

  size_type size() const { return size; }

private:
  struct address_ref {
    size_type inode;
    size_type block;
    size_type offset;
  };

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

  struct block_inode {
    block_inode(size_type block_size, size_type block_max_cap) :
      blocks{},
      block_size{block_size},
      block_max_cap{block_max_cap} {}

    std::vector<data_block> blocks;
    size_type block_size;
    size_type block_max_cap;
  };

  std::vector<std::unique_ptr<block_inode>> inodes_;
  size_type size_;
  size_type inode_cap_;
};

} // namespace csci4117
#endif // OPT_SIZE_VECTOR__
