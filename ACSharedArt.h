/*
 *  ACSharedArt.h
 *  ArtCore
 *
 *  Created by Justin Lazarow on 7/8/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */

/* While ArtFile.bin is most commonly used for storing graphics and art of many user interface widgets, SArtFile.bin serves to cover
 the "gap" between elements like glyphs, menus, search tools, resizing grips, drawer frames, etc. Elements in the SArtFile.bin are
 very fixed in many ways including size, state, variant although there are some elements that do appear to have some of these features
 in it. SArtFile.bin has changed from TIFF,PNG,PDF data to ARGB,PDF data from Leopard to Snow Leopard giving many the notion that it
 has "lost" elements to ArtFile.bin when in fact it has not. In addition, SArtFile.bin lacks no metadata concerning mappings or keys.
 Further inspection into CoreUI presents these SArtFile.bin entry indices are represented by IDs starting at 20000 as a marker to denote
 them from ArtFile.bin elements. */

#import "CFRuntime.h"
#import "ACMemoryMappedFile.h"
#import "ACThemeStructures.h"


struct __ACSharedArtHeader {
	uint16_t magic;
	uint16_t imageCount;
	uint32_t dataOffset;
};

struct __ACSharedArt {
	CFRuntimeBase base;
	ACMemoryMappedFileRef sartFile;
	struct __ACSharedArtHeader * header;
	CFOptionFlags flags;
};


typedef const struct __ACSharedArt * ACSharedArtRef;
typedef struct __ACSharedArt * ACMutableSharedArtRef;

ACSharedArtRef ACSharedArtCreate(CFStringRef filePath);
ACMutableSharedArtRef ACSharedArtCreateMutable(CFStringRef referencePath);
UInt16 ACSharedArtGetImageCount(ACSharedArtRef sart);
UInt32 ACSharedArtGetImageHeaderOffsetForIndex(ACSharedArtRef sart, CFIndex index);
uint8_t * ACSharedArtGetBytePtr(ACSharedArtRef sart);
void ACSharedArtWriteImagesToPath(ACSharedArtRef sart, CFStringRef path);
void ACMutableSharedArtWrite(ACMutableSharedArtRef msart, CFStringRef writePath, CFDictionaryRef destinations);
void ACSharedArtClose(ACSharedArtRef sharedArt);

struct __ACSharedArtImageHeader * _ACSharedArtGetImageHeaderForIndex(ACSharedArtRef sart, CFIndex headerIndex);