/*
 *  ACMemoryMappedFile.h
 *  ArtCore
 *
 *  Created by Justin Lazarow on 9/1/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 *
 *  ACMemoryMappedFile is internal - do not use.
 */
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

struct __ACMemoryMappedFile {
	CFRuntimeBase base;
	int fd;
	CFStringRef path;
	CFIndex size;
	uint8_t * bytes;
	CFOptionFlags flags;
};

typedef const struct __ACMemoryMappedFile * ACMemoryMappedFileRef;
typedef struct __ACMemoryMappedFile * ACMutableMemoryMappedFileRef;

ACMemoryMappedFileRef ACMemoryMappedFileCreate(CFStringRef filePath);
ACMutableMemoryMappedFileRef ACMemoryMappedFileCreateMutable(CFStringRef existingFilePath);
CFStringRef ACMemoryMappedFileGetPath(ACMemoryMappedFileRef mf);
CFIndex ACMemoryMappedFileGetSize(ACMemoryMappedFileRef mf);
bool ACMutableMemoryMappedFileSetSize(ACMutableMemoryMappedFileRef mmf, CFIndex newSize);
uint8_t * ACMemoryMappedFileGetBytePtr(ACMemoryMappedFileRef mf);
CFTypeID ACMemoryMappedFileGetTypeID(void);