# Names reserved by implementation

[<- back to README.md](..)

Due to the header-only nature of the library, all internal identifiers can be accessed from outside.

Identifiers that are not a part of the public interface are considered **implementation identifiers**. All such identifiers follow a standardized naming convention that aims to reduce the probability of accidental name collisions.

## Reserved Local Identifiers

All identifiers residing inside the `impl` namespace of a module:

```cpp
utl::<module_name>::impl
```

## Reserved Global Identifiers

Macros prefixed with `utl_`:

```cpp
utl_<global_identifier_name>
```

