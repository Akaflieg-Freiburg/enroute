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

- (bool) hasLocationPermissionDenied {
    if (![CLLocationManager locationServicesEnabled]) {
      return true;
    }
    bool enabled = NO;
    CLLocationManager *locationManager = [CLLocationManager new];
    switch([locationManager authorizationStatus]) {
        case kCLAuthorizationStatusRestricted:
        case kCLAuthorizationStatusDenied:
            enabled = YES;
            break;
        default:
            enabled = NO;
            break;
    }
    [locationManager release];
    return enabled;
}

+ (ObjectiveC *) sharedInstance {
    static ObjectiveC *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[ObjectiveC alloc] init];
    });
    return sharedInstance;
}

@end
