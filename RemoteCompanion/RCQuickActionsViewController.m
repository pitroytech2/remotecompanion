#import "RCQuickActionsViewController.h"
#import "RCServerClient.h"
@interface RCQuickActionsViewController ()
@property(nonatomic,strong) UITextView *report;
@end
@implementation RCQuickActionsViewController
- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = @"App Quick Shortcut";
    self.view.backgroundColor = UIColor.systemBackgroundColor;
    self.report = [[UITextView alloc] initWithFrame:self.view.bounds];
    self.report.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.report.editable = NO;
    self.report.text = @"Tap Scan to list registered shortcuts and start menu capture. Then long-press an app icon. Return here and tap Report. Copy the report to send it. Stop ends capture. This is diagnostics, not shortcut activation.";
    [self.view addSubview:self.report];
    self.navigationItem.rightBarButtonItems = @[
        [[UIBarButtonItem alloc] initWithTitle:@"Scan" style:0 target:self action:@selector(scan)],
        [[UIBarButtonItem alloc] initWithTitle:@"Report" style:0 target:self action:@selector(refresh)],
        [[UIBarButtonItem alloc] initWithTitle:@"Copy" style:0 target:self action:@selector(copyReport)],
        [[UIBarButtonItem alloc] initWithTitle:@"Stop" style:0 target:self action:@selector(stop)]];
}
- (void)command:(NSString *)command {
    for (UIBarButtonItem *item in self.navigationItem.rightBarButtonItems) item.enabled = NO;
    __weak typeof(self) weakSelf = self;
    [[RCServerClient sharedClient] executeCommand:command completion:^(NSString *output, NSError *error) {
        dispatch_async(dispatch_get_main_queue(), ^{
            weakSelf.report.text = error ? error.localizedDescription : (output ?: @"No response");
            for (UIBarButtonItem *item in weakSelf.navigationItem.rightBarButtonItems) item.enabled = YES;
        });
    }];
}
- (void)scan { [self command:@"quickactions scan"]; }
- (void)refresh { [self command:@"quickactions report"]; }
- (void)stop { [self command:@"quickactions stop"]; }
- (void)copyReport { UIPasteboard.generalPasteboard.string = self.report.text; }
@end
