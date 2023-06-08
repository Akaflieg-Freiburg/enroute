//
//  ObjectiveC.h
//  
//
//  Created by Simon Schneider on 02.05.23.
//

@interface ObjectiveC : NSObject

- (bool) hasNotificationPermission;
- (bool) hasLocationPermission;

+ (ObjectiveC *)sharedInstance;

@end
