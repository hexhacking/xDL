#include <android/api-level.h>
#include <android/log.h>
#include <fcntl.h>
#include <inttypes.h>
#include <jni.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "xdl.h"

#define SAMPLE_JNI_VERSION    JNI_VERSION_1_6
#define SAMPLE_JNI_CLASS_NAME "io/github/hexhacking/xdl/sample/NativeSample"

// log
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, "xdl_tag", fmt, ##__VA_ARGS__)
#pragma clang diagnostic pop
#define LOG_END "*** --------------------------------------------------------------"

#if defined(__LP64__)
#define BASENAME_LINKER      "linker64"
#define BASENAME_APP_PROCESS "app_process64"
#define PATHNAME_LIBC        "/system/lib64/libc.so"
#define PATHNAME_LIBC_Q      "/apex/com.android.runtime/lib64/bionic/libc.so"
#define PATHNAME_LIBCPP      "/system/lib64/libc++.so"
#else
#define BASENAME_LINKER      "linker"
#define BASENAME_APP_PROCESS "app_process32"
#define PATHNAME_LIBC        "/system/lib/libc.so"
#define PATHNAME_LIBC_Q      "/apex/com.android.runtime/lib/bionic/libc.so"
#define PATHNAME_LIBCPP      "/system/lib/libc++.so"
#endif
#define BASENAME_VDSO        "[vdso]"
#define BASENAME_LIBNETUTILS "libnetutils.so"
#define BASENAME_LIBCAP      "libcap.so"
#define BASENAME_LIBOPENCL   "libOpenCL.so"
#define BASENAME_LIBART      "libart.so"

#define PATHNAME_LIBC_FIXED \
  (android_get_device_api_level() < __ANDROID_API_Q__ ? PATHNAME_LIBC : PATHNAME_LIBC_Q)

static int callback(struct dl_phdr_info *info, size_t size, void *arg) {
  (void)size, (void)arg;

  LOG(">>> %" PRIxPTR " %s (phdr: %" PRIxPTR ", phnum: %zu)", (uintptr_t)info->dlpi_addr, info->dlpi_name,
      (uintptr_t)info->dlpi_phdr, (size_t)info->dlpi_phnum);
  return 0;
}

static void sample_test_iterate(void) {
  LOG("+++ xdl_iterate_phdr(XDL_DEFAULT)");
  xdl_iterate_phdr(callback, NULL, XDL_DEFAULT);
  LOG(LOG_END);
  usleep(100 * 1000);

  LOG("+++ xdl_iterate_phdr(XDL_FULL_PATHNAME)");
  xdl_iterate_phdr(callback, NULL, XDL_FULL_PATHNAME);
  LOG(LOG_END);
  usleep(100 * 1000);
}

static void *sample_test_dlsym(const char *filename, const char *symbol, bool debug_symbol, void **cache,
                               bool try_force_dlopen) {
  xdl_info_t info;

  if (try_force_dlopen) {
    void *linker_handle = dlopen(filename, RTLD_NOW);
    LOG("--- dlopen(%s) : handle %" PRIxPTR, filename, (uintptr_t)linker_handle);
    if (NULL != linker_handle) dlclose(linker_handle);
  }

  LOG("+++ xdl_open + xdl_info + %s + xdl_addr", debug_symbol ? "xdl_dsym" : "xdl_sym");

  // xdl_open
  void *handle = xdl_open(filename, try_force_dlopen ? XDL_TRY_FORCE_LOAD : XDL_DEFAULT);
  LOG(">>> xdl_open(%s) : handle %" PRIxPTR, filename, (uintptr_t)handle);

  // xdl_info
  memset(&info, 0, sizeof(xdl_info_t));
  xdl_info(handle, XDL_DI_DLINFO, &info);
  LOG(">>> xdl_info(%" PRIxPTR ") : %" PRIxPTR " %s (phdr %" PRIxPTR ", phnum %zu)", (uintptr_t)handle,
      (uintptr_t)info.dli_fbase, (NULL == info.dli_fname ? "(NULL)" : info.dli_fname),
      (uintptr_t)info.dlpi_phdr, info.dlpi_phnum);

  // xdl_dsym / xdl_sym
  size_t symbol_size = 0;
  void *symbol_addr = (debug_symbol ? xdl_dsym : xdl_sym)(handle, symbol, &symbol_size);
  LOG(">>> %s(%s) : addr %" PRIxPTR ", sz %zu", debug_symbol ? "xdl_dsym" : "xdl_sym", symbol,
      (uintptr_t)symbol_addr, symbol_size);

  // xdl_close
  void *linker_handle = xdl_close(handle);

  // xdl_addr
  memset(&info, 0, sizeof(xdl_info_t));
  if (0 == xdl_addr(symbol_addr, &info, cache))
    LOG(">>> xdl_addr(%" PRIxPTR ") : FAILED", (uintptr_t)symbol_addr);
  else
    LOG(">>> xdl_addr(%" PRIxPTR ") : %" PRIxPTR " %s (phdr %" PRIxPTR ", phnum %zu), %" PRIxPTR
        " %s (sz %zu)",
        (uintptr_t)symbol_addr, (uintptr_t)info.dli_fbase,
        (NULL == info.dli_fname ? "(NULL)" : info.dli_fname), (uintptr_t)info.dlpi_phdr, info.dlpi_phnum,
        (uintptr_t)info.dli_saddr, (NULL == info.dli_sname ? "(NULL)" : info.dli_sname), info.dli_ssize);

  LOG(LOG_END);

  return linker_handle;
}

