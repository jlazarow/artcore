/*
 *  ACBase.h
 *  ArtCore
 *
 *  Created by Justin Lazarow on 9/6/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */

/* Taken from Apple's CF-Lite distribution */
#define __CFBitfieldMask(N1, N2)	((((UInt32)~0UL) << (31UL - (N1) + (N2))) >> (31UL - N1))
#define __CFBitfieldGetValue(V, N1, N2)	(((V) & __CFBitfieldMask(N1, N2)) >> (N2))

static CGColorSpaceRef sRGBColorSpace = NULL;