# forge — formal specification

Version 0.1 — draft

---

## 1. Purpose and scope

forge is a convention-based build tool for C and C++ projects. Its design
principle is that project structure implies build instructions. The user
declares what exists and how things relate; forge derives how to build,
link, and test everything from that declaration plus a fixed set of
filesystem conventions.

forge is intentionally scoped to single-developer and small-team projects.
It does not manage third-party packages, cross-compilation targets, or
installation. It is a personal build tool, not a distribution mechanism.

---

## 2. Definitions

**Project** — a directory tree rooted at the directory containing
`project.toml`. All paths in this specification are relative to the
project root unless stated otherwise.

**Library** (`[[lib]]`) — a named unit of compiled C or C++ source code
that produces a static archive and optionally a Python extension module.
Libraries may depend on other libraries.

**Executable** (`[[exe]]`) — a named unit of compiled C or C++ source code
that produces a binary. Executables depend on libraries but cannot be
depended upon.

**Grouping lib** — a `[[lib]]` entry with no resolvable source directory.
It produces no compiled output. It exists solely to aggregate dependencies
under a single name.

**Library root** — the directory inferred from a lib's `src` field by
stripping the last path component. Given `src = "libs/memory/src"`, the
root is `libs/memory`. All per-lib conventions are resolved relative to
the library root.

**Transitive closure** — for a given lib or exe, the set containing itself
and all libs reachable by following `deps` links recursively.

**Topo order** — a total ordering of `[[lib]]` entries such that for every
lib `A` that depends on lib `B`, `B` appears before `A`.

**Binding type** — one of `pybind11`, `cffi`, or `ctypes`. Determined
automatically by scanning `bindings/` source files.

**Extension suffix** — the platform- and Python-version-specific suffix for
Python extension modules, e.g. `.cpython-312-x86_64-linux-gnu.so`.
Obtained at build time from `sysconfig.get_config_var('EXT_SUFFIX')`.

---

## 3. Project layout

### 3.1 Multi-library project

```
project/
├── project.toml
├── include/                    global headers (optional)
└── libs/
    └── NAME/
        ├── src/                source files → .build/lib/libNAME.a
        │   └── **/*.{c,cpp,cc,cxx}
        ├── bindings/           Python binding sources (optional)
        │   └── *.{c,cpp,cc,cxx}
        └── tests/              test suite (optional)
            ├── *.{c,cpp,cc,cxx}
            └── *.py
```

### 3.2 Single-library / executable project

Used when no `[[lib]]` entries are declared.

```
project/
├── project.toml
├── include/                    global headers (optional)
├── src/                        source files
│   └── **/*.{c,cpp,cc,cxx}
├── bindings/                   Python binding sources (optional)
│   └── *.{c,cpp,cc,cxx}
└── tests/                      test suite (optional)
    ├── *.{c,cpp,cc,cxx}
    └── *.py
```

If `src/` contains a file named `main.c`, `main.cpp`, or `main.cc` at any
depth, the project is treated as an executable project and produces
`.build/bin/NAME`. Otherwise it produces `.build/lib/libNAME.a`.

### 3.3 Executable sources

```
project/
└── exe/
    └── NAME/
        └── src/
            └── **/*.{c,cpp,cc,cxx}
```

The default `src` path for `[[exe]]` entries is `exe/NAME/src`. An
explicit `src` field overrides this.

### 3.4 Build output tree

```
.build/
├── obj/        object files (flat namespace, path separators → underscores)
├── deps/       compiler-generated .d dependency files
├── lib/        static archives (.a) and Python extension modules (.so)
└── bin/
    ├── NAME              executables
    └── tests/            native test binaries
```

---

## 4. project.toml

### 4.1 Format

`project.toml` is a strict subset of TOML. forge supports only the
following constructs:

- Top-level scalar assignments: `key = "value"`
- Array-of-table headers: `[[lib]]` and `[[exe]]`
- Inline string arrays: `deps = ["a", "b"]`

