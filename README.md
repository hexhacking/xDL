# xDL

![](https://img.shields.io/badge/license-MIT-brightgreen.svg?style=flat)
![](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat)
![](https://img.shields.io/badge/release-1.0.0-red.svg?style=flat)
![](https://img.shields.io/badge/Android-4.1%20--%2011-blue.svg?style=flat)
![](https://img.shields.io/badge/arch-armeabi--v7a%20%7C%20arm64--v8a%20%7C%20x86%20%7C%20x86__64-blue.svg?style=flat)

xDL is an enhanced implementation of the Android DL series functions.

[README 中文版](README.zh-CN.md)


## Features

* Enhanced `dlopen()` + `dlsym()` + `dladdr()`.
    * Lookup all loaded ELF in the current process, including system libraries.
    * Lookup dynamic link symbols in `.dynsym`
    * Lookup debuging symbols in `.symtab` and "`.symtab` in `.gnu_debugdata`".
* Enhanced `dl_iterate_phdr()`.
    * Compatible with Android 4.x on ARM32.
    * Including linker / linker64 (for Android <= 8.x).
    * Return full pathname instead of basename (for Android 5.x).
    * Return app\_process32 / app\_process64 instead of package name.
* Support Android 4.1 - 11 (API level 16 - 30).
* Support armeabi-v7a, arm64-v8a, x86 and x86_64.
* MIT licensed.


## Artifacts Size

| ABI         | Compressed (KB) | Uncompressed (KB) |
| :---------- | --------------: | ----------------: |
| armeabi-v7a | 5.8             | 13.9              |
| arm64-v8a   | 6.2             | 14.2              |
| x86         | 6.4             | 13.8              |
| x86_64      | 6.6             | 14.5              |


## Usage

### 1. Add dependency

```Gradle
dependencies {
    implementation 'io.hexhacking.xdl:xdl-android-lib:1.0.0'
}
```

### 2. Specify one or more ABI(s) you need

```Gradle
android {
    defaultConfig {
        ndk {
            abiFilters 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
        }
    }
}
```

### 3. Download header file

Download the header file from [here](xdl_lib/src/main/cpp/xdl.h), pay attention to check the version number in the file. Then put the header file in the right place of your project.

There is a sample app in the [xdl-sample](xdl_sample) folder you can refer to.


## API

include xDL's header file:

```c
#include "xdl.h"
```

### 1. `xdl_open()` and `xdl_close()`

```c
void *xdl_open(const char *filename);
void  xdl_close(void *handle);
```

They are very similar to `dlopen()` and `dlclose()`. But need to pay attention: `xdl_open()` can NOT actually "load" ELF from disk, it just "open" ELF that has been loaded into memory.

`filename` can be basename or full pathname. However, Android linker has used the namespace mechanism since 8.0. If you pass basename, you need to make sure that no duplicate ELF is loaded into the current process. `xdl_open()` will only return the first matching ELF. Please consider this fragment of `/proc/self/maps` on Android 10:

```
756fc2c000-756fc7c000 r--p 00000000 fd:03 2985  /system/lib64/vndk-sp-29/libc++.so
756fc7c000-756fcee000 --xp 00050000 fd:03 2985  /system/lib64/vndk-sp-29/libc++.so
756fcee000-756fcef000 rw-p 000c2000 fd:03 2985  /system/lib64/vndk-sp-29/libc++.so
756fcef000-756fcf7000 r--p 000c3000 fd:03 2985  /system/lib64/vndk-sp-29/libc++.so
7571fdd000-757202d000 r--p 00000000 07:38 20    /apex/com.android.conscrypt/lib64/libc++.so
757202d000-757209f000 --xp 00050000 07:38 20    /apex/com.android.conscrypt/lib64/libc++.so
757209f000-75720a0000 rw-p 000c2000 07:38 20    /apex/com.android.conscrypt/lib64/libc++.so
75720a0000-75720a8000 r--p 000c3000 07:38 20    /apex/com.android.conscrypt/lib64/libc++.so
760b9df000-760ba2f000 r--p 00000000 fd:03 2441  /system/lib64/libc++.so
760ba2f000-760baa1000 --xp 00050000 fd:03 2441  /system/lib64/libc++.so
760baa1000-760baa2000 rw-p 000c2000 fd:03 2441  /system/lib64/libc++.so
760baa2000-760baaa000 r--p 000c3000 fd:03 2441  /system/lib64/libc++.so
```

### 2. `xdl_sym()` and `xdl_dsym()`

```c
void *xdl_sym(void *handle, const char *symbol);
void *xdl_dsym(void *handle, const char *symbol);
```

They are very similar to `dlsym()`. They all takes a "handle" of an ELF returned by `xdl_open()` and the null-terminated symbol name, returning the address where that symbol is loaded into memory.

`xdl_sym()` lookup "dynamic link symbols" in `.dynsym` as `dlsym()` does.

`xdl_dsym()` lookup "debuging symbols" in `.symtab` and "`.symtab` in `.gnu_debugdata`".

Notice:

* All the dynamic link symbols that have been included in the debuging symbols.
* `xdl_dsym()` needs to load debuging symbols from disk file, and `xdl_sym()` only lookup dynamic link symbols from memory. So `xdl_dsym()` runs slower than `xdl_sym()`. If the symbol you are looking for is a dynamic link symbol, please use `xdl_sym()`.

### 3. `xdl_addr()`

```c
int xdl_addr(void *addr, Dl_info *info, char *tmpbuf, size_t tmpbuf_len);
```

`xdl_addr()` is similar to `dladdr()`. `xdl_addr()` can lookup not only dynamic link symbols, but also debugging symbols.

`xdl_addr()` needs to pass an temporary buffer and the length of the buffer. This buffer is used to store the `dlpi_name` and `dli_sname` strings in `Dl_info *info` struct. It is recommended to pass a 512 or 1024 bytes buffer. After `xdl_addr()` returns, please ensure that the lifetime of `tmpbuf` is longer than or equal to the lifetime of `Dl_info *info` struct.

### 4. `xdl_iterate_phdr()`

```c
#define XDL_DEFAULT       0x00
#define XDL_WITH_LINKER   0x01
#define XDL_FULL_PATHNAME 0x02

int xdl_iterate_phdr(int (*callback)(struct dl_phdr_info *, size_t, void *), void *data, int flags);
```

`xdl_iterate_phdr()` is similar to `dl_iterate_phdr()`. But `xdl_iterate_phdr()` is compatible with android 4.x on ARM32.

`xdl_iterate_phdr()` has an additional "flags" parameter, one or more flags can be bitwise-or'd in it:

* `XDL_DEFAULT`: Default behavior.
* `XDL_WITH_LINKER`: Always including linker / linker64.
* `XDL_FULL_PATHNAME`: Always return full pathname instead of basename.

These flags are needed because these capabilities require additional execution time, and you don't always need them.


## Official Repositories

* https://github.com/hexhacking/xDL
* https://gitlab.com/hexhacking/xDL
* https://gitee.com/hexhacking/xDL


## Support

* Check the [xdl-sample](xdl_sample).
* Communicate on [GitHub issues](https://github.com/hexhacking/xDL/issues).
* Email: <a href="mailto:caikelun@gmail.com">caikelun@gmail.com</a>
* QQ group: 603635869


## Contributing

[xDL Contributing Guide](CONTRIBUTING.md).


## License

xDL is MIT licensed, as found in the [LICENSE](LICENSE) file.

xDL documentation is Creative Commons licensed, as found in the [LICENSE-docs](LICENSE-docs) file.


## History

[xCrash 2.x](https://github.com/hexhacking/xCrash/tree/4748d183c1395c54bfb760ec6c454966d52ab73f) contains a very rudimentary module [xc_dl](https://github.com/hexhacking/xCrash/blob/4748d183c1395c54bfb760ec6c454966d52ab73f/src/native/libxcrash/jni/xc_dl.c) for searching system library symbols, which has many problems in performance and compatibility. xCrash 2.x uses it to search a few symbols from libart, libc and libc++.

Later, some other projects began to use the xc_dl module alone, including in some performance-sensitive usage scenarios. At this time, we began to realize that we need to rewrite this module, and we need a better implementation.
