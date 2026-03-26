#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
extern "C" int get_ios_major_version_uidevice() {
    NSString* version = [UIDevice currentDevice].systemVersion;
    // systemVersion is a string like "26.2" — take the integer part.
    return (int)[version integerValue];
}
extern "C" void ios_request_jit() {
    NSString* bundleID = [[NSBundle mainBundle] bundleIdentifier];
    if (!bundleID) return;

    NSString* urlString =
        [NSString stringWithFormat:@"apple-magnifier://enable-jit?bundle-id=%@", bundleID];

    NSURL* url = [NSURL URLWithString:urlString];
    if ([[UIApplication sharedApplication] canOpenURL:url]) {
        [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
    }
}