No other TOML constructs (nested tables, integers, booleans, multiline
strings, dotted keys) are required or guaranteed to parse correctly.

### 4.2 Top-level fields

| Field      | Type   | Default     | Description |
|------------|--------|-------------|-------------|
| `name`     | string | `"project"` | Project name. Used to name build outputs in single-lib mode |
| `compiler` | string | `"cc"`      | Compiler binary. Any value accepted by the host shell |
| `std`      | string | `"c11"`     | Passed verbatim as `-std=VALUE` |
| `python`   | string | `"python3"` | Python interpreter. Used for binding introspection and pytest |
| `werror`   | string | `"true"`    | `"true"` passes `-Werror`. Any other value suppresses it |

### 4.3 [[lib]] fields

| Field  | Type         | Required | Default         | Description |
|--------|--------------|----------|-----------------|-------------|
| `name` | string       | yes      | —               | Library name. Must be unique across all `[[lib]]` entries |
| `src`  | string       | no       | `libs/NAME/src` | Source directory. Walked recursively |
| `deps` | string array | no       | `[]`            | Names of `[[lib]]` entries this lib depends on |

### 4.4 [[exe]] fields

| Field  | Type         | Required | Default        | Description |
|--------|--------------|----------|----------------|-------------|
| `name` | string       | yes      | —              | Executable name. Must not collide with any `[[lib]]` name |
| `src`  | string       | no       | `exe/NAME/src` | Source directory. Walked recursively |
| `deps` | string array | no       | `[]`           | Names of `[[lib]]` entries to link against |

### 4.5 Validation rules

The following conditions are checked before any compilation begins. Any
failure is a hard error that prints a diagnostic and exits non-zero.

1. `project.toml` must exist in the current directory.
2. Every `[[lib]]` entry must have a `name` field.
3. Every `[[exe]]` entry must have a `name` field.
4. All `[[lib]]` names must be unique.
5. No `[[exe]]` name may equal any `[[lib]]` name.
6. Every name in a `deps` array of a `[[lib]]` must refer to a declared
   `[[lib]]` name.
7. Every name in a `deps` array of a `[[exe]]` must refer to a declared
   `[[lib]]` name.
8. The `[[lib]]` dependency graph must be acyclic.
9. Every `[[exe]]` entry must have a resolvable `src` directory (either
   explicit or via the default inference).

---

## 5. Convention resolution

### 5.1 Library source directory

Given a `[[lib]]` entry with name `NAME`:

1. If an explicit `src` field is present, use it.
2. Otherwise check if `libs/NAME/src` exists. If so, use it.
3. Otherwise the lib is a grouping lib with no source directory.

### 5.2 Library root

Strip the last path component from `src`:

```
"libs/memory/src"       →  "libs/memory"
"src"                   →  "."
```

For grouping libs: `libs/NAME`.

### 5.3 Per-lib derived directories

Given library root `ROOT`:

| Convention      | Path              | Purpose |
|-----------------|-------------------|---------|
| Bindings        | `ROOT/bindings/`  | Python extension sources |
| Tests           | `ROOT/tests/`     | Native and Python tests |

Both are optional. forge checks for their existence at build time and
silently skips them if absent.

### 5.4 Executable source directory

Given an `[[exe]]` entry with name `NAME`:

1. If an explicit `src` field is present, use it.
2. Otherwise check if `exe/NAME/src` exists. If so, use it.
3. Otherwise it is a hard error.

### 5.5 Object file naming

Source file paths are flattened into a single namespace under `.build/obj/`
by replacing every `/` with `_` and replacing the file extension with `.o`.

```
libs/memory/src/allocators/scratch.cpp  →  .build/obj/libs_memory_src_allocators_scratch.o
```

The same transformation with `.d` extension is used for dependency files
under `.build/deps/`.

Collisions are possible if two files produce the same mangled name. This
is a known limitation and the user is responsible for avoiding it.

### 5.6 Global include path

