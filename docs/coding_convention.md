Coding style
=============

The general coding style is derived from Java coding and naming convention with some minor C++ specific practical changes.

In particular every include file must start with '#pragma once' after copyright header.
- Rational: all target compilers support this pragma so no need to rely on define guards

Include guards, when user, must follow the pattern: {PROJECT_NAME}_{NAMESPACE}_{FILE_BASENAME -without suffix}_{FILE_SUFFIX}
Each #endif must be followed by a comment which 'condition' it closures

 * Type names are nouns and start with a Capital letter
 * Names are cameCase
 * Method names are generally verbs like a.doSomethig()
 ** One notable exception is methods that return boolean and take no arguments.
    bool isEmpty() const;

Return statement is always to be separated by an empty line from the code above it.
Only exception is when return is the only statement in a method.
- Rational: clear visual high light of a function return points.
 * Example:
 ```
    // Example of a return being the only statement:
    int getNumber() {
        return 42;
    }

    // Multiline method:
    int doCalculations() {
        int result = 0;
        ....
        result += computeAstroResult();

        return result;
    }
```

# Resource management and move semantics
All classes that manage resources (sucha as memory, files, etc) - are creates via
factory functions. This is done so that user could explicitly pass resource manager.
For example - if you need a `vector` that needs to allocate memory for storage - you need to pass
an instance of memeory manager - used for memory allocation - to a factory that constructs `vector`.

Thus, following conventions are used:

 - All non-trivial library types are movable.
 - All default constructors are `constexpr noexcept`
   If class provides a default constructor it is `noexcept` as it is not managing resources
 - size(), empty(), equals() - getters are `constexpr noexcept`
