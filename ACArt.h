/*
 *  ACArt.h
 *  ArtCore
 *
 *  Created by Justin Lazarow on 7/8/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */

#import <CoreFoundation/CFString.h>
#import "ACArtElement.h"



typedef const struct __ACArt * ACArtRef;
typedef struct __ACArt * ACMutableArtRef;

ACArtRef ACArtCreate(CFStringRef filePath);
ACArtElementRef ACArtGetElementAtIndex(ACArtRef art);

CFStringRef ACArtGetStringForToken(UInt8 token);
void ACArtClose(ACArtRef art);

ACMutableArtRef ACArtCreateMutable(CFStringRef newPath, CFStringRef referenceArtPath);
ACMutableArtRef ACArtCreateMutableWithArt(CFStringRef newPath, ACArtRef referenceArt);
