# Status-bar exclusions and app Quick Actions

## Audit (2026-09-29)
Upstream: saihgupr/remotecompanion; fork: pitroytech2/remotecompanion.
MIT license retained. This document is research, not a completed implementation.

Existing blacklist exists in Tweak/Tweak.x:1352-1420, via rc blacklist
list/add/remove/reset (~5995). RCExecuteTrigger (~2019) blocks ALL triggers.
No corresponding blacklist UI found in RemoteCompanion sources.
Foreground result is cached for 0.5s; blacklist reload takes up to10s.
Status-bar touch tracking (~10620) starts before the execution gate; holds,
swipes and haptic feedback may already be processed. Region is a50-point edge
rotated with orientation, not necessarily an actual visible status-bar view.
App picker uses LSApplicationWorkspace allInstalledApplications, explaining
AAUIViewService and other internal apps visible in the screenshot.

## First implementation slice
1. Add dedicated statusBarExcludedApps (bundle IDs) in existing config, keeping
   global blacklist semantics unchanged. Searchable multi-select app/icon UI.
2. Add optional disableStatusBarInLandscape, default OFF.
3. Reject only status-bar recognition at touch begin; revalidate foreground and
   policy before hold timer/swipe/double-tap execution. Cancel timer/tracking
   when ownership changes. Do not suppress native touch handling or unrelated
   hardware/bottom-bar triggers. A hold begun in A must not fire after entering B.
4. Cache parsed exclusion set on configuration notification; no disk reads or
   scanning app lists per touch. Avoid stale 0.5s foreground-result cache for
   admission. Verify current main-thread foreground lookup before selecting
   an event-driven invalidation hook; no invented private selectors.
5. Filter app picker to launchable user-visible apps, retaining real system
   apps; do not simply hide every com.apple bundle.

## Home Screen Quick Actions
Apple documentation:
https://developer.apple.com/documentation/uikit/uiapplicationshortcutitem
https://developer.apple.com/documentation/uikit/uiapplication/shortcutitems
Static items: UIApplicationShortcutItems in app Info.plist.
Dynamic items: UIApplication.shortcutItems. Apps receive selected item via
scene/application delegate callbacks and cold-launch connection options.
These are NOT Shortcuts.app workflows, nor automatically URL schemes.
Public APIs describe the calling app's items, not arbitrary cross-app querying
and execution. A SpringBoard-side private integration is needed for this tweak.

Proposed UI: Open App -> choose app -> Open Normally / available Quick Actions.
Persist bundle ID + item type, not array index/localized title. On invocation,
resolve the current native item so dynamic userInfo and app updates are honored.
Do not invent a deep link or call another process's UIApplication delegate.
Discover/probe SpringBoard's existing Quick Action collection and activation
path on the target iOS before implementing dispatch. Exclude system/tweak menu
entries (Delete/Share/Add to folder/Immortal) from app-defined actions.
If action disappears, show unavailable; never silently launch a different one.
Sensitive userInfo must not be copied into normal logs.

## Required evidence and tests
Use be Giao hang as first slice: record current native item type, source,
selector availability and activation result, compare with normal long-press.
Cold/warm app; dynamic items after app launch; missing/deleted action; locked
screen; denied action; app removed. Respect normal unlock/confirmation flow.
Blacklist tests: portrait/landscape; rapid A->B; switch during long press;
config changed without respring; gestures elsewhere unaffected.
Do not install builds or alter device prefs during research.
