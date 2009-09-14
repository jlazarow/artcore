/*
 *  ACBase.c
 *  ArtCore
 *
 *  Created by Justin Lazarow on 9/6/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */

#include "ACBase.h"


__attribute__((constructor))
static void ArtCoreInit()
{
	static int initialized = 0;
	if (!initialized)
	{
		/* Initialize all the opaque types */
		__ACMemoryMappedFileInitialize();
		__ACArtInitialize();
		__ACSharedArtInitialize();
		__ACSharedArtImageSourceInitialize();
		__ACSharedArtImageDestinationInitialize();
		/* Initialize globally used color space */
		sRGBColorSpace = CGColorSpaceCreateDeviceRGB();
		
	}
}