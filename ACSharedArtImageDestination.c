/*
 *  ACSharedArtImageDestination.c
 *  ArtCore
 *
 *  Created by Justin Lazarow on 7/14/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */


static CFStringRef __ACSharedArtImageDestinationCopyDescription(CFTypeRef cf) {
	ACSharedArtImageDestinationRef idest = (ACSharedArtImageDestinationRef) cf;
	CFMutableStringRef description = CFStringCreateMutable(kCFAllocatorDefault,0);
	CFStringAppendFormat(description,NULL,CFSTR("#<ACSharedArtDestinationSource:%p> Header Index: %d Type: %d EntryCount: %d Holding %d Destinations>")
						 ,idest,idest->headerIndex,idest->imageType,idest->entryCount,CFDictionaryGetCount(idest->data));

	return (CFStringRef) description;
}

static void __ACSharedArtImageDestinationDeallocate(CFTypeRef cf) {
	ACSharedArtImageDestinationRef idst = (ACSharedArtImageDestinationRef) cf;
	CFRelease(idst->owner);
	CFRelease(idst->data);
}

static CFTypeID __kACSharedArtImageDestinationTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __ACSharedArtImageDestinationClass = {
    0,
    "ACSharedArtImageDestination",
    NULL,	// init
    NULL,	// copy
    __ACSharedArtImageDestinationDeallocate,
    NULL,
    NULL,
    NULL,	// 
    __ACSharedArtImageDestinationCopyDescription
};

__private_extern__ void __ACSharedArtImageDestinationInitialize(void) {
	__kACSharedArtImageDestinationTypeID = _CFRuntimeRegisterClass(&__ACSharedArtImageDestinationClass);
}

CFTypeID ACSharedArtImageDestinationGetTypeID(void) {
	return __kACSharedArtImageDestinationTypeID;
}


static ACSharedArtImageDestinationRef __ACSharedArtImageDestinationInit(CFAllocatorRef allocator, ACMutableSharedArtRef owner, CFIndex headerIndex)
{
	ACSharedArtImageDestinationRef memory;
	CFIndex size = sizeof(struct __ACSharedArtImageDestination) - sizeof(CFRuntimeBase);
	memory = (struct __ACSharedArtImageDestination *) _CFRuntimeCreateInstance(allocator, __kACSharedArtImageDestinationTypeID,size,NULL);
	if (memory == NULL)
		return NULL;
	CFRetain(owner);
	memory->owner = owner;
	
	struct __ACSharedArtImageHeader * header = _ACSharedArtGetImageHeaderForIndex(memory->owner, headerIndex);
	memory->entryCount = header->entryCount;
	memory->imageType = header->type;
	memory->headerIndex = headerIndex;
	
	/* Use ACSharedArtImageSource to read out the data and add it to the data dictionary */
	memory->data = CFDictionaryCreateMutable(NULL,2,NULL,&kCFTypeDictionaryValueCallBacks);
	ACSharedArtImageSourceRef reader = ACSharedArtImageSourceCreate(owner, headerIndex);

	switch (memory->imageType) {
		case kSharedArtImageTypeHIRes:
			/* Read both pngs in */
			;
			if (memory->entryCount == 1)
			{
				CGImageRef hiresImage = ACSharedArtImageSourceCreateImageAtIndex(reader, 0);
				CFDictionarySetValue(memory->data, HIRES_IMAGE_KEY, hiresImage);
				CFRelease(hiresImage);
			}
			else {
				CGImageRef regImage = ACSharedArtImageSourceCreateImageAtIndex(reader, 0);
				CGImageRef hiresImage = ACSharedArtImageSourceCreateImageAtIndex(reader, 1);
				CFDictionarySetValue(memory->data, REG_IMAGE_KEY , regImage);
				CFDictionarySetValue(memory->data, HIRES_IMAGE_KEY, hiresImage);
				CFRelease(regImage);
				CFRelease(hiresImage);
			}
			break;
		case kSharedArtImageTypePDF:
			/* Read pdf/image in */
			;
			CFDataRef pdfData = ACSharedArtImageSourceCreateDataAtIndex(reader, 0);
			CGImageRef pdfImage = ACSharedArtImageSourceCreateImageAtIndex(reader, 1);
			CFDictionarySetValue(memory->data, PDF_DATA_KEY, pdfData);
			CFDictionarySetValue(memory->data, REG_IMAGE_KEY, pdfImage);
			CFRelease(pdfData);
			CFRelease(pdfImage);
			break;
		case kSharedArtImageTypeImage:
			/* Just have to read in a single image */
			;
			CGImageRef image = ACSharedArtImageSourceCreateImageAtIndex(reader, 0);
			CFDictionarySetValue(memory->data, REG_IMAGE_KEY, image);
			CFRelease(image);
			break;
	}
	
	CFRelease(reader);
	
	return memory;
	
}



ACSharedArtImageDestinationRef ACSharedArtImageDestinationCreate(ACMutableSharedArtRef owner, CFIndex headerIndex)
{
	return __ACSharedArtImageDestinationInit(NULL, owner, headerIndex);
}

