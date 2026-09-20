# KFighter Project Steering Document
### Guidelines for High-Quality Build, Fix, and Architecture

> **Purpose:** This steering document serves as the standard operating procedure and quality contract for building, testing, fixing, and evolving the **KFighter** Android project. All contributors, agents, and pipelines must adhere to these directives.

---

## 1. Core Engineering Directives

### 1.1 Portability First
- **Clean Decoupling**: Core game simulation (coordinate math, physics, collision detection, state machine, AI, data parsers) must remain **100% platform-independent C++20**.
- **No Direct Platform Intrusion**: Never sprinkle Android-specific headers (`<jni.h>`, `<android/log.h>`, etc.) into core game logic. All platform hooks must be isolated behind an abstraction layer (HAL) implemented via SDL2 or dedicated native platform interfaces.
- **Host Testability**: The core engine must compile and run on both the host machine (Windows / Linux / macOS) for rapid iteration and unit testing, and cross-compile cleanly to Android ARM64 (`arm64-v8a`) and x86_64.

### 1.2 Memory Safety & Performance Budget
- **Zero Runtime Allocations in Game Loop**: Frame updates (`tick()`) and render loops must never invoke heap allocation (`new`, `malloc`, dynamic vector reallocations). Pre-allocate entity pools, particle buffers, and vertex arrays at level load time.
- **Deterministic 60 FPS Target**: Budget frame processing at **< 16.6 ms per frame** on target Android devices. Physics update is locked at a fixed timestep (60 Hz or 30 Hz LF2 standard).
- **64-bit Clean**: Strictly avoid assumptions about pointer width, structure alignment, or endianness. The legacy code's hardcoded Win32 pointer arithmetic (`(void *) 0x458B00`) must be fully replaced by structured instances.

### 1.3 Documentation & History Maintenance
- **Living Design Document (`DESIGN.md`)**: Whenever an architectural change, new subsystem, or feature is added, `DESIGN.md` **must be updated immediately**.
- **Timestamped History Log (`HISTORY.md`)**: Every modification session must append a new timestamped entry in `HISTORY.md` detailing the rationale, modified files, and outcomes.

---

## 2. Build Pipeline & Standards

### 2.1 Android Build Configuration
- **Android Target**: Android 14 / 15 (API level 34+), Min SDK API 24 (Android 7.0+).
- **NDK Version**: NDK r26+ (LTS).
- **Architectures (ABIs)**: `arm64-v8a` (primary target), `x86_64` (emulator support).
- **C++ Standard**: C++20 (`-std=c++20`), with strict compiler warnings enabled:
  ```cmake
  target_compile_options(${PROJECT_NAME} PRIVATE
      -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror=return-type
  )
  ```

### 2.2 Dual Build System Architecture
1. **Desktop Harness (CMake)**:
   - Compiles the game natively for desktop to facilitate sub-second build/test loops and immediate visual debugging.
2. **Android Gradle Plugin + CMake (Android Studio / CLI)**:
   - Builds the shared native library (`libkfighter.so`) and bundles it into an APK alongside assets in `assets/`.

---

## 3. High-Quality "Fix and Debug" Workflows

### 3.1 Resolving Native Crashes (SIGSEGV / SIGABRT)
When a crash occurs on Android:
1. **Extract Tombstone / Logcat**:
   ```bash
   adb logcat -d -s DEBUG > crash_dump.txt
   ```
2. **Symbolicate the Stack Trace**:
   Use `ndk-stack` with the unstripped shared library (`app/build/intermediates/merged_native_libs/...`):
   ```bash
   $NDK/ndk-stack -sym path/to/unstripped/libkfighter.so -dump crash_dump.txt
   ```
3. **Inspect Precise Line via LLVM Addr2Line**:
   ```bash
   $NDK/toolchains/llvm/prebuilt/windows-x86_64/bin/llvm-addr2line -e libkfighter.so -f -C 0x12345
   ```
4. **Fix & Validate**: Identify bounds violations, null pointer dereferences, or use-after-free conditions. Add unit tests reproducing the crash before committing the fix.

### 3.2 Resolving Build & Link Failures
- **Missing Symbols (`undefined reference to ...`)**: Verify `CMakeLists.txt` includes the new source file in `target_sources()` or file glob, and ensure proper export macros (`JNIEXPORT`, `extern "C"`) if called across JNI boundaries.
- **Calling Convention Mismatches**: Strip all legacy MSVC Win32 `__thiscall` or `__stdcall` attributes. Use standard C++ method invocation.
- **Dependency Missing**: Ensure SDL2 / SDL_image / SDL_mixer Android libraries are linked in `target_link_libraries()`.

### 3.3 Touch Latency & Stutter Troubleshooting
- **Input Lag**: Ensure touch events are polled and buffered in a lock-free queue rather than blocking on the rendering thread.
- **Garbage Collection Pauses**: Minimize JNI crossings. Keep touch dispatch strictly on the native activity event queue.
- **Audio Lag**: Configure OpenSL ES / AAudio buffer sizes to matching native device sample rates (typically 48000 Hz with low buffer counts) to prevent audio crackling and delay.

---

## 4. Kerala Asset & Content Quality Gates

### 4.1 Visual Assets (Characters & Stages)
- **Format**: PNG with 32-bit RGBA color and premultiplied alpha where applicable.
- **Sprite Alignment**: All animation frames must declare precise `centerx` and `centery` alignment points to prevent jitter between stance, movement, and attack frames.
- **Cultural Authenticity**: Ensure Kalaripayattu stances (*Chuvadukal* like *Gaja Vadivu*, *Simha Vadivu*, *Ashwa Vadivu*) and traditional weapons (*Urumi*, *Churika*, *Otta*) conform to historical and artistic martial arts references.

### 4.2 Audio Assets
- **Format**: Ogg Vorbis (`.ogg`) for background music and uncompressed WAV / Ogg for low-latency sound effects.
- **Normalization**: All SFX must be LUFS-normalized to prevent clipping or uneven volume transitions between different character hits and environmental sounds.

---

## 5. Definition of Done (DoD) for Releases

Before any APK release or milestone completion:
- [ ] Compiles with zero compiler warnings or errors on ARM64 and x86_64.
- [ ] No memory leaks detected under AddressSanitizer (ASan).
- [ ] Consistent 60 FPS maintained during a 4-fighter combat scenario.
- [ ] Virtual touch controls respond seamlessly without deadzones or missed taps.
- [ ] Audio plays back without distortion or latency on test devices.
- [ ] `DESIGN.md` updated with all new systems and design adjustments.
- [ ] `HISTORY.md` appended with complete timestamped record of changes.
