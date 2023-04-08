#include "pch.h"
#include "CppUnitTest.h"
#include <utility>
#include <typeinfo>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

import SandBECS;

enum class State
{
	Alive,
	Dead
};

using namespace SandBECS;

namespace Microsoft
{
	namespace VisualStudio
	{
		namespace CppUnitTestFramework
		{
			template<>
			std::wstring ToString<State>(const State* q)
			{
				if(*q == State::Alive)
				{
					return L"Alive";
				}
				else
				{
					return L"Dead";
				}
			}

			template<>
			std::wstring ToString<State>(const State& q)
			{
				if(q == State::Alive)
				{
					return L"Alive";
				}
				else
				{
					return L"Dead";
				}
			}

			template<>
			std::wstring ToString<State>(State* q)
			{
				if(*q == State::Alive)
				{
					return L"Alive";
				}
				else
				{
					return L"Dead";
				}
			}
		}
	}
}

struct TestObject : public EnableSharedFromThis<TestObject>
{
	State& state;
	int aliasObj = 0;

	TestObject(PrivateCounter counter, State& state) :
		EnableSharedFromThis<TestObject>(counter),
		state(state)
	{

	}

	virtual ~TestObject()
	{
		state = State::Dead;
	}
};

struct TestObjectChild : public TestObject
{
	TestObjectChild(PrivateCounter counter, State& state) :
		TestObject(counter, state)
	{

	}


};

struct UnrelatedClass : public EnableSharedFromThis<UnrelatedClass>
{
	using EnableSharedFromThis<UnrelatedClass>::EnableSharedFromThis;
	virtual ~UnrelatedClass() = default;
};

template<class Ty>
class WeakRefCountedTest : public ReferenceCountedObject<Ty>
{
	State& state;

public:
	template<class... Params>
	WeakRefCountedTest(State& state, Params&&... params) : 
		ReferenceCountedObject<Ty>(std::forward<Params>(params)...),
		state(state)
	{

	};

	~WeakRefCountedTest()
	{
		state = State::Dead;
	}

protected:
	void DeleteThis() noexcept override
	{
		delete this;
	}
};

template<class Ty, class... Params>
SharedPtr<Ty> MakeSharedTest(State& state, Params&&... params)
{
	auto refCounter = new WeakRefCountedTest<Ty>(state, std::forward<Params>(params)...);
	return SharedPtr<Ty>::TakeOwner(refCounter->Get(), *refCounter);
}

namespace SHARED_OBJECT_TESTS
{
	TEST_CLASS(ReferenceCounterTests)
	{
	public:

		TEST_METHOD(SingleOwnerTest)
		{
			State state = State::Alive;

			ReferenceCountedObject<TestObject>* refObj = new ReferenceCountedObject<TestObject>{ state };
			refObj->DecrementStrongReference();

			Assert::AreEqual(State::Dead, state);
		}

		TEST_METHOD(MultiOwnerTest)
		{
			State state = State::Alive;

			ReferenceCountedObject<TestObject>* refObj = new ReferenceCountedObject<TestObject>{ state };

			refObj->IncrementStrongReference();
			refObj->DecrementStrongReference();
			Assert::AreEqual(State::Alive, state);
		}

		TEST_METHOD(MultiOwnerDestroyTest)
		{
			State state = State::Alive;

			ReferenceCountedObject<TestObject>* refObj = new ReferenceCountedObject<TestObject>{ state };

			refObj->IncrementStrongReference();
			refObj->DecrementStrongReference();
			Assert::AreEqual(State::Alive, state);

			refObj->DecrementStrongReference();
			Assert::AreEqual(State::Dead, state);
		}

		TEST_METHOD(SingleWeakOwnerTest)
		{
			State state = State::Alive;
			State refCounterState = State::Alive;

			WeakRefCountedTest<TestObject>* refObj = new WeakRefCountedTest<TestObject>{ refCounterState, state };
			refObj->IncrementWeakReference();

			refObj->DecrementStrongReference();
			Assert::AreEqual(State::Dead, state);
			Assert::AreEqual(State::Alive, refCounterState);

			refObj->DecrementWeakReference();
			Assert::AreEqual(State::Dead, refCounterState);
		}

		TEST_METHOD(MultiWeakOwnerTest)
		{
			State state = State::Alive;
			State refCounterState = State::Alive;

			WeakRefCountedTest<TestObject>* refObj = new WeakRefCountedTest<TestObject>{ refCounterState, state };
			refObj->IncrementWeakReference();
			refObj->IncrementWeakReference();

			refObj->DecrementStrongReference();
			Assert::AreEqual(State::Dead, state);
			Assert::AreEqual(State::Alive, refCounterState);

			refObj->DecrementWeakReference();
			Assert::AreEqual(State::Alive, refCounterState);

			refObj->DecrementWeakReference();
			Assert::AreEqual(State::Dead, refCounterState);
		}
	};

