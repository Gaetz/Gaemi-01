# Code conventions

## Language and its use
Gaemi uses modern C++ - currently C++17. 

It is recommended to keep the API the simplest and the most straightforward, using appropriate modern C++ syntax and 
tools. We use the STL.

By default, we use plain variables, references and const-references. Raw pointer should be avoided as member variables, 
if it does not break the API simplicity. Prefer unique pointers if you want to store a pointer.

Implement const-correctness.

## File names
- Code files are named with PascalCasing. 
- Headers are in .h files
- Sources are in .cpp files
- .hpp extension is used for directly implemented headers.
- Assets and shader files use camelCasing. They can contain an underscore to categorize assets.

## Code
### Names
- namespaces use a single word in minus. E.g.: `engine`
- Classes use PascalCasing. E.g.: `InputSystem`
- functions use camelCasing. E.g.: `processInput()`
- variables and function parameters use camelCasing. E.g.: `isCursorDisplayed`
- Let libraries break code name conventions. No need to wrap them.
- Macros and constexpr use SCREAMING_SNAKE_CASE.

### Class member and associated function parameters
Prefixes are difficult to read, especially when used with several levels of depth or in combination with pointers. 
Therefore, we won't use prefixes, but rather mark variables with specific roles with suffixes.

- Member variables are just named normally. E.g.: `isCursorDisplayed`
- If a parameter is meant to set the value of a variable, for instance through a setter function or in a class 
  initializer, this parameter take a `P` suffix (P for Parameter). E.g.: `isCursorDisplayedP`
- References do not need to be named specifically.
- If a variable is actually a raw pointer and if this information is not self-evident, use the `Ptr` suffix. E.g.: 
  `SDL_GameController* controllerPtr;`
- Shared pointers and weak pointers are suffixed: `SPtr`, `WPtr`. 
- Usually unique pointers are not suffixed, for it has no interest. If you need to suffix them for clarity, use `UPtr`.

