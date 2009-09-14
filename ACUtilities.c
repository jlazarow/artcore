/*
 *  ACUtilities.c
 *  ArtCore
 *
 *  Created by Justin Lazarow on 7/16/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */

#include "ACUtilities.h"
#include <CoreFoundation/CFURLAccess.h>
#import <Accelerate/Accelerate.h>

bool ACWriteCGImageToPath(CGImageRef imageRef,CFStringRef imageType, CFURLRef path)
{
	CGImageDestinationRef destRef = CGImageDestinationCreateWithURL(path,imageType,1,NULL);
	CGImageDestinationAddImage(destRef,imageRef,NULL);
	bool result = CGImageDestinationFinalize(destRef);
	CFRelease(destRef);
	return result;
}

bool ACWriteCGImageToPathAsPNG(CGImageRef imageRef, CFURLRef path)
{
	return ACWriteCGImageToPath(imageRef,kUTTypePNG,path);
}

bool ACWriteCGImageToPathAsPDF(CGImageRef imageRef, CFURLRef path)
{
	return ACWriteCGImageToPath(imageRef,kUTTypePDF,path);
}

bool ACWriteCGImageToPathAsTIFF(CGImageRef imageRef, CFURLRef path)
{
	return ACWriteCGImageToPath(imageRef,kUTTypeTIFF,path);
}

SInt32 ACWriteCFDataToPathAsPDF(CFDataRef dataRef, CFURLRef path)
{
	SInt32 err;
	CFURLWriteDataAndPropertiesToResource(path,dataRef,NULL,&err);
	return err;
}

CGImageRef ACReadImageAtPath(CFStringRef path)
{
	CFURLRef url = CFURLCreateWithString(NULL, path, NULL);
	CGImageSourceRef isrc = CGImageSourceCreateWithURL(url, NULL);
	CFRelease(url);
	CGImageRef image = CGImageSourceCreateImageAtIndex(isrc, 0, NULL);
	CFRelease(isrc);
	return image;
}

CGImageRef ACReadPNGImageAtPath(CFStringRef path)
{
	CFURLRef url = CFURLCreateWithString(NULL, path, NULL);
	CGDataProviderRef prov = CGDataProviderCreateWithURL(url);
	CFRelease(url);
	CGImageRef image = CGImageCreateWithPNGDataProvider(prov, NULL, 1, 0);
	return image;
}

CGImageRef ACARGBImageFromRGBAImage(CGImageRef image)
{
	CFDataRef imageData = CGDataProviderCopyData(CGImageGetDataProvider(image));
	CFIndex length = CFDataGetLength(imageData);
	uint8_t pixelData[length];
	CFDataGetBytes(imageData,CFRangeMake(0, length), pixelData);
	
	/* save data */
	size_t width = CGImageGetWidth(image);
	size_t height = CGImageGetHeight(image);
	size_t bitsPerComponent = CGImageGetBitsPerComponent(image);
	size_t bitsPerPixel = CGImageGetBitsPerPixel(image);
	size_t bytesPerRow = CGImageGetBytesPerRow(image);
	CGColorSpaceRef colorSpace = CGImageGetColorSpace(image);
	CGBitmapInfo info = CGImageGetBitmapInfo(image);
	
	/* transform data */
	
	vImage_Buffer transformBuffer;
	transformBuffer.data = pixelData;
	transformBuffer.height = height;
	transformBuffer.width = width;
	transformBuffer.rowBytes = bytesPerRow;
	vImage_Buffer destinationBuffer = transformBuffer;
	uint8_t transformMap[] = { 3, 0, 1, 2 };
	vImagePermuteChannels_ARGB8888(&transformBuffer, &destinationBuffer, transformMap, kvImageDoNotTile);
	
	
	/* wrap the pixelData into a CFData since pixelData is gone after this method returns */
	CFDataRef pixelCFData = CFDataCreate(NULL, pixelData, length);
	CGDataProviderRef newProvider = CGDataProviderCreateWithCFData(pixelCFData);
	CFRelease(pixelCFData);
	CGImageRef newImage = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpace, kCGImageAlphaPremultipliedFirst, 
										newProvider, 0, 1, 0);
	CFRelease(imageData);
	CFRelease(newProvider);
	return newImage;
}