	TEST_CLASS(SharedPtrTests)
	{
	public:
		TEST_METHOD(SingleOwnerScopeTest)
		{
			State s = State::Alive;
			{
				SharedPtr<TestObject> ptr = MakeShared<TestObject>(s);
			}

			Assert::AreEqual(State::Dead, s);
		}

		TEST_METHOD(MultiOwnerTest)
		{
			State s = State::Alive;
			SharedPtr<TestObject> ptr = MakeShared<TestObject>(s);
			SharedPtr<TestObject> ptr2 = ptr;
			Assert::AreEqual(State::Alive, s);
		}

		TEST_METHOD(MultiOwnerEqualityTest)
		{
			State s = State::Alive;
			SharedPtr<TestObject> ptr = MakeShared<TestObject>(s);
			SharedPtr<TestObject> ptr2 = ptr;
			Assert::IsTrue(ptr == ptr2);
		}

		TEST_METHOD(MultiOwnerScopeTest)
		{
			State s = State::Alive;
			{
				SharedPtr<TestObject> ptr = MakeShared<TestObject>(s);
				{
					SharedPtr<TestObject> ptr2 = ptr;
				}

				Assert::AreEqual(State::Alive, s);
			}

			Assert::AreEqual(State::Dead, s);
		}

		TEST_METHOD(MoveTest)
		{
			State s = State::Alive;
			SharedPtr<TestObject> ptr = MakeShared<TestObject>(s);
			SharedPtr<TestObject> ptr2 = std::move(ptr);

			Assert::IsNull(ptr.Get());
			Assert::IsNotNull(ptr2.Get());
		}

		TEST_METHOD(PolymorphicEqualityTest)
		{
			State s = State::Alive;
			SharedPtr<TestObjectChild> ptr = MakeShared<TestObjectChild>(s);
			SharedPtr<TestObject> ptr2 = ptr;

			Assert::IsTrue(ptr == ptr2);
		}

		TEST_METHOD(PolymorphicMoveTest)
		{
			State s = State::Alive;
			SharedPtr<TestObjectChild> ptr = MakeShared<TestObjectChild>(s);
			SharedPtr<TestObject> ptr2 = std::move(ptr);

			Assert::IsNull(ptr.Get());
			Assert::IsNotNull(ptr2.Get());
		}

		TEST_METHOD(AliasLiveTest)
		{
			State s = State::Alive;
			SharedPtr<int> ptr;
			{
				SharedPtr<TestObject> ptr2 = MakeShared<TestObject>(s);
				ptr = SharedPtr<int>(ptr2, &ptr2->aliasObj);
			}

			Assert::AreEqual(State::Alive, s);
		}

		TEST_METHOD(AliasDeadTest)
		{
			State s = State::Alive;
			{
				SharedPtr<TestObject> ptr2 = MakeShared<TestObject>(s);
				SharedPtr<int> ptr = SharedPtr<int>(ptr2, &ptr2->aliasObj);
			}

			Assert::AreEqual(State::Dead, s);
		}
	};

