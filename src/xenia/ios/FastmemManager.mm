// Copyright 2026 XeniOS Project
// SPDX-License-Identifier: GPL-2.0-or-later

#import "FastmemManager.h"

#import "xenia/base/memory.h"

@interface FastmemManager ()
@property (readwrite) bool fastmemAvailable;
@end

@implementation FastmemManager

+ (FastmemManager*)shared {
  static FastmemManager* sharedInstance = nil;
  static dispatch_once_t onceToken;
  
  dispatch_once(&onceToken, ^{
    sharedInstance = [[self alloc] init];
  });
  
  return sharedInstance;
}

- (id)init {
  if (self = [super init]) {
#if XE_PLATFORM_IOS && XE_ARCH_ARM64
    // Test fastmem availability by attempting the allocation
    self.fastmemAvailable = xe::memory::xe::memory::TestFastmemAvailability();
#else
    self.fastmemAvailable = false;
#endif
  }
  
  return self;
}

@end