If `include/` exists at the project root, `-I include` is added to every
compilation command in the project. This applies to lib sources, exe
sources, binding sources, and test sources.

---

## 6. Dependency resolution

### 6.1 Topological sort

forge sorts `[[lib]]` entries using an iterative algorithm (Kahn's
algorithm):

1. Initialise the ordered list as empty.
2. Find any lib whose deps are all already in the ordered list.
3. Append it to the ordered list. Repeat from step 2.
4. If no progress is made and libs remain, a cycle exists — hard error.

### 6.2 Transitive closure

For a given lib or exe, the transitive closure is computed by BFS over the
dependency graph. The closure is used to determine which `.a` files are
passed to the linker.

### 6.3 Link order

Static archives must appear on the linker command line in reverse
topological order: the most dependent lib first, the least dependent last.
This ensures that undefined symbols in a dependent lib are resolved by
archives that appear later on the command line.

---

## 7. Compilation

### 7.1 Source files

forge recognises files with the following extensions as source files:
`.c`, `.cpp`, `.cc`, `.cxx`. All other files are ignored.

Source directories (`src/`, `bindings/`, `tests/`) are walked recursively
for `.c`/`.cpp`/`.cc`/`.cxx` files, with the exception of `bindings/`
which is walked non-recursively (flat).

Python test files (`.py`) in `tests/` are not compiled by forge. They are
passed to pytest.

### 7.2 Compiler invocation — object file

```
COMPILER -std=STD WARN_FLAGS INC_FLAGS EXTRA_FLAGS -MMD -MF DEPFILE -c SOURCE -o OBJECT
```

| Placeholder   | Value |
|---------------|-------|
| `COMPILER`    | `compiler` field from `project.toml` |
| `STD`         | `-std=` + `std` field |
| `WARN_FLAGS`  | See §8 |
| `INC_FLAGS`   | `-I include` if `include/` exists, plus any binding-specific headers |
| `EXTRA_FLAGS` | Empty for lib/exe sources. `-fPIC` for ctypes/cffi bindings. `-fPIC -fexceptions -frtti` for pybind11 bindings |
| `DEPFILE`     | `.build/deps/MANGLED.d` |
| `SOURCE`      | Path to source file |
| `OBJECT`      | `.build/obj/MANGLED.o` |

### 7.3 Compiler invocation — static archive

```
ar rcs .build/lib/libNAME.a OBJECTS...
```

### 7.4 Compiler invocation — Python extension

```
COMPILER -shared OBJECTS LINK_ARGS [PY_LDFLAGS] [-Wno-unused-command-line-argument] -o .build/lib/MODULE.EXT_SUFFIX
```

`PY_LDFLAGS` is obtained from `python-config --ldflags`. The
`-Wno-unused-command-line-argument` flag is appended to suppress a
diagnostic that occurs on macOS when Python ldflags include arguments
irrelevant to the link step.

### 7.5 Compiler invocation — executable

```
COMPILER OBJECTS LINK_ARGS -o .build/bin/NAME
```

### 7.6 Compiler invocation — native test binary

```
COMPILER OBJECT LINK_ARGS -o .build/bin/tests/TESTNAME
```

Each test source file is compiled and linked independently. Test binaries
are named after their source file with the extension stripped.

### 7.7 Failure behaviour

A non-zero exit code from any compiler or archiver invocation is a hard
error. forge prints the failing source file path and exits immediately
without attempting further compilation.

---

## 8. Warning flags

### 8.1 Compiler family detection

The compiler family is detected by substring matching on the `compiler`
field:

| Substring match | Family |
|-----------------|--------|
| `gcc` or `g++`  | GCC    |
| `clang`         | Clang  |
| (neither)       | Generic |

### 8.2 Common flags (all families)

