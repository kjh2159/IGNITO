# Perfetto Example (`perfetto/qwen3_perf.cpp`)

This directory contains a **Perfetto tracing example** for IGNITE/mllm experiments on Android.

- Source: `qwen3_perf.cpp`
- Build script: `perfetto/CMakeLists.txt`
- Output trace file: `./traces.perfetto-trace`

For the perfetto execution, it might require to root the Android device.

---

## What this example does

`qwen3_perf.cpp` includes a helper function:

- `collect_power_rails(std::atomic<bool>& sigterm)`

When enabled, it creates a Perfetto tracing session using the **system backend** and collects Android power data:

- Battery counters
  - capacity percent
  - charge
  - current
  - voltage
- Power rails (`collect_power_rails=true`)
- Energy estimation breakdown (`collect_energy_estimation_breakdown=true`)

Trace buffer size is currently set to `1024 KB`.

When `sigterm` becomes `true`, the session stops and writes:

- `traces.perfetto-trace`

---

## Current code status (important)

At the top of `main()` in `qwen3_perf.cpp`, the program currently returns early:

```cpp
#if !defined(PERFETTO_IMPLEMENTATION)
  ...
  return 1;
#endif

return 0;
```

That means the full Qwen3 streaming/inference path below is **not executed** if `PERFETTO_IMPLEMENTATION` is not active (`-DPERFETTO=ON`).

So, the file should be treated as a **Perfetto integration example / scaffold**, not a fully active benchmark binary.

---

## Build

From project root:

```bash
cmake -S . -B build -DPERFETTO=ON
cmake --build build -j
```

Notes:

- Root `CMakeLists.txt` adds Perfetto only when `PERFETTO=ON`.
- `perfetto/CMakeLists.txt` builds executable:
  - `qwen3_perf`
- This target links Perfetto and Android log library in this folder setup.

---

## Run (example)

```bash
./bin/qwen3_perf
```

(Actual output path may vary by your build directory/output settings.)

If tracing is active and the tracing loop runs, it should generate:

- `traces.perfetto-trace`

Open trace in Perfetto UI:

- https://ui.perfetto.dev

---

<!-- ## Platform requirements / caveats

1. **Android system backend only**
   - This code uses `perfetto::kSystemBackend`.
   - Intended for Android devices with Perfetto support.

2. **Root/permission constraints**
   - Power rails and some counters may require device/kernel permissions.

3. **Device compatibility**
   - Comments in code indicate Pixel-centric validation.

4. **Current early return**
   - If you want integrated inference + tracing, remove/adjust the early `return 0;` in `main()`.

---

## Suggested next cleanup

To make this example production-ready:

1. Add a CLI switch (e.g. `--trace-only`) instead of hard early return.
2. Make output path configurable (`--trace-out`).
3. Add explicit logs for session start/stop and trace size.
4. Guard Android-only linking/headers with platform checks.
5. Reuse common runner helpers from `src/ignite/*` so tracing path and normal stream path share one execution flow. -->
