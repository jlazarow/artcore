/*
 *  ACSharedArtImageSource.h
 *  ArtCore
 *
 *  Created by Justin Lazarow on 7/14/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */
#import <ApplicationServices/ApplicationServices.h>

#define PDF_TEMPLATE CFSTR("%d.pdf")
#define HIRES_TEMPLATE CFSTR("%d-HIRes.png")
#define PNG_TEMPLATE CFSTR("%d.png")

struct __ACSharedArtImageSource {
	CFRuntimeBase base;
	struct __ACSharedArtImageHeader * header;
	CFIndex headerIndex;
	ACSharedArtRef owner;
};

typedef const struct __ACSharedArtImageSource * ACSharedArtImageSourceRef;

void __ACSharedArtImageSourceInitialize(void);
ACSharedArtImageSourceRef ACSharedArtImageSourceCreate(ACSharedArtRef sart, CFIndex headerIndex);
UInt16 ACSharedArtImageSourceGetType(ACSharedArtImageSourceRef isrc);
UInt16 ACSharedArtImageSourceGetEntryCount(ACSharedArtImageSourceRef isrc);
CGImageRef ACSharedArtImageSourceCreateImageAtIndex(ACSharedArtImageSourceRef isrc, size_t index);
CGImageRef ACSharedArtImageSourceCreateHIResImage(ACSharedArtImageSourceRef isrc);
CFDataRef ACSharedArtImageSourceCreateDataAtIndex(ACSharedArtImageSourceRef isrc, size_t index);
void ACSharedArtImageSourceWriteResourceToPathAtIndex(ACSharedArtImageSourceRef isrc, CFStringRef path, CFIndex index);
CFStringRef __ACSharedArtImageSourceGetExtensionForWritingAtIndex(ACSharedArtImageSourceRef isrc, CFIndex index);