```
-Wall -Wextra -Wpedantic
-Wconversion -Wsign-conversion -Wshadow
-Wformat=2 -Wformat-security
-Wnull-dereference -Wdouble-promotion
-Wcast-align -Wcast-qual
-Wimplicit-fallthrough
-Wswitch-enum -Wswitch-default
-Wmissing-declarations -Wmissing-noreturn
-Wvla -Wstrict-overflow=2
-Wfloat-equal -Wundef
-Wpointer-arith -Wwrite-strings
-fstack-protector-strong
```

### 8.3 GCC-only flags

```
-Wduplicated-cond -Wduplicated-branches
-Wlogical-op -Wformat-signedness
-Warray-bounds=2 -Wuseless-cast
```

### 8.4 Clang-only flags

```
-Wunreachable-code -Wunreachable-code-break
-Wunreachable-code-return -Wloop-analysis
-Wconditional-uninitialized -Wcomma
-Wover-aligned -Warray-bounds
```

### 8.5 -Werror policy

`-Werror` is appended when `werror = "true"` (the default).

`-Werror` is **never** appended when compiling sources in `bindings/`,
regardless of the `werror` setting. Third-party binding headers (pybind11,
cffi) may produce warnings that the project does not control.

---

## 9. Python bindings

### 9.1 Detection

forge scans every source file in `bindings/` for the following markers:

| Marker                      | Binding type |
|-----------------------------|--------------|
| `PYBIND11_MODULE`           | `pybind11`   |
| `ffi.cdef` or `cffi`        | `cffi`       |
| (none of the above)         | `ctypes`     |

Detection stops at the first match. Files are scanned in filesystem order.

### 9.2 Module name (pybind11 only)

forge extracts the module name from the first occurrence of
`PYBIND11_MODULE(name, ...)` found by scanning binding sources in
filesystem order. The extracted name is used as the base of the output
filename. If no match is found, the project `name` field is used as
fallback.

### 9.3 Extension suffix

The output filename suffix is obtained by running:

```
python3 -c "import sysconfig; print(sysconfig.get_config_var('EXT_SUFFIX'))"
```

This produces a platform- and interpreter-specific suffix such as
`.cpython-312-x86_64-linux-gnu.so`. If the invocation fails, `.so` is
used as fallback.

### 9.4 Include paths

For pybind11 bindings, forge adds two additional include paths:

- Python headers: obtained from `python3-config --includes` or
  `sysconfig.get_path('include')` as fallback.
- pybind11 headers: obtained from
  `python3 -c "import pybind11; print(pybind11.get_include())"`.
  A failure here is a hard error with a message suggesting
  `pip install pybind11`.

ctypes and cffi bindings receive no additional include paths.

### 9.5 Link dependencies

A lib's bindings are linked against exactly the same set of static archives
as the lib itself — the lib's full transitive closure in reverse topo order.
This is not configurable.

### 9.6 Output placement

After a successful link the `.so` file is copied into the lib's `tests/`
directory (if it exists) so that pytest can import it without any
`PYTHONPATH` manipulation.

---

## 10. Testing

### 10.1 Native tests

Each `.c`, `.cpp`, `.cc`, or `.cxx` file found in `tests/` is compiled
and linked as an independent binary. The binary is run immediately after
linking. Exit code 0 is a pass; any other exit code is a failure.

Native test binaries are linked against the lib's full transitive dep
closure, identical to the lib's own link args.

### 10.2 Python tests

If any `.py` file exists in `tests/`, forge runs a single pytest invocation
across the entire `tests/` directory:

```
cd tests/ && python3 -m pytest -v --tb=short .
```

pytest is invoked from inside `tests/` so that the copied `.so` file and
any relative imports resolve correctly.

A missing pytest installation is a hard error with a message suggesting
`pip install pytest`.

### 10.3 Pass/fail accounting

Each native test binary counts as one test suite. Each lib's pytest
invocation counts as one test suite. forge reports total suites passed and
failed at the end of `forge test`. A non-zero number of failures causes
forge to exit non-zero.

---

## 11. Commands

### 11.1 `forge build`

