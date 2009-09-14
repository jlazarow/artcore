/*
 *  ACSharedArt.c
 *  ArtCore
 *
 *  Created by Justin Lazarow on 7/8/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */

#define HEADER_OFF_LENGTH 4


enum {
	kACSharedArtImmutable = 0,
	kACSharedArtMutable = 1,
};


static CFStringRef __ACSharedArtCopyDescription(CFTypeRef cf)
{
	ACSharedArtRef sart = (ACSharedArtRef) cf;
	return CFStringCreateWithFormat(kCFAllocatorDefault,NULL,CFSTR("#<ACSharedArt:%p> Image Count: %d"),sart,ACSharedArtGetImageCount(sart));
}

static void __ACSharedArtDeallocate(CFTypeRef cf)
{
	ACSharedArtRef sart = (ACSharedArtRef) cf;
	CFRelease(sart->sartFile);
}

static CFTypeID __kACSharedArtTypeID = _kCFRuntimeNotATypeID;

static CFRuntimeClass __ACSharedArtClass = {0};

void __ACSharedArtInitialize(void) {
    __ACSharedArtClass.version = 0;
    __ACSharedArtClass.className = "ACSharedArt";
    __ACSharedArtClass.init = NULL;
    __ACSharedArtClass.copy = NULL;
    __ACSharedArtClass.finalize = __ACSharedArtDeallocate;
    __ACSharedArtClass.equal = NULL;
    __ACSharedArtClass.hash = NULL;
    __ACSharedArtClass.copyFormattingDesc = NULL;
    __ACSharedArtClass.copyDebugDesc = __ACSharedArtCopyDescription;
    __kACSharedArtTypeID = _CFRuntimeRegisterClass((const CFRuntimeClass * const)&__ACSharedArtClass);
}

CFTypeID ACSharedArtGetTypeID(void) {
    return __kACSharedArtTypeID;
}

static ACMutableSharedArtRef __ACSharedArtInit(CFAllocatorRef allocator, CFOptionFlags flags, CFStringRef filePath)
{	
	struct __ACSharedArt * sart;
	CFIndex size = sizeof(struct __ACSharedArt) - sizeof(CFRuntimeBase);
	sart = (struct __ACSharedArt *) _CFRuntimeCreateInstance(allocator, __kACSharedArtTypeID,size,NULL);
	sart->flags = flags;
	switch (__CFBitfieldGetValue(flags,1,0))
	{
		case kACSharedArtImmutable:
			sart->sartFile = ACMemoryMappedFileCreate(filePath);
			break;
		case kACSharedArtMutable:
			sart->sartFile = ACMemoryMappedFileCreateMutable(filePath);
			break;
	}

	sart->header = (struct __ACSharedArtHeader * ) ACMemoryMappedFileGetBytePtr(sart->sartFile);
	return sart;
}

ACSharedArtRef ACSharedArtCreate(CFStringRef filePath)
{
	return __ACSharedArtInit(kCFAllocatorDefault,kACSharedArtImmutable,filePath);
}

ACMutableSharedArtRef ACSharedArtCreateMutable(CFStringRef referencePath)
{
	return __ACSharedArtInit(kCFAllocatorDefault,kACSharedArtMutable,referencePath);
}
		   
UInt16 ACSharedArtGetImageCount(ACSharedArtRef sart)
{
	return sart->header->imageCount;
}

UInt32 ACSharedArtGetImageHeaderOffsetForIndex(ACSharedArtRef sart, CFIndex index)
{
	UInt32 offsetToEntryOffset = sizeof(struct __ACSharedArtHeader) + HEADER_OFF_LENGTH * index;
	return *(UInt32 *)(ACMemoryMappedFileGetBytePtr(sart->sartFile) + offsetToEntryOffset);
}

struct __ACSharedArtImageHeader * _ACSharedArtGetImageHeaderForIndex(ACSharedArtRef sart, CFIndex headerIndex)
{
	void * headerBytes = ACSharedArtGetBytePtr(sart) + ACSharedArtGetImageHeaderOffsetForIndex(sart,headerIndex);
	return (struct __ACSharedArtImageHeader *) headerBytes;
}

uint8_t * ACSharedArtGetBytePtr(ACSharedArtRef sart)
{
	/* Doesn't seem like the *right* thing to do, but I can't think of any other way to keep a ACSharedArtRef from knowing too much
	 about ACSharedArtImageSource other than this */
	
	return ACMemoryMappedFileGetBytePtr(sart->sartFile);
}

/* In the future we will hopefully have a function to change the sartfile in memory with rewriting it as it is costly */


