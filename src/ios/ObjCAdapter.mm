#include "ObjCAdapter.h"
#include <QtCore/QString>
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <UIKit/UIKit.h>

#import <UserNotifications/UserNotifications.h>
#import <CoreLocation/CoreLocation.h>
#import "ObjectiveC.h"


void ObjCAdapter::vibrateBrief() {
    AudioServicesPlaySystemSound(1519);
}

void ObjCAdapter::vibrateError() {
	/*UINotificationFeedbackGenerator *myGen = [[UINotificationFeedbackGenerator alloc] init];
	[myGen prepare];
	[myGen notificationOccurred: UINotificationFeedbackTypeError];*/
    AudioServicesPlayAlertSound(1107);
}


//TODO: Reduce redundant code
double ObjCAdapter::safeAreaTopInset() {
    UIWindow *window = UIApplication.sharedApplication.windows.firstObject;
    return window.safeAreaInsets.top;
}
double ObjCAdapter::safeAreaLeftInset() {
    UIWindow *window = UIApplication.sharedApplication.windows.firstObject;
    return window.safeAreaInsets.left;
}
double ObjCAdapter::safeAreaBottomInset() {
    ObjectiveC* instance = [ObjectiveC sharedInstance];

    UIWindow *window = UIApplication.sharedApplication.windows.firstObject;
    auto result = window.safeAreaInsets.bottom;
    float keyboardHeight = [instance keyboardHeight];
    return result + keyboardHeight;
}
double ObjCAdapter::safeAreaRightInset() {
    UIWindow *window = UIApplication.sharedApplication.windows.firstObject;
    return window.safeAreaInsets.right;
}

void ObjCAdapter::disableScreenSaver() {
  [[UIApplication sharedApplication] setIdleTimerDisabled:YES];
}

bool ObjCAdapter::hasLocationPermission() {
  return [CLLocationManager locationServicesEnabled];
}

bool ObjCAdapter::hasNotificationPermission() {
  return [[ObjectiveC sharedInstance] hasNotificationPermission];
}

void ObjCAdapter::requestNotificationPermission() {
  UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
  [center requestAuthorizationWithOptions:(UNAuthorizationOptionAlert + UNAuthorizationOptionSound)
                        completionHandler:^(BOOL granted, NSError * _Nullable error) {
                            if (granted) {
                              //ObjCAdapter::sendNotification("Test", "This is a test body");
                            }
                        }];

}

void ObjCAdapter::sendNotification(QString title, QString message){
  UNMutableNotificationContent* content = [[UNMutableNotificationContent alloc] init];
  content.title = [NSString localizedUserNotificationStringForKey:title.toNSString() arguments:nil];
  content.body = [NSString localizedUserNotificationStringForKey:message.toNSString() arguments:nil];
  content.sound = [UNNotificationSound defaultSound];

  // Deliver the notification in five seconds.
  UNTimeIntervalNotificationTrigger* trigger = [UNTimeIntervalNotificationTrigger
              triggerWithTimeInterval:5 repeats:NO];
  UNNotificationRequest* request = [UNNotificationRequest requestWithIdentifier:[[NSUUID UUID] UUIDString]
              content:content trigger:trigger];

  // Schedule the notification.
  UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
  [center addNotificationRequest:request withCompletionHandler:^(NSError *__nullable error) {
    NSLog(@"Error: %@", error.localizedDescription);
  }];
}
