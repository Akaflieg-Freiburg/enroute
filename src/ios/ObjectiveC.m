//
//  ObjectiveC.m
//  
//
//  Created by Simon Schneider on 02.05.23.
//

#include "CoreLocation/CoreLocation.h"
#include "UIKit/UIKit.h"
#import <Foundation/Foundation.h>
#import "ObjectiveC.h"
#import <UserNotifications/UserNotifications.h>


@implementation ObjectiveC

-(void) initializeListeners {
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillChange:) name:UIKeyboardWillChangeFrameNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];
}

- (void)keyboardWillChange:(NSNotification *)notification {
    CGRect keyboardRect = [notification.userInfo[UIKeyboardFrameEndUserInfoKey] CGRectValue];
    [self setKeyboardHeight: keyboardRect.size.height];
}

- (void)keyboardWillHide:(NSNotification *)notification {
    [self setKeyboardHeight: 0];
}

- (bool) hasNotificationPermission {
    __block bool enabled = false;
    __block bool hasResult = false;
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    UNUserNotificationCenter* current = [UNUserNotificationCenter currentNotificationCenter];
    ([current getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings *settings) {
      switch(settings.authorizationStatus) {
        case UNAuthorizationStatusDenied:
        case UNAuthorizationStatusNotDetermined:
          enabled = NO;
          break;
        default:
          enabled = YES;
          break;
      }
      hasResult = true;
      dispatch_semaphore_signal(semaphore);
    }]);

    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
    dispatch_release(semaphore);
    return enabled;
}

- (bool) hasLocationPermission {
    bool enabled = NO;
    switch([CLLocationManager authorizationStatus]) {
      case kCLAuthorizationStatusAuthorizedWhenInUse:
      case kCLAuthorizationStatusAuthorizedAlways:
        enabled = YES;
        break;
      default:
        enabled = NO;
        break;
    }
    return enabled;
}

+ (ObjectiveC *) sharedInstance {
    static ObjectiveC *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[ObjectiveC alloc] init];
        [sharedInstance initializeListeners];
    });
    return sharedInstance;
}

@end