	TEST_CLASS(WeakPtrTests)
	{
	public:
		TEST_METHOD(LockEqualityTest)
		{
			State s = State::Alive;

			SharedPtr<TestObject> ptr = MakeShared<TestObject>(s);
			WeakPtr<TestObject> wptr = ptr;
			SharedPtr<TestObject> ptr2 = wptr.Lock();

			Assert::IsTrue(ptr == ptr2);
			Assert::AreEqual(State::Alive, s);
		}

		TEST_METHOD(PolymorphicTest)
		{
			State s = State::Alive;

			SharedPtr<TestObjectChild> ptr = MakeShared<TestObjectChild>(s);
			WeakPtr<TestObject> wptr = ptr;
			SharedPtr<TestObject> ptr2 = wptr.Lock();

			Assert::IsTrue(ptr == ptr2);
			Assert::AreEqual(State::Alive, s);
		}

		TEST_METHOD(ReferenceCountLiveTest)
		{
			State s = State::Alive;
			State refCountState = State::Alive;

			{
				WeakPtr<TestObject> ptr = MakeSharedTest<TestObject>(refCountState, s);
				Assert::AreEqual(State::Alive, refCountState);
			}
			Assert::AreEqual(State::Dead, refCountState);
		}

		TEST_METHOD(MultiWeakPtrLiveTest)
		{
			State s = State::Alive;
			State refCountState = State::Alive;

			WeakPtr<TestObject> ptr = MakeSharedTest<TestObject>(refCountState, s);
			{
				WeakPtr<TestObject> ptr2 = ptr;
			}
			Assert::AreEqual(State::Alive, refCountState);
		}

		TEST_METHOD(MoveTest)
		{
			State s = State::Alive;
			State refCountState = State::Alive;

			WeakPtr<TestObject> ptr = MakeSharedTest<TestObject>(refCountState, s);
			{
				WeakPtr<TestObject> ptr2 = std::move(ptr);
			}
			Assert::AreEqual(State::Dead, refCountState);
		}

		TEST_METHOD(DeadObjectTest)
		{
			State s = State::Alive;
			State refCountState = State::Alive;

			WeakPtr<TestObject> ptr = MakeSharedTest<TestObject>(refCountState, s);
			Assert::AreEqual(State::Dead, s);
		}
	};

	TEST_CLASS(SharedFromThisTests)
	{
	public:
		TEST_METHOD(SharedEqualityTest)
		{
			State s = State::Alive;
			SharedPtr<TestObject> ptr = MakeShared<TestObject>(s);
			SharedPtr<TestObject> ptr2 = ptr->SharedFromThis();

			Assert::IsTrue(ptr == ptr2);

		}
		TEST_METHOD(WeakLockEqualityTest)
		{
			State s = State::Alive;
			SharedPtr<TestObject> ptr = MakeShared<TestObject>(s);
			WeakPtr<TestObject> ptr2 = ptr->WeakFromThis();

			Assert::IsTrue(ptr == ptr2.Lock());
		}
	};

	TEST_CLASS(CastingTests)
	{
	public:
		TEST_METHOD(NullTest)
		{
			SharedPtr<TestObject> ptr;

			Assert::IsNull(DynamicPointerCast<TestObjectChild>(ptr).Get());
		}

		TEST_METHOD(BadCastTest)
		{
			State s = State::Alive;
			SharedPtr<TestObject> ptr = MakeShared<TestObject>(s);

			Assert::IsNull(DynamicPointerCast<UnrelatedClass>(ptr).Get());
		}

		TEST_METHOD(CastEqualityTest)
		{
			State s = State::Alive;
			SharedPtr<TestObject> ptr = MakeShared<TestObjectChild>(s);

			Assert::IsTrue(ptr == DynamicPointerCast<TestObjectChild>(ptr));
		}
	};
}

class NullGameObjectContainer : public PolymorphicGameObjectContainer
{
protected:
	void OnGameObjectCreated(const SharedPtr<GameObject> gameObject, const std::type_info& type) override
	{

	}
	void OnComponentCreated(const TempOwner<Component> component, const std::type_info& type) override
	{

	}
};

struct NestedGameObject : public GameObject
{
public:
	SharedPtr<GameObject> nested;

	NestedGameObject(PrivateCounter counter, PolymorphicGameObjectContainer& container) :
		GameObject(counter, container)
	{
		nested = CreateGameObject<GameObject>();
	}
};

struct NestedComponent : public Component
{
public:
	SharedPtr<GameObject> nested;

	NestedComponent(PrivateCounter counter, GameObject& owner) :
		Component(counter, owner)
	{
		nested = CreateGameObject<GameObject>();
	}
};

