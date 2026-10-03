# Magnet Link Generator

A lightweight desktop application for creating standard BitTorrent magnet links.
Built with **C++17**, **Qt 6**, and **CMake**.

## Features

- Generate magnet links from a BitTorrent info hash.
- Support for:
  - `BTIH` / SHA-1 hashes for BitTorrent v1.
  - `BTMH` / SHA-256 hashes for BitTorrent v2.
- Optional magnet-link fields:
  - Display name (`dn`)
  - File size in bytes (`xl`)
  - Keywords (`kt`)
  - Web seed URL (`ws`)
  - Multiple trackers (`tr`)
- Load a built-in list of public trackers.
- Copy generated links to the clipboard.
- Open generated links directly with qBittorrent.
- Light and dark color-scheme support.
- Hash validation with an option to continue when the format is unusual.

## Requirements

- CMake 3.16 or newer
- A C++17-compatible compiler
- Qt 6.5 or newer with the `Widgets` component
- qBittorrent is optional and only required for the **Test with qBittorrent** feature

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build --config Release
```

The executable will be placed in the appropriate build output directory for your generator and platform.

### Windows with Visual Studio

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

After building with Qt, use `windeployqt` if you want to package the required Qt runtime libraries alongside the executable.

## Usage

1. Start **Magnet Link Generator**.
2. Select the hash type (`BTIH` or `BTMH`).
3. Enter the required info hash.
4. Optionally enter a display name, file size, keywords, web seed, or trackers.
5. Click **Generate Magnet Link**.
6. Copy the generated link or open it with qBittorrent.

A typical BitTorrent v1 magnet link looks like this:

```text
magnet:?xt=urn:btih:<INFO_HASH>&dn=<NAME>&tr=<TRACKER_URL>
```

## Project Structure

- `src/main.cpp` - Application entry point.
- `src/mainwindow.h` - Main window declaration.
- `src/mainwindow.cpp` - User interface and magnet-link generation logic.
- `CMakeLists.txt` - CMake build configuration.

## License

This project is licensed under the MIT License. See [License.txt](License.txt) for details.
