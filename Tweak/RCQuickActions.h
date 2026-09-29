// On-demand read-only Quick Action diagnostics. Never serialize userInfo.
#import <substrate.h>
static NSMutableArray *RCQAEvents;
static BOOL RCQACapturing;
static id RCQAGet(id object, NSString *name) {
    SEL selector = NSSelectorFromString(name);
    if (!object || ![object respondsToSelector:selector]) return nil;
    NSMethodSignature *sig = [object methodSignatureForSelector:selector];
    if (sig.numberOfArguments != 2 || sig.methodReturnType[0] != '@') return nil;
    @try { return ((id (*)(id, SEL))objc_msgSend)(object, selector); }
    @catch (__unused NSException *e) { return nil; }
}
static NSString *RCQAString(id value) {
    return [value isKindOfClass:NSString.class] ? [value substringToIndex:MIN([value length], 200)] : @"";
}
static NSArray *RCQAItems(id items) {
    if (![items isKindOfClass:NSArray.class]) return @[];
    NSMutableArray *rows = [NSMutableArray array];
    for (id item in items) {
        if (rows.count >= 32) break;
        NSString *type = RCQAString([item isKindOfClass:NSDictionary.class] ? item[@"UIApplicationShortcutItemType"] : RCQAGet(item, @"type"));
        NSString *title = RCQAString([item isKindOfClass:NSDictionary.class] ? item[@"UIApplicationShortcutItemTitle"] : RCQAGet(item, @"localizedTitle"));
        if (type.length) [rows addObject:@{@"type":type, @"title":title}];
    }
    return rows;
}
static NSString *RCQAJSON(id data) {
    NSData *bytes = [NSJSONSerialization dataWithJSONObject:data options:NSJSONWritingPrettyPrinted error:nil];
    return bytes ? [[NSString alloc] initWithData:bytes encoding:NSUTF8StringEncoding] : @"{}";
}
static void RCQARecord(id data) {
    if (!RCQAEvents) RCQAEvents = [NSMutableArray array];
    if (RCQAEvents.count >= 128) [RCQAEvents removeObjectAtIndex:0];
    [RCQAEvents addObject:data];
}
static NSMutableDictionary *RCQACatalog;
static NSMapTable *RCQAViews;
static BOOL RCQAExcludedType(NSString *type) {
    return [type hasPrefix:@"com.apple.springboardhome.application-shortcut-item."] ||
        [type isEqualToString:@"CustomAddToFolderItem"] ||
        [type hasPrefix:@"com.opa334.choicy."] || [type hasPrefix:@"com.sergy.immortalizer."];
}
static void RCQARemember(id view, id raw) {
    NSString *bundle = RCQAString(RCQAGet(RCQAGet(view,@"icon"),@"applicationBundleID"));
    if (!bundle.length || ![raw isKindOfClass:NSArray.class]) return;
    if (!RCQACatalog) RCQACatalog = [NSMutableDictionary dictionary];
    if (!RCQAViews) RCQAViews = [NSMapTable strongToWeakObjectsMapTable];
    NSMutableArray *items = [NSMutableArray array];
    for (id item in raw) {
        NSString *type = RCQAString(RCQAGet(item,@"type"));
        if (type.length && !RCQAExcludedType(type) && items.count < 32) [items addObject:item];
    }
    if (RCQACatalog.count >= 512 && !RCQACatalog[bundle]) return;
    RCQACatalog[bundle] = items;
    [RCQAViews setObject:view forKey:bundle];
}
static NSString *RCQARun(NSString *encoded) {
    NSData *data = [[NSData alloc] initWithBase64EncodedString:encoded options:0];
    id request = data ? [NSJSONSerialization JSONObjectWithData:data options:0 error:nil] : nil;
    if (![request isKindOfClass:NSDictionary.class]) return @"Invalid Quick Shortcut request";
    NSString *bundle = RCQAString(request[@"bundle"]), *type = RCQAString(request[@"type"]);
    id view = [RCQAViews objectForKey:bundle];
    if (!view || ![RCQAString(RCQAGet(RCQAGet(view,@"icon"),@"applicationBundleID")) isEqual:bundle]) return @"Open this app's Home Screen menu once to refresh Quick Shortcuts.";
    id lock = RCQAGet(NSClassFromString(@"SBLockScreenManager"),@"sharedInstance");
    SEL locked = NSSelectorFromString(@"isUILocked");
    NSMethodSignature *ls = [lock methodSignatureForSelector:locked];
    if (!ls || ls.numberOfArguments != 2 || (ls.methodReturnType[0] != 'B' && ls.methodReturnType[0] != 'c')) return @"Lock status unavailable";
    if (((BOOL (*)(id,SEL))objc_msgSend)(lock,locked)) return @"Unlock the phone first";
    RCQARemember(view,RCQAGet(view,@"applicationShortcutItems"));
    id selected = nil;
    for (id item in RCQACatalog[bundle]) if ([RCQAString(RCQAGet(item,@"type")) isEqual:type]) { selected=item; break; }
    if (!selected) return @"Quick Shortcut no longer available";
    Class cls = NSClassFromString(@"SBIconView");
    SEL action = NSSelectorFromString(@"activateShortcut:withBundleIdentifier:forIconView:");
    NSMethodSignature *sig = [cls methodSignatureForSelector:action];
    if (!sig || sig.numberOfArguments != 5 || sig.methodReturnType[0] != 'v') return @"Native Quick Shortcut activation unavailable";
    for (NSUInteger i=2;i<5;i++) if ([sig getArgumentTypeAtIndex:i][0]!='@') return @"Unsupported activation signature";
    @try {
        ((void (*)(id,SEL,id,id,id))objc_msgSend)(cls,action,selected,bundle,view);
        RCQARecord(@{@"event":@"activation_dispatched",@"bundle":bundle,@"type":type});
        return @"Quick Shortcut dispatched";
    } @catch (__unused NSException *e) { return @"Quick Shortcut activation failed"; }
}
static id (*RCQAOriginalItems)(id, SEL);
static id RCQAObservedItems(id object, SEL selector) {
    id result = RCQAOriginalItems(object, selector);
    RCQARemember(object, result);
    if (RCQACapturing) {
        NSArray *items = RCQAItems(result);
        if (items.count) {
            id icon = RCQAGet(object, @"icon");
            RCQARecord(@{@"event":@"menu_items", @"bundle":RCQAString(RCQAGet(icon,@"applicationBundleID")), @"items":items});
        }
    }
    return result;
}
static void RCQAInstallProbe(void) {
    static BOOL installed;
    if (installed) return;
    Class cls = NSClassFromString(@"SBIconView");
    SEL selector = NSSelectorFromString(@"applicationShortcutItems");
    Method method = class_getInstanceMethod(cls, selector);
    char *type = method ? method_copyReturnType(method) : NULL;
    BOOL valid = method && method_getNumberOfArguments(method)==2 && type && type[0]=='@';
    if (type) free(type);
    if (valid) {
        MSHookMessageEx(cls, selector, (IMP)RCQAObservedItems, (IMP *)&RCQAOriginalItems);
        installed = YES;
    }
    RCQARecord(@{@"event":@"probe", @"selector":@"SBIconView applicationShortcutItems", @"installed":@(installed)});
}
static NSString *RCQACommand(NSString *command) {
    if ([command hasPrefix:@"quickactions list "]) {
        NSString *bundle = [command substringFromIndex:18];
        return RCQAJSON(@{@"items":RCQAItems(RCQACatalog[bundle])});
    }
    if ([command hasPrefix:@"quickactions run "]) return RCQARun([command substringFromIndex:17]);

    if ([command isEqualToString:@"quickactions stop"]) RCQACapturing = NO;
    else if ([command isEqualToString:@"quickactions scan"]) {
        RCQAEvents = [NSMutableArray array];
        RCQACapturing = YES;
        RCQAInstallProbe();
        id workspace = RCQAGet(NSClassFromString(@"LSApplicationWorkspace"), @"defaultWorkspace");
        id applications = RCQAGet(workspace, @"allInstalledApplications");
        if (![applications isKindOfClass:NSArray.class]) {
            RCQARecord(@{@"event":@"inventory_unavailable"});
        } else {
            NSUInteger count = 0;
            for (id app in applications) {
                if (++count > 512) break;
                NSString *bundle = RCQAString(RCQAGet(app,@"applicationIdentifier"));
                NSMutableDictionary *sources = [NSMutableDictionary dictionary];
                NSMutableArray *available = [NSMutableArray array];
                for (NSString *name in @[@"staticShortcutItems",@"dynamicShortcutItems",@"applicationShortcutItems"]) {
                    if ([app respondsToSelector:NSSelectorFromString(name)]) [available addObject:name];
                    NSArray *items = RCQAItems(RCQAGet(app,name));
                    if (items.count) sources[name] = items;
                }
                id url = RCQAGet(app,@"bundleURL");
                if ([url isKindOfClass:NSURL.class] && [url isFileURL]) {
                    NSDictionary *info = [NSDictionary dictionaryWithContentsOfURL:[url URLByAppendingPathComponent:@"Info.plist"]];
                    NSArray *items = RCQAItems(info[@"UIApplicationShortcutItems"]);
                    if (items.count) sources[@"Info.plist"] = items;
                }
                RCQARecord(@{@"event":@"app",@"bundle":bundle,@"availableGetters":available,@"sources":sources});
            }
            RCQARecord(@{@"event":@"scan_complete",@"appsVisited":@(count),@"note":@"Bounded report; absent dynamic getters do not mean no shortcuts."});
        }
    } else if (![command isEqualToString:@"quickactions report"]) {
        return @"Usage: quickactions scan|report|stop";
    }
    return RCQAJSON(@{@"captureEnabled":@(RCQACapturing),@"events":RCQAEvents ?: @[]});
}
