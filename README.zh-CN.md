# xDL

![](https://img.shields.io/badge/license-MIT-brightgreen.svg?style=flat)
![](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat)
![](https://img.shields.io/badge/release-1.1.1-red.svg?style=flat)
![](https://img.shields.io/badge/Android-4.1%20--%2011-blue.svg?style=flat)
![](https://img.shields.io/badge/arch-armeabi--v7a%20%7C%20arm64--v8a%20%7C%20x86%20%7C%20x86__64-blue.svg?style=flat)

xDL 是 Android DL 系列函数的增强实现。

[README English Version](README.md)


## 特征

* 增强的 `dlopen()` + `dlsym()` + `dladdr()`。
    * 绕过 Android 7.0+ linker namespace 的限制。
    * 查询 `.dynsym` 中的动态链接符号。
    * 查询 `.symtab` 和 “`.gnu_debugdata` 里的 `.symtab`” 中的调试符号。
* 增强的 `dl_iterate_phdr()`。
    * 兼容 ARM32 平台的 Android 4.x。
    * 在 Android <= 8.x 时，包含 linker / linker64。
    * 在 Android 5.x 中，返回完整的路径名（full pathname），而不是文件名（basename）。
    * 返回 app\_process32 / app\_process64，而不是包名。
* 支持 Android 4.1 - 11 (API level 16 - 30)。
* 支持 armeabi-v7a, arm64-v8a, x86 和 x86_64。
* 使用 MIT 许可证授权。


## 产出物体积

如果将 xDL 编译成独立的动态库：

| ABI         | 压缩后 (KB) | 未压缩 (KB) |
| :---------- | ---------: | ---------: |
| armeabi-v7a | 6.8        | 13.9       |
| arm64-v8a   | 7.5        | 18.3       |
| x86         | 7.7        | 17.9       |
| x86_64      | 7.9        | 18.6       |


## 使用

### 1. 在 build.gradle 中增加依赖

xDL 发布在 [Maven Central](https://search.maven.org/) 上。为了使用 [native 依赖项](https://developer.android.com/studio/build/native-dependencies)，xDL 使用了从 [Android Gradle Plugin 4.0+](https://developer.android.com/studio/releases/gradle-plugin?buildsystem=cmake#native-dependencies) 开始支持的 [Prefab](https://google.github.io/prefab/) 包格式。

```Gradle
allprojects {
    repositories {
        mavenCentral()
    }
}
```

```Gradle
android {
    buildFeatures {
        prefab true
    }
}

dependencies {
    implementation 'io.hexhacking:xdl:1.1.1'
}
```

### 2. 在 CMakeLists.txt 或 Android.mk 中增加依赖

> CMakeLists.txt

```CMake
find_package(xdl REQUIRED CONFIG)

add_library(mylib SHARED mylib.c)
target_link_libraries(mylib xdl::xdl)
```

> Android.mk

```
include $(CLEAR_VARS)
LOCAL_MODULE           := mylib
LOCAL_SRC_FILES        := mylib.c
LOCAL_SHARED_LIBRARIES += xdl
include $(BUILD_SHARED_LIBRARY)

$(call import-module,prefab/xdl)
```

### 3. 指定一个或多个你需要的 ABI

```Gradle
android {
    defaultConfig {
        ndk {
            abiFilters 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
        }
    }
}
```

### 4. 增加打包选项

如果你是在一个 SDK 工程里使用 xDL，你可能需要避免把 libxdl.so 打包到你的 AAR 里，以免 app 工程打包时遇到重复的 libxdl.so 文件。

```Gradle
android {
    packagingOptions {
        exclude '**/libxdl.so'
    }
}
```

另一方面, 如果你是在一个 APP 工程里使用 xDL，你可以需要增加一些选项，用来处理重复的 libxdl.so 文件引起的冲突。

```Gradle
android {
    packagingOptions {
        pickFirst '**/libxdl.so'
    }
}
```

你可以参考 [xdl-sample](xdl_sample) 文件夹中的示例 app。


## API

```C
#include "xdl.h"
```

### 1. `xdl_open()` 和 `xdl_close()`

```C
#define XDL_DEFAULT           0x00
#define XDL_TRY_FORCE_LOAD    0x01
#define XDL_ALWAYS_FORCE_LOAD 0x02

void *xdl_open(const char *filename, int flags);
void *xdl_close(void *handle);
```

它们和 `dlopen()` / `dlclose()` 很相似。但是 `xdl_open()` 可以绕过 Android 7.0+ linker namespace 的限制。

根据 `flags` 参数值的不同，`xdl_open()` 的行为会有一些差异：

* `XDL_DEFAULT`: 如果动态库已经被加载到内存中了，`xdl_open()` 不会再使用 `dlopen()` 加载它。（但依然会返回一个有效的 `handle`）
* `XDL_TRY_FORCE_LOAD`: 如果动态库还没有被加载到内存中，`xdl_open()` 将尝试使用 `dlopen()` 加载它。
* `XDL_ALWAYS_FORCE_LOAD`: `xdl_open()` 将总是使用 `dlopen()` 加载动态库。

如果 `xdl_open()` 真的使用 `dlopen()` 加载了动态库，`xdl_close()` 将返回从 linker 那里取得的 handle（`dlopen()` 的返回值），然后你可以决定是否以及什么时候使用标准的 `dlclose()` 来关闭它。否则，将返回 `NULL`。

`filename` 可是是文件名（basename）也可以是完整的路径名（full pathname）。然而，Android linker 从 7.0 开始启用了 namespace 机制。如果你传递文件名，你需要确认当前进程中没有重名的 ELF 文件。`xdl_open()` 只会返回第一个匹配到的 ELF文件。请考虑以下 Android 10 中的 `/proc/self/maps` 片段：

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

```C
void *xdl_sym(void *handle, const char *symbol, size_t *symbol_size);
void *xdl_dsym(void *handle, const char *symbol, size_t *symbol_size);
```

它们和 `dlsym()` 很相似。 它们都需要传递一个 `xdl_open()` 返回的 “handle”，和一个 null 结尾的符号名字，返回该符号在内存中的加载地址。

如果 `symbol_size` 参数不为 `NULL`，它将被赋值为“符号对应的内容在 ELF 中占用的字节数”，如果你不需要这个信息，传递 `NULL` 就可以了。

`xdl_sym()` 从 `.dynsym` 中查询 “动态链接符号”，就像 `dlsym()` 做的那样。

`xdl_dsym()` 从 `.symtab` 和 “`.gnu_debugdata` 里的 `.symtab`” 中查询 “调试符号”。

注意：

* `.dynsym` 和 `.symtab` 中的符号集合并没有相互包含的关系。有些符号只存在于 `.dynsym` 中，有些则只存在于 `.symtab` 中。你可能需要通过 readelf 之类的工具确定你要查找的符号在哪个 ELF section 中。
* `xdl_dsym()` 需要从磁盘文件中加载调试符号，而 `xdl_sym()` 只从内存中查询动态链接符号。所以 `xdl_dsym()` 比 `xdl_sym()` 执行的更慢。
* 动态链接器只使用 `.dynsym` 中的符号。调试器其实同时使用了 `.dynsym` 和 `.symtab` 中的符号。

### 3. `xdl_addr()`

```C
int xdl_addr(void *addr, Dl_info *info, void **cache);
void xdl_addr_clean(void **cache);
```

`xdl_addr()` 和 `dladdr()` 很相似。但有以下几点不同：

* `xdl_addr()` 不仅能查询动态链接符号，还能查询调试符号。
* `xdl_addr()` 需要传递一个附加的参数（cache），其中会缓存 `xdl_addr()` 执行过程中打开的 ELF handle，缓存的目的是使后续对同一个 ELF 的 `xdl_addr()` 执行的更快。当不需要再执行 `xdl_addr()` 时，请使用 `xdl_addr_clean()` 清除缓存。举例：

```C
void *cache = NULL;
Dl_info info;
xdl_addr(addr_1, &info, &cache);
xdl_addr(addr_2, &info, &cache);
xdl_addr(addr_3, &info, &cache);
xdl_addr_clean(&cache);
```

### 4. `xdl_iterate_phdr()`

```C
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
