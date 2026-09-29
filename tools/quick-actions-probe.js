// Read-only SpringBoard probe: frida -U -n SpringBoard -l tools/quick-actions-probe.js
// Attach only after the owner explicitly enables Frida. No action execution.
'use strict';
if (!ObjC.available) throw new Error('Objective-C runtime unavailable');
let remaining = 600;
function emit(event, data) {
  if (remaining-- > 0) console.log(JSON.stringify({time: new Date().toISOString(), event, ...data}));
}
function describe(pointer) {
  try {
    if (pointer.isNull()) return null;
    const o = new ObjC.Object(pointer);
    const result = {class: o.$className};
    // Never serialize userInfo, descriptions, URLs, or arbitrary object graphs.
    for (const key of ['type', 'localizedTitle', 'bundleIdentifier', 'applicationBundleIdentifier']) {
      if (typeof o[key] === 'function') {
        const value = o[key]();
        if (value && value.isKindOfClass_(ObjC.classes.NSString)) result[key] = value.toString().slice(0, 180);
      }
    }
    if (o.isKindOfClass_(ObjC.classes.NSArray)) {
      result.count = Number(o.count());
      result.items = [];
      for (let i = 0; i < Math.min(result.count, 30); i++) {
        const item = o.objectAtIndex_(i);
        if (item.$className.toLowerCase().includes('shortcut')) result.items.push(describe(item.handle));
      }
    }
    return result;
  } catch (_) { return {unreadable: true}; }
}
const hooked = new Set();
for (const name of ['SBIconView', 'SBIconController', 'SBApplication', 'SBApplicationController', 'SBApplicationShortcutStoreManager']) {
  const cls = ObjC.classes[name];
  if (!cls) { emit('class_missing', {name}); continue; }
  for (const selector of cls.$ownMethods.filter(x => /shortcut/i.test(x))) {
    const method = cls[selector];
    emit('available_method', {name, selector, returnType: method.returnType, argumentTypes: method.argumentTypes});
    const address = method.implementation.toString();
    if (hooked.has(address)) continue;
    hooked.add(address);
    Interceptor.attach(method.implementation, {
      onEnter(args) {
        if (remaining <= 0) return;
        const objects = [];
        method.argumentTypes.forEach((t, i) => {
          if (i >= 2 && t === 'object') objects.push({index: i, value: describe(args[i])});
        });
        emit('call', {name, selector, receiver: describe(args[0]), objects});
      },
      onLeave(retval) {
        if (remaining > 0 && method.returnType === 'object') emit('return', {name, selector, value: describe(retval)});
      }
    });
  }
}
emit('ready', {instruction: 'Long-press an app icon, then select its Quick Action. Stop after reproducing.'});
