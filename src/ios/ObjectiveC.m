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


+ (ObjectiveC *) sharedInstance {
    static ObjectiveC *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[ObjectiveC alloc] init];
    });
    return sharedInstance;
}

@end
