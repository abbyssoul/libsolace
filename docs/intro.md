# Introduction

libsolace project aim is to provide consistent collection of tools that are easy to use
and helpful when building hight perfromance and high availability systems.
In order to achive this goal the library adhere to recomendataions outlined in NASA's Power of 10 where possible.
Library provides all controll over resources to a developer and thus it will never spawns a thread or allocate memory
without user explicit action. This approach - while somewhat demanding on developer's part - provide opportunity
to practice resource management and make optimal decision about performance and resournce utilization.


## Basic structure
Fundamentally the library consists of a number of layers:
 0. language extention: Preconditions, asserts, etc.
 1. Fundamental classes: Optional, Error, Result, Atom, etc.
 2. OS interface / Memory management: MemoryManager, Env.
 3. Resources and views: MemoryResource, StringResource, etc.
 4. Memory manager aware collections: Array, Vector, Dictionary
 5. Misc high level utility classes: Version, UUID, ByteReaded/ByteWriter, etc.

Each layer builds on top of another. Thus, for example Optional and Error classes do not allocate memory by themselfs.
Memory management rely on optional, result and error to report success or failure of any system call.
Collections allocate required memory using provided MemoryManager.

## Fundanmental classes
Classes such as Optional nad Result don't represent any business object of system resource.
They exist for engineering convenience to better express intentins in code.

### Optional
TDB

### Result

### Error
TDB

### Future
TDB

## Operating system interface
Implementation most programs requires management of resources - which is the function of an operating system
a program is executed by. That is why the library includes an interface for the underlaying OS calls.
The goal is to provide minimal functional subset of operations required that is supported by all target OSs.

### MemoryManager
At the very minimum a program need allocate system memory to store data. MemoryManager is the class responsinble
for interfacing with memory management subsystem of the target platform.
Note that it is possible to provide other implementations of MemoryManager that, for example, allocate GPU memory etc.

### Env
Env is a simple class representing running process environment as defind by the POSIX standard. Note that on some systems
it might be empty in case no environment is provided.


## Resources and views
A number of classes require variable amount of memory allocated for its operation. For example a string can occupy
variable amount of space in memory. Such object requier memory management and thus refered to as `resources`.
Resource classes provide a way to 'pick' into the memory content the managed using `view()` method.
In case of a string resource `res.view()` returns a view into a string buffer not the resource object.


## Collections
library provide minimalistic set of fixed size collections/containers that are memory manager aware.
See [collections](./collections.md) for detailed information.

## Utility Classes
TBD
