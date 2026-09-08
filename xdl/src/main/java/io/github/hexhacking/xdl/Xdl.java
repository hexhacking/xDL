// Copyright (c) 2020-2026 HexHacking Team
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// Created by caikelun on 2026-09-08.

package io.github.hexhacking.xdl;

public final class Xdl {
    private static native long nativeOpen(String filename, boolean preload_symbols);
    private static native void nativeClose(long handle);

    private static final String libName = "xdl";
    private static boolean loaded = false;

    private static synchronized boolean loadLibrary() {
        if (loaded) {
            return true;
        }

        try {
            System.loadLibrary(libName);
            loaded = true;
            return true;
        } catch (Throwable ignored) {
            return false;
        }
    }

    public static long open(String filename, boolean preload_symbols) {
        if (loadLibrary()) {
            return nativeOpen(filename, preload_symbols);
        }
        return 0;
    }

    public static void close(long handle) {
        if (loadLibrary()) {
            nativeClose(handle);
        }
    }
}
