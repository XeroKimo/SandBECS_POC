module;

#include <utility>
#include <memory>
#include <concepts>
#include <ranges>
#include <typeinfo>

export module SandBECS;
export import :SharedObject;

namespace SandBECS
{
	export class Component;
	export class GameObject;
	export class PolymorphicGameObjectContainer;

	export template<std::derived_from<Component> Ty>
		class Owner
	{
		template<std::derived_from<Component> Ty2>
		friend class Owner;

		template<std::derived_from<Component> Ty2>
		friend class TempOwner;

		template<std::derived_from<Component> Ty2>
		friend class Weak;

		template<class Derived, class Base>
		friend Owner<Derived> DynamicCast(Owner<Base>&& other) noexcept;
	private:
		SharedPtr<Ty> m_component;

	public:
		Owner() noexcept = default;
		Owner(std::nullptr_t) noexcept {}
		Owner(SharedPtr<Ty> component) noexcept :
			m_component(std::move(component))
		{

		}

		Owner(const Owner& other) noexcept = delete;
		Owner(Owner&& other) noexcept = default;
		~Owner() = default;

		template<class Ty2>
		Owner(const Owner<Ty2>& other) = delete;

		template<class Ty2>
		Owner(Owner<Ty2>&& other) noexcept :
			m_component(std::move(other.m_component))
		{

		}

	public:
		operator bool() const noexcept { return m_component; }

		Ty* Get() const noexcept { return m_component.Get(); }
		Ty* operator->() const noexcept { return m_component.Get(); }
		Ty& operator*() const noexcept { return *m_component; }

	public:
		Owner& operator=(const Owner& other) noexcept = delete;
		Owner& operator=(Owner&& other) noexcept = default;

		Owner& operator=(std::nullptr_t) noexcept
		{
			m_component = nullptr;
			return *this;
		}

		template<class Ty2>
		Owner& operator=(const Owner<Ty2>& other) = delete;

		template<class Ty2>
		Owner& operator=(Owner<Ty2>&& other)
		{
			m_component = std::move(other.m_component);
			return *this;
		}

	public:
		template<class Ty2>
		bool operator==(Owner<Ty2>&& other) const noexcept { return m_component == other.m_component; }
		template<class Ty2>
		bool operator!=(Owner<Ty2>&& other) const noexcept { return m_component != other.m_component; }

		bool operator==(std::nullptr_t) const noexcept { return m_component == nullptr; }
		bool operator!=(std::nullptr_t) const noexcept { return m_component != nullptr; }
	};

	export template<std::derived_from<Component> Ty>
		class TempOwner
	{
		template<std::derived_from<Component> Ty2>
		friend class TempOwner;

		template<std::derived_from<Component> Ty2>
		friend class Weak;

		using game_object_ty = std::conditional_t<std::is_const_v<Ty>, const GameObject, GameObject>;

		template<class Derived, class Base>
		friend TempOwner<Derived> DynamicCast(const TempOwner<Base>& other) noexcept;

		template<class Derived, class Base>
		friend TempOwner<Derived> DynamicCast(TempOwner<Base>&& other) noexcept;
	private:
		SharedPtr<game_object_ty> m_gameObject;
		SharedPtr<Ty> m_component;

	public:
		TempOwner() = default;
		TempOwner(std::nullptr_t) noexcept {}

		template<class Ty2>
		TempOwner(SharedPtr<Ty2> component) noexcept;

		template<class Ty2>
		TempOwner(const Owner<Ty2>& component) noexcept :
			TempOwner(component.m_component)
		{

		}

		template<class Ty2>
		TempOwner(const TempOwner<Ty2>& other) noexcept :
			m_gameObject(other.m_gameObject),
			m_component(other.m_component)
		{

		}

		template<class Ty2>
		TempOwner(TempOwner<Ty2>&& other) noexcept :
			m_gameObject(std::move(other.m_gameObject)),
			m_component(std::move(other.m_component))
		{

		}

	public:
		operator bool() const noexcept { return m_component; }

		Ty* Get() const noexcept { return m_component.Get(); }
		Ty* operator->() const noexcept { return m_component.Get(); }
		Ty& operator*() const noexcept { return *m_component; }

		void Swap(TempOwner& other) noexcept
		{
			m_gameObject.Swap(other.m_gameObject);
			m_component.Swap(other.m_component);
		}
	public:
		TempOwner& operator=(std::nullptr_t) noexcept
		{
			m_gameObject = nullptr;
			m_component = nullptr;
			return *this;
		}