/* Writes a new SharedArtFile to the path given replacing those entries given by the dictionary of ACSharedArtImageDestinations */
void ACMutableSharedArtWrite(ACMutableSharedArtRef msart, CFStringRef writePath, CFDictionaryRef destinations)
{
	CFRetain(destinations);
	CFRetain(writePath);
	CFIndex sizeWithZeroData = msart->header->dataOffset;
	CFMutableDataRef outputData = CFDataCreateMutable(NULL, 0);
	CFDataSetLength(outputData, sizeWithZeroData);
	uint8_t * bytePtr = ACSharedArtGetBytePtr(msart);
	/* Reads first header offset to see how many bytes apple is padding, although it isn't *totally* neccessary to pad like apple, it doesnt hurt
	 in case they somehow use these bits in the future */
	uint32_t firstOffset = *(uint32_t *) (bytePtr + 8);
	CFDataReplaceBytes(outputData, CFRangeMake(0, firstOffset), bytePtr, firstOffset);

	for (CFIndex imageIdx = 0; imageIdx < ACSharedArtGetImageCount(msart); imageIdx++)
	{
		uint32_t headerOffset = *(uint32_t *) (bytePtr + HEADER_OFF_LENGTH * imageIdx + sizeof(struct __ACSharedArtHeader));
		CFDataReplaceBytes(outputData, CFRangeMake(headerOffset, 12), bytePtr + headerOffset, 12);
		struct __ACSharedArtImageHeader * oldHeader = (struct __ACSharedArtImageHeader *) (bytePtr + headerOffset);
		for (CFIndex entryIdx = 0; entryIdx < oldHeader->entryCount; entryIdx++)
		{
			/* We must do this every inner loop iteration since the MutableBytePtr is subject to change as we modify it */
			struct __ACSharedArtImageHeader * newHeader = (struct __ACSharedArtImageHeader *) (CFDataGetMutableBytePtr(outputData) + headerOffset);
			ACSharedArtImageDestinationRef ref = (ACSharedArtImageDestinationRef) CFDictionaryGetValue(destinations, CFNumberCreate(NULL, kCFNumberCFIndexType , &imageIdx));
			newHeader->data_info[entryIdx].relativeOffset = CFDataGetLength(outputData) - sizeWithZeroData;
			if (!ref)
			{
				newHeader->data_info[entryIdx].width = oldHeader->data_info[entryIdx].width;
				newHeader->data_info[entryIdx].height = oldHeader->data_info[entryIdx].height;
				newHeader->data_info[entryIdx].length = oldHeader->data_info[entryIdx].length;
				CFDataAppendBytes(outputData, (bytePtr + sizeWithZeroData + oldHeader->data_info[entryIdx].relativeOffset),newHeader->data_info[entryIdx].length);
			}
			else 
			{
				if (newHeader->type == kSharedArtImageTypePDF)
				{
					newHeader->data_info[entryIdx].width = oldHeader->data_info[entryIdx].width;
					newHeader->data_info[entryIdx].height = oldHeader->data_info[entryIdx].height;
					
					if (entryIdx == 0)
					{
						CFDataRef pdfData = CFDictionaryGetValue(ref->data,PDF_DATA_KEY);
						CFRetain(pdfData);
						newHeader->data_info[entryIdx].length = CFDataGetLength(pdfData);
						CFDataAppendBytes(outputData, CFDataGetBytePtr(pdfData), newHeader->data_info[entryIdx].length);
						CFRelease(pdfData);
					}
					else if (entryIdx == 1) {
						CGImageRef pdfImage = (CGImageRef) CFDictionaryGetValue(ref->data,PDF_IMAGE_KEY);
						CFRetain(pdfImage);
						newHeader->data_info[entryIdx].width = (uint16_t) CGImageGetWidth(pdfImage);
						newHeader->data_info[entryIdx].height = (uint16_t) CGImageGetHeight(pdfImage);
						newHeader->data_info[entryIdx].length = (uint32_t) 4 * newHeader->data_info[entryIdx].width * newHeader->data_info[entryIdx].height;
						CFDataRef imageData = ACImageGetData(pdfImage);
						CFDataAppendBytes(outputData, 
										  CFDataGetBytePtr(imageData), 4 * newHeader->data_info[entryIdx].width * newHeader->data_info[entryIdx].height);
						CFRelease(imageData);
						CFRelease(pdfImage);
					}

				}
				else {
					CFStringRef keyToUse;
					if ((newHeader->type == kSharedArtImageTypeHIRes && entryIdx == 1)
						|| (newHeader->type == kSharedArtImageTypeHIRes && entryIdx == 0 && newHeader->entryCount == 1))
						keyToUse = HIRES_IMAGE_KEY;
					else 
						keyToUse = REG_IMAGE_KEY;
					CGImageRef image = (CGImageRef) CFDictionaryGetValue(ref->data, keyToUse);
					CFRetain(image);
					newHeader->data_info[entryIdx].width = (uint16_t) CGImageGetWidth(image);
					newHeader->data_info[entryIdx].height = (uint16_t) CGImageGetHeight(image);
					newHeader->data_info[entryIdx].length = (uint32_t) 4 * newHeader->data_info[entryIdx].width * newHeader->data_info[entryIdx].height;
					CFDataRef imageData = ACImageGetData(image);
					CFDataAppendBytes(outputData, 
									  CFDataGetBytePtr(imageData), 4 * newHeader->data_info[entryIdx].width * newHeader->data_info[entryIdx].height);
					CFRelease(image);
					CFRelease(imageData);
				}
			}


		}
	}
	
	SInt32 err;
	CFURLWriteDataAndPropertiesToResource(CFURLCreateWithString(NULL,writePath,NULL) , outputData, NULL, &err);	
	if (err != 0)
		fprintf(stderr, "ACSharedArt Error: Writing of new file to disk failed with err = %d", err);
	CFRelease(outputData);
	CFRelease(writePath);
	CFRelease(destinations);
}

