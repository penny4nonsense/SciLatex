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

- Qt 6.8+ (Only dependency not vendorized — must be installed separately)
- CMake 3.16+
- C++17-compatible compiler

> Note: All other required libraries are vendorized. You don’t need to install them separately.

### Build Instructions (Windows / Linux)

```bash
git clone https://github.com/YOURUSERNAME/SciLatex.git
cd SciLatex
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_PREFIX_PATH="path/to/Qt"
ninja
```

If you're on Windows, you can also build the project using **Qt Creator**:
1. Open `SciLatex.pro` or `CMakeLists.txt` in Qt Creator  
2. Configure the project using your installed Qt kit  
3. Build and run

### Mac Support

SciLatex currently does **not** support native Mac builds out of the box. However, you may be able to cross-compile with the appropriate toolchains. This is a work in progress.

## 🗂️ Folder Structure

```
SciLatex/
├── src/                # Main source code
├── vendor/             # Vendorized dependencies (excluding Qt)
├── resources/          # Icons, themes, UI files
├── build/              # Temporary build folder
└── CMakeLists.txt      # CMake build script
```

## 🛣️ Roadmap / Next Steps

- [ ] **Create Windows distributable** using `windeployqt` and installer packaging  
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