static void sample_test(JNIEnv *env, jobject thiz) {
  (void)env;
  (void)thiz;

  // cache for xdl_addr()
  void *cache = NULL;

  // iterate test
  sample_test_iterate();

  // linker
  sample_test_dlsym(BASENAME_LINKER, "__dl__ZL10g_dl_mutex", true, &cache, false);

  // app_process
  sample_test_dlsym(BASENAME_APP_PROCESS, "sigaction", false, &cache, false);

  // vDSO
  sample_test_dlsym(BASENAME_VDSO, "__kernel_rt_sigreturn", false, &cache, false);

  // libc.so
  sample_test_dlsym(PATHNAME_LIBC_FIXED, "android_set_abort_message", false, &cache, false);
  sample_test_dlsym(PATHNAME_LIBC_FIXED, "je_mallctl", true, &cache, false);

  // libc++.so
  sample_test_dlsym(PATHNAME_LIBCPP, "_ZNSt3__14cerrE", false, &cache, false);
  sample_test_dlsym(PATHNAME_LIBCPP, "abort_message", true, &cache, false);

  // libart.so
  sample_test_dlsym(BASENAME_LIBART, "_ZN3artL16FindOatMethodForEPNS_9ArtMethodENS_11PointerSizeEPb", true,
                    &cache, false);

  // libnetutils.so (may need to be loaded from disk into memory)
  void *linker_handle_libnetutils =
      sample_test_dlsym(BASENAME_LIBNETUTILS, "ifc_get_hwaddr", false, &cache, true);

  // libcap.so (may need to be loaded from disk into memory)
  void *linker_handle_libcap = sample_test_dlsym(BASENAME_LIBCAP, "cap_dup", false, &cache, true);

  // libOpenCL.so (may need to be loaded from disk into memory)
  void *linker_handle_libOpenCL =
      sample_test_dlsym(BASENAME_LIBOPENCL, "clCreateContext", false, &cache, true);

  // clean cache for xdl_addr()
  xdl_addr_clean(&cache);

  // dlclose (may need to be unloaded from memory)
  if (NULL != linker_handle_libnetutils) {
    LOG("--- dlclose(%s) : linker_handle %" PRIxPTR, BASENAME_LIBNETUTILS,
        (uintptr_t)linker_handle_libnetutils);
    dlclose(linker_handle_libnetutils);
  }
  if (NULL != linker_handle_libcap) {
    LOG("--- dlclose(%s) : linker_handle %" PRIxPTR, BASENAME_LIBCAP, (uintptr_t)linker_handle_libcap);
    dlclose(linker_handle_libcap);
  }
  if (NULL != linker_handle_libOpenCL) {
    LOG("--- dlclose(%s) : linker_handle %" PRIxPTR, BASENAME_LIBOPENCL, (uintptr_t)linker_handle_libOpenCL);
    dlclose(linker_handle_libOpenCL);
  }
}

static JNINativeMethod sample_jni_methods[] = {{"nativeTest", "()V", (void *)sample_test}};

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  JNIEnv *env;
  jclass cls;

  (void)reserved;

  if (NULL == vm) return JNI_ERR;
  if (JNI_OK != (*vm)->GetEnv(vm, (void **)&env, SAMPLE_JNI_VERSION)) return JNI_ERR;
  if (NULL == env || NULL == *env) return JNI_ERR;
  if (NULL == (cls = (*env)->FindClass(env, SAMPLE_JNI_CLASS_NAME))) return JNI_ERR;
  if (0 != (*env)->RegisterNatives(env, cls, sample_jni_methods,
                                   sizeof(sample_jni_methods) / sizeof(sample_jni_methods[0])))
    return JNI_ERR;

  return SAMPLE_JNI_VERSION;
}
