module;

#include <memory>

export module SandBECS:SharedObject;

namespace SandBECS
{
	export template<class Ty>
		class SharedPtr;

	export template<class Ty>
		class WeakPtr;

	class ReferenceCounter;

	export class PrivateCounter
	{
		friend class ReferenceCounter;

		template<class Ty>
		friend class WeakPtr;

		ReferenceCounter* m_referenceCounter;

		PrivateCounter(ReferenceCounter& referenceCounter) noexcept :
			m_referenceCounter(&referenceCounter)
		{

		}
	};

	export class ReferenceCounter
	{
	public:
		using underlying_type = int;
		using ref_count_type = underlying_type;

	private:
		ref_count_type m_strongOwner = 1;
		ref_count_type m_weakOwner = 1;

	public:
		ReferenceCounter() = default;
		ReferenceCounter(const ReferenceCounter&) = delete;
		ReferenceCounter(ReferenceCounter&&) noexcept = default;
		~ReferenceCounter() = default;

		ReferenceCounter& operator=(const ReferenceCounter&) = delete;
		ReferenceCounter& operator=(ReferenceCounter&&) noexcept = default;

	public:
		void IncrementStrongReference() noexcept
		{
			++m_strongOwner;
		}

		void DecrementStrongReference() noexcept
		{
			--m_strongOwner;

			if(m_strongOwner == 0)
			{
				DeleteObject();
				DecrementWeakReference();
			}
		}

		void IncrementWeakReference() noexcept
		{
			++m_weakOwner;
		}

		void DecrementWeakReference() noexcept
		{
			--m_weakOwner;

			if(m_weakOwner == 0)
				DeleteThis();
		}

		underlying_type UseCount() const noexcept
		{
			return m_strongOwner;
		}

	protected:
		virtual void DeleteObject() noexcept = 0;
		virtual void DeleteThis() noexcept = 0;
		PrivateCounter MakePrivate() noexcept { return PrivateCounter(*this); }
	};

	export template<class Ty>
		class ReferenceCountedObject : public ReferenceCounter
	{
	private:
		union
		{
			Ty m_storage;
		};

	public:
		template<class... Params>
		ReferenceCountedObject(Params&&... params)
		{
			std::construct_at(&m_storage, MakePrivate(), std::forward<Params>(params)...);
		}

		~ReferenceCountedObject() {}

	public:
		Ty& Get() { return m_storage; }

	protected:
		void DeleteObject() noexcept override
		{
			std::destroy_at(&m_storage);
		}

		void DeleteThis() noexcept override
		{
			delete this;
		}
	};

	template<class Ty>
	class SharedPtr
	{
		template<class Ty2>
		friend class SharedPtr;

		template<class Ty2>
		friend class WeakPtr;

	private:
		Ty* m_ptr = nullptr;
		ReferenceCounter* m_referenceCounter = nullptr;

	public:
		static SharedPtr TakeOwner(Ty& ptr, ReferenceCounter& referenceCounter) noexcept
		{
			SharedPtr p;
			p.m_ptr = &ptr;
			p.m_referenceCounter = &referenceCounter;
			return p;
		}
		static SharedPtr Copy(Ty* ptr, ReferenceCounter* referenceCounter) noexcept
		{
			if(!referenceCounter || referenceCounter->UseCount() == 0)
				return {};

			referenceCounter->IncrementStrongReference();
			SharedPtr p;
			p.m_ptr = ptr;
			p.m_referenceCounter = referenceCounter;
			return p;
		}

	public:
		SharedPtr() noexcept = default;
		SharedPtr(std::nullptr_t) noexcept {}
		SharedPtr(const SharedPtr& other) noexcept
		{
			if(other.m_referenceCounter)
			{
				other.m_referenceCounter->IncrementStrongReference();

				m_ptr = other.m_ptr;
				m_referenceCounter = other.m_referenceCounter;
			}
		}

		SharedPtr(SharedPtr&& other) noexcept
		{
			m_ptr = std::exchange(other.m_ptr, nullptr);
			m_referenceCounter = std::exchange(other.m_referenceCounter, nullptr);
		}

		template<class Ty2>
		SharedPtr(const SharedPtr<Ty2>& other) noexcept
		{
			if(other.m_referenceCounter)
			{
				other.m_referenceCounter->IncrementStrongReference();

				m_ptr = other.m_ptr;
				m_referenceCounter = other.m_referenceCounter;
			}
		}

