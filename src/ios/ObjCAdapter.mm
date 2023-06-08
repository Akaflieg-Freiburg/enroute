#include "ObjCAdapter.h"
#include <QtCore/QString>
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <UIKit/UIKit.h>

#import <UserNotifications/UserNotifications.h>
#import <CoreLocation/CoreLocation.h>
#import "ObjectiveC.h"
#import "SafeAreaService.h"


//MARK: Vibration
void ObjCAdapter::vibrateBrief() {
    AudioServicesPlaySystemSound(1519);
}

void ObjCAdapter::vibrateError() {
    /*UINotificationFeedbackGenerator *myGen = [[UINotificationFeedbackGenerator alloc] init];
     [myGen prepare];
     [myGen notificationOccurred: UINotificationFeedbackTypeError];*/
    AudioServicesPlayAlertSound(1107);
}

void ObjCAdapter::vibrateLong() {
  //TODO: Test whether SystemSoundId is correct
  AudioServicesPlayAlertSound(1520);
}


//MARK: Safe Area
double ObjCAdapter::safeAreaTopInset() {
    return [[SafeAreaService sharedInstance] safeAreaTop];
}
double ObjCAdapter::safeAreaLeftInset() {
    return [[SafeAreaService sharedInstance] safeAreaLeft];
}
double ObjCAdapter::safeAreaBottomInset() {
    return [[SafeAreaService sharedInstance] safeAreaBottom];
}
double ObjCAdapter::safeAreaRightInset() {
    return [[SafeAreaService sharedInstance] safeAreaRight];
}


//MARK: Location
bool ObjCAdapter::hasLocationPermission() {
    
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


//MARK: Notifications
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
    [content release];
    
    // Schedule the notification.
    UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
    [center addNotificationRequest:request withCompletionHandler:^(NSError *__nullable error) {
        //TODO: Handle error
        //NSLog(@"Error: %@", error.localizedDescription);
    }];
}


//MARK: File Transfer
auto ObjCAdapter::shareContent(const QByteArray& contentByteArray, const QString& mimeType, const QString& fileNameTemplate, const QString& fileExtension) -> QString
{
    
    auto content = contentByteArray.toNSData();
    
    NSURL *tmpDirURL = [NSURL fileURLWithPath:NSTemporaryDirectory() isDirectory:YES];
    NSURL *fileURL = [[tmpDirURL URLByAppendingPathComponent:fileNameTemplate.toNSString()] URLByAppendingPathExtension:fileExtension.toNSString()];
    
    if (fileURL && [content writeToURL: fileURL atomically: YES]) {
        UIViewController *qtUIViewController = [[UIApplication sharedApplication].keyWindow rootViewController];
        UIActivityViewController *activityController = [[UIActivityViewController alloc]
                                                        initWithActivityItems: @[fileURL]
                                                        applicationActivities: nil];
        
        if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
            CGSize screenSize = [UIScreen mainScreen].bounds.size;
            activityController.popoverPresentationController.sourceView = qtUIViewController.view;
            activityController.popoverPresentationController.sourceRect = CGRectMake(screenSize.width - 30, 30,0,0);
        }
        [qtUIViewController presentViewController:activityController animated:YES completion:nil];
        [activityController release];
        return {};
    }
    
    return "Failed to write file";
}


//MARK: Misc
QString ObjCAdapter::preferredLanguage() {
    //TODO: Use preferredLocalizationsFromArray: later
    NSString *language = [[[NSBundle mainBundle] preferredLocalizations] objectAtIndex: 0];
    return QString::fromNSString(language);
}


void ObjCAdapter::disableScreenSaver() {
    [[UIApplication sharedApplication] setIdleTimerDisabled:YES];
}
