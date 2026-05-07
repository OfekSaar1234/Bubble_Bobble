// Copyright (C) 2026 Moshe Sulamy

#pragma once
#include <cstdlib> // ADDED TO FIX COMPILER ERRORS
#include <cstdint>
#include <type_traits>
#include <algorithm> // ADDED TO FIX COMPILER ERRORS

namespace bagel
{
	/**** Parameters ****/
	constexpr int	MaxComponents = 6;
	constexpr bool	DynamicBags = true;
	/** end parameters **/

	using id_type = int;
	struct ent_type { id_type id; };
	using mask_type =
		std::conditional_t<MaxComponents<=8, std::uint_fast8_t,
		std::conditional_t<MaxComponents<=16, std::uint_fast16_t,
		std::conditional_t<MaxComponents<=32, std::uint_fast32_t,
			std::uint_fast64_t>>>;


	struct NoInstance {	NoInstance() = delete; };
	struct NoCopy {
		NoCopy() = default; // default constructor
		NoCopy(const NoCopy&) = delete;
		NoCopy& operator=(const NoCopy&) = delete;
	};

	template <class T, int N>
	class StaticBag
	{
	public:
		int size() const { return _size; }
		static void ensure(int) {}
		void push(const T& val) { _arr[_size++] = val; }
		T pop() { return _arr[--_size]; }

		T& operator[](int idx) { return _arr[idx]; }
		const T& operator[](int idx) const { return _arr[idx]; }
	private:
		T	_arr[N];
		int _size = 0;
	};
	template <class T, int N>
	class DynamicBag : NoCopy
	{
	public:
		int size() const { return _size; }
		void ensure(int new_capacity)
		{
			if (new_capacity > _capacity)
			{
				_capacity = std::max(_capacity*2, new_capacity);
				_arr = static_cast<T*>(
					realloc(_arr, sizeof(T)*_capacity));
			}
		}
		~DynamicBag()
		{
			free(_arr);
		}
		void push(const T& val) { _arr[_size++] = val; }
		T pop() { return _arr[--_size]; }

		T& operator[](int idx) { return _arr[idx]; }
		const T& operator[](int idx) const { return _arr[idx]; }
	private:
		T*		_arr = static_cast<T*>(malloc(sizeof(T) * N));
		int		_capacity = N;
		int		_size = 0;
	};
	template <class T, int N>
	using Bag = std::conditional_t<DynamicBags, DynamicBag<T,N>, StaticBag<T,N>>;

	// Forward Declarations
	class World;
	class Query;

	// Component Interface
	template <class T>
	struct Component { static constexpr int Bit = -1; };

	// Storage Interfaces
	template <class T>
	class SparseStorage final : NoInstance
	{
		static Bag<T, 32>	_data;
		static Bag<int, 32>	_sparse; // Maps Entity ID -> Index in _data
	public:
		static void attach(const ent_type e, const T& t)
		{
			_sparse.ensure(e.id + 1);
			_sparse[e.id] = _data.size();
			_data.push(t);
		}
		static void detach(const ent_type e) {} // TODO: proper swap-pop removal
		static T& get(const ent_type e) { return _data[_sparse[e.id]]; }
	};
	template <class T> Bag<T, 32> SparseStorage<T>::_data;
	template <class T> Bag<int, 32> SparseStorage<T>::_sparse;

	template <class T>
	class TaggedStorage final : NoInstance
	{
	public:
		static void attach(const ent_type, const T&) {}
		static void detach(const ent_type) {}
		static T get(const ent_type) { return T{}; } // Tag components hold no data
	};

	template <class T>
	class PackedStorage final : NoInstance
	{
		static Bag<T, 128>	_data; // Packed storage, continuous array
	public:
		static void attach(const ent_type e, const T& t)
		{
			_data.ensure(e.id + 1);
			_data[e.id] = t;
		}
		static void detach(const ent_type) {} // Entity retains unused slot
		static T& get(const ent_type e) { return _data[e.id]; }
	};
	template <class T> Bag<T, 128> PackedStorage<T>::_data;

	template <class T>
	class StackStorage final : NoInstance
	{
		static T			_data[128]; // Fixed size stack-based array
	public:
		static void attach(const ent_type e, const T& t) { _data[e.id] = t; }
		static void detach(const ent_type) {} // Retains unused slot
		static T& get(const ent_type e) { return _data[e.id]; }
	};
	template <class T> T StackStorage<T>::_data[128];


	// Default to Sparse
	template <class T> struct Storage : NoInstance { using type = SparseStorage<T>; };

	// Mask operations
	struct Mask
	{
		mask_type _mask = 0;
		bool test(const int b) const { return _mask & (1 << b); }
		bool test(const Mask m) const { return (_mask & m._mask) == m._mask; }
		void set(const int b) { _mask |= (1 << b); }
		void clear(const int b) { _mask &= ~(1 << b); }
	};

	// The ECS World
	class World final : NoInstance
	{
		friend class Query;
		static Bag<mask_type, 128> _masks;
		static int _maxId;
	public:
		static ent_type create()
		{
			_masks.ensure(_maxId + 1);
			_masks[_maxId] = 0;
			return ent_type{ _maxId++ };
		}
		static void destroy(const ent_type e)
		{
			_masks[e.id] = 0; // Entity is now dead (0 mask)
		}
		template <class T>
		static void attach(const ent_type e, const T& t)
		{
			Storage<T>::type::attach(e, t);
			_masks[e.id] |= (1 << Component<T>::Bit);
		}
		template <class T>
		static void detach(const ent_type e)
		{
			Storage<T>::type::detach(e);
			_masks[e.id] &= ~(1 << Component<T>::Bit);
		}
		static const Mask mask(const ent_type e)
		{
			return Mask{ _masks[e.id] };
		}
		static int maxId() { return _maxId; }
	};
	Bag<mask_type, 128> World::_masks;
	int World::_maxId = 0;

	// Query Builder
	class QueryBuilder
	{
		Mask m;
	public:
		template <class T> QueryBuilder& with() { m.set(Component<T>::Bit); return *this; }
		Mask build() const { return m; }
	};

	// The Query Result/Iterator
	class Query
	{
		Mask _q;
		ent_type _ent{0};
		void advance()
		{
			while (_ent.id < World::maxId() && !World::mask(_ent).test(_q))
				_ent.id++;
		}
	public:
		Query(const Mask q) : _q(q) { advance(); }

		ent_type entity() const { return _ent; }
		const Mask& mask() const { return World::mask(_ent); }

		template <class T>
		T& get() const { return Storage<T>::type::get(_ent); }

		template <class T>
		bool has() const { return mask().test(Component<T>::Bit); }
		bool test(const Mask& m) const { return mask().test(m); }

		bool eof() const { return _ent.id >= World::maxId(); }
		void next() { _ent.id++; advance(); }
	};
}