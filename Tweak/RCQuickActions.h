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
static id (*RCQAOriginalItems)(id, SEL);
static id RCQAObservedItems(id object, SEL selector) {
    id result = RCQAOriginalItems(object, selector);
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
