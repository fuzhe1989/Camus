# Camus

[Albert Camus](https://en.wikipedia.org/wiki/Albert_Camus)

## Build

Install [conan](https://conan.io/).

```
pip install conan
```

Install gcc-11

Then:

```
mkdir build && cd build
conan install .. --build=missing -pr ../conan_profile/gcc -s build_type=Release
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo 
make
```
