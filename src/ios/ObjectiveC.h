//
//  ObjectiveC.h
//  
//
//  Created by Simon Schneider on 02.05.23.
//

@interface ObjectiveC : NSObject

@property float keyboardHeight;

- (bool) hasNotificationPermission;
- (bool) hasLocationPermission;

+(ObjectiveC *)sharedInstance;

@end
