//
//  ObjectiveC.h
//  
//
//  Created by Simon Schneider on 02.05.23.
//

@interface SafeAreaService : NSObject

@property float keyboardHeight;

+(SafeAreaService *)sharedInstance;

-(double) safeAreaTop;
-(double) safeAreaRight;
-(double) safeAreaLeft;
-(double) safeAreaBottom;

@end