CFDataRef ACImageGetData(CGImageRef image)
{
	return CGDataProviderCopyData(CGImageGetDataProvider(image));
}


void ACSharedArtReadImagesFromPathUsingReferenceFileToOutputFile(CFStringRef resourcesPath,CFStringRef oldSArtFilePath, CFStringRef newSArtFilePath)
{
	CFRetain(resourcesPath);
	CFRetain(newSArtFilePath);
	ACMutableSharedArtRef msart = ACSharedArtCreateMutable(oldSArtFilePath);
	CFMutableDictionaryRef destinations = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	for (CFIndex imageIndex = 0; imageIndex < ACSharedArtGetImageCount(msart); imageIndex++)
	{
		ACSharedArtImageDestinationRef idest = ACSharedArtImageDestinationCreate(msart, imageIndex);
		uint16_t type = ACSharedArtImageDestinationGetType(idest);
		for (CFIndex entryIndex = 0; entryIndex < ACSharedArtImageDestinationGetEntryCount(idest); entryIndex++)
		{
			if (type == kSharedArtImageTypePDF && entryIndex == 0)
			{
				CFStringRef path = CFStringCreateWithFormat(NULL, NULL, CFSTR("file://%@/%d.pdf"),resourcesPath,imageIndex,entryIndex);
				CFURLRef url = CFURLCreateWithString(NULL, path, NULL);
				CFRelease(path);
				CFDataRef pdfData;
				SInt32 err;
				if (!CFURLCreateDataAndPropertiesFromResource(NULL, url, &pdfData, NULL, NULL, &err))
					fprintf(stderr, "Failed to read resource with err %d",err);
				CFRelease(url);
				ACSharedArtImageDestinationSetPDFData(idest, pdfData);
				CFRelease(pdfData);
			}
			else {
				CFStringRef path;
				if ((entryIndex == 1 || ACSharedArtImageDestinationGetEntryCount(idest) == 1) && type == kSharedArtImageTypeHIRes)
					path = CFStringCreateWithFormat(NULL, NULL, CFSTR("file://%@/%d-HIRes.png"),resourcesPath,imageIndex);
				else 
					path = CFStringCreateWithFormat(NULL, NULL, CFSTR("file://%@/%d.png"),resourcesPath,imageIndex);
				CGImageRef image = ACARGBImageFromRGBAImage(ACReadPNGImageAtPath(path));
				CFRelease(path);
				ACSharedArtImageDestinationSetImageAtIndex(idest, image, entryIndex);
				CFRelease(image);
			}

		}
		CFNumberRef index = CFNumberCreate(NULL, kCFNumberCFIndexType, &imageIndex);
		CFDictionarySetValue(destinations, index, idest);
		CFRelease(index);
		CFRelease(idest);
	}
	ACMutableSharedArtWrite(msart, newSArtFilePath, destinations);
	CFRelease(newSArtFilePath);
	CFRelease(resourcesPath);
	CFRelease(destinations);
	CFRelease(msart);
}

/*void ACWriteCUIAndImageMapToPath(CFStringRef path)
{
	void * handle = dlopen("/System/Library/PrivateFrameworks/CoreUI.framework/CoreUI", RTLD_LAZY | RTLD_LOCAL);
	void * CUIImageConstantKeysAddr = dlsym(handle, CUIImageConstantKeysSymbol);
	void * CUIImageConstantIDsAddr = dlsym(handle, CUIImageConstantIDsSymbol);
	CFMutableDictionaryRef imageMapDict = CFDictionaryCreateMutable(NULL, ImageCount, NULL, NULL);
	for (int i = 0; i < ImageCount; i++)
	{
		CFNumberRef id = CFNumberCreate(NULL, kCFNumberSInt32Type, CUIImageConstantIDsAddr + 4 * i);
		CFStringRef constant = (CFStringRef) 
	}
	

	//void * CUIConstantKeysAddr = dlsym(handle, CUIConstantKeysSymbol);
	//void * CUIConstantIDsAddr = dlsym(handle, CUIConstantIDsSymbol);
	
	
}


CFStringRef ACCFStringFromSerializedOffset(uint32_t offset)
{
}*/



