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
	/* RGBA to premul ARGB conversion */
	for (CFIndex pixelIndex = 0; pixelIndex < length; pixelIndex += 4)
	{
		uint8_t a = pixelData[pixelIndex+3];
		uint8_t r = pixelData[pixelIndex];
		uint8_t g = pixelData[pixelIndex+1];
		uint8_t b = pixelData[pixelIndex+2];
		pixelData[pixelIndex] = a;
		pixelData[pixelIndex+1] = (r * a) / 255;
		pixelData[pixelIndex+2] = (g * a) / 255;
		pixelData[pixelIndex+3] = (b * a) / 255;
	}
	/* save data */
	size_t width = CGImageGetWidth(image);
	size_t height = CGImageGetHeight(image);
	size_t bitsPerComponent = CGImageGetBitsPerComponent(image);
	size_t bitsPerPixel = CGImageGetBitsPerPixel(image);
	size_t bytesPerRow = CGImageGetBytesPerRow(image);
	CGColorSpaceRef colorSpace = CGImageGetColorSpace(image);
	CGBitmapInfo info = CGImageGetBitmapInfo(image);

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



