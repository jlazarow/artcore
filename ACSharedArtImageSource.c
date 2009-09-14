/*
 *  ACSharedArtImageSource.c
 *  ArtCore
 *
 *  Created by Justin Lazarow on 7/14/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */


/* The API and philosophy around ACSharedArtImageSource is similar to the ideas behind a CGImageSource. However, instead of thinking
 of the disk as the "source", ArKit delegates the SArtFile.bin as the source. In a similar fashion, @see ACSharedArtImageDestination
 for an abstraction around writing images/elements to an SArtFile.bin. In SArtFile.bin there are 3 "image types", the first being ARGB
 data called by Apple as HIRes art that contains 2 ACSharedArtImageHeaderDataInfo entries and while the first entry points to the
 image you would expect while the second holds a very very high resolution of the normally sized image seen in the first entry. The second type is ARGB image
 data with only one ACSharedArtImageHeaderDataInfo entry. The third type represents PDF data. Similar to HIRes art, these have two
 ACSharedArtImageHeaderDataInfo entries although the second entry seems unused as with HIRes art. The PDF data is interesting in that
 it can be scaled when needed to match a certain DPI. In order to achieve this, Apple uses a CGImageSource from the PDF data and when
 they call CGImageSourceCreateImageAtIndex() they provide a CFDictionary with a private constant - kCGImageSourceRasterizationDPI to
 scale the PDF art. */


static CFStringRef __ACSharedArtImageSourceCopyFormat(CFTypeRef cf, CFDictionaryRef format)
{
	ACSharedArtImageSourceRef isrc = (ACSharedArtImageSourceRef) cf;
	CFMutableStringRef description = CFStringCreateMutable(kCFAllocatorDefault,0);
	CFStringAppendFormat(description,NULL,CFSTR("Type: %d EntryCount: %d\n"),isrc->header->type,isrc->header->entryCount);
	for (int i = 0; i < isrc->header->entryCount; i++)
	{
		struct __ACSharedArtImageHeaderDataInfo dataInfoAtIndex = isrc->header->data_info[i];
		CFStringAppendFormat(description,NULL,CFSTR("\tImage %d Size: %dx%d Length: %d @ %d \n"),i,dataInfoAtIndex.width,dataInfoAtIndex.height,
							 dataInfoAtIndex.length,dataInfoAtIndex.relativeOffset);
	}
	return (CFStringRef) description;
}

static CFStringRef __ACSharedArtImageSourceCopyDescription(CFTypeRef cf)
{
	ACSharedArtImageSourceRef isrc = (ACSharedArtImageSourceRef) cf;
	CFMutableStringRef description = CFStringCreateMutable(kCFAllocatorDefault,0);
	CFStringAppendFormat(description,NULL,CFSTR("#<ACSharedArtImageSource:%p> Type: %d EntryCount: %d\n unk1 %d unk2 %d"),isrc,isrc->header->type,
						 isrc->header->entryCount,isrc->header->unknown1,isrc->header->unknown2);
	for (int i = 0; i < isrc->header->entryCount; i++)
	{
		struct __ACSharedArtImageHeaderDataInfo dataInfoAtIndex = isrc->header->data_info[i];
		CFStringAppendFormat(description,NULL,CFSTR("\tImage %d Size: %dx%d Length: %d @ %d \n"),i,dataInfoAtIndex.width,dataInfoAtIndex.height,
							 dataInfoAtIndex.length,dataInfoAtIndex.relativeOffset);
	}
	return (CFStringRef) description;
}

static void __ACSharedArtImageSourceDeallocate(CFTypeRef cf)
{
	ACSharedArtImageSourceRef sart = (ACSharedArtImageSourceRef) cf;
	CFRelease(sart->owner);
	/* Don't free the header since we don't own it */
}

static CFTypeID __kACSharedArtImageSourceTypeID = _kCFRuntimeNotATypeID;

static CFRuntimeClass __ACSharedArtImageSourceClass = {0};

void __ACSharedArtImageSourceInitialize(void) {
	__ACSharedArtImageSourceClass.version = 0;
	__ACSharedArtImageSourceClass.className = "ACSharedArtImageSource";
	__ACSharedArtImageSourceClass.init = NULL;
	__ACSharedArtImageSourceClass.copy = NULL;
	__ACSharedArtImageSourceClass.finalize = __ACSharedArtImageSourceDeallocate;
	__ACSharedArtImageSourceClass.equal = NULL;
	__ACSharedArtImageSourceClass.hash = NULL;
	__ACSharedArtImageSourceClass.copyFormattingDesc = __ACSharedArtImageSourceCopyFormat;
	__ACSharedArtImageSourceClass.copyDebugDesc = __ACSharedArtImageSourceCopyDescription;
    __kACSharedArtImageSourceTypeID = _CFRuntimeRegisterClass((const CFRuntimeClass *)&__ACSharedArtImageSourceClass);
}

CFTypeID ACSharedArtImageSourceGetTypeID(void) {
    return __kACSharedArtImageSourceTypeID;
}

