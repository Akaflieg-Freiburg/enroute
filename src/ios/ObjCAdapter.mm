#include "ObjCAdapter.h"
#include <QtCore/QString>
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <UIKit/UIKit.h>


QString ObjCAdapter::objectiveC_Call()
{
	NSDate *today = [NSDate date];
	NSString *description = [today descriptionWithLocale: [NSLocale currentLocale]];
	return QString::fromNSString(description);
}

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
    UIWindow *window = UIApplication.sharedApplication.windows.firstObject;
    return window.safeAreaInsets.bottom;
}
double ObjCAdapter::safeAreaRightInset() {
    UIWindow *window = UIApplication.sharedApplication.windows.firstObject;
    return window.safeAreaInsets.right;
}
