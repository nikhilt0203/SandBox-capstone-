#ifndef NST_INPLACE_VECTOR_HPP_
#define NST_INPLACE_VECTOR_HPP_

#include <array>
#include <cassert>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <new>
#include <type_traits>

// Vector with fixed-size inplace storage.
namespace nst {

template <typename T, std::size_t N> class inplace_vector {
  public:
	using value_type = T;
	using iterator = T *;
	using const_iterator = const T *;

	constexpr inplace_vector() = default;

	inplace_vector(std::initializer_list<T> elements) {
		for (const auto &e : elements) {
			emplace_back(e);
		}
	}

	~inplace_vector() { clear(); }

	inplace_vector(const inplace_vector &other) {
		for (auto &element : other) {
			emplace_back(element);
		}
	}

	inplace_vector(inplace_vector &&other) noexcept {
		for (auto &element : other) {
			emplace_back(std::move(element));
		}
		other.clear();
	}

	inplace_vector &operator=(const inplace_vector &other) {
		if (this == &other) {
			return *this;
		}
		clear();
		for (auto &element : other) {
			emplace_back(element);
		}
		return *this;
	}

	inplace_vector &operator=(inplace_vector &&other) {
		if (this == &other) {
			return *this;
		}
		clear();
		for (auto &element : other) {
			emplace_back(std::move(element));
		}
		other.clear();
		return *this;
	}

	constexpr bool push_back(const T &element) { return emplace_back(element); }

	constexpr bool push_back(T &&element) {
		return emplace_back(std::move(element));
	}

	template <typename... Args> constexpr bool emplace_back(Args &&...args) {
		if (size_ >= N) {
			return false;
		}
		::new (&storage_[size_]) T(std::forward<Args>(args)...);
		++size_;
		return true;
	}

	constexpr T *erase(T *pos) {
		if (pos < begin() || pos >= end()) {
			return end();
		}

		const auto erase_index = pos - begin();

		for (std::size_t i = erase_index; i < size_ - 1; ++i) {
			*address_of(i) = std::move(*address_of(i + 1));
		}

		std::destroy_at(address_of(size_ - 1));
		--size_;

		return begin() + erase_index;
	}

	constexpr T *erase(T *first, T *last) {
		if (!first || !last || first >= last || first >= end() ||
		    last <= begin()) {
			return end();
		}

		if (first < begin()) {
			first = begin();
		}
		if (last > end()) {
			last = end();
		}

		std::size_t first_idx = first - begin();
		std::size_t last_idx = last - begin();
		std::size_t num_to_remove = last_idx - first_idx;

		if (num_to_remove == 0) {
			return begin() + first_idx;
		}

		for (std::size_t i = last_idx; i < size_; ++i) {
			*address_of(i - num_to_remove) = std::move(*address_of(i));
		}

		for (std::size_t i = size_ - num_to_remove; i < size_; ++i) {
			std::destroy_at(address_of(i));
		}

		size_ -= num_to_remove;

		return begin() + first_idx;
	}

	void pop_back() noexcept {
		assert(!is_empty());
		std::destroy_at(address_of(size_ - 1));
		--size_;
	}

	[[nodiscard]] T pop() {
		assert(!is_empty());
		T tmp = std::move(back());
		std::destroy_at(address_of(size_ - 1));
		--size_;
		return tmp;
	}

	void clear() {
		for (std::size_t i{}; i < size_; ++i) {
			std::destroy_at(address_of(i));
		}
		size_ = 0;
	}

	[[nodiscard]] constexpr bool is_empty() const noexcept {
		return size_ == 0;
	}
	[[nodiscard]] constexpr bool is_full() const noexcept { return size_ >= N; }

	[[nodiscard]] const T &at(std::size_t index) const {
		assert(index < size_);
		return *address_of(index);
	}

	[[nodiscard]] T &at(std::size_t index) {
		assert(index < size_);
		return *address_of(index);
	}

	[[nodiscard]] constexpr const T &
	operator[](std::size_t index) const noexcept {
		return *address_of(index);
	}
	[[nodiscard]] constexpr T &operator[](std::size_t index) noexcept {
		return *address_of(index);
	}

	[[nodiscard]] constexpr const T &back() const noexcept {
		return *address_of(size_ - 1);
	}
	[[nodiscard]] constexpr T &back() noexcept {
		return *address_of(size_ - 1);
	}

	[[nodiscard]] constexpr std::size_t size() const { return size_; }
	[[nodiscard]] static constexpr std::size_t capacity() { return N; }

	[[nodiscard]] constexpr T *begin() { return address_of(0); }
	[[nodiscard]] constexpr T *end() { return begin() + size_; }
	[[nodiscard]] constexpr const T *begin() const { return address_of(0); }
	[[nodiscard]] constexpr const T *end() const { return begin() + size_; }
	[[nodiscard]] constexpr auto cbegin() const { return begin(); }
	[[nodiscard]] constexpr auto cend() const { return end(); }

  private:
	constexpr T *address_of(std::size_t index) {
		return std::launder(reinterpret_cast<T *>(&storage_[index]));
	}

	constexpr const T *address_of(std::size_t index) const {
		return std::launder(reinterpret_cast<const T *>(&storage_[index]));
	}

  private:
	std::size_t size_{};
	std::aligned_storage_t<sizeof(T), alignof(T)> storage_[N];
};

template <typename T> using vector_4U = inplace_vector<T, 4>;
template <typename T> using vector_8U = inplace_vector<T, 8>;
template <typename T> using vector_16U = inplace_vector<T, 16>;
template <typename T> using vector_32U = inplace_vector<T, 32>;
template <typename T> using vector_64U = inplace_vector<T, 64>;
template <typename T> using vector_128U = inplace_vector<T, 128>;

} // namespace nst

#endif