1. Load `project.toml`. Apply validation rules from §4.5.
2. If `[[lib]]` entries exist:
   a. Topologically sort libs (§6.1).
   b. Print the build order to stdout.
   c. For each lib in topo order:
      - If the lib has a source directory: compile all source files,
        produce `.build/lib/libNAME.a`.
      - If `ROOT/bindings/` exists: detect binding type, obtain Python
        headers, compile binding sources, link `.so`, copy to `ROOT/tests/`.
   d. For each `[[exe]]` in declaration order:
      - Compile all source files under `src/`.
      - Link against transitive closure of `deps`.
      - Produce `.build/bin/NAME`.
3. If no `[[lib]]` entries exist (single-lib mode):
   - Compile all source files under `src/`.
   - If a `main.*` file is present at any depth: link executable to
     `.build/bin/NAME`. Otherwise archive to `.build/lib/libNAME.a`.
   - If `bindings/` exists: build Python extension as above.

### 11.2 `forge test`

1. Run `forge build`. If it fails, exit immediately.
2. If `[[lib]]` entries exist:
   - For each lib in topo order, if `ROOT/tests/` exists:
     run native tests and pytest as described in §10.
3. If no `[[lib]]` entries exist:
   - Run native tests and pytest from `tests/` if it exists.
4. Report pass/fail summary. Exit non-zero if any suite failed.

### 11.3 `forge clean`

1. Remove `.build/` recursively.
2. Remove all `*.so` files found inside any `tests/` directory anywhere
   in the project tree (at any depth).

`forge clean` is idempotent. Running it on an already-clean project
produces no error.

### 11.4 `forge` (no arguments)

Print usage information and exit 0.

### 11.5 Unknown command

Print an error identifying the unknown command and exit non-zero.

---

## 12. Acceptance conditions

The implementation satisfies this specification when all of the following
hold.

### 12.1 project.toml parsing

- [ ] A missing `project.toml` produces a clear error and exits non-zero
- [ ] All top-level fields fall back to their documented defaults when absent
- [ ] A `[[lib]]` entry with no `name` field is a hard error
- [ ] A `[[exe]]` entry with no `name` field is a hard error
- [ ] A `[[exe]]` name that collides with a `[[lib]]` name is a hard error
- [ ] `deps = []` (empty array) parses without error
- [ ] `deps = ["a"]` (single entry) parses correctly
- [ ] `deps = ["a", "b", "c"]` (multiple entries) parses correctly
- [ ] A `deps` entry in a `[[lib]]` referencing an undeclared name is a hard error before any compilation
- [ ] A `deps` entry in a `[[exe]]` referencing an undeclared name is a hard error before any compilation
- [ ] A `[[exe]]` with no resolvable `src` directory is a hard error

### 12.2 Convention resolution

- [ ] A `[[lib]]` with no `src` and no `libs/NAME/src` directory is treated as a grouping lib
- [ ] A `[[lib]]` with no `src` but with `libs/NAME/src` present uses that directory automatically
- [ ] A `[[exe]]` with no `src` but with `exe/NAME/src` present uses that directory automatically
- [ ] Library root is derived correctly by stripping the last path component of `src`
- [ ] `ROOT/bindings/` and `ROOT/tests/` are inferred from the library root with no config
- [ ] `include/` at the project root is added to `-I` for every compilation if it exists
- [ ] Two source files that produce the same mangled object name are treated as a collision (known limitation — not required to error, but must not silently produce incorrect output)

### 12.3 Build order

- [ ] All `[[lib]]` entries are built before any `[[exe]]` entry
- [ ] A linear dep chain `A → B → C` builds in order `A, B, C`
- [ ] A diamond `A, B → C, A, B → D` builds `C` before `A` and `B`, and `A` and `B` before `D`
- [ ] A cycle in the `[[lib]]` graph is a hard error before any compilation
- [ ] The build order is printed to stdout before compilation begins

### 12.4 Compilation

