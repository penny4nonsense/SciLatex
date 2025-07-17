# SciLatex

**SciLatex** is a modern LaTeX editor with symbolic math support, designed as a lightweight, cross-platform alternative to *Scientific WorkPlace*. It integrates structured LaTeX editing with dynamic math input, placeholder navigation, and a clean Qt-based GUI — perfect for technical writing in math, physics, and engineering.

## ✨ Features

- **Live LaTeX editor** with structured math insertion  
- **Symbolic math support**, including placeholders like `$1`, `$2` for easy keyboard navigation  
- **Matrix and table insertion tools**  
- **Integrated dialogs** for fonts, spacing, brackets, operators, and math naming  
- **Recent file tracking** with infinite history  
- **PDF preview** of compiled LaTeX documents  
- **Matlab integration** (experimental)  
- **Full keyboard-based workflow** (with keychain sequences like Ctrl+f for fractions, Ctrl+r for radicals, etc.)

## 🚀 Getting Started

### Prerequisites

Windows x64 version: None

Building on Mac/Linux:
- Qt 6.8+ (Only dependency not vendorized — must be installed separately)
- CMake 3.16+
- C++17-compatible compiler

> Note: All other required libraries are vendorized. You don’t need to install them separately.

### Build Instructions (Windows)

No need to build — just download and run:

📦 [Download SciLatex for Windows](https://github.com/penny4nonsense/SciLatex/raw/refs/heads/main/SciLatex-Windows-x64.zip)
- Extract the ZIP
- Run SciLatex.exe inside the folder
- No installation required

If you want to build from source, use Qt Creator:

- Open CMakeLists.txt in Qt Creator
- Choose your Qt 6 kit (MSVC 64-bit)
- Configure and build
- Run the application directly

### Build Instructions (Mac / Linux)

```bash
git clone https://github.com/penny4nonsense/SciLatex.git
cd SciLatex
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_PREFIX_PATH="path/to/Qt"
ninja
```

If you're on Windows, you can also build the project using **Qt Creator**:
1. Open `SciLatex.pro` or `CMakeLists.txt` in Qt Creator  
2. Configure the project using your installed Qt kit  
3. Build and run

## 🗂️ Folder Structure

```
SciLatex/
├── build/                      # Temporary build artifacts (ignored)
├── portable_windows_x64/      # Final distributable (ignored)
├── source/                    # All source/header files
│   ├── main.cpp
│   ├── mainwindow.cpp
│   ├── ...
│   └── CMakeLists.txt         # Source-specific CMake
├── third_party/               # Vendored dependencies (e.g., curl)
├── .gitignore
├── CMakeLists.txt             # Top-level build config
├── LICENSE                    # Your software license
├── README.md                  # Project description & build info
├── SciLatex-Windows-x64.zip   # Windows portable zip folder
├── todo.txt                   # Development notes

```

## 🛣️ Roadmap / Next Steps

- [X] **Create Windows distributable** using `windeployqt` and installer packaging  
- [ ] **Build Linux distributable** with all dependencies bundled for easy install  
- [ ] **Add spelling and grammar checker** (suggested: integrate `hunspell` or `LanguageTool`)  
- [ ] **Implement tab-based placeholder navigation** for smooth editing and cursor movement  
- [ ] [Planned] macOS build and support  

## 🐛 Known Issues

- Cross-platform PDF rendering may vary  
- macOS builds are not yet tested  
- Some placeholder behavior is still being refined

## 🙏 Acknowledgments

SciLatex is inspired by the discontinued *Scientific WorkPlace* and aims to provide a modern, open-source alternative for technical writers and researchers.