CFDataRef ACSharedArtImageDestinationGetResourceDataAtIndex(ACSharedArtImageDestinationRef idst, CFIndex index)
{
	CFStringRef key = __ACSharedArtImageDestinationGetKeyForDataAtIndex(idst, index);
	if (ACSharedArtImageDestinationGetType(idst) == kSharedArtImageTypePDF && index == 0)
		return CFDictionaryGetValue(idst->data, key);
	else
		return ACImageGetData((CGImageRef) CFDictionaryGetValue(idst->data, key));
}


CFStringRef __ACSharedArtImageDestinationGetKeyForDataAtIndex(ACSharedArtImageDestinationRef idst, CFIndex index)
{
	ACSharedArtImageType type = ACSharedArtImageDestinationGetType(idst);
	CFStringRef keyToUse;
	if (type == kSharedArtImageTypePDF && index == 0)
		keyToUse = PDF_DATA_KEY;
	else if (ACSharedArtImageDestinationIsSpecialHIRes(idst) || (type == kSharedArtImageTypeHIRes && index == 1))
		keyToUse = HIRES_IMAGE_KEY;
	else 
		keyToUse = REG_IMAGE_KEY;
	
	return keyToUse;
}

/* This image data is expected to be in ARGB format, if it isn't please use the ACUtilities  ACARGBImageFromRGBAImage function first
 and pass the result */
bool ACSharedArtImageDestinationSetImageAtIndex(ACSharedArtImageDestinationRef idst, CGImageRef newImage, CFIndex index)
{
	if (index < 0 || index >= ACSharedArtImageDestinationGetEntryCount(idst))
		return false;
	
	switch (ACSharedArtImageDestinationGetType(idst))
	{
		case kSharedArtImageTypePDF:
			if (index == 0)
				return false;
			else 
				CFDictionarySetValue(idst->data, PDF_IMAGE_KEY, newImage);
			return true;
		case kSharedArtImageTypeImage:
			if (index > 0) /* Probably redundant but just in case apple changes this in the future */
				return false;
			CFDictionarySetValue(idst->data, REG_IMAGE_KEY, newImage);
			return true;
		case kSharedArtImageTypeHIRes:
			if (ACSharedArtImageDestinationGetEntryCount(idst) == 1)
				CFDictionarySetValue(idst->data,HIRES_IMAGE_KEY, newImage);
			else 
			{
				CFStringRef key = index == 0 ? REG_IMAGE_KEY : HIRES_IMAGE_KEY;
				CFDictionarySetValue(idst->data, key , newImage);
			}
			return true;
	}
	
	return false;
}

/* This image data is expected to be in ARGB format, if it isn't please use the ACUtilities  ACARGBImageFromRGBAImage function first
 and pass the result */
bool ACSharedArtImageDestinationSetHIResImage(ACSharedArtImageDestinationRef idst, CGImageRef newHIResImage)
{
	if (ACSharedArtImageDestinationGetEntryCount(idst) == 1)
		return ACSharedArtImageDestinationSetImageAtIndex(idst, newHIResImage, 0);
	else
		return ACSharedArtImageDestinationSetImageAtIndex(idst, newHIResImage, 1);
}

bool ACSharedArtImageDestinationSetPDFData(ACSharedArtImageDestinationRef idst, CFDataRef newData)
{
	if (ACSharedArtImageDestinationGetType(idst) != kSharedArtImageTypePDF)
		return false;
	
	CFDictionarySetValue(idst->data, PDF_DATA_KEY, newData);
	return true;
}

/* This image data is expected to be in ARGB format, if it isn't please use the ACUtilities  ACARGBImageFromRGBAImage function first
 and pass the data of the result */
bool ACSharedArtImageDestinationSetImageDataAtIndex(ACSharedArtImageDestinationRef idst, CFDataRef newData, size_t width, size_t height, CFIndex index)
{
	if (index < 0 || index >= ACSharedArtImageDestinationGetEntryCount(idst))
		return false;
	if (ACSharedArtImageDestinationGetType(idst) == kSharedArtImageTypePDF)
		return false;
	
	CGDataProviderRef provider = CGDataProviderCreateWithCFData(newData);
	CGImageRef newImage = CGImageCreate(width, height, 8, 32, width * 4, sRGBColorSpace, 2, provider, 0, 1, 0);
	
	if (ACSharedArtImageDestinationGetEntryCount(idst) == 1 && ACSharedArtImageDestinationGetType(idst) == kSharedArtImageTypeHIRes)
		CFDictionarySetValue(idst->data,HIRES_IMAGE_KEY, newImage);
	else
		CFDictionarySetValue(idst->data, index == 0 ? REG_IMAGE_KEY : HIRES_IMAGE_KEY, newImage);
	CFRelease(provider);
	CFRelease(newImage);
	return true;
}
		
uint16_t ACSharedArtImageDestinationGetEntryCount(ACSharedArtImageDestinationRef idst)
{
	return idst->entryCount;
}


bool ACSharedArtImageDestinationIsSpecialHIRes(ACSharedArtImageDestinationRef idst)
{
	return ACSharedArtImageDestinationGetType(idst) == kSharedArtImageTypeHIRes && ACSharedArtImageDestinationGetEntryCount(idst) == 1;
}
		
uint16_t ACSharedArtImageDestinationGetType(ACSharedArtImageDestinationRef idst)
{
	return idst->imageType;
}

