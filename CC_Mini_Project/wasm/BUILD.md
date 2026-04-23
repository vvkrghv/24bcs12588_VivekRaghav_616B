# How to compile engine.cpp → engine.wasm + engine.js

## Prerequisites
Install Emscripten SDK (https://emscripten.org/docs/getting_started/downloads.html):
```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

## Compile command
Run this from the `wasm/` directory:

```bash
emcc engine.cpp \
  -o ../public/engine.js \
  -s WASM=1 \
  -s MODULARIZE=0 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
  --bind \
  -O2
```

This outputs:
- `public/engine.js`   — Emscripten glue (JS)
- `public/engine.wasm` — compiled WebAssembly binary

## Changes from original engine.cpp
- Changed `std::vector<int>` → `std::vector<double>` so the same engine
  handles all three datasets (marks=int, sales=float, latency=long).
- Added all 6 algorithms: Merge, Quick, Heap, Counting, Radix, Bucket.
- SortMetrics now includes `stable` (bool) and all 4 complexity strings,
  so index.html can read them directly from WASM instead of hardcoding.
