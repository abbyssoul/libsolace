# Strings
Generally libSolace's view of the string type very nicely summarised in [cpp-string-theory](https://github.com/deanberris/cpp-string-theory/blob/master/cpp-string-theory.rst) article.
In short currently available std::string although useful sometimes and widely used - considered broken.
Main issue is that it is mutable. In that std::string mixing two functions of string building and representing a fixed sequence.
This responsibilities are often implemented by two different classes. Furthermore, immutable string is in odds with the concept of this library to utilise fixed size containers.
That is why libsolace features a custom implementation of a String an immutable sequence of characters.
In addition to that the library also proved extra ways to represent string to special accommodate cases such as C-string literals. In some cases developers know that the string is a literal
and memory management can be significantly simplified.
