//
//  MyCppWrapper.mm
//  Pods
//
//  Created by 马远 on 2025/3/26.
//

#import "MyCppWrapper.h"
#include "PitchDetect.h"

@interface MyCppWrapper () {
    PitchDetect* cppClass;
}
@end

@implementation MyCppWrapper

- (instancetype)initWithCallback:(MyCallback)callback {
    self = [super init];
    if (self) {
        cppClass = new PitchDetect([callback](const std::string& value) {
            NSString *str = [NSString stringWithUTF8String:value.c_str()];
            dispatch_async(dispatch_get_main_queue(), ^{
                callback(str);
            });
        });
    }
    return self;
}

- (void)startRecord {
    cppClass->startRecord();
}

- (void)dealloc {
    delete cppClass;
}

- (void)processByteArray:(NSData *)data {
    cppClass->recvData(0,(unsigned char *)data.bytes, data.length);
}

@end
