# 3.6.3+pitroy2 Rootless
Source88e418b; CI36550513793 passed. Unified app/tweak verified under var/jb.
SHA256 e74c79535cdb8efa55421af7bd02350b2409c23834b7c2822363e899f2eacb6d
Sent Telegram; not installed or device-tested. No public repo publication.
Settings > App Quick Shortcut Diagnostics: Scan, Report, Copy, Stop.
CLI quickactions scan|report|stop uses same diagnostic state. On-demand only.
Scan caps512 apps; rolling report128 records. Memory-only, lost at respring.
Captures SBIconView applicationShortcutItems only if signature matches; missing
selector reports installed=false. Static Info.plist and capability-checked proxy
getters. No userInfo/URLs. No activation hook or execution yet. Dynamic inventory
completeness NOT established. Main-thread scan IO may need batching after timing
on device; do not automatically scan at startup. Existing config unchanged.
Blacklist row now at bottom of Screen Gestures. Draft upstream PR39 separate.
