/*
 *  ACFileUtilities.c
 *  ArtCore
 *
 *  Created by Justin Lazarow on 9/19/09.
 *  Copyright 2009 University of Texas at Austin. All rights reserved.
 *
 */

#include "ACFileUtilities.h"


bool ACWriteImageToURLWithType(CGImageRef imageRef, CFURLRef path, CFStringRef imageType)
{
	CGImageDestinationRef destRef = CGImageDestinationCreateWithURL(path,imageType,1,NULL);
	CGImageDestinationAddImage(destRef,imageRef,NULL);
	bool result = CGImageDestinationFinalize(destRef);
	CFRelease(destRef);
	return result;
}

SInt32 ACWriteDataToURL(CFDataRef dataRef, CFURLRef path)
{
	SInt32 err;
	CFURLWriteDataAndPropertiesToResource(path,dataRef,NULL,&err);
	return err;
}