		template<class Ty2>
		SharedPtr(SharedPtr<Ty2>&& other) noexcept
		{
			m_ptr = std::exchange(other.m_ptr, nullptr);
			m_referenceCounter = std::exchange(other.m_referenceCounter, nullptr);
		}

		template<class Ty2>
		SharedPtr(const SharedPtr<Ty2>& other, Ty* aliasPtr) noexcept
		{
			if(other.m_referenceCounter)
			{
				other.m_referenceCounter->IncrementStrongReference();

				m_ptr = aliasPtr;
				m_referenceCounter = other.m_referenceCounter;
			}
		}

		template<class Ty2>
		SharedPtr(SharedPtr<Ty2>&& other, Ty* aliasPtr) noexcept
		{
			std::exchange(other.m_ptr, nullptr);
			m_ptr = aliasPtr;
			m_referenceCounter = std::exchange(other.m_referenceCounter, nullptr);
		}

		~SharedPtr()
		{
			if(m_referenceCounter)
			{
				m_referenceCounter->DecrementStrongReference();
			}
		}

	public:
		Ty* Get() const noexcept { return m_ptr; }
		Ty* operator->() const noexcept { return m_ptr; }
		Ty& operator*() const noexcept { return *m_ptr; }

		operator bool() const noexcept { return m_ptr != nullptr; }

	public:
		SharedPtr& operator=(std::nullptr_t) noexcept
		{
			SharedPtr().Swap(*this);
			return *this;
		}

		SharedPtr& operator=(const SharedPtr& other) noexcept
		{
			SharedPtr(other).Swap(*this);
			return *this;
		}

		SharedPtr& operator=(SharedPtr&& other) noexcept
		{
			SharedPtr(std::move(other)).Swap(*this);
			return *this;
		}

		template<class Ty2>
		SharedPtr& operator=(const SharedPtr<Ty2>& other) noexcept
		{
			SharedPtr(other).Swap(*this);
			return *this;
		}

		template<class Ty2>
		SharedPtr& operator=(SharedPtr<Ty2>&& other) noexcept
		{
			SharedPtr(std::move(other)).Swap(*this);
			return *this;
		}

	public:
		bool operator==(std::nullptr_t) const noexcept { return m_ptr == nullptr; }
		bool operator!=(std::nullptr_t) const noexcept { return m_ptr == nullptr; }

		template<class Ty2>
		bool operator==(const SharedPtr<Ty2>& other) const noexcept
		{
			return Get() == other.Get();
		}

		template<class Ty2>
		bool operator!=(const SharedPtr<Ty2>& other) const noexcept
		{
			return Get() != other.Get();
		}

	public:
		void Swap(SharedPtr& other)
		{
			std::swap(m_ptr, other.m_ptr);
			std::swap(m_referenceCounter, other.m_referenceCounter);
		}
	};

	export template<class Ty, class... Params>
		SharedPtr<Ty> MakeShared(Params&&... params)
	{
		auto refCounter = new ReferenceCountedObject<Ty>(std::forward<Params>(params)...);
		return SharedPtr<Ty>::TakeOwner(refCounter->Get(), *refCounter);
	}

	export template<class Derived, class Base>
		SharedPtr<Derived> StaticPointerCast(const SharedPtr<Base>& ptr)
	{
		if(!ptr)
			return {};

		Derived* derivedPtr = static_cast<Derived*>(ptr.Get());
		return SharedPtr<Derived>(ptr, derivedPtr);
	}

	export template<class Derived, class Base>
		SharedPtr<Derived> StaticPointerCast(SharedPtr<Base>&& ptr)
	{
		if(!ptr)
			return {};

		Derived* derivedPtr = static_cast<Derived*>(ptr.Get());
		return SharedPtr<Derived>(std::move(ptr), derivedPtr);
	}

	export template<class Derived, class Base>
		SharedPtr<Derived> DynamicPointerCast(const SharedPtr<Base>& ptr)
	{
		Derived* derivedPtr = dynamic_cast<Derived*>(ptr.Get());

		if(!derivedPtr)
			return {};

		return SharedPtr<Derived>(ptr, derivedPtr);
	}

