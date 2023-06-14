//
//  ObjectiveC.m
//  
//
//  Created by Simon Schneider on 02.05.23.
//

#include "CoreLocation/CoreLocation.h"
#include "UIKit/UIKit.h"
#import <Foundation/Foundation.h>
#import "SafeAreaService.h"


@implementation SafeAreaService

- (void) initializeListeners {
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center addObserver:self selector:@selector(keyboardWillChange:) name:UIKeyboardWillChangeFrameNotification object:nil];
    [center addObserver:self selector:@selector(keyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];
}

- (void) keyboardWillChange:(NSNotification *)notification {
    CGRect keyboardRect = [notification.userInfo[UIKeyboardFrameEndUserInfoKey] CGRectValue];
    [self setKeyboardHeight: keyboardRect.size.height];
}

- (void) keyboardWillHide:(NSNotification *)notification {
    [self setKeyboardHeight: 0];
}

- (double) safeAreaTop {
    return [self primaryWindow].safeAreaInsets.top;
}
- (double) safeAreaLeft {
    return [self primaryWindow].safeAreaInsets.left;
}
- (double) safeAreaRight {
    return [self primaryWindow].safeAreaInsets.right;
}
- (double) safeAreaBottom {
    if (self.keyboardHeight > 0) {
        return self.keyboardHeight;
    } else {
        return [self primaryWindow].safeAreaInsets.bottom;
    }
}

- (UIWindow *) primaryWindow {
    return UIApplication.sharedApplication.windows.firstObject;
}

+ (SafeAreaService *) sharedInstance {
    static SafeAreaService *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[SafeAreaService alloc] init];
        [sharedInstance initializeListeners];
    });
    return sharedInstance;
}

@end
