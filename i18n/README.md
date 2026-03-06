# i18n

This directory contains Qt Linguist translation source files (`.ts`).

Current files:
- `ro-control_en.ts`
- `ro-control_tr.ts`

To update translations:
1. Ensure `Qt6::LinguistTools` is installed.
2. Run CMake configure/build.
3. Use Qt Linguist tools (`lupdate`, `linguist`, `lrelease`) in your workflow.

Note: Translation coverage is currently partial and will be expanded incrementally.