	export template<class Derived, class Base>
		SharedPtr<Derived> DynamicPointerCast(SharedPtr<Base>&& ptr)
	{
		Derived* derivedPtr = dynamic_cast<Derived*>(ptr.Get());

		if(!derivedPtr)
			return {};

		return SharedPtr<Derived>(std::move(ptr), derivedPtr);
	}

	template<class Ty>
	class WeakPtr
	{
		template<class Ty2>
		friend class WeakPtr;

	private:
		Ty* m_ptr = nullptr;
		ReferenceCounter* m_referenceCounter = nullptr;

	public:
		WeakPtr() noexcept = default;
		WeakPtr(std::nullptr_t) noexcept {}
		WeakPtr(const WeakPtr& other) noexcept
		{
			if(other.m_referenceCounter)
			{
				other.m_referenceCounter->IncrementWeakReference();

				m_ptr = other.m_ptr;
				m_referenceCounter = other.m_referenceCounter;
			}
		}
		WeakPtr(WeakPtr&& other) noexcept
		{
			m_ptr = std::exchange(other.m_ptr, nullptr);
			m_referenceCounter = std::exchange(other.m_referenceCounter, nullptr);
		}

		template<class Ty2>
		WeakPtr(const WeakPtr<Ty2>& other) noexcept
		{
			if(other.m_referenceCounter)
			{
				other.m_referenceCounter->IncrementWeakReference();

				m_ptr = other.m_ptr;
				m_referenceCounter = other.m_referenceCounter;
			}
		}

		template<class Ty2>
		WeakPtr(WeakPtr<Ty2>&& other) noexcept
		{
			m_ptr = std::exchange(other.m_ptr, nullptr);
			m_referenceCounter = std::exchange(other.m_referenceCounter, nullptr);
		}

		template<class Ty2>
		WeakPtr(const SharedPtr<Ty2>& other) noexcept
		{
			if(other.m_referenceCounter)
			{
				other.m_referenceCounter->IncrementWeakReference();

				m_ptr = other.m_ptr;
				m_referenceCounter = other.m_referenceCounter;
			}
		}

		~WeakPtr()
		{
			if(m_referenceCounter)
				m_referenceCounter->DecrementWeakReference();
		}

		WeakPtr(Ty* ptr, PrivateCounter counter)
		{
			counter.m_referenceCounter->IncrementWeakReference();
			m_ptr = ptr;
			m_referenceCounter = counter.m_referenceCounter;
		}

	public:
		WeakPtr& operator=(std::nullptr_t) noexcept
		{
			WeakPtr().Swap(*this);
			return *this;
		}
		WeakPtr& operator=(const WeakPtr& other) noexcept
		{
			WeakPtr(other).Swap(*this);
			return *this;
		}
		WeakPtr& operator=(WeakPtr&& other) noexcept
		{
			WeakPtr(std::move(other)).Swap(*this);
			return *this;
		}

		template<class Ty2>
		WeakPtr& operator=(const WeakPtr<Ty2>& other) noexcept
		{
			WeakPtr(other).Swap(*this);
			return *this;
		}

		template<class Ty2>
		WeakPtr& operator=(WeakPtr<Ty2>&& other) noexcept
		{
			WeakPtr(std::move(other)).Swap(*this);
			return *this;
		}

	public:
		void Swap(WeakPtr& other) noexcept
		{
			std::swap(m_ptr, other.m_ptr);
			std::swap(m_referenceCounter, other.m_referenceCounter);
		}

	public:
		SharedPtr<Ty> Lock() const noexcept
		{
			return SharedPtr<Ty>::Copy(m_ptr, m_referenceCounter);
		}
	};

	export template<class Ty>
		class EnableSharedFromThis
	{
		WeakPtr<Ty> m_self;
	public:
		EnableSharedFromThis(PrivateCounter counter) :
			m_self(static_cast<Ty*>(this), counter)
		{

		}

	public:
		SharedPtr<Ty> SharedFromThis() noexcept
		{
			return m_self.Lock();
		}

		SharedPtr<const Ty> SharedFromThis() const noexcept
		{
			return m_self.Lock();
		}

		WeakPtr<Ty> WeakFromThis() noexcept
		{
			return m_self;
		}

		WeakPtr<const Ty> WeakFromThis() const noexcept
		{
			return m_self;
		}
	};
}