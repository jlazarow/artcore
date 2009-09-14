/*
 *  ACArtImageSource.c
 *  ArtCore
 *
 *  Created by Justin Lazarow on 8/1/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */

#include "ACArtImageSource.h"


struct __ACArtImageSource {
	CFRuntimeBase base;
	//struct __ACArtImageHeader * header;
	//ACArtRef owner;
};


static CFStringRef __ACArtImageSourceCopyFormat(CFTypeRef cf, CFDictionaryRef format)
{
	return CFSTR("orly?");
}

static CFStringRef __ACArtImageSourceCopyDescription(CFTypeRef cf)
{
	
	return CFSTR("was?");
}

static void __ACArtImageSourceDeallocate(CFTypeRef cf)
{

}

static CFTypeID __kACArtImageSourceTypeID = _kCFRuntimeNotATypeID;

static CFRuntimeClass __ACArtImageSourceClass = {0};

void __ACArtImageSourceInitialize(void) {
	__ACArtImageSourceClass.version = 0;
	__ACArtImageSourceClass.className = "ACArtImageSource";
	__ACArtImageSourceClass.init = NULL;
	__ACArtImageSourceClass.copy = NULL;
	__ACArtImageSourceClass.finalize = __ACArtImageSourceDeallocate;
	__ACArtImageSourceClass.equal = NULL;
	__ACArtImageSourceClass.hash = NULL;
	__ACArtImageSourceClass.copyFormattingDesc = __ACArtImageSourceCopyFormat;
	__ACArtImageSourceClass.copyDebugDesc = __ACArtImageSourceCopyDescription;
    __kACArtImageSourceTypeID = _CFRuntimeRegisterClass((const CFRuntimeClass *)&__ACArtImageSourceClass);
}

CFTypeID ACArtImageSourceGetTypeID(void) {
    return __kACArtImageSourceTypeID;
}


CGImageRef ACArtImageSourceGetImageAtIndex(ACArtImageSourceRef isrc, CFIndex index)
{
}