# pitroy3
Source a870598; CI36552404675 PASS. Rootless delivered Telegram, not device-tested.
SHA256 2d5f394ed611829e3e62013c2347475e31c450e42da1ec84039cfcb2ad65aa39
Action picker App Quick Shortcut -> app -> observed action. Saves base64 JSON
bundle/type/title as quickactions run command. Same existing execution pipeline.
SBIconView applicationShortcutItems observation confirmed by owner logs.
SBIconView +activateShortcut:withBundleIdentifier:forIconView: located in
https://github.com/opa334/Choicy/blob/master/ChoicySB/SpringBoard.x .
Activation capability/ABI checked; device operation still pending. No fallback
launch. Requires unlocked phone (fail closed if lock selector unavailable).
Captures current native objects, weak icon view; checks icon identity and refreshes
native items before execution. No archive of userInfo. Known system/third-party
menu additions filtered, not a universal provenance filter. Static-only items
not yet offered unless observed. Catalog memory-only; reopen icon menu after
respring; weak view expiration also requires reopening. UI explains this.
Probe and main fork only, NOT part of upstream blacklist PR39.
