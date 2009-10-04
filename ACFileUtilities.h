/*
 *  ACFileUtilities.h
 *  ArtCore
 *
 *  Created by Justin Lazarow on 9/19/09.
 *  Copyright 2009 University of Texas at Austin. All rights reserved.
 *
 */

bool ACWriteImageToURLWithType(CGImageRef imageRef, CFURLRef path,CFStringRef imageType);
SInt32 ACWriteDataToURL(CFDataRef dataRef, CFURLRef path);
