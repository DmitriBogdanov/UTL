# Release: utl::random v2.0.0

## Convenient random improvements

Existing `rand_...()` methods were deprecated in of a more generic API which:

- Provides a generic way of variating distributions using a **thread-local automatically seeded PRNG**
- Makes underlying distributions explicit
- Standardizes naming convention
- Adds template overloads

This is a significant technical improvement over the previous version as it provides a simple, fast and thread-safe way of generating rng. Underlying PRNG state is now thread-local and lazily seeded with entropy, this removes any need for seeding in common applications yet still provides an option of using a fixed seed if necessary. Any user additions to the API can now be trivially implemented in terms of `random::variate()`.

### Previous API reference

```cpp
// Convenient random functions
int rand_int(          int min,          int max) noexcept;
int rand_uint(unsigned int min, unsigned int max) noexcept;

float rand_float()                     noexcept; // U[0, 1)     uniform distribution
float rand_float(float min, float max) noexcept; // U[min, max) uniform distribution
float rand_normal_float();                       // N(0, 1)      normal distribution

double rand_double()                       noexcept; // U[0, 1)     uniform distribution
double rand_double(double min, double max) noexcept; // U[min, max) uniform distribution
double rand_normal_float();                          // N(0, 1)      normal distribution

bool rand_bool() noexcept;

template<class T>
T rand_choice(std::initializer_list<T> objects) noexcept;
```

### New API reference

```cpp
// Convenient random (generic)
template <class Dist> auto variate(Dist&& dist); // any distribution

template <class T> T uniform(T min, T max); // integer U[min, max]
template <class T> T uniform(            ); // boolean U[0, 1]
template <class T> T uniform(T min, T max); // float   U[min, max)
template <class T> T uniform(            ); // float   U[0, 1)

template <class T> T normal(T mean, T stddev); // float N(mean, stddev)
template <class T> T normal(                ); // float N(0, 1)

// Convenient random (shortcuts for standard types)
using Uint = unsigned int;

int    uniform_int   (   int min,    int max); // U[min, max]
Uint   uniform_uint  (  Uint min,   Uint max); // U[min, max]
bool   uniform_bool  (                      ); // U[0, 1]
float  uniform_float ( float min,  float max); // U[min, max)
double uniform_double(double min, double max); // U[min, max)
float  uniform_float (                      ); // U[0, 1)
double uniform_double(                      ); // U[0, 1)

float  normal_float ( float mean,  float stddev); // N(mean, stddev)
double normal_double(double mean, double stddev); // N(mean, stddev)
float  normal_float (                          ); // N(0, 1)
double normal_double(                          ); // N(0, 1)

// Convenient random (other)
template <class T>            T choose(std::initializer_list<T> list);
template <class Container> auto choose(const Container&         list);
```

Key improvements relative

### General improvements

- Added `std::this_thread::get_id()` as an additional source of entropy which guarantees different seeding on each thread
- Fixed several inclusive / exclusive range mixups in floating-point random documentation