- [ ] All `.c`, `.cpp`, `.cc`, `.cxx` files under `src/` are compiled including files in subdirectories
- [ ] Files in subdirectories of `src/` produce distinct object file names
- [ ] `bindings/` is walked non-recursively — only files directly in that directory are compiled
- [ ] `-MMD -MF` dependency files are emitted into `.build/deps/` for every compiled translation unit
- [ ] A compilation failure prints the failing source file path and exits immediately without further compilation
- [ ] The correct compiler family is detected and the correct warning flag set is applied
- [ ] `-Werror` is applied to lib, exe, and test sources when `werror = "true"`
- [ ] `-Werror` is never applied to binding sources regardless of the `werror` setting

### 12.5 Linking

- [ ] Each lib with source files is archived into `.build/lib/libNAME.a`
- [ ] Grouping libs produce no `.a` file
- [ ] Linker arguments for a lib include the lib itself and its full transitive dep closure
- [ ] Linker arguments are in reverse topological order
- [ ] A lib with no compiled output does not appear in linker arguments
- [ ] Executables are linked against the full transitive closure of their `deps`
- [ ] Executables are produced at `.build/bin/NAME`

### 12.6 Python bindings

- [ ] `PYBIND11_MODULE` in any binding source triggers pybind11 mode
- [ ] `ffi.cdef` or `cffi` in any binding source triggers cffi mode
- [ ] No marker triggers ctypes mode
- [ ] The pybind11 module name is extracted from `PYBIND11_MODULE(name, ...)` and used as the output base name
- [ ] The extension suffix is obtained from `sysconfig.get_config_var('EXT_SUFFIX')`
- [ ] pybind11 bindings are compiled with `-fPIC -fexceptions -frtti`
- [ ] ctypes and cffi bindings are compiled with `-fPIC` only
- [ ] A missing pybind11 installation is a hard error with a helpful message
- [ ] The built `.so` is copied into `ROOT/tests/` after a successful link
- [ ] Binding link args are identical to the owning lib's link args

### 12.7 Testing

- [ ] Each `.c`/`.cpp`/`.cc`/`.cxx` file in `tests/` produces an independent test binary
- [ ] Each native test binary is linked against the lib's full transitive dep closure
- [ ] Exit code 0 from a native binary is reported as a pass
- [ ] Non-zero exit code from a native binary is reported as a failure with the exit code
- [ ] All `.py` files in a `tests/` directory are run under a single pytest invocation
- [ ] pytest is invoked from inside `tests/` with `python3 -m pytest -v --tb=short .`
- [ ] A missing pytest installation is a hard error with a helpful message
- [ ] `forge test` exits non-zero if any suite fails
- [ ] `forge test` runs all suites even if an earlier suite fails (unless build failed)

### 12.8 Clean

- [ ] `forge clean` removes `.build/`
- [ ] `forge clean` removes all `*.so` files inside any `tests/` directory in the tree
- [ ] `forge clean` is idempotent — running it twice produces no error

### 12.9 General

- [ ] `forge` with no arguments prints usage and exits 0
- [ ] An unknown command prints a diagnostic and exits non-zero
- [ ] The single-lib fallback (no `[[lib]]` entries, `src/` present, no `main.*`) builds correctly
- [ ] The executable fallback (no `[[lib]]` entries, `main.*` in `src/`) produces `.build/bin/NAME`
- [ ] forge itself compiles with `cc -std=c11 -O2 -o forge forge.c` with no warnings

---

## 13. Current limitations

- **No incremental builds.** `.d` files are emitted but not consumed. Every
  `forge build` recompiles all sources.
- **No package management.** Third-party dependencies must be managed
  outside forge.
- **Flat `bindings/`.** Only files directly in `bindings/` are compiled.
  Subdirectories are ignored.
- **Single binding module per lib.** A lib cannot produce more than one
  Python extension module.
- **Object name collisions.** Two source files that produce the same
  mangled name will silently overwrite each other's object file.
- **POSIX only.** forge uses POSIX APIs throughout. Windows is not
  supported.
- **No install target.** forge does not support installing headers or
  libraries to system paths.
