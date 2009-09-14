/*
 *  ACArt.c
 *  ArtCore
 *
 *  Created by Justin Lazarow on 7/8/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 */

#import "ACArt.h"

struct __ACArtHeader {
	uint16_t imageCount;
	uint16_t pathEntrySize;
	uint32_t tokenCount;
	uint32_t tokenTableOffset;
	uint32_t tokenStringTableOffset;
	uint32_t imageEntryOffset;
	uint32_t imageDataOffset;
};

struct __ACArt {
	CFRuntimeBase base;
	CFStringRef filePath;
	int fdesc;
	struct __ACArtHeader * header;
	void * bytes;
};



struct __ACArtPathTableEntry {
	uint32_t stringOffset;
	UInt8 associcatedTokens[4];
};

struct __ACArtImageDescriptor {
	UInt32 entry_off;
	UInt8 tokens[8];
};


static CFTypeID __kACArtTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __ACArtClass = {
	_kCFRuntimeScannedObject,
	"ACArt",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

void __ACArtInitialize(void) {
	__kACArtTypeID = _CFRuntimeRegisterClass(&__ACArtClass);
}


static ACArtRef __ACArtInit(CFAllocatorRef allocator,CFStringRef filePath)
{	
	struct __ACArt * art;
	struct stat file_stat;
	UInt32 size = sizeof(struct __ACArt) - sizeof(CFRuntimeBase);
	art = (struct __ACArt *) _CFRuntimeCreateInstance(allocator, __kACArtTypeID,size,NULL);
	CFRetain(filePath);
	art->filePath = filePath;
	CFIndex length = CFStringGetLength(filePath) + 1;
	char filePathCharacters[length];
	CFStringGetCString(art->filePath,filePathCharacters,length,kCFStringEncodingUTF8);
	int fd = open(filePathCharacters,O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fd == -1 || fstat(fd, &file_stat) == -1)
	{
		fprintf(stderr, "ACArtInit Error: Could not read ArtFile.bin at %s\n", filePathCharacters);
		CFRelease(art);
		return NULL;
	}
	art->bytes = mmap(0,file_stat.st_size,PROT_READ,MAP_PRIVATE,fd,0);
	art->header = (struct __ACArtHeader * ) art->bytes;
	art->fdesc = fd;
	return (ACArtRef) art;
}


ACArtRef ACArtCreate(CFStringRef filePath)
{
	ACArtRef result = __ACArtInit(kCFAllocatorDefault,filePath);
	return result;
}

UInt16 ACArtGetImageCount(ACArtRef art)
{
	return art->header->imageCount;
}



void * ACArtGetBytes(ACArtRef art)
{
	return art->bytes;
}
