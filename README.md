# MD and HTML GUI TOOL

> [!WARNING]
> This application is still in development.
It is not feature complete and it is not tested on all platforms. 

<p style="text-align: center;"><img style="margin:2px auto;width:100%;" src='https://raw.githubusercontent.com/pavelkral/wedit/refs/heads/main/media/wedit.png' /></p>

## Build from Source

### Build Dependencies

- Qt6
- CMAKE
- cmark

### Build Steps
```
mkdir build
cd build
cmake .. -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.9/msvc2019_64" -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j8
```



## References

- https://qt.io



