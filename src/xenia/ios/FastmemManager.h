// Copyright 2026 XeniOS Project
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef XENIA_IOS_FASTMEM_MANAGER_H_
#define XENIA_IOS_FASTMEM_MANAGER_H_

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface FastmemManager : NSObject

+ (FastmemManager*)shared;

@property (readonly) bool fastmemAvailable;

@end

NS_ASSUME_NONNULL_END

#endif  // XENIA_IOS_FASTMEM_MANAGER_H_