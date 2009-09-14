/*
 *  ACUtilities.h
 *  ArtCore
 *
 *  Created by Justin Lazarow on 7/16/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */

#include <dlfcn.h>
#define CUIImageConstantKeysSymbol "__ZZ29_CUIGetImageConstantKeysArrayvE21sCUIImageConstantKeys"
#define CUIImageConstantIDsSymbol "_kCUIImageConstantIDs_131385"
#define CUIConstantKeysSymbol "_kCUIConstantKeys_131382"
#define CUIConstantIDsSymbol "_kCUIConstantIDs_131383"
#define ImageCount 378
#define ConstantCount 333


bool ACWriteCGImageToPath(CGImageRef imageRef,CFStringRef imageType, CFURLRef path);
bool ACWriteCGImageToPathAsPNG(CGImageRef imageRef, CFURLRef path);
bool ACWriteCGImageToPathAsPDF(CGImageRef imageRef, CFURLRef path);
bool ACWriteCGImageToPathAsTIFF(CGImageRef imageRef, CFURLRef path);
SInt32 ACWriteCFDataToPathAsPDF(CFDataRef dataRef, CFURLRef path);
CGImageRef ACReadImageAtPath(CFStringRef path);
CGImageRef ACReadPNGImageAtPath(CFStringRef path);
CGImageRef ACARGBImageFromRGBAImage(CGImageRef image);
CFDataRef ACImageGetData(CGImageRef image);
void ACSharedArtReadImagesFromPathUsingReferenceFileToOutputFile(CFStringRef resourcesPath,CFStringRef oldSArtFilePath, CFStringRef newSArtFilePath);
void ACWriteCUIAndImageMapToPath(CFStringRef path);