		template<class Ty2>
		TempOwner& operator=(const Owner<Ty2>& component) noexcept
		{
			TempOwner(component).Swap(*this);
			return *this;
		}

		template<class Ty2>
		TempOwner& operator=(const TempOwner<Ty2>& other) noexcept
		{
			TempOwner(other).Swap(*this);
			return *this;
		}

		template<class Ty2>
		TempOwner& operator=(TempOwner<Ty2>&& other) noexcept
		{
			TempOwner(std::move(other)).Swap(*this);
			return *this;
		}
	};

	export template<std::derived_from<Component> Ty>
		class Weak
	{
		template<std::derived_from<Component> Ty2>
		friend class Weak;
	private:
		WeakPtr<Ty> m_component;

	public:
		Weak() noexcept = default;
		Weak(std::nullptr_t) noexcept {}
		Weak(const Weak& other) noexcept = default;
		Weak(Weak&& other) noexcept = default;
		~Weak() noexcept = default;

		template<class Ty2>
		Weak(const Weak<Ty2>& other) noexcept :
			m_component(other.m_component)
		{

		}
		template<class Ty2>
		Weak(Weak<Ty2>&& other) noexcept :
			m_component(std::move(other.m_component))
		{
		}

		template<class Ty2>
		Weak(const Owner<Ty2>& owner) noexcept :
			m_component(owner.m_component)
		{

		}

		template<class Ty2>
		Weak(TempOwner<Ty2> owner) noexcept :
			m_component(std::move(owner.m_component))
		{

		}

		template<class Ty2>
		Weak(SharedPtr<Ty2> component) noexcept :
			m_component(std::move(component))
		{

		}

		template<class Ty2>
		Weak(WeakPtr<Ty2> component) noexcept :
			m_component(std::move(component))
		{

		}

	public:
		Weak& operator=(std::nullptr_t) noexcept
		{
			m_component = nullptr;
			return *this;
		}
		Weak& operator=(const Weak& other)noexcept = default;
		Weak& operator=(Weak&& other) noexcept = default;


		template<class Ty2>
		Weak& operator=(const Weak<Ty2>& other) noexcept
		{
			m_component = other;
			return *this;
		}
		template<class Ty2>
		Weak& operator=(Weak<Ty2>&& other) noexcept
		{
			m_component = std::move(other);
			return *this;
		}

		template<class Ty2>
		Weak& operator=(const Owner<Ty2>& owner) noexcept
		{
			m_component = owner.m_component;
			return *this;
		}

		template<class Ty2>
		Weak& operator=(TempOwner<Ty2> owner) noexcept
		{
			m_component = owner.m_component;
			return *this;
		}
	public:
		TempOwner<Ty> Lock() const noexcept;
	};

	export template<class Derived, class Base>
		TempOwner<Derived> DynamicCast(const TempOwner<Base>& other) noexcept
	{
		return TempOwner<Derived>(DynamicPointerCast<Derived>(other.m_component));
	}

	export template<class Derived, class Base>
		TempOwner<Derived> DynamicCast(TempOwner<Base>&& other) noexcept
	{
		return TempOwner<Derived>(DynamicPointerCast<Derived>(std::move(other.m_component)));
	}

	export template<class Derived, class Base>
		Owner<Derived> DynamicCast(Owner<Base>&& other) noexcept
	{
		return Owner<Derived>(DynamicPointerCast<Derived>(std::move(other.m_component)));
	}

	class Component : public EnableSharedFromThis<Component>
	{
	private:
		GameObject* m_owner;

	public:
		Component(PrivateCounter counter, GameObject& owner) noexcept :
			EnableSharedFromThis<Component>(counter),
			m_owner(&owner)
		{

		}

		Component(const Component& other) noexcept = delete;
		Component(Component&& other) noexcept :
			EnableSharedFromThis(std::move(other))
		{
			std::swap(m_owner, other.m_owner);
		}
		virtual ~Component() = default;

	public:
		Component& operator=(const Component& other) noexcept = delete;
		Component& operator=(Component&& other) noexcept
		{
			m_owner = std::exchange(other.m_owner, nullptr);
			return *this;
		}

	public:
		template<std::derived_from<GameObject> Ty, class... Params>
		SharedPtr<Ty> CreateGameObject(Params&&... params);

		SharedPtr<GameObject> GetOwner() noexcept;

		SharedPtr<const GameObject> GetOwner() const noexcept;

		TempOwner<Component> SharedFromThis() noexcept
		{
			return TempOwner<Component>(EnableSharedFromThis<Component>::SharedFromThis());
		}