static ACSharedArtImageSourceRef __ACSharedArtImageSourceInit(CFAllocatorRef allocator, ACSharedArtRef owner, CFIndex headerIndex)
{	
	struct __ACSharedArtImageSource * isrc;
	UInt32 size = sizeof(struct __ACSharedArtImageSource) - sizeof(CFRuntimeBase);
	isrc = (struct __ACSharedArtImageSource *) _CFRuntimeCreateInstance(allocator, __kACSharedArtImageSourceTypeID,size,NULL);
	CFRetain(owner);
	isrc->owner = owner;
	
	void * headerBytes = ACSharedArtGetBytePtr(isrc->owner) + ACSharedArtGetImageHeaderOffsetForIndex(isrc->owner,headerIndex);
	isrc->header = (struct __ACSharedArtImageHeader *) (headerBytes);

	return (ACSharedArtImageSourceRef) isrc;
}

/* Returns the type of image data contained within this ACSharedArtImageSource represented by:
	kSharedArtImageSourceTypeHIRes
	kSharedArtImageSourceTypeImage
	kSharedArtImageSourceTypePDF 
*/
UInt16 ACSharedArtImageSourceGetType(ACSharedArtImageSourceRef isrc)
{
	return isrc->header->type;
}

/*
 Returns the amount of images contained within this ACSharedArtImageSource
*/
UInt16 ACSharedArtImageSourceGetEntryCount(ACSharedArtImageSourceRef isrc)
{
	return isrc->header->entryCount;
}

/*
 TACes a ACSharedArtRef as the "owner" file in order to provide certain data in order to create the image source along
 with an index specifying where in the HeaderOffset table should this source's offset be loaded from. You are responsible
 for releasing this source when you are done with it.
 
 Returns a ACSharedArtImageSourceRef representing various image info and data concerning this element in the SArtFile.bin
*/
ACSharedArtImageSourceRef ACSharedArtImageSourceCreate(ACSharedArtRef owner, CFIndex headerIndex)
{
	ACSharedArtImageSourceRef result = __ACSharedArtImageSourceInit(kCFAllocatorDefault,owner,headerIndex);
	return result;
}


/*
 Attempts to create a CGImageRef from the ACSharedArtImageSourceRef. The data held by the CGImageRef can be
 ARGB or PDF, one must query ACSharedArtImageSourceGetType in order to make this determination. A common
 way to write this data if you need to a disk is by making a CGImageDestination, adding the CGImageRef, and
 using the corresponding UTType to write it to the disk.
 */

CGImageRef ACSharedArtImageSourceCreateImageAtIndex(ACSharedArtImageSourceRef isrc, size_t index)
{
	if (index > ACSharedArtImageSourceGetEntryCount(isrc) - 1)
		return NULL;
	
	struct __ACSharedArtImageHeaderDataInfo dataInfoAtIndex = isrc->header->data_info[index];
	CGDataProviderRef dataProvider = CGDataProviderCreateWithData(NULL,
										(const void *) (ACSharedArtGetBytePtr(isrc->owner) + dataInfoAtIndex.relativeOffset + isrc->owner->header->dataOffset), 
										dataInfoAtIndex.length,
									    0);
	CGColorSpaceRef clrSpace = CGColorSpaceCreateDeviceRGB();
	CGImageRef imageRef = CGImageCreate(dataInfoAtIndex.width,dataInfoAtIndex.height,8,32,dataInfoAtIndex.width * 4,clrSpace,2,dataProvider,0,1,0);
	CGColorSpaceRelease(clrSpace);
	CGDataProviderRelease(dataProvider);
	return imageRef;
}

CGImageRef ACSharedArtImageSourceCreateHIResImage(ACSharedArtImageSourceRef isrc)
{
	if (ACSharedArtImageSourceGetType(isrc) != kSharedArtImageTypeHIRes || ACSharedArtImageSourceGetEntryCount(isrc) < 2)
		return NULL;
	return ACSharedArtImageSourceCreateImageAtIndex(isrc,1);
}

/* Since some parts of the ACSharedArtImageSource are actual PDFs in their full format, we cannot use
 CGImage on certain data (mainly the 1st entry of PDF data). Instead call this method to retrieve
 a CFDataRef to the bytes pointed to with the exact length specified by the source. You are
 responsible for releasing this reference when you are done. */
CFDataRef ACSharedArtImageSourceCreateDataAtIndex(ACSharedArtImageSourceRef isrc, size_t index)
{
	if (index > ACSharedArtImageSourceGetEntryCount(isrc) - 1)
		return NULL;
	
	struct __ACSharedArtImageHeaderDataInfo dataInfoAtIndex = isrc->header->data_info[index];
	return CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,(const UInt8 *) (ACSharedArtGetBytePtr(isrc->owner) + dataInfoAtIndex.relativeOffset +
																			isrc->owner->header->dataOffset),dataInfoAtIndex.length,kCFAllocatorNull);
}
