# xDL

![](https://img.shields.io/badge/license-MIT-brightgreen.svg?style=flat)
![](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat)
![](https://img.shields.io/badge/release-1.0.1-red.svg?style=flat)
![](https://img.shields.io/badge/Android-4.1%20--%2011-blue.svg?style=flat)
![](https://img.shields.io/badge/arch-armeabi--v7a%20%7C%20arm64--v8a%20%7C%20x86%20%7C%20x86__64-blue.svg?style=flat)

xDL 是 Android DL 系列函数的增强实现。

[README English Version](README.md)


## 特征

* 增强的 `dlopen()` + `dlsym()` + `dladdr()`。
    * 查询当前进程中所有已加载的 ELF，包括系统库。
    * 查询 `.dynsym` 中的动态链接符号。
    * 查询 `.symtab` 和 “`.gnu_debugdata` 里的 `.symtab`” 中的调试符号。
* 增强的 `dl_iterate_phdr()`。
    * 兼容 ARM32 平台的 Android 4.x。
    * 在 Android <= 8.x 时，包含 linker / linker64。
    * 在 Android 5.x 中，返回完整的路径名（full pathname），而不是文件名（basename）。
    * 返回 app\_process32 / app\_process64，而不是包名。
* 支持 Android 4.1 - 11 (API level 16 - 30)。
* 支持 armeabi-v7a, arm64-v8a, x86 and x86_64。
* 使用 MIT 许可证授权。


## 产出物体积

如果将 xDL 编译成独立的动态库：

| ABI         | 压缩后 (KB) | 未压缩 (KB) |
| :---------- | ---------: | ---------: |
| armeabi-v7a | 6.3        | 13.9       |
| arm64-v8a   | 6.9        | 18.3       |
| x86         | 7.0        | 13.8       |
| x86_64      | 7.3        | 18.6       |


## 使用

* 通过包含源码使用 xDL：

```
git submodule add https://github.com/hexhacking/xDL.git external/xdl
```

xDL 的源码在这个文件夹里：[xdl_lib/src/main/cpp](xdl_lib/src/main/cpp)

* 或者，通过 gradle 依赖使用 xDL：

### 1. 增加依赖

```Gradle
dependencies {
    implementation 'io.hexhacking.xdl:xdl-android-lib:1.0.1'
}
```

### 2. 指定一个或多个你需要的 ABI

```Gradle
android {
    defaultConfig {
        ndk {
            abiFilters 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
        }
    }
}
```

### 3. 下载头文件

从 [这里](xdl_lib/src/main/cpp/xdl.h) 下载头文件，从 [这里](https://dl.bintray.com/hexhacking/maven/io/hexhacking/xdl/xdl-android-lib/) 下载 AAR 并解压。

把头文件和动态库放在你工程的合适位置，然后配置你的 CMakeLists.txt 或 Android.mk。

你可以参考 [xdl-sample](xdl_sample) 文件中的示例 app。


## API

包含 xDL 的头文件：

```c
#include "xdl.h"
```

### 1. `xdl_open()` 和 `xdl_close()`

```c
void *xdl_open(const char *filename);
void  xdl_close(void *handle);
```

它们和 `dlopen()` / `dlclose()` 很相似。但是需要注意：`xdl_open()` 并不能真正的从磁盘中“加载” ELF 文件，它只是从内存中“打开”已加载的 ELF 文件。

`filename` 可是是文件名（basename）也可以是完整的路径名（full pathname）。然而，Android linker 从 8.0 开始启用了 namespace 机制。如果你传递文件名，你需要确认当前进程中没有重名的 ELF 文件。`xdl_open()` 只会返回第一个匹配到的 ELF文件。请考虑以下 Android 10 中的 `/proc/self/maps` 片段：

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

### 2. `xdl_sym()` 和 `xdl_dsym()`

```c
void *xdl_sym(void *handle, const char *symbol);
void *xdl_dsym(void *handle, const char *symbol);
```

它们和 `dlsym()` 很相似。 它们都需要传递一个 `xdl_open()` 返回的 “handle”，和一个 null 结尾的符号名字，返回该符号在内存中的加载地址。

`xdl_sym()` 从 `.dynsym` 中查询 “动态链接符号”，就像 `dlsym()` 做的那样。

`xdl_dsym()` 从 `.symtab` 和 “`.gnu_debugdata` 里的 `.symtab`” 中查询 “调试符号”。

注意：

* `.dynsym` 和 `.symtab` 中的符号集合并没有相互包含的关系。有些符号只存在于 `.dynsym` 中，有些则只存在于 `.symtab` 中。你可能需要通过 readelf 之类的工具确定你要查找的符号在哪个 ELF section 中。
* `xdl_dsym()` 需要从磁盘文件中加载调试符号，而 `xdl_sym()` 只从内存中查询动态链接符号。所以 `xdl_dsym()` 比 `xdl_sym()` 执行的更慢。
* 动态链接器只使用 `.dynsym` 中的符号。调试器其实同时使用了 `.dynsym` 和 `.symtab` 中的符号。

### 3. `xdl_addr()`

```c
int xdl_addr(void *addr, Dl_info *info, void **cache);
void xdl_addr_clean(void **cache);
```

`xdl_addr()` 和 `dladdr()` 很相似。但有以下几点不同：

* `xdl_addr()` 不仅能查询动态链接符号，还能查询调试符号。
* `xdl_addr()` 需要传递一个附加的参数（cache），其中会缓存 `xdl_addr()` 执行过程中打开的 ELF handle，缓存的目的是使后续对同一个 ELF 的 `xdl_addr()` 执行的更快。当不需要再执行 `xdl_addr()` 时，请使用 `xdl_addr_clean()` 清除缓存。举例：

```c
void *cache = NULL;
Dl_info info;
xdl_addr(addr_1, &info, &cache);
xdl_addr(addr_2, &info, &cache);
xdl_addr(addr_3, &info, &cache);
xdl_addr_clean(&cache);
```

### 4. `xdl_iterate_phdr()`

```c
#define XDL_DEFAULT       0x00
#define XDL_WITH_LINKER   0x01
#define XDL_FULL_PATHNAME 0x02

int xdl_iterate_phdr(int (*callback)(struct dl_phdr_info *, size_t, void *), void *data, int flags);
```

`xdl_iterate_phdr()` 和 `dl_iterate_phdr()` 很相似。但是 `xdl_iterate_phdr()` 兼容 ARM32 平台的 Android 4.x 系统。

`xdl_iterate_phdr()` 有一个额外的“flags”参数，一个或多个“flag”可以按位“或”后传递给它:

* `XDL_DEFAULT`: 默认行为。
* `XDL_WITH_LINKER`: 总是包含 linker / linker64。
* `XDL_FULL_PATHNAME`: 总是返回完整的路径名（full pathname），而不是文件名（basename）。

需要这些 flags 的原因是，这些额外的能力也需要花费额外的执行时间，而你并不总是需要这些能力。


## 官方仓库

* https://github.com/hexhacking/xDL
* https://gitlab.com/hexhacking/xDL
* https://gitee.com/hexhacking/xDL


## 技术支持

* 查看 [xdl-sample](xdl_sample)。
* 在 [GitHub issues](https://github.com/hexhacking/xDL/issues) 交流。
* 邮件：<a href="mailto:caikelun@gmail.com">caikelun@gmail.com</a>
* QQ 群：603635869


## 贡献

[xDL Contributing Guide](CONTRIBUTING.md).


## 许可证

xDL 使用 [MIT 许可证](LICENSE)。

xDL 的文档使用 [Creative Commons 许可证](LICENSE-docs)。


## 历史

[xCrash 2.x](https://github.com/hexhacking/xCrash/tree/4748d183c1395c54bfb760ec6c454966d52ab73f) 包含一个非常原始的 [xc_dl](https://github.com/hexhacking/xCrash/blob/4748d183c1395c54bfb760ec6c454966d52ab73f/src/native/libxcrash/jni/xc_dl.c) 模块，用它来查询系统库的符号，这个模块在性能和兼容性上都有一些问题。xCrash 2.x 使用它在 libart，libc 和 libc++ 中查询很少量的几个符号。

后来，一些其他的项目开始单独使用 xc_dl 模块，其中也包含一些性能敏感的场景。这时候，我们开始意识到我们需要重写这个模块，并且我们需要一个更好的实现。
