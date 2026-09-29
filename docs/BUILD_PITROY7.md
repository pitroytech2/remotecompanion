# pitroy7 / failed persistence investigation
2026-09-29 device142 unlocked. dpkg confirmed3.6.3+pitroy6.
MCP read_file and read-only ls both confirmed quickshortcuts.plist absent.
rc quickactions list xyz.be.customer returned empty items. Owner says post-respring
activation fails. Thus persistent action config != saved native payload.
No evidence yet that nil icon view is actual failing point. Prior secure archive
path swallowed errors and skipped entire save; replaced with explicit plist
properties type/localizedTitle/localizedSubtitle/bundleIdentifierToLaunch/
targetContentIdentifier/userInfo. Native object re-created via guarded setters.
No arbitrary class unarchive. Values validated; payload bounded64KiB per item.
Save failure emits generic domain/code in existing log, no payload data.
Source2af0e74 CI36557376054 PASS. Sent Telegram, not installed by agent.
SHA25660de0287343a6d0ad5bcf3ee794815504761f6b9cac89ffa745cd6f43f4a050f
NEXT: owner installs and opens be menu once; inspect file and verify restore
before asking another respring. Nil-icon dispatch still needs device acceptance.
Quick Shortcut PR on hold. No diagnostic UI reintroduced.
