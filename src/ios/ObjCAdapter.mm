#include "ObjCAdapter.h"
#include <QtCore/QString>
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <UIKit/UIKit.h>

#import <UserNotifications/UserNotifications.h>
#import <CoreLocation/CoreLocation.h>
#import "ObjectiveC.h"


//MARK: Vibration
void ObjCAdapter::vibrateBrief() {
    AudioServicesPlayAlertSound(1519);
}

void ObjCAdapter::vibrateError() {
    /*UINotificationFeedbackGenerator *myGen = [[UINotificationFeedbackGenerator alloc] init];
     [myGen prepare];
     [myGen notificationOccurred: UINotificationFeedbackTypeError];*/
    AudioServicesPlayAlertSound(1107);
}

void ObjCAdapter::vibrateLong() {

    AudioServicesPlayAlertSound(kSystemSoundID_Vibrate);
}



//MARK: File Transfer
auto ObjCAdapter::shareContent(const QByteArray& contentByteArray, const QString& mimeType, const QString& fileNameTemplate, const QString& fileExtension) -> QString
{
    
    auto content = contentByteArray.toNSData();
    
    NSURL *tmpDirURL = [NSURL fileURLWithPath:NSTemporaryDirectory() isDirectory:YES];
    NSURL *fileURL = [[tmpDirURL URLByAppendingPathComponent:fileNameTemplate.toNSString()] URLByAppendingPathExtension:fileExtension.toNSString()];
    
    if (fileURL && [content writeToURL: fileURL atomically: YES]) {
        UIWindow *keyWindow = nil;
        for (UIWindowScene* windowScene in [UIApplication sharedApplication].connectedScenes) {
            if (windowScene.activationState == UISceneActivationStateForegroundActive) {
                keyWindow = windowScene.windows.firstObject;
                break;
            }
        }
        UIViewController *rootViewController = keyWindow.rootViewController;
        UIActivityViewController *activityController = [[UIActivityViewController alloc]
                                                        initWithActivityItems: @[fileURL]
                                                        applicationActivities: nil];
        
        if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
            CGSize screenSize = [UIScreen mainScreen].bounds.size;
            activityController.popoverPresentationController.sourceView = rootViewController.view;
            activityController.popoverPresentationController.sourceRect = CGRectMake(screenSize.width - 30, 30,0,0);
        }
        [rootViewController presentViewController:activityController animated:YES completion:nil];
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

void ObjCAdapter::saveToGallery(QString& path) {
    UIImage* image = [UIImage imageNamed:path.toNSString()];
    UIImageWriteToSavedPhotosAlbum(image, Nil, Nil, Nil);
}


//MARK: Flight Notifications

void ObjCAdapter::requestNotificationPermission() {
    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
    [center requestAuthorizationWithOptions:(UNAuthorizationOptionAlert | UNAuthorizationOptionSound)
                          completionHandler:^(BOOL granted, NSError * _Nullable error) {
        Q_UNUSED(granted)
        Q_UNUSED(error)
    }];
}

void ObjCAdapter::postNotification(const QString& title, const QString& body) {
    UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
    content.title = title.toNSString();
    content.body = body.toNSString();
    content.sound = [UNNotificationSound defaultSound];

    // A nil trigger delivers the notification immediately.
    UNNotificationRequest *request = [UNNotificationRequest
        requestWithIdentifier:[[NSUUID UUID] UUIDString]
        content:content
        trigger:nil];

    [[UNUserNotificationCenter currentNotificationCenter]
        addNotificationRequest:request
         withCompletionHandler:nil];
    [content release];
}


//MARK: Background Location

static CLLocationManager* s_bgLocationManager = nil;
// Intentionally never released: this is a singleton for the lifetime of the
// process, not a leak. ObjCAdapter has no instance/destructor to hook a
// teardown into, and the manager must stay alive for as long as the app can
// still be asked to re-enable background location (e.g. the next flight).

bool ObjCAdapter::enableBackgroundLocation() {
    if (s_bgLocationManager == nil) {
        s_bgLocationManager = [[CLLocationManager alloc] init];
    }

    CLAuthorizationStatus status = s_bgLocationManager.authorizationStatus;
    if (status == kCLAuthorizationStatusNotDetermined) {
        // First time ever: show the system dialog. The result arrives
        // asynchronously; the caller will find out on its next attempt to
        // enable background location (e.g. the next flight).
        [s_bgLocationManager requestAlwaysAuthorization];
        return false;
    }

    if (status != kCLAuthorizationStatusAuthorizedAlways) {
        // Denied, restricted, or only "While Using" -- requestAlwaysAuthorization
        // is a silent no-op once the status is already determined, so there is
        // no way to re-prompt here. The caller is responsible for telling the
        // user to fix this in Settings.
        return false;
    }

    // This manager's only job is to keep the process alive in the background;
    // Qt's own CLLocationManager (QGeoPositionInfoSource) already provides the
    // full-accuracy fix used for actual navigation. Requesting best accuracy
    // here as well would run a second full-precision GPS session for the
    // whole flight, for no functional benefit -- a needless battery cost on a
    // device that may be the pilot's only navigation instrument.
    s_bgLocationManager.desiredAccuracy = kCLLocationAccuracyThreeKilometers;
    s_bgLocationManager.allowsBackgroundLocationUpdates = YES;
    s_bgLocationManager.pausesLocationUpdatesAutomatically = NO;
    [s_bgLocationManager startUpdatingLocation];
    return true;
}

void ObjCAdapter::disableBackgroundLocation() {
    if (s_bgLocationManager != nil) {
        s_bgLocationManager.allowsBackgroundLocationUpdates = NO;
        [s_bgLocationManager stopUpdatingLocation];
    }
}