/* Something for the future :D */
void ACMutableSharedArtWritePatchPropertyList(CFDictionaryRef destinations)
{
}

void ACSharedArtWriteImagesToPath(ACSharedArtRef sart, CFStringRef path)
{
	for (int index = 0; index < ACSharedArtGetImageCount(sart); index++)
	{
		ACSharedArtImageSourceRef isrc = ACSharedArtImageSourceCreate(sart,index);
		CFShow(isrc);
		
		for (int imageIndex = 0; imageIndex < ACSharedArtImageSourceGetEntryCount(isrc);imageIndex++)
		{
			
			UInt16 type = ACSharedArtImageSourceGetType(isrc);
			
			if (type == kSharedArtImageTypeHIRes || type == kSharedArtImageTypeImage)
			{
				CGImageRef imgRef = ACSharedArtImageSourceCreateImageAtIndex(isrc,imageIndex);
				CFURLRef baseURLRef = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,path,kCFURLPOSIXPathStyle,true);
				CFStringRef fileTypeRef;
					if (imageIndex == 1 && type == kSharedArtImageTypeHIRes)
						fileTypeRef = CFSTR("%d-HIRes.png");
					else if (imageIndex == 0 && (type == kSharedArtImageTypeHIRes) && (ACSharedArtImageSourceGetEntryCount(isrc) == 1))
						fileTypeRef = CFSTR("%d-HIRes.png");
					else 
						fileTypeRef = CFSTR("%d.png");
				CFStringRef fileString = CFStringCreateWithFormat(kCFAllocatorDefault,NULL,fileTypeRef,index,imageIndex);
				CFURLRef urlRef = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorDefault,fileString,kCFURLPOSIXPathStyle,false,baseURLRef);
				bool result = ACWriteCGImageToPathAsPNG(imgRef,urlRef);
				CFRelease(baseURLRef);
				CFRelease(fileString);
				CFRelease(fileTypeRef);
				CFRelease(urlRef);
				CGImageRelease(imgRef);
			}
			else if (type == kSharedArtImageTypePDF)
			{
				CFURLRef baseURLRef = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,path,kCFURLPOSIXPathStyle,true);
				CFStringRef fileString = CFStringCreateWithFormat(kCFAllocatorDefault,NULL,imageIndex == 0 ? CFSTR("%d.pdf") : CFSTR("%d.png") ,index,imageIndex);
				CFURLRef urlRef = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorDefault,fileString,kCFURLPOSIXPathStyle,false,baseURLRef);
				if (imageIndex == 0)
				{
					CFDataRef pdfData = ACSharedArtImageSourceCreateDataAtIndex(isrc,imageIndex);
					ACWriteCFDataToPathAsPDF(pdfData,urlRef);
					CFRelease(pdfData);
				}
				else if (imageIndex == 1)
				{
					CGImageRef pdfImage = ACSharedArtImageSourceCreateImageAtIndex(isrc,imageIndex);
					bool result = ACWriteCGImageToPathAsPNG(pdfImage,urlRef);
					CGImageRelease(pdfImage);
				}
				CFRelease(baseURLRef);
				CFRelease(fileString);
				CFRelease(urlRef);
			}
		}
		CFRelease(isrc);
	}
}

inline CFIndex __ACSharedArtGetType(ACSharedArtRef sart)
{
	return __CFBitfieldGetValue(sart->flags,1,0);
}




