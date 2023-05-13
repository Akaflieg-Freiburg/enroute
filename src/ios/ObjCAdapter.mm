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
    float keyboardHeight = [instance keyboardHeight];
    if (keyboardHeight > 0) {
      return keyboardHeight;
    } else {
      UIWindow *window = UIApplication.sharedApplication.windows.firstObject;
      auto bottom = window.safeAreaInsets.bottom;
      return bottom;
    }
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

  if ([CLLocationManager locationServicesEnabled]) {
    return [[ObjectiveC sharedInstance] hasLocationPermission];
  }

  return false;
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






auto ObjCAdapter::shareContent(const QByteArray& contentByteArray, const QString& mimeType, const QString& fileNameTemplate, const QString& fileExtension) -> QString
{
    NSMutableArray *sharingItems = [NSMutableArray new];

    auto content = contentByteArray.toNSData();

    NSURL *tmpDirURL = [NSURL fileURLWithPath:NSTemporaryDirectory() isDirectory:YES];
    NSURL *fileURL = [[tmpDirURL URLByAppendingPathComponent:fileNameTemplate.toNSString()] URLByAppendingPathExtension:fileExtension.toNSString()];

    [content writeToURL: fileURL atomically: YES];


    [sharingItems addObject: fileURL];
    // get the main window rootViewController
    UIViewController *qtUIViewController = [[UIApplication sharedApplication].keyWindow rootViewController];
    UIActivityViewController *activityController = [[UIActivityViewController alloc] initWithActivityItems: sharingItems applicationActivities:nil];

    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad )
    {
        activityController.popoverPresentationController.sourceView = qtUIViewController.view;
        activityController.popoverPresentationController.sourceRect = CGRectMake(0,0,0,0);
    }
    [qtUIViewController presentViewController:activityController animated:YES completion:nil];
    return {};
}
