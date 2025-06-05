# Semantic versioning

[<- back to README.md](..)

All UTL libraries follow [semantic versioning](https://semver.org/), which is defined by 3 numbers **MAJOR.MINOR.PATCH**:

1. **MAJOR** — version that makes incompatible API changes
2. **MINOR** — version that adds functionality in a backward compatible manner
3. **PATCH** — version that contains backwards compatible chores & bug fixes

The version is defined at the top of each header with the following format:

```cpp
#define UTL_<module_name>_VERSION_MAJOR <major_integer>
#define UTL_<module_name>_VERSION_MINOR <minor_integer>
#define UTL_<module_name>_VERSION_PATCH <patch_integer>
```