struct UnrelatedComponent : public Component
{
public:
	using Component::Component;
};
namespace ECS_TEST
{
	TEST_CLASS(ECS_TEST)
	{
	public:
		TEST_METHOD(GameObjectCreationTest)
		{
			NullGameObjectContainer container;
			Assert::IsNotNull(container.CreateGameObject<GameObject>().Get());
		}
		TEST_METHOD(ComponentCreationTest)
		{
			NullGameObjectContainer container;
			SharedPtr<GameObject> gameObject = container.CreateGameObject<GameObject>();
			Assert::IsNotNull(gameObject->CreateComponent<Component>().Get());
		}
		TEST_METHOD(NestedGameObjectCreationTest)
		{
			NullGameObjectContainer container;
			SharedPtr<NestedGameObject> gameObject = container.CreateGameObject<NestedGameObject>();
			Assert::IsNotNull(gameObject.Get());
			Assert::IsNotNull(gameObject->nested.Get());
		}
		TEST_METHOD(ComponentNestedGameObjectCreationTest)
		{
			NullGameObjectContainer container;
			SharedPtr<GameObject> gameObject = container.CreateGameObject<GameObject>();
			Owner<NestedComponent> component = gameObject->CreateComponent<NestedComponent>();
			Assert::IsNotNull(component->nested.Get());
		}
		TEST_METHOD(OwningComponentToBase)
		{
			NullGameObjectContainer container;
			SharedPtr<GameObject> gameObject = container.CreateGameObject<GameObject>();
			Owner<NestedComponent> component = gameObject->CreateComponent<NestedComponent>();

			Owner<Component> base = std::move(component);

			Assert::IsNull(component.Get());
			Assert::IsNotNull(base.Get());

			static_assert(!std::copy_constructible<Owner<NestedComponent>>);
			static_assert(!std::assignable_from<Owner<NestedComponent>, Owner<NestedComponent>>);
			static_assert(!std::assignable_from<Owner<NestedComponent>, Owner<Component>>);
		}
		TEST_METHOD(CastOwningComponent)
		{
			NullGameObjectContainer container;
			SharedPtr<GameObject> gameObject = container.CreateGameObject<GameObject>();
			Owner<NestedComponent> component = gameObject->CreateComponent<NestedComponent>();

			Owner<Component> base = std::move(component);
			component = DynamicCast<NestedComponent>(std::move(base));
			Assert::IsNull(base.Get());
			Assert::IsNotNull(component.Get());
		}


		TEST_METHOD(FailCastOwningComponent)
		{
			NullGameObjectContainer container;
			SharedPtr<GameObject> gameObject = container.CreateGameObject<GameObject>();
			Owner<NestedComponent> component = gameObject->CreateComponent<NestedComponent>();

			Owner<Component> base = std::move(component);
			Assert::IsNull(DynamicCast<UnrelatedComponent>(std::move(base)).Get());
		}


		TEST_METHOD(TempOwnerToOwnerEquality)
		{
			NullGameObjectContainer container;
			SharedPtr<GameObject> gameObject = container.CreateGameObject<GameObject>();
			Owner<NestedComponent> component = gameObject->CreateComponent<NestedComponent>();
			TempOwner<NestedComponent> temp = component;

			Assert::IsTrue(temp == component);
		}


		TEST_METHOD(BaseTempOwnerToOwnerEquality)
		{
			NullGameObjectContainer container;
			SharedPtr<GameObject> gameObject = container.CreateGameObject<GameObject>();
			Owner<NestedComponent> component = gameObject->CreateComponent<NestedComponent>();
			TempOwner<Component> temp = component;

			Assert::IsTrue(temp == component);
		}


		TEST_METHOD(TempOwnerBaseEquality)
		{
			NullGameObjectContainer container;
			SharedPtr<GameObject> gameObject = container.CreateGameObject<GameObject>();
			Owner<NestedComponent> component = gameObject->CreateComponent<NestedComponent>();
			TempOwner<NestedComponent> temp = component;
			TempOwner<Component> temp2 = component;

			Assert::IsTrue(temp == temp2);
		}
	};
};