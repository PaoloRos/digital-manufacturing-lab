# CNCpp

CNCpp is a CNC kernel written in C++20. It reads a G-code program and can run it
using either a local machine configuration or a [MADS](https://github.com/pbosetti/mads)
network with a broker, an FMU simulator, and a machine viewer.

This project is based on the course reference repository:
[pbosetti/cncpp](https://github.com/pbosetti/cncpp).

## Requirements

The following tools are required to configure and build the project:

- a Clang compiler with C++20 support;
- CMake 3.20 or newer;
- Ninja (recommended; Make can be used as an alternative);
- an installation of MADS, including its headers, libraries, plugins, and CMake
  configuration files;
- Internet access during the first configuration, because CMake downloads
  `fmt`, `nlohmann/json`, `rang`, `toml++`, and `keystroker`.

Check that the required tools are available:

```bash
clang++ --version
cmake --version
ninja --version
mads --version
mads --info
```

MADS must be visible to both CMake and the shell. If it is installed under a
non-standard prefix, add its `bin` directory to `PATH` and make the prefix
available to CMake. For example, for MADS installed in `~/usr/local`:

```bash
export PATH="$HOME/usr/local/bin:$PATH"
export CMAKE_PREFIX_PATH="$HOME/usr/local${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
```

## Configuration and build

Run these commands from the repository root:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j5
```

The resulting executable is `build/cnc`. The configuration enforces C++20 and
also generates `build/compile_commands.json` for editor tools.

If CMake cannot find MADS, explicitly provide the directory containing
`MadsConfig.cmake`:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DMads_DIR="$HOME/usr/local/lib/cmake/Mads"
cmake --build build -j5
```

If Ninja is not available, omit `-G Ninja`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j5
```

To configure a build with different options, prefer using a new directory such
as `build-debug`, or deliberately remove the old `build` directory before
configuring it again.

## Tests

After building, run the tests with:

```bash
ctest --test-dir build --output-on-failure
```

## MADS configuration

The two main configuration files are:

- `mads.ini`: broker addresses, topics, and agent parameters;
- `director.toml`: processes that make up the network and their startup order.

The included configuration uses the following local ports:

| Service | Address |
| --- | --- |
| Publication/frontend | `tcp://localhost:9090` |
| Subscription/backend | `tcp://localhost:9091` |
| Settings | `tcp://localhost:9092` |

The CNC publishes setpoints to the `machine_tool_control` topic and receives the
simulated state from the `fmu_machine_tool` topic.

The provided `director.toml` enables:

- `broker`, which starts the MADS broker;
- `feedback`, which displays exchanged messages;
- `fmu`, which starts the machine dynamics model.

The `fmu` process currently uses the external `../MADS-plugins/FMU_agent`
project and the `models/fmu/machine_tool.fmu` model. Before starting the
network, verify that this path exists relative to the repository directory. If
it does not, update `workdir` in `director.toml` or temporarily disable the
process with `enabled = false`.

The `machinetool` viewer is included in the configuration but is disabled. To
use it, build its plugin in the project referenced by `workdir`, then set:

```toml
[machinetool]
enabled = true
```

## Starting the MADS network

Open a terminal in the repository root and run:

```bash
mads-director director.toml
```

The director reads its commands from `director.toml`. MADS agents started in the
project directory read their corresponding sections from `mads.ini`. In the
director interface, check that at least `broker`, `feedback`, and `fmu` start
without errors.

To stop the network cleanly, use the director interface controls or interrupt
the director with `Ctrl+C`.

## Running the CNC

Once the MADS network is running, open a second terminal in the repository root
and run:

```bash
./build/cnc test.g
```

If no second argument is provided, `cnc` connects to the default MADS settings
service:

```text
tcp://localhost:9092
```

You can explicitly provide a different endpoint:

```bash
./build/cnc test.g tcp://localhost:9092
```

During interactive execution of the example program:

- `Z` moves the machine to its zero position;
- `Space` starts rapid movement and machining.

To test the parser and controller without starting MADS, provide the local
machine configuration file instead:

```bash
./build/cnc test.g machine.toml
```

## Complete quick-start sequence

Terminal 1:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j5
ctest --test-dir build --output-on-failure
mads-director director.toml
```

Terminal 2, after the network has started:

```bash
./build/cnc test.g
```

## Troubleshooting

### CMake cannot find MADS

Check the installation and configure the project again with the correct path:

```bash
mads --info
find "$HOME/usr/local" -name MadsConfig.cmake -print
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DMads_DIR="$HOME/usr/local/lib/cmake/Mads"
```

### `std::span` error in `agent.hpp`

MADS requires C++20. Check that the generated configuration actually uses it:

```bash
rg -- '-std=c\+\+20' build/compile_commands.json
```

If it is not present, configure the build directory again using the commands
shown above.

### The CNC cannot connect to the broker

Verify that the director is still running, that the `broker` process has not
failed, and that `mads.ini` and the endpoint passed to `cnc` use the same ports.

### The FMU process does not start

Check the `workdir` in the `[fmu]` section of `director.toml`, verify that the
`.fmu` file exists, and check plugin availability with:

```bash
mads --plugins
```
