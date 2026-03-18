# i18n

This directory contains the Qt Linguist translation sources for ro-Control.

## Current locales

- `ro-control_en.ts` - English source catalog kept in sync with extracted strings
- `ro-control_tr.ts` - Turkish translation catalog

English is the source language used directly in code. Additional locales should be
added as new `ro-control_<locale>.ts` files.

## Runtime model

- QML strings use `qsTr(...)`
- C++ strings use `tr(...)`
- `main.cpp` loads the best matching `.qm` file from the embedded `/i18n` resource
- If no locale-specific translation is found, the app falls back to English

## Updating translations

1. Ensure Qt Linguist tools are available on the system.
2. Reconfigure the build directory with CMake.
3. Refresh translation sources with `lupdate`.
4. Translate in Qt Linguist.
5. Build again so CMake generates updated `.qm` files.

Example workflow:

```bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
lupdate src -ts i18n/ro-control_en.ts i18n/ro-control_tr.ts
linguist i18n/ro-control_tr.ts
cmake --build build
```

## Adding a new language

1. Copy `i18n/ro-control_en.ts` to `i18n/ro-control_<locale>.ts`
2. Set the `language` attribute in the new TS file
3. Add the file to `TS_FILES` in `CMakeLists.txt`
4. Translate all entries and verify the UI on a narrow window size
5. Update AppStream metadata and screenshots if the change affects store-facing text

## Translation quality rules

- Keep product names, package names, and command names untranslated
- Prefer concise labels because the UI targets 980x640 and larger windows
- Do not translate technical strings blindly when they match package names
- Check both light and dark themes before submitting
