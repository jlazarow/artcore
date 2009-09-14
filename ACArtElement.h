/*
 *  ACArtElement.h
 *  ArtCore
 *
 *  Created by Justin Lazarow on 7/12/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */

typedef struct {
	UInt16 x;
	UInt16 y;
	UInt16 width;
	UInt16 height;
} small_rect;

struct __ACArtElement {
	UInt16 rows;
	UInt16 columns;
	small_rect rects[3];
	UInt32 ignored;
	UInt32 image_off[9];
	UInt16 image_width[9];
	UInt16 image_height[9];
};


typedef const struct __ACArtElement * ACArtElementRef;

//ACArtElementRef ACArtElementCreateFromArtAtOffset(ACArtRef art, UInt32 absoluteOffset);
//ACArtElementRef ACArtElementCreateFromArtAtIndex(ACArtRef art, CFIndex index);
UInt16 ACArtElementGetRows(ACArtElementRef element);
UInt16 ACArtElementGetColumns(ACArtElementRef element);
CGImageRef ACArtElementGetImageAtIndex(ACArtElementRef element, CFIndex index);

