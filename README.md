# Camus

[Albert Camus](https://en.wikipedia.org/wiki/Albert_Camus)

## Build

Install [conan](https://conan.io/).

```
pip install conan
```

Then:

```
mkdir build && cd build
conan install .. --build=missing -pr ../conanprofile.release
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo 
make
```
