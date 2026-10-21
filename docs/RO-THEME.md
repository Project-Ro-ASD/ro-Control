# Ro-Theme integration

Ro-Control consumes the accepted Ro-Theme decisions documented in
[Ro-Theme-Docs](https://github.com/Project-Ro-ASD/Ro-Theme-Docs): TOK-01/02/04/08/12/13/16,
COL-04/24/28/33, APP-14 and FNT-01/09. This integration covers application content;
Plasma, window decorations, login, wallpaper and distribution defaults belong to their own packages.

## Shared source

`data/theme/upstream.json` pins the upstream commit and checksums. The two color JSON snapshots
and generated `src/qml/style/RoTokens.qml` come unchanged from that commit of
[Ro-Theme](https://github.com/Project-Ro-ASD/Ro-Theme/tree/6d15eadf96b19afdab6b58ae6e14e372050c08ca).
RoTokens retains its upstream license header. There is no runtime dependency on ro-theme.

`python3 tools/sync-theme.py` generates `Palettes.js` from the snapshots;
`python3 tools/sync-theme.py --check` verifies the generated copy in CI.
To update upstream, replace all three snapshots from one reviewed upstream commit, record the
new commit/checksums, regenerate and run the UI tests. Do not edit generated QML or JS values.

## Application adaptation

`style/Theme.qml` is the single adapter for surfaces, text, semantic states and selection text.
Explicit light/dark modes use the Ro palettes. System mode reads Qt's native palette, including
the user's highlight and highlighted-text colors, when its light/dark appearance matches.
Named fallback semantic roles remain available outside KDE; Qt's palette does not expose KDE's
positive/neutral/negative roles. Semantic messages retain their explanatory text.

Application surfaces are opaque, without gradients or decorative card stripes. Cards, controls,
spacing and short transitions use the shared tokens. The application inherits the platform font
family; larger platform fonts increase the application scale. It does not install or force fonts.
Primary controls use `onAccent` (dark ink on the dark theme's light blue), rather than fixed white.
Secondary semantic actions use colored text/borders on neutral surfaces.

KDE's `AnimationDurationFactor` is read from the XDG `kdeglobals` configuration at startup;
zero disables custom transitions. Restart after changing this desktop setting. Test durations
and hardware sampling timers are independent of visual motion.

## Verification

Build with `-DBUILD_TESTS=ON` and run CTest. `test_theme` renders actual Qt Quick controls and checks
WCAG contrast for both palettes, keyboard focus/Space activation, disabled actions, empty/loading/error
states and two banner widths. Existing driver-page integration tests exercise the backend contract.
Use `QT_SCALE_FACTOR=1.5 ctest --test-dir build -R test_theme --output-on-failure` for scaled controls.
Manual previews should also cover all four pages and popup dismissal at the supported minimum window
size. Offscreen previews do not qualify real Fedora/KWin hardware behavior.
