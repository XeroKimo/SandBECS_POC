# SandBECS_POC
A proof of concept classic OOP style ECS library written in C++20

Most modern day ECS goes the DOD route for various good reasons. However in terms of structuring entities and components, I prefer to think in OOP still similar to Unreal or Unity's monobehaviour. Those however, I assume were written pre-C++11 if I assume they don't just toss out their old code. There have been a lot of useful things added to the library and the compiler since then, so I set out to try writing a more modern OOP ECS library.

# Disclaimer
This is library is experimental, if it's not part of my immediate goals, they are not a concern, use at your own risk

# Goals and Restrictions
## Avoiding multi-stage initialization / de-initialization
I dislike multi-stage init, at least if it was part of the same class. Multi-stage makes an inidividual class much harder to reason, which makes it in my opinion harder to change, and more error prone to use. Multi-stage should be represented by another type such as using std::optional

## Clear ownership and lifetimes

# Usage
There are 3 main classes:
- GameObjectContainer
- GameObject
- Component

## GameObjectContainer
GameObjectContainer is a concept currently implemented as `PolymorphicGameObjectContainer`. They hold direct or indirect references of all game objects. 
### Restrictions
- A game object cannot outlive containers. This gives game objects what should be a clear lifetime.

## GameObject
GameObject is basically just a container of components, however I decided to go the Unreal route and allow inheriting from it, so ideally once a game object is initialized, it is free to use. 
### Features
- A game object may own another game object

## Component
Component are well, something reusuable across different game objects, it is the composition part of the ECS. 
### Features
- A component "A" may own another component "B", so long as "B" has the same owner as "A"
### Restrictions
- Components cannot outlive the game object which owns it.

Game objects and components are currently a custom made shared pointer where `shared_from_this()` is available from the constructor. Since lifetimes are clear, I had the option to not use shared pointer, however not using shared pointers does restrict usage somewhat such as, not allowing other objects to hold a persitent non-owning pointer to another object. This is because even if we know who owns the object and lifetimes are clear, knowning that all non-owning pointer has a shorter lifetime than it's owners is a bit tricky in ECS unless it was fine tuned to a specific use case.

There are other schemes other than shared pointer, but I think this was the easiest to hack together, so it is what I did
