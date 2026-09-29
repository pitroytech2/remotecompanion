# App Quick Shortcut capture

Prototype, not device-tested. No activation or installation is performed.
Run only with owner-authorized Frida on the test phone:

```
frida -U -n SpringBoard -l tools/quick-actions-probe.js -o quick-actions.jsonl
```
Use a Frida CLI exposing the Objective-C bridge (frida-tools). The script checks
classes and discovers shortcut selectors actually present; no assumed activation
selector is invoked. Long-press be, then choose Giao hang. Stop capture afterward.
Events: available_method, call, return, class_missing. Logs stop after600 events.
Return events include up to30 shortcut items; userInfo/URLs are not dumped.
No events does NOT mean no shortcuts: class coverage may need adjustment based
on available_method evidence. Attach failure does not diagnose the app.

Apple's UIApplicationShortcutItem and UIApplication.shortcutItems document
static and dynamic Home Screen Quick Actions. Static declarations can be read
from installed app Info.plist UIApplicationShortcutItems. A device-wide dynamic
inventory requires SpringBoard's registered shortcut store; it cannot discover
conditional actions that an app has never registered. Probe observed collection
getters before implementing an inventory command. Do not confuse Home Screen
system/tweak menu entries with application-provided actions.

https://developer.apple.com/documentation/uikit/uiapplicationshortcutitem
https://developer.apple.com/documentation/uikit/uiapplication/shortcutitems

Blacklist prototype: Settings > Status Bar: Excluded Apps; tap to toggle check.
Stored in shared rc_triggers.plist statusBarExcludedApps, normal save/notification
path. Only status-bar triggers gated, global CLI blacklist unchanged. Gate at
begin, move/end, hold timer and final dispatch. No disk read in added gates.
Requires full app + tweak build; existing installed app cannot show new UI.
Not compiled/device-validated yet. A->B->A entirely between observed events can
still escape identity comparison: foreground generation tracking is follow-up.
