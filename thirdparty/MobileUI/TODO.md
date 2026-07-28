I'll take a look at the module structure first to see what's in there.

The five remaining agents will notify me as they complete — no polling needed. I'll consolidate everything into the final report once they're all done.

Agent 2 (Ownership & Lifecycle) is done — one confirmed finding (the singleton's static pointer can dangle or be created parentless around app construction/teardown) plus four investigation targets. Four agents still running.

Agent 4 (API & C++ Correctness) is done with 11 confirmed findings — including a doc/code mismatch on the luminance cutoff, a `screenBrightness` property with inconsistent read/write semantics, and a misdocumented "bit flags" enum. Three agents still running (Thread Safety, Error Handling, Performance).

Agent 3 (Thread Safety) is done — two confirmed findings: a data race where Qt window objects are read from the Android UI thread in a fire-and-forget lambda, and four synchronous JNI bridges that can block the Qt GUI thread indefinitely around pause/resume. Two agents remaining.

Agent 5 (Error Handling) is done — five confirmed findings, notably a dead JNI exception check in `setTorch` that makes it report success after a `CameraAccessException`, and inconsistent `isValid()` guarding across the Android helpers. One agent left (Performance & Quality), then I'll consolidate the full report.

All six agents are done. Consolidating and deduplicating now — several agents converged on the same hotspots (the `screenBrightness` property, the statusbar/navbar duplication, the one-shot `connectSignals()`), which I've merged into single findings.

## Qt Code Review Report

**Scope**: files: `MobileUI/` (MobileUI.h, MobileUI.cpp, MobileUI_android.cpp, MobileUI_ios.mm, MobileUI_dummy.cpp, MobileUI_private.h, MobileUI_QmlDispatcher.h/.cpp, CMakeLists.txt)
**Files reviewed**: 9
**Issues found**: 42 (18 from lint, 24 from deep analysis) + 9 investigation targets

Overall the library is in good shape: pimpl via `unique_ptr` is correct, QML singleton ownership is pinned with `CppOwnership`, all connects use context objects, JNI local refs are adopted properly, and ARC is enforced in the `.mm`. The findings cluster around a few real bugs (torch error handling, stale auto-theme, one data race) and a larger set of consistency/duplication cleanups.

---

### Lint findings

#### [L-001] Unscoped enums without explicit underlying type
- **File**: [MobileUI.h:133](MobileUI/MobileUI.h:133) (`Theme`), [MobileUI.h:388](MobileUI/MobileUI.h:388) (`ScreenLockOrientation`), [MobileUI.h:498](MobileUI/MobileUI.h:498) (`HapticFeedback`)
- **Rule**: ENM-2
- **Finding**: All three public enums have no explicit underlying type, so the compiler-chosen type can change and break binary compatibility.
- **Mitigation**: Declare them as `enum Theme : int { ... }` (safe, non-breaking; they're all Q_ENUM'd and cross the QML boundary as int anyway).

#### [L-002] `get`-prefix on plain getters (15 occurrences)
- **File**: [MobileUI.h](MobileUI/MobileUI.h) lines 330, 340, 348, 354, 360, 369, 426, 442, 454, 474, 527, 545; [MobileUI_private.h](MobileUI/MobileUI_private.h) lines 38, 56, 58
- **Rule**: API-5
- **Finding**: Qt convention reserves `get` for out-parameter decomposition; property accessors should be `statusbarColor()`, not `getStatusbarColor()`.
- **Mitigation**: This is the library's established published API, so renaming the public accessors is a breaking change — if you care, add convention-named accessors and keep the old ones as deprecated aliases. The private-header ones ([MobileUI_private.h](MobileUI/MobileUI_private.h)) can be renamed freely.

---

### Deep analysis findings

#### [D-001] Auto-derived bar theme goes stale when the OS switches light/dark
- **File**: [MobileUI.cpp:131](MobileUI/MobileUI.cpp:131), [MobileUI.cpp:160](MobileUI/MobileUI.cpp:160), [MobileUI.cpp:287](MobileUI/MobileUI.cpp:287)
- **Category**: Performance & Quality (correctness)
- **Confidence**: 82/100
- **Finding**: In Auto mode with no usable reference color, `m_statusbarThemeSet` is pinned to `m_osTheme`. When the OS theme later flips, the `colorSchemeChanged` lambda runs `refreshMobileUI()` *before* `refreshDeviceTheme()`, and nothing ever re-runs `set*Theme_fromColor_refresh()` — so the old OS theme keeps getting actively re-applied on every refresh (including all four retry timers).
- **Trace**: Followed `QStyleHints::colorSchemeChanged` → lambda ordering → `refreshDeviceTheme()` only emits the signal, never touches `m_*ThemeSet`; `refreshSystemBars()` re-applies the stale value.
- **Mitigation**: In `refreshDeviceTheme()`, when `m_osTheme` changes, re-run both `_fromColor_refresh()` helpers (or reorder the lambda so the device theme is refreshed before the bars).

#### [D-002] `setTorch` reports success after `CameraAccessException` — the JNI exception check is dead code
- **File**: [MobileUI_android.cpp:707](MobileUI/MobileUI_android.cpp:707), state cached at [MobileUI.cpp:611](MobileUI/MobileUI.cpp:611)
- **Category**: Error Handling
- **Confidence**: 85/100
- **Finding**: `QJniObject::callMethod` checks and clears Java exceptions internally, so the explicit `env.checkAndClearExceptions()` after `setTorchMode` never fires. On failure, `setTorch` returns `on`, `m_torchEnabled` caches the wrong state, and `torchUpdated` is emitted for a torch that never lit (or never turned off).
- **Trace**: Every fallible camera call goes through `QJniObject`; only the raw `GetArrayLength`/`GetObjectArrayElement` calls touch the env directly, and those don't throw here.
- **Mitigation**: Call `setTorchMode` through raw JNI and check the exception yourself, verify via `CameraManager.registerTorchCallback`, or document `torchEnabled` as best-effort.

#### [D-003] Data race: Qt window objects read from the Android UI thread
- **File**: [MobileUI_android.cpp:279](MobileUI/MobileUI_android.cpp:279) (`setColor_navbar`)
- **Category**: Thread Safety
- **Confidence**: 85/100
- **Finding**: The fire-and-forget `runOnAndroidMainThread` lambda calls `qApp->allWindows()` and `QWindow::visibility()` from the Android UI thread while the Qt main thread keeps running — a data race and potential use-after-free during shutdown or window re-creation on resume (exactly when `refreshSystemBars()` fires).
- **Trace**: Unlike `getSafeAreaMetrics` (serialized by `waitForFinished()`), this is the only unserialized Qt-object access in the backend.
- **Mitigation**: Evaluate the fullscreen state on the Qt thread and capture a `const bool` by value — the pattern `setScreenLockOrientation` already uses.

#### [D-004] Qt GUI thread blocked with an infinite deadline on the Android UI thread
- **File**: [MobileUI_android.cpp:203](MobileUI/MobileUI_android.cpp:203), [MobileUI_android.cpp:339](MobileUI/MobileUI_android.cpp:339), [MobileUI_android.cpp:392](MobileUI/MobileUI_android.cpp:392), [MobileUI_android.cpp:409](MobileUI/MobileUI_android.cpp:409)
- **Category**: Thread Safety
- **Confidence**: 80/100
- **Finding**: Four synchronous bridges (`getDeviceTheme`, `getSafeAreaMetrics`, `getKeyboardHeight`, `getScreenBrightness`) use `.result()`/`.waitForFinished()` with the default `QDeadlineTimer::Forever`. If the app is pausing (runnables are queued, not run), the Qt GUI thread parks indefinitely — the classic ANR/deadlock shape, triggered exactly around lifecycle transitions.
- **Trace**: Verified `runOnAndroidMainThread` pause-queueing semantics against Qt docs; the call chains fire from `colorSchemeChanged`, retry timers, and `QInputMethod` signals.
- **Mitigation**: Pass a bounded `QDeadlineTimer` (keep the cached value on timeout) or restructure as `.then()` continuations. Caveat: `getSafeAreaMetrics` captures its outputs by reference (`[&]`), which is only safe because of the blocking wait — any deadline change must switch it to returning values through the `QVariant`.

#### [D-005] `screenBrightness` property: blocking JNI getter, bypassed cache, stale/over-eager NOTIFY, unclamped cache, non-const accessor
- **File**: [MobileUI.h:97](MobileUI/MobileUI.h:97), [MobileUI.cpp:518](MobileUI/MobileUI.cpp:518), [MobileUI_android.cpp:409](MobileUI/MobileUI_android.cpp:409), [MobileUI_QmlDispatcher.cpp:42](MobileUI/MobileUI_QmlDispatcher.cpp:42)
- **Category**: Performance & Quality / API
- **Confidence**: 88/100
- **Finding**: Several converging defects on one property. The READ accessor live-queries the backend — on Android a *blocking* main-thread hop plus 4+ JNI calls — on every binding evaluation, while the maintained `m_screenBrightness` cache is never returned. `screenUpdated` is shared by five unrelated setters and the dispatcher fans it into `screenBrightnessChanged`, so toggling `screenSecure` forces every brightness binding to redo the blocking round-trip. Read-back can differ from the value just written (backend clamps, cache doesn't — despite the header promising clamping), the getter's value can change with no NOTIFY (user changes system brightness), and it's the only property with a non-const READ accessor (it can be made `const` with zero other changes).
- **Trace**: Followed the property through `getScreenBrightness()` → `d->getScreenBrightness()` → `.result().toInt()`; compared setter cache/change-detection at [MobileUI.cpp:523](MobileUI/MobileUI.cpp:523) against backend clamping.
- **Mitigation**: Pick one semantic: return the cached value (with a separate `Q_INVOKABLE` for a live OS read), normalize/clamp once at the API boundary before caching, give brightness its own NOTIFY signal, and mark the getter `const`.

#### [D-006] Singleton static can dangle after app teardown, and crashes/leaks if created before `QGuiApplication`
- **File**: [MobileUI.cpp:38](MobileUI/MobileUI.cpp:38), constructor at [MobileUI.cpp:53](MobileUI/MobileUI.cpp:53)
- **Category**: Ownership & Lifecycle
- **Confidence**: 80/100
- **Finding**: `getInstance()` news the instance parented to `QCoreApplication::instance()` and never resets the static pointer. After app destruction the parent deletes the object but `getInstance()` keeps returning the dangling pointer. Before app construction, `instance()` is null, so the singleton is created parentless (leak) and the constructor immediately dereferences `qApp` (`inputMethod()` at line 59, `primaryScreen()` at line 76) — a null-pointer crash on device.
- **Trace**: No `qAddPostRoutine`, `QPointer`, or `destroyed()` hookup clears the static; no null check on `qApp` in the constructor; the QML `create()` path funnels into the same static.
- **Mitigation**: Null the static on `destroyed()` (or use a `QPointer`-based accessor), and guard/document the "must be created after QGuiApplication" precondition.

#### [D-007] Inconsistent `isValid()` guarding across Android JNI helpers
- **File**: [MobileUI_android.cpp:93](MobileUI/MobileUI_android.cpp:93) (`getAndroidWindow`), also lines 183, 203, 414, 442
- **Category**: Error Handling
- **Confidence**: 82/100
- **Finding**: `getAndroidDisplay()`, `getVibrator()`, `setTorch()`, and `setScreenLockOrientation()` check validity rigorously; `getAndroidWindow()`, `getAndroidDecorView()`, `getDeviceTheme()`, `dimenHeight()`, and the brightness/refresh-rate paths chain calls unchecked. Qt degrades to invalid objects returning default values, so the result is silent wrong answers (e.g. `getDeviceTheme()` reporting Light when the context is invalid) with zero diagnostics — and two coding styles in one file.
- **Trace**: Compared all `context()` call sites: five guarded, four not; all nine consumers of `getAndroidWindow()` proceed without validating it.
- **Mitigation**: One convention: `getAndroidWindow()` bails early on invalid context; callers bail (optionally one `qWarning`) on an invalid window.

#### [D-008] ~120 lines of statusbar/navbar logic copy-pasted, already drifting
- **File**: [MobileUI.cpp:187](MobileUI/MobileUI.cpp:187) vs [MobileUI.cpp:313](MobileUI/MobileUI.cpp:313)
- **Category**: Performance & Quality
- **Confidence**: 90/100
- **Finding**: The six statusbar functions and their navbar twins are textual clones differing only in members/backend call/signal. Drift has started: line 277 has a three-space indent and is missing the `const` its navbar twin at line 403 has. Every fix in this area (D-001, D-010) must be applied twice.
- **Trace**: Side-by-side diff — only identifiers differ.
- **Mitigation**: Factor a private helper parameterized over a "bar" (member refs, backend setter, signal) so the derivation logic exists once. At minimum fix the `const`/indent at line 277.

#### [D-009] System-bar refresh fans out into ~25 Android main-thread hops per rotation/resume
- **File**: [MobileUI.cpp:140](MobileUI/MobileUI.cpp:140), [MobileUI_android.cpp:221](MobileUI/MobileUI_android.cpp:221)
- **Category**: Performance & Quality
- **Confidence**: 85/100
- **Finding**: `refreshMobileUI()` runs the refresh immediately plus four timer-driven retries; each round posts up to four separate `runOnAndroidMainThread` runnables (each re-resolving window/insets controller from scratch) plus one *blocking* safe-area dispatch — up to ~20 posts and 5 GUI-thread blocks per orientation/visibility event.
- **Trace**: Followed `connectSignals()` → timers → `refreshSystemBars()` → the four backend setters, each with its own dispatch.
- **Mitigation**: Add one combined backend entry point (`applySystemBars(...)`) that resolves the window once per round — the pattern `getSafeAreaMetrics()` already uses — and make the retry-driven safe-area re-read non-blocking.

#### [D-010] Color setters can emit their update signal twice per call; no re-entrancy guard
- **File**: [MobileUI.cpp:192](MobileUI/MobileUI.cpp:192) with [MobileUI.cpp:273](MobileUI/MobileUI.cpp:273) (navbar twins at 318/399)
- **Category**: Performance & Quality
- **Confidence**: 82/100
- **Finding**: `setStatusbarColor()` calls `setStatusbarTheme_fromColor_refresh()`, which may emit `statusbarUpdated` itself, then the setter emits it again — two emissions per write, which the dispatcher amplifies into six property-changed notifications. A slot writing back into a statusbar setter re-enters mid-update.
- **Trace**: Followed the emit at line 283 inside the helper, then `if (changed) Q_EMIT statusbarUpdated()` at line 206.
- **Mitigation**: Have the `_fromColor` helpers return whether the theme changed and let the public setter emit exactly once.

#### [D-011] `ScreenLockOrientation` documented as bit flags but implemented as exclusive values; no enum-range validation
- **File**: [MobileUI.h:382](MobileUI/MobileUI.h:382), [MobileUI_android.cpp:460](MobileUI/MobileUI_android.cpp:460), [MobileUI_ios.mm:200](MobileUI/MobileUI_ios.mm:200)
- **Category**: API & C++ Correctness
- **Confidence**: 84/100
- **Finding**: The doc says "the values are bit flags, so a sensor mode is conceptually the union of its two fixed orientations" — but `Portrait_sensor = (1 << 2)` is not `Portrait | Portrait_upsidedown`, the enum is `Q_ENUM` not `Q_FLAG`, and both backends compare with exact `==`. Any OR-ed combination or out-of-range int silently falls through to *unlocked* (unspecified/MaskAll) while the property caches and reports the bogus value as in force. Theme setters accept out-of-range values the same way.
- **Trace**: Read the enum doc; confirmed both backends' if/else chains never mask bits; followed the theme ternary `(theme == Light) ? ... : 0`.
- **Mitigation**: Drop the "bit flags/union" wording (cheap, honest fix) or make it a real `Q_FLAG` with mask decoding; validate incoming values with a `qWarning` and only cache values actually mapped to a platform request.

#### [D-012] Doc/code mismatch: luminance cutoff "~0.66" vs actual `0.8`
- **File**: [MobileUI.h:571](MobileUI/MobileUI.h:571) vs [MobileUI.cpp:639](MobileUI/MobileUI.cpp:639)
- **Category**: API & C++ Correctness
- **Confidence**: 90/100
- **Finding**: The header claims `isColorLight_android()` uses the ~0.66 cutoff matching Android's behavior; the code uses `> 0.8`. This function is the pivot of all Auto theme derivation, so the documented contract materially misleads users.
- **Trace**: Direct comparison; both `derive*Theme` functions call it.
- **Mitigation**: Align doc and code, and name the constant (alongside the HyperOS `0.5`) so they can't drift again.

#### [D-013] Header docs describe a "hand control back to the OS" fallback the code doesn't do
- **File**: [MobileUI.h:224](MobileUI/MobileUI.h:224), [MobileUI.h:285](MobileUI/MobileUI.h:285), [MobileUI.h:641](MobileUI/MobileUI.h:641) vs [MobileUI.cpp:299](MobileUI/MobileUI.cpp:299)
- **Category**: Performance & Quality (documentation)
- **Confidence**: 85/100
- **Finding**: The header says that with no usable color, theme control returns to the OS; the implementation actively pins `m_*ThemeSet = m_osTheme` and pushes it — a deterministic override, not a release. The README documents the *actual* behavior, so the header docs are the stale side.
- **Trace**: Compared header doc blocks against the `else` branch of `set*Theme_fromColor_refresh()` and the README.
- **Mitigation**: Update the header doc comments to match the deterministic-fallback behavior.

#### [D-014] No coherent failure-reporting policy across the three backends
- **File**: [MobileUI_ios.mm:213](MobileUI/MobileUI_ios.mm:213), [MobileUI_ios.mm:233](MobileUI/MobileUI_ios.mm:233), [MobileUI_android.cpp](MobileUI/MobileUI_android.cpp) throughout
- **Category**: Error Handling
- **Confidence**: 82/100
- **Finding**: Across all failure/unsupported paths: 2 use `qDebug`, 1 uses `qWarning`, ~15 are silent, and 1 returns a bool. The iOS orientation `errorHandler` receives a concrete `NSError` and discards it for a fixed string. The Android backend never logs anything. Only `setTorch` reports failure upward, so the public layer caches and emits regardless of what actually happened.
- **Trace**: Enumerated every failure path in the three `MobileUIPrivate` implementations.
- **Mitigation**: One convention — e.g. `qCWarning(lcMobileUI)` under a logging category for genuine failures, silence for documented platform no-ops; include `error.localizedDescription` on iOS; consider extending the bool-return pattern where the platform can refuse.

#### [D-015] `vibrate()` is an undocumented exact duplicate of `hapticFeedback()`
- **File**: [MobileUI.h:515](MobileUI/MobileUI.h:515), [MobileUI.cpp:600](MobileUI/MobileUI.cpp:600)
- **Category**: API & C++ Correctness
- **Confidence**: 85/100
- **Finding**: Two Q_INVOKABLEs with identical signatures and byte-for-byte identical bodies, neither marked as an alias — duplicate public API with no deprecation path.
- **Trace**: Compared both bodies; README documents them as interchangeable.
- **Mitigation**: Have `vibrate()` forward to `hapticFeedback()` and mark it a legacy alias (or `[[deprecated]]`).

#### [D-016] Three names for one property: `screenHighRefreshRate` / `getHighRefreshRate` / `setScreenHighRefreshRate`
- **File**: [MobileUI.h:100](MobileUI/MobileUI.h:100), [MobileUI_QmlDispatcher.h:114](MobileUI/MobileUI_QmlDispatcher.h:114)
- **Category**: API & C++ Correctness
- **Confidence**: 82/100
- **Finding**: The property is `screenHighRefreshRate` but its accessors drop "Screen" (`getHighRefreshRate`/`setHighRefreshRate`), unlike every sibling pair; the dispatcher then invents `setScreenHighRefreshRate` forwarding to `setHighRefreshRate`.
- **Trace**: Compared Q_PROPERTY lines 96–100 against accessor names; followed the dispatcher forwarders.
- **Mitigation**: Standardize on the `Screen`-prefixed accessors, keeping old names as thin deprecated wrappers since this is published API.

#### [D-017] Signal `devicethemeUpdated` breaks camelCase
- **File**: [MobileUI.h:105](MobileUI/MobileUI.h:105)
- **Category**: API & C++ Correctness
- **Confidence**: 80/100
- **Finding**: Every other signal is camelCase; QML users typing the conventional `onDeviceThemeUpdated` get a runtime "no such signal" surprise.
- **Trace**: Grepped the signal block and the emission at MobileUI.cpp:166.
- **Mitigation**: Rename to `deviceThemeUpdated`; if compatibility matters, emit both and deprecate the old spelling.

#### [D-018] `MobileUIPrivate` query methods are non-const and `getDeviceTheme()` returns `int`
- **File**: [MobileUI_private.h:38](MobileUI/MobileUI_private.h:38) and lines 53, 56, 58
- **Category**: API & C++ Correctness
- **Confidence**: 82/100
- **Finding**: The four pure queries mutate nothing in any backend (the class has no data members) yet none is `const` — which is what makes the non-const public getter in D-005 look justified. `getDeviceTheme()` returns raw `int`, forcing a `static_cast<MobileUI::Theme>` at its one call site.
- **Trace**: Checked all three backend implementations of each query.
- **Mitigation**: Const-qualify the four queries; return `MobileUI::Theme` and convert inside the Android backend.

#### [D-019] iOS helper functions with unintended external linkage
- **File**: [MobileUI_ios.mm:74](MobileUI/MobileUI_ios.mm:74), [MobileUI_ios.mm:89](MobileUI/MobileUI_ios.mm:89)
- **Category**: API & C++ Correctness
- **Confidence**: 82/100
- **Finding**: `statusBarStyle()` and `updatePreferredStatusBarStyle()` lack `static` while their three siblings have it — generic symbol names leak out of the static library and can collide with app symbols.
- **Trace**: Compared linkage of the five file-local helpers; both are used only inside this TU.
- **Mitigation**: Mark both `static` or use an anonymous namespace.

#### [D-020] Always-true `else if` in the Android theme setters
- **File**: [MobileUI_android.cpp:252](MobileUI/MobileUI_android.cpp:252), [MobileUI_android.cpp:320](MobileUI/MobileUI_android.cpp:320)
- **Category**: Performance & Quality
- **Confidence**: 85/100
- **Finding**: `if (sdkVersion() >= 30) {...} else if (sdkVersion() < 30) {...}` — the second condition is tautological and performs a redundant JNI-backed `sdkVersion()` query.
- **Trace**: Direct inspection of `setTheme_statusbar`/`setTheme_navbar`.
- **Mitigation**: Plain `else`, with `sdkVersion()` hoisted into a local (as `getSafeAreaMetrics` already does).

#### [D-021] README recommends a helper that QML can't call (and nothing uses)
- **File**: [README.md:270](MobileUI/README.md:270), [MobileUI.h:563](MobileUI/MobileUI.h:563)
- **Category**: Performance & Quality
- **Confidence**: 82/100
- **Finding**: The README tells (QML-facing) users to use `isColorLight_hyperos()`, but the three color helpers are plain static member functions — not `Q_INVOKABLE` — so they're unreachable from QML; `isColorLight_hyperos` has zero call sites repo-wide.
- **Trace**: Grepped the whole repo; checked the Q_INVOKABLE list.
- **Mitigation**: Make the helpers `Q_INVOKABLE` static, or reword the README to say they're C++-only.

#### [D-022] Commented-out code and disabled guards left in place
- **File**: [MobileUI.cpp:194](MobileUI/MobileUI.cpp:194), [MobileUI.cpp:320](MobileUI/MobileUI.cpp:320), [MobileUI_android.cpp:163](MobileUI/MobileUI_android.cpp:163), [MobileUI_android.cpp:179](MobileUI/MobileUI_android.cpp:179), [CMakeLists.txt:38](MobileUI/CMakeLists.txt:38)
- **Category**: Performance & Quality
- **Confidence**: 85/100
- **Finding**: Commented-out `qDebug` lines, two commented-out SDK-version guards (the `dimenHeight` one is genuinely ambiguous — `getRootWindowInsets` is API 28+ but the path only runs for `sdk < 30`), and a commented CMake `OUTPUT_DIRECTORY`.
- **Trace**: Direct inspection.
- **Mitigation**: Delete, or for the guards, decide and either enable them or note the minSdk assumption.

#### [D-023] Load-bearing magic numbers without named constants
- **File**: [MobileUI.cpp:79](MobileUI/MobileUI.cpp:79) (7.0" tablet threshold, mm-to-inch factor), [MobileUI.cpp:639](MobileUI/MobileUI.cpp:639) (0.8/0.5 luminance cutoffs), [MobileUI_android.cpp:364](MobileUI/MobileUI_android.cpp:364) (24/48 dp bar fallbacks)
- **Category**: Performance & Quality
- **Confidence**: 80/100
- **Finding**: Device classification, theme derivation, and legacy bar sizing all hinge on anonymous literals; the 0.8 cutoff has already drifted from its own documentation (D-012).
- **Trace**: Direct inspection; retry-timer delays and the JNI flag defines are well handled by contrast.
- **Mitigation**: Named `constexpr` values (`kTabletDiagonalInches`, `kAndroidLightCutoff`, etc.).

#### [D-024] Missing trailing comma on `Theme`'s last enumerator
- **File**: [MobileUI.h:136](MobileUI/MobileUI.h:136)
- **Category**: API & C++ Correctness
- **Confidence**: 80/100
- **Finding**: The other two enums end with a trailing comma; `Theme` doesn't. Pure consistency/diff-noise.
- **Mitigation**: Add the comma.

---

### Investigation targets (human verification needed)

#### [I-001] One-shot `connectSignals()` can permanently miss the window/screen
- **File**: [MobileUI.cpp:88](MobileUI/MobileUI.cpp:88), [MobileUI.cpp:117](MobileUI/MobileUI.cpp:117)
- **Category**: Ownership & Lifecycle (flagged independently by three agents)
- **Confidence**: 72/100
- **Finding**: The deferred `singleShot(0)` hookup grabs `allWindows().first()` and the primary screen once, never retried. If no `QWindow` exists at that tick (C++-first instantiation before `engine.load()`), or a foldable replaces the `QScreen`/`QWindow`, visibility/orientation tracking silently stops.
- **Unverified because**: real-device event ordering and screen-recreation behavior on foldables couldn't be tested statically.
- **How to verify**: Instantiate `MobileUI::getInstance()` in `main()` before loading QML; check whether `safeAreaUpdated` still fires on fullscreen/rotation. Fix would be re-hooking on `focusWindowChanged`/`screenAdded`.

#### [I-002] No thread-affinity guard on `getInstance()` and the setters
- **File**: [MobileUI.cpp:38](MobileUI/MobileUI.cpp:38), [MobileUI.h:656](MobileUI/MobileUI.h:656)
- **Category**: Thread Safety
- **Confidence**: 70/100
- **Finding**: First `getInstance()` from a worker thread would create the object with worker affinity: parenting rejected (leak), the four `QTimer` members unusable, later main-thread `start()` calls warn and no-op.
- **Unverified because**: no in-tree consumer does this; it's a latent contract issue for the C++ surface.
- **How to verify**: Grep downstream apps; or add a temporary main-thread assert in the constructor. Documenting a main-thread-only precondition also covers I-006.

#### [I-003] `Qt6::Quick` link dependency appears unnecessary
- **File**: [CMakeLists.txt:28](MobileUI/CMakeLists.txt:28), lines 35, 41
- **Category**: API & C++ Correctness
- **Confidence**: 70/100
- **Finding**: No source includes any Qt Quick header — only Core/Gui/Qml symbols are used — yet the library finds, links, and declares `DEPENDENCIES QtQuick`.
- **Unverified because**: couldn't build to confirm nothing relies on the transitive dependency.
- **How to verify**: Switch to `Core Gui Qml`, drop `DEPENDENCIES QtQuick`, rebuild demo for desktop + Android + iOS.

#### [I-004] iOS: unchecked cast of `rootViewController` to Qt's private `QIOSViewController`
- **File**: [MobileUI_ios.mm:44](MobileUI/MobileUI_ios.mm:44), [MobileUI_ios.mm:82](MobileUI/MobileUI_ios.mm:82)
- **Category**: Ownership & Lifecycle / Error Handling
- **Confidence**: 68/100
- **Finding**: The file re-declares Qt's private class and `static_cast`s the key window's root view controller to it. If the key window isn't Qt's (hybrid app, alert window), `setPreferredStatusBarStyle:` raises "unrecognized selector" — a crash.
- **Unverified because**: depends on runtime window configuration and the private class's setter surviving Qt version bumps.
- **How to verify**: Test in a hybrid/embedded-Qt host; a `respondsToSelector:` guard makes it safe regardless.

#### [I-005] Torch state cache can go stale from outside
- **File**: [MobileUI.cpp:611](MobileUI/MobileUI.cpp:611)
- **Category**: Performance & Quality
- **Confidence**: 65/100
- **Finding**: The OS or another app can turn the torch off (camera acquisition, screen off); nothing invalidates `m_torchEnabled` on resume. The header does document it as "last state MobileUI successfully applied", so this may be intended scope.
- **Unverified because**: intent question, not a code trace.
- **How to verify**: Decide intent; `CameraManager.registerTorchCallback` provides authoritative tracking if wanted (and would also fix D-002 cleanly).

#### [I-006] iOS backend issues UIKit calls with no main-queue marshalling
- **File**: [MobileUI_ios.mm](MobileUI/MobileUI_ios.mm) throughout (lines 53, 86, 169, 213, 225, 247–284)
- **Category**: Thread Safety
- **Confidence**: 65/100
- **Finding**: Unlike the Android backend, iOS calls UIKit directly on the caller's thread. Fine for QML-driven use (Qt's iOS main thread *is* the native main thread), but a worker-thread C++ call hits UIKit off-main — undefined behavior.
- **Unverified because**: only manifests in the same off-main-thread scenario as I-002.
- **How to verify**: Xcode Main Thread Checker + a setter call from a secondary thread. Resolving I-002's contract covers this too; otherwise wrap bodies in `dispatch_async(main queue)`.

#### [I-007] Android `getScreenBrightness` returns 0 instead of the −1 sentinel on an invalid window
- **File**: [MobileUI_android.cpp:414](MobileUI/MobileUI_android.cpp:414)
- **Category**: Error Handling
- **Confidence**: 65/100
- **Finding**: `getField<jfloat>` on an invalid `layoutParams` returns `0.0f`, which passes the `>= 0.f` test and reports "screen fully dim" instead of the documented −1.
- **Unverified because**: depends on `QJniObject::getField` default-constructing on invalid objects in the deployed Qt version.
- **How to verify**: Check `qjniobject.cpp` for your Qt version; guard with `layoutParams.isValid()` regardless.

#### [I-008] Redundant first branch in `getStatusbarThemeSet()`/`getNavbarThemeSet()`
- **File**: [MobileUI.cpp:230](MobileUI/MobileUI.cpp:230), [MobileUI.cpp:356](MobileUI/MobileUI.cpp:356)
- **Category**: Performance & Quality
- **Confidence**: 65/100
- **Finding**: The write paths appear to maintain the invariant `m_theme > Auto ⇒ m_themeSet == m_theme`, making the getter's first branch dead.
- **Unverified because**: a future write path could break the invariant.
- **How to verify**: Confirm the invariant is intended, then simplify or assert it.

#### [I-009] Silent no-op of bar colors on Android 15+ (API 35) is undocumented in the header
- **File**: [MobileUI_android.cpp:224](MobileUI/MobileUI_android.cpp:224), [MobileUI_android.cpp:274](MobileUI/MobileUI_android.cpp:274)
- **Category**: Error Handling
- **Confidence**: 62/100
- **Finding**: `setColor_statusbar`/`setColor_navbar` do nothing on API ≥ 35 (intentional — mandatory edge-to-edge, per your recent commits), yet the C++ layer still caches, emits, and reports the colors as set, and the setter docs in [MobileUI.h:199](MobileUI/MobileUI.h:199) don't mention the cutoff.
- **Unverified because**: intent is clear from commit history, but whether the header silence is deliberate isn't.
- **How to verify**: If intentional, state the API-35 behavior in the setter docs so "silent success" is a documented contract.

---

### Summary

| Category | Lint | Deep | Investigate | Total |
|----------|------|------|-------------|-------|
| Enums & API naming (lint) | 18 | — | — | 18 |
| Model Contracts | — | 0 | 0 | 0 |
| Ownership & Lifecycle | — | 1 | 2 | 3 |
| Thread Safety | — | 2 | 2 | 4 |
| API & C++ Correctness | — | 8 | 1 | 9 |
| Error Handling | — | 3 | 2 | 5 |
| Performance & Quality | — | 10 | 2 | 12 |
| **Total** | **18** | **24** | **9** | **51** |

Findings below confidence 60 were suppressed.

**If I had to pick five to fix first**: D-001 (stale auto-theme after OS dark/light switch — user-visible bug), D-002 (torch reports success on failure), D-003 (data race in `setColor_navbar` — one-line fix), D-004 (unbounded GUI-thread blocking around pause — ANR risk), and D-008 (the statusbar/navbar duplication, since fixing D-001/D-010 properly means touching that code twice otherwise). This was a read-only review — nothing was modified. Want me to apply any of these fixes?
