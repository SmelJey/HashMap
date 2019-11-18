#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <type_traits>

namespace fefu
{
    template<typename T>
    class allocator {
    public:
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = typename std::add_lvalue_reference<T>::type;
        using const_reference = typename std::add_lvalue_reference<const T>::type;
        using value_type = T;

        allocator() noexcept {}

        allocator(const allocator&) noexcept {}

        template <class U>
        allocator(const allocator<U>&) noexcept {}

        ~allocator() {}

        pointer allocate(size_type n) {
            pointer ptr = static_cast<pointer>(::operator new(n * sizeof(value_type)));
            return ptr;
        }

        void deallocate(pointer p, size_type n) noexcept {
            ::operator delete(static_cast<void*>(p), n * sizeof(value_type));
        }
    };

    template<typename ValueType>
    class hash_map_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ValueType;
        using difference_type = std::ptrdiff_t;
        using reference = ValueType&;
        using pointer = ValueType*;

        hash_map_iterator() noexcept;
        hash_map_iterator(const hash_map_iterator& other) noexcept;

        reference operator*() const;
        pointer operator->() const;

        // prefix ++
        hash_map_iterator& operator++();
        // postfix ++
        hash_map_iterator operator++(int);

        friend bool operator==(const hash_map_iterator<ValueType>&, const hash_map_iterator<ValueType>&);
        friend bool operator!=(const hash_map_iterator<ValueType>&, const hash_map_iterator<ValueType>&);
    };

    template<typename ValueType>
    class hash_map_const_iterator {
        // Shouldn't give non const references on value
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ValueType;
        using difference_type = std::ptrdiff_t;
        using reference = const ValueType&;
        using pointer = const ValueType*;

        hash_map_const_iterator() noexcept;
        hash_map_const_iterator(const hash_map_const_iterator& other) noexcept;
        hash_map_const_iterator(const hash_map_iterator<ValueType>& other) noexcept;

        reference operator*() const;
        pointer operator->() const;

        // prefix ++
        hash_map_const_iterator& operator++();
        // postfix ++
        hash_map_const_iterator operator++(int);

        friend bool operator==(const hash_map_const_iterator<ValueType>&, const hash_map_const_iterator<ValueType>&);
        friend bool operator!=(const hash_map_const_iterator<ValueType>&, const hash_map_const_iterator<ValueType>&);
    };
    template<typename K, typename T,
        typename Hash = std::hash<K>,
        typename Pred = std::equal_to<K>,
        typename Alloc = allocator<std::pair<const K, T>>>
        class hash_map
    {
    public:
        using key_type = K;
        using mapped_type = T;
        using hasher = Hash;
        using key_equal = Pred;
        using allocator_type = Alloc;
        using value_type = std::pair<const key_type, mapped_type>;
        using reference = value_type&;
        using const_reference = const value_type&;
        using iterator = hash_map_iterator<value_type>;
        using const_iterator = hash_map_const_iterator<value_type>;
        using size_type = std::size_t;

        /// Default constructor.
        hash_map() = default;

        /**
         *  @brief  Default constructor creates no elements.
         *  @param n  Minimal initial number of buckets.
         */
        explicit hash_map(size_type n) : mData(mAlloc.allocate(n)), mIsSet(n) {}

        /**
         *  @brief  Builds an %hash_map from a range.
         *  @param  first  An input iterator.
         *  @param  last  An input iterator.
         *  @param  n  Minimal initial number of buckets.
         *
         *  Create an %hash_map consisting of copies of the elements from
         *  [first,last).  This is linear in N (where N is
         *  distance(first,last)).
         */
        template<typename InputIterator>
        hash_map(InputIterator first, InputIterator last,
            size_type n = 0) : mData(mAlloc.allocate(n)), mIsSet(n) {

            for (auto i = first; i != last; i++) {
                (*this)[i->first] = i->second;
            }
        }

        /// Copy constructor.
        hash_map(const hash_map& src) : mIsSet(src.mIsSet), mAlloc(src.mAlloc),
                                        maxLoadFactor(src.maxLoadFactor), mCount(src.mCount),
                                        mHash(src.mHash), mKeyEqual(src.mKeyEqual) {
            mData = mAlloc.allocate(src.mIsSet.size());

            for (size_t i = 0; i < mIsSet.size(); i++) {
                if (src.mIsSet[i] == 1) {
                    new(mData + i) value_type(src.mData[i]);
                }
            }
        }

        /// Move constructor.
        hash_map(hash_map&& rvalue) {
            swap(std::move(rvalue));
            rvalue.mData = nullptr;
        }

        /**
         *  @brief Creates an %hash_map with no elements.
         *  @param a An allocator object.
         */
        explicit hash_map(const allocator_type& a) : mAlloc(a) {
            mData = mAlloc.allocate(0);
        }

        /*
        *  @brief Copy constructor with allocator argument.
        * @param  uset  Input %hash_map to copy.
        * @param  a  An allocator object.
        */
        hash_map(const hash_map& umap,
            const allocator_type& a) : mAlloc(a), mIsSet(umap.mIsSet.size()) {

            mData = mAlloc.allocate(umap.mIsSet.size());

            for (size_t i = 0; i < mIsSet.size(); i++) {
                if (umap.mIsSet[i] == 1) {
                    mIsSet[i] = 1;
                    new(mData + i) value_type(umap.mData[i]);
                }
            }
        }

        /*
        *  @brief  Move constructor with allocator argument.
        *  @param  uset Input %hash_map to move.
        *  @param  a    An allocator object.
        */
        hash_map(hash_map&& umap,
            const allocator_type& a) : mAlloc(a), mIsSet(std::move(umap.mIsSet)),
                                    mHash(std::move(umap.mHash)), mKeyEqual(std::move(umap.mKeyEqual)),
                                    mCount(std::move(umap.mCount)), maxLoadFactor(std::move(umap.maxLoadFactor)) {
            mData = mAlloc.allocate(mIsSet.size());

            for (size_t i = 0; i < mIsSet.size(); i++) {
                if (mIsSet[i] == 1) {
                    new(mData + i) value_type(umap.mData[i]);
                }
            }

            umap.mAlloc.deallocate(umap.mData, mIsSet.size());
            umap.mData = nullptr;
        }

        /**
         *  @brief  Builds an %hash_map from an initializer_list.
         *  @param  l  An initializer_list.
         *  @param n  Minimal initial number of buckets.
         *
         *  Create an %hash_map consisting of copies of the elements in the
         *  list. This is linear in N (where N is @a l.size()).
         */
        hash_map(std::initializer_list<value_type> l,
            size_type n = 0) : hash_map(l.begin(), l.end(), n) {}

        ~hash_map() {
            mAlloc.deallocate(mData, mIsSet.size());
        }

        /// Copy assignment operator.
        hash_map& operator=(const hash_map&src) {
            hash_map tmp(src);
            swap(tmp);
            return *this;
        }

        /// Move assignment operator.
        hash_map& operator=(hash_map&&src) {
            swap(src);
            return *this;
        }

        /**
         *  @brief  %hash_map list assignment operator.
         *  @param  l  An initializer_list.
         *
         *  This function fills an %hash_map with copies of the elements in
         *  the initializer list @a l.
         *
         *  Note that the assignment completely changes the %hash_map and
         *  that the resulting %hash_map's size is the same as the number
         *  of elements assigned.
         */
        hash_map& operator=(std::initializer_list<value_type> l) {
            hash_map tmp(l);
            swap(tmp);
        }

        ///  Returns the allocator object used by the %hash_map.
        allocator_type get_allocator() const noexcept {
            return mAlloc;
        }

        // size and capacity:

        ///  Returns true if the %hash_map is empty.
        bool empty() const noexcept {
            return (mCount == 0);
        }

        ///  Returns the size of the %hash_map.
        size_type size() const noexcept {
            return mCount;
        }

        ///  Returns the maximum size of the %hash_map.
        size_type max_size() const noexcept {
            return MAXSIZE_T;
        }

        // iterators.

        /**
         *  Returns a read/write iterator that points to the first element in the
         *  %hash_map.
         */
        iterator begin() noexcept;

        //@{
        /**
         *  Returns a read-only (constant) iterator that points to the first
         *  element in the %hash_map.
         */
        const_iterator begin() const noexcept;

        const_iterator cbegin() const noexcept;

        /**
         *  Returns a read/write iterator that points one past the last element in
         *  the %hash_map.
         */
        iterator end() noexcept;

        //@{
        /**
         *  Returns a read-only (constant) iterator that points one past the last
         *  element in the %hash_map.
         */
        const_iterator end() const noexcept;

        const_iterator cend() const noexcept;
        //@}

        // modifiers.

        /**
         *  @brief Attempts to build and insert a std::pair into the
         *  %hash_map.
         *
         *  @param args  Arguments used to generate a new pair instance (see
         *	        std::piecewise_contruct for passing arguments to each
        *	        part of the pair constructor).
        *
        *  @return  A pair, of which the first element is an iterator that points
        *           to the possibly inserted pair, and the second is a bool that
        *           is true if the pair was actually inserted.
        *
        *  This function attempts to build and insert a (key, value) %pair into
        *  the %hash_map.
        *  An %hash_map relies on unique keys and thus a %pair is only
        *  inserted if its first element (the key) is not already present in the
        *  %hash_map.
        *
        *  Insertion requires amortized constant time.
        */
        template<typename... _Args>
        std::pair<iterator, bool> emplace(_Args&&... args);

        /**
         *  @brief Attempts to build and insert a std::pair into the
         *  %hash_map.
         *
         *  @param k    Key to use for finding a possibly existing pair in
         *                the hash_map.
         *  @param args  Arguments used to generate the .second for a
         *                new pair instance.
         *
         *  @return  A pair, of which the first element is an iterator that points
         *           to the possibly inserted pair, and the second is a bool that
         *           is true if the pair was actually inserted.
         *
         *  This function attempts to build and insert a (key, value) %pair into
         *  the %hash_map.
         *  An %hash_map relies on unique keys and thus a %pair is only
         *  inserted if its first element (the key) is not already present in the
         *  %hash_map.
         *  If a %pair is not inserted, this function has no effect.
         *
         *  Insertion requires amortized constant time.
         */
        template <typename... _Args>
        std::pair<iterator, bool> try_emplace(const key_type& k, _Args&&... args);

        // move-capable overload
        template <typename... _Args>
        std::pair<iterator, bool> try_emplace(key_type&& k, _Args&&... args);

        //@{
        /**
         *  @brief Attempts to insert a std::pair into the %hash_map.
         *  @param x Pair to be inserted (see std::make_pair for easy
         *	     creation of pairs).
        *
        *  @return  A pair, of which the first element is an iterator that
        *           points to the possibly inserted pair, and the second is
        *           a bool that is true if the pair was actually inserted.
        *
        *  This function attempts to insert a (key, value) %pair into the
        *  %hash_map. An %hash_map relies on unique keys and thus a
        *  %pair is only inserted if its first element (the key) is not already
        *  present in the %hash_map.
        *
        *  Insertion requires amortized constant time.
        */
        std::pair<iterator, bool> insert(const value_type& x);

        std::pair<iterator, bool> insert(value_type&& x);

        //@}

        /**
         *  @brief A template function that attempts to insert a range of
         *  elements.
         *  @param  first  Iterator pointing to the start of the range to be
         *                   inserted.
         *  @param  last  Iterator pointing to the end of the range.
         *
         *  Complexity similar to that of the range constructor.
         */
        template<typename _InputIterator>
        void insert(_InputIterator first, _InputIterator last);

        /**
         *  @brief Attempts to insert a list of elements into the %hash_map.
         *  @param  l  A std::initializer_list<value_type> of elements
         *               to be inserted.
         *
         *  Complexity similar to that of the range constructor.
         */
        void insert(std::initializer_list<value_type> l);


        /**
         *  @brief Attempts to insert a std::pair into the %hash_map.
         *  @param k    Key to use for finding a possibly existing pair in
         *                the map.
         *  @param obj  Argument used to generate the .second for a pair
         *                instance.
         *
         *  @return  A pair, of which the first element is an iterator that
         *           points to the possibly inserted pair, and the second is
         *           a bool that is true if the pair was actually inserted.
         *
         *  This function attempts to insert a (key, value) %pair into the
         *  %hash_map. An %hash_map relies on unique keys and thus a
         *  %pair is only inserted if its first element (the key) is not already
         *  present in the %hash_map.
         *  If the %pair was already in the %hash_map, the .second of
         *  the %pair is assigned from obj.
         *
         *  Insertion requires amortized constant time.
         */
        template <typename _Obj>
        std::pair<iterator, bool> insert_or_assign(const key_type& k, _Obj&& obj);

        // move-capable overload
        template <typename _Obj>
        std::pair<iterator, bool> insert_or_assign(key_type&& k, _Obj&& obj);

        //@{
        /**
         *  @brief Erases an element from an %hash_map.
         *  @param  position  An iterator pointing to the element to be erased.
         *  @return An iterator pointing to the element immediately following
         *          @a position prior to the element being erased. If no such
         *          element exists, end() is returned.
         *
         *  This function erases an element, pointed to by the given iterator,
         *  from an %hash_map.
         *  Note that this function only erases the element, and that if the
         *  element is itself a pointer, the pointed-to memory is not touched in
         *  any way.  Managing the pointer is the user's responsibility.
         */
        iterator erase(const_iterator position);

        // LWG 2059.
        iterator erase(iterator position);
        //@}

        /**
         *  @brief Erases elements according to the provided key.
         *  @param  x  Key of element to be erased.
         *  @return  The number of elements erased.
         *
         *  This function erases all the elements located by the given key from
         *  an %hash_map. For an %hash_map the result of this function
         *  can only be 0 (not present) or 1 (present).
         *  Note that this function only erases the element, and that if the
         *  element is itself a pointer, the pointed-to memory is not touched in
         *  any way.  Managing the pointer is the user's responsibility.
         */
        size_type erase(const key_type& x);

        /**
         *  @brief Erases a [first,last) range of elements from an
         *  %hash_map.
         *  @param  first  Iterator pointing to the start of the range to be
         *                  erased.
         *  @param last  Iterator pointing to the end of the range to
         *                be erased.
         *  @return The iterator @a last.
         *
         *  This function erases a sequence of elements from an %hash_map.
         *  Note that this function only erases the elements, and that if
         *  the element is itself a pointer, the pointed-to memory is not touched
         *  in any way.  Managing the pointer is the user's responsibility.
         */
        iterator erase(const_iterator first, const_iterator last);

        /**
         *  Erases all elements in an %hash_map.
         *  Note that this function only erases the elements, and that if the
         *  elements themselves are pointers, the pointed-to memory is not touched
         *  in any way.  Managing the pointer is the user's responsibility.
         */
        void clear() noexcept;

        /**
         *  @brief  Swaps data with another %hash_map.
         *  @param  x  An %hash_map of the same element and allocator
         *  types.
         *
         *  This exchanges the elements between two %hash_map in constant
         *  time.
         *  Note that the global std::swap() function is specialized such that
         *  std::swap(m1,m2) will feed to this function.
         */
        void swap(hash_map& x) {
            std::swap(this->mData, x.mData);
            std::swap(this->mIsSet, x.mIsSet);
            std::swap(this->mCount, x.mCount);
            std::swap(this->maxLoadFactor, x.maxLoadFactor);
            std::swap(this->mAlloc, x.mAlloc);
            std::swap(this->mKeyEqual, x.mKeyEqual);
            std::swap(this->mHash, x.mHash);
        }

        template<typename _H2, typename _P2>
        void merge(hash_map<K, T, _H2, _P2, Alloc>& source);

        template<typename _H2, typename _P2>
        void merge(hash_map<K, T, _H2, _P2, Alloc>&& source);

        // observers.

        ///  Returns the hash functor object with which the %hash_map was
        ///  constructed.
        Hash hash_function() const {
            return mHash;
        }

        ///  Returns the key comparison object with which the %hash_map was
        ///  constructed.
        Pred key_eq() const {
            return mKeyEqual;
        }

        // lookup.

        //@{
        /**
         *  @brief Tries to locate an element in an %hash_map.
         *  @param  x  Key to be located.
         *  @return  Iterator pointing to sought-after element, or end() if not
         *           found.
         *
         *  This function takes a key and tries to locate the element with which
         *  the key matches.  If successful the function returns an iterator
         *  pointing to the sought after element.  If unsuccessful it returns the
         *  past-the-end ( @c end() ) iterator.
         */
        iterator find(const key_type& x);

        const_iterator find(const key_type& x) const;
        //@}

        /**
         *  @brief  Finds the number of elements.
         *  @param  x  Key to count.
         *  @return  Number of elements with specified key.
         *
         *  This function only makes sense for %unordered_multimap; for
         *  %hash_map the result will either be 0 (not present) or 1
         *  (present).
         */
        size_type count(const key_type& x) const {
            return contains(x);
        }

        /**
         *  @brief  Finds whether an element with the given key exists.
         *  @param  x  Key of elements to be located.
         *  @return  True if there is any element with the specified key.
         */
        bool contains(const key_type& x) const {
            size_type indx = innerFind(x);
            return (mIsSet[indx] == 1);
        }

        //@{
        /**
         *  @brief  Subscript ( @c [] ) access to %hash_map data.
         *  @param  k  The key for which data should be retrieved.
         *  @return  A reference to the data of the (key,data) %pair.
         *
         *  Allows for easy lookup with the subscript ( @c [] )operator.  Returns
         *  data associated with the key specified in subscript.  If the key does
         *  not exist, a pair with that key is created using default values, which
         *  is then returned.
         *
         *  Lookup requires constant time.
         */
        mapped_type& operator[](const key_type& k) {
            checkForRehash();

            size_type indx = innerFind(k);
            if (mIsSet[indx] != 1) {
                new(mData + indx) value_type(k, mapped_type{});
                mIsSet[indx] = 1;
                mCount++;
            }

            return mData[indx].second;
        }

        mapped_type& operator[](key_type&& k) {
            return (*this)[k];
        }
        //@}

        //@{
        /**
         *  @brief  Access to %hash_map data.
         *  @param  k  The key for which data should be retrieved.
         *  @return  A reference to the data whose key is equal to @a k, if
         *           such a data is present in the %hash_map.
         *  @throw  std::out_of_range  If no such data is present.
         */
        mapped_type& at(const key_type& k) {
            size_type indx = innerFind(k);
            if (mIsSet[indx] != 1) {
                throw std::out_of_range("This key is not presented in map");
            }

            return mData[indx].second;
        }

        const mapped_type& at(const key_type& k) const {
            size_type indx = innerFind(k);
            if (mIsSet[indx] != 1) {
                throw std::out_of_range("This key is not presented in map");
            }

            return mData[indx].second;
        }
        //@}

        // bucket interface.

        /// Returns the number of buckets of the %hash_map.
        size_type bucket_count() const noexcept {
            return mIsSet.size();
        }

        /*
        * @brief  Returns the bucket index of a given element.
        * @param  _K  A key instance.
        * @return  The key bucket index.
        */
        size_type bucket(const key_type& _K) const {
            size_type indx = innerFind(_K);
            if (mIsSet[indx] != 1) {
                throw std::out_of_range("This key is not presented in map");
            }
            return indx;
        }

        // hash policy.

        /// Returns the average number of elements per bucket.
        float load_factor() const noexcept {
            return mCount * 1.0 / mIsSet.size();
        }

        /// Returns a positive number that the %hash_map tries to keep the
        /// load factor less than or equal to.
        float max_load_factor() const noexcept {
            return maxLoadFactor;
        }

        /**
         *  @brief  Change the %hash_map maximum load factor.
         *  @param  z The new maximum load factor.
         */
        void max_load_factor(float z) {
            if (z <= 0.0 || z >= 1)
                throw invalid_argument("Load factor must be positive and less than 1");
            maxLoadFactor = z;
            checkForRehash();
        }

        /**
         *  @brief  May rehash the %hash_map.
         *  @param  n The new number of buckets.
         *
         *  Rehash will occur only if the new number of buckets respect the
         *  %hash_map maximum load factor.
         */
        void rehash(size_type n) {
            hash_map newHashMap(n);
            for (size_type i = 0; i < mIsSet.size(); i++) {
                if (mIsSet[i] == 1) {
                    newHashMap[mData[i].first] = mData[i].second;
                }
            }
            swap(newHashMap);
        }

        /**
         *  @brief  Prepare the %hash_map for a specified number of
         *          elements.
         *  @param  n Number of elements required.
         *
         *  Same as rehash(ceil(n / max_load_factor())).
         */
        void reserve(size_type n) {
            rehash(ceil(n / maxLoadFactor));
        }

        bool operator==(const hash_map& other) const {
            if (this->size() != other.size())
                return false;

            for (size_type i = 0; i < mIsSet.size(); i++) {
                if (mIsSet[i] == 1 && (!other.contains(mData[i].first)
                                    || other.at(mData[i].first) != mData[i].second)) {
                    return false;
                }
            }
            return true;
        }


    private:
        size_type mCount = 0;
        Alloc mAlloc;
        hasher mHash;
        key_equal mKeyEqual;

        std::vector<char> mIsSet;
        value_type* mData;

        float maxLoadFactor = 0.4;

        const size_t capacityGrowth = 6;
        

        size_type innerFind(const key_type& k) const {
            size_type indx = mHash(k) % mIsSet.size();
            size_type d = 1;
            while (!mKeyEqual(mData[indx].first, k) && mIsSet[indx] == 1) {
                indx = (indx + d) % mIsSet.size();
            }

            return indx;
        }

        bool checkForRehash() {
            if (mIsSet.size() == 0) {
                rehash(1);
                return true;
            }
            if (load_factor() >= maxLoadFactor) {
                rehash(mIsSet.size() * capacityGrowth);
                return true;
            }
            return false;
        }
    };

} // namespace fefu