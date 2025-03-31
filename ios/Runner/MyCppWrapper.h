#import <Foundation/Foundation.h>

typedef void (^MyCallback)(NSString *value);

@interface MyCppWrapper : NSObject

- (instancetype)initWithCallback:(MyCallback)callback;
- (void)startRecord;
- (void)processByteArray:(NSData *)data;
@end