		TempOwner<const Component> SharedFromThis() const noexcept
		{
			return TempOwner<const Component>(EnableSharedFromThis<Component>::SharedFromThis());
		}

		Weak<Component> WeakFromThis() noexcept
		{
			return Weak<Component>(EnableSharedFromThis<Component>::WeakFromThis());
		}

		Weak<const Component> WeakFromThis() const noexcept
		{
			return Weak<const Component>(EnableSharedFromThis<Component>::WeakFromThis());
		}
	private:
		using EnableSharedFromThis<Component>::SharedFromThis;
		using EnableSharedFromThis<Component>::WeakFromThis;

	};

	class PolymorphicGameObjectContainer;

	class GameObject : public EnableSharedFromThis<GameObject>
	{
		friend class GameObject;

	private:
		PolymorphicGameObjectContainer* m_container;
		std::unique_ptr<GameObject> m_class;

	public:
		GameObject(PrivateCounter counter, PolymorphicGameObjectContainer& container) :
			EnableSharedFromThis<GameObject>(counter),
			m_container(&container)
		{

		}

	public:
		template<std::derived_from<GameObject> Ty, class... Params>
		SharedPtr<Ty> CreateGameObject(Params&&... params);

		template<std::derived_from<Component> Ty, class... Params>
		Owner<Ty> CreateComponent(Params&&... params);
	};

	export class PolymorphicGameObjectContainer
	{
	public:
		template<std::derived_from<GameObject> Ty, class... Params>
		SharedPtr<Ty> CreateGameObject(Params&&... params)
		{
			SharedPtr<Ty> gameObject = MakeShared<Ty>(*this, std::forward<Params>(params)...);
			OnGameObjectCreated(gameObject, typeid(Ty));
			return gameObject;
		}

		template<std::derived_from<Component> Ty, class... Params>
		Owner<Ty> CreateComponent(GameObject& gameObject, Params&&... params)
		{
			Owner<Ty> component = MakeShared<Ty>(gameObject, std::forward<Params>(params)...);
			OnComponentCreated(component, typeid(Ty));
			return component;
		}

	protected:
		virtual void OnGameObjectCreated(const SharedPtr<GameObject> gameObject, const std::type_info& type) = 0;
		virtual void OnComponentCreated(const TempOwner<Component> component, const std::type_info& type) = 0;
	};

	SharedPtr<GameObject> Component::GetOwner() noexcept
	{
		return m_owner->SharedFromThis();
	}

	SharedPtr<const GameObject> Component::GetOwner() const noexcept
	{
		return m_owner->SharedFromThis();
	}

	template<std::derived_from<GameObject> Ty, class ...Params>
	SharedPtr<Ty> Component::CreateGameObject(Params&& ...params)
	{
		return m_owner->CreateGameObject<Ty>(std::forward<Params>(params)...);
	}

	template<std::derived_from<Component> Ty>
	TempOwner<Ty> Weak<Ty>::Lock() const noexcept
	{
		return TempOwner<Ty>(m_component.Lock());
	}

	template<std::derived_from<Component> Ty>
	template<class Ty2>
	TempOwner<Ty>::TempOwner(SharedPtr<Ty2> component) noexcept
	{
		if(!component)
			return;

		m_gameObject = component->GetOwner();

		if(m_gameObject)
			m_component = std::move(component);
	}

	template<std::derived_from<GameObject> Ty, class ...Params>
	SharedPtr<Ty> GameObject::CreateGameObject(Params && ...params)
	{
		return m_container->CreateGameObject<Ty>(std::forward<Params>(params)...);
	}

	template<std::derived_from<Component> Ty, class ...Params>
	Owner<Ty> GameObject::CreateComponent(Params && ...params)
	{
		return m_container->CreateComponent<Ty>(*this, std::forward<Params>(params)...);
	}

	template<class Ty1, class Ty2>
	bool operator==(const Owner<Ty1>& lh, const TempOwner<Ty2>& rh) noexcept
	{
		return lh.Get() == rh.Get();
	}

	template<class Ty1, class Ty2>
	bool operator==(const TempOwner<Ty1>& lh, const Owner<Ty2>& rh) noexcept
	{
		return lh.Get() == rh.Get();
	}

	template<class Ty1, class Ty2>
	bool operator!=(const Owner<Ty1>& lh, const TempOwner<Ty2>& rh) noexcept
	{
		return lh.Get() != rh.Get();
	}

	template<class Ty1, class Ty2>
	bool operator!=(const TempOwner<Ty1>& lh, const Owner<Ty2>& rh) noexcept
	{
		return lh.Get() != rh.Get();
	}
}