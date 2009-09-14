/*
 *  ACSharedArtImageDestination.h
 *  ArtCore
 *
 *  Created by Justin Lazarow on 7/14/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 *
 *
 *  When creating an ACSharedArtImageDestination, the current resource/data will be loaded from the owner SharedArt until it is changed.
 *
 *
 */

#define REG_IMAGE_KEY CFSTR("REGIMAGE")
#define PDF_DATA_KEY CFSTR("PDFDATA")
#define PDF_IMAGE_KEY CFSTR("PDFIMAGE")
#define HIRES_IMAGE_KEY CFSTR("HIRES")

struct __ACSharedArtImageDestination
{
	CFRuntimeBase base;
	ACMutableSharedArtRef owner; /* not entirely neccesary to cache this */
	CFIndex headerIndex;
	uint16_t entryCount;
	uint16_t imageType;
	CFMutableDictionaryRef data; /* dictionary of CFDataRefs of image or pdf data (determined by the type field */
};

typedef struct __ACSharedArtImageDestination * ACSharedArtImageDestinationRef;
ACSharedArtImageDestinationRef ACSharedArtImageDestinationCreate(ACMutableSharedArtRef owner, CFIndex headerIndex);
bool ACSharedArtImageDestinationSetImageAtIndex(ACSharedArtImageDestinationRef idst, CGImageRef newImage, CFIndex index);
uint16_t ACSharedArtImageDestinationGetEntryCount(ACSharedArtImageDestinationRef idst);
uint16_t ACSharedArtImageDestinationGetType(ACSharedArtImageDestinationRef idst);
/*bool ACSharedArtImageDestinationAddImage(ACSharedArtImageDestinationRef idest, CGImageRef imageRef);
bool ACSharedArtImageDestinaACtionAddImageAtIndex(ACSharedArtImageDestinationRef idest, CGImageRef imageRef, size_t index);
bool ACSharedArtImageDestinationSetHIResImage(ACSharedArtImageDestinationRef idest, CGImageRef imageRef, size_t index);
void ACSharedArtImageDestinationAddImageFromSource(ACSharedArtImageDestinationRef idest, ACSharedArtImageSourceRef isrc, size_t index);*/