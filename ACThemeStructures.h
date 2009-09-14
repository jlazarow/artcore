/*
 *  ACThemeStructures.h
 *  ArtCore
 *
 *  Created by Justin Lazarow on 9/5/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */

enum {
	kSharedArtImageTypeHIRes = 1,
	kSharedArtImageTypeImage = 2,
	kSharedArtImageTypePDF = 3
};

struct __ACSharedArtImageHeaderDataInfo {
	uint16_t width; 
	uint16_t height; 
	uint32_t length; 
	uint32_t relativeOffset; 
};

struct __ACSharedArtImageHeader {
	uint16_t type;
	uint16_t entryCount; 
	uint32_t unknown1;
	uint32_t unknown2;
	struct __ACSharedArtImageHeaderDataInfo data_info[];
};