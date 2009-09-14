/*
 *  ACMemoryMappedFile.c
 *  ArtCore
 *
 *  Created by Justin Lazarow on 9/1/09.
 *  Copyright 2009 Justin Lazarow. All rights reserved.
 *
 *
 *  ACMemoryMappedFile is internal - do not use.
 */

#include <stdlib.h>
#include <CoreFoundation/CFString.h>


static CFStringRef __ACMemoryMappedFileCopyDescription(CFTypeRef cf) {
	return NULL;
}

enum {
	kACMemoryMappedFileImmutable = 0,
	kACMemoryMappedFileMutable = 1
};

static void __ACMemoryMappedFileDeallocate(CFTypeRef cf) {
	ACMemoryMappedFileRef idest = (ACMemoryMappedFileRef) cf;
	if (__CFBitfieldGetValue(idest->flags,1,0) == kACMemoryMappedFileMutable)
	{
		free(idest->bytes);
	}
	else {
		munmap(idest->bytes, idest->size);
	}
	
	CFRelease(idest->path);
	close(idest->fd);
}

static CFTypeID __kACMemoryMappedFileTypeID = _kCFRuntimeNotATypeID;

static const CFRuntimeClass __ACMemoryMappedFileClass = {
    0,
    "ACMemoryMappedFile",
    NULL,	// init
    NULL,	// copy
    __ACMemoryMappedFileDeallocate,
    NULL,
    NULL,
    NULL,	// 
    __ACMemoryMappedFileCopyDescription
};

void __ACMemoryMappedFileInitialize(void) {
	__kACMemoryMappedFileTypeID = _CFRuntimeRegisterClass(&__ACMemoryMappedFileClass);
}

CFTypeID ACMemoryMappedFileGetTypeID(void) {
	return __kACMemoryMappedFileTypeID;
}

CFIndex __ACMemoryMappedFileGetType(ACMemoryMappedFileRef mf)
{
	return __CFBitfieldGetValue(mf->flags,1,0);
}


static ACMutableMemoryMappedFileRef __ACMemoryMappedFileInit(CFAllocatorRef allocator, CFOptionFlags flags, CFStringRef filePath)
{
	ACMutableMemoryMappedFileRef memory;
	CFIndex size = sizeof(struct __ACMemoryMappedFile) - sizeof(CFRuntimeBase);
	struct stat file_stat;
	
	memory = (ACMutableMemoryMappedFileRef) _CFRuntimeCreateInstance(allocator, __kACMemoryMappedFileTypeID, size, NULL);
	if (memory == NULL)
		return NULL;
	memory->flags = flags;
	CFRetain(filePath);
	memory->path = filePath;
	CFIndex length = CFStringGetLength(filePath) + 1;
	char filePathCharacters[length];
	CFStringGetCString(memory->path,filePathCharacters,length,kCFStringEncodingUTF8);
	int fd = open(filePathCharacters,O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	size_t fs = fstat(fd, &file_stat);
	memory->fd = fd;
	memory->size = size;
	
	if (fd == -1 || fs == -1)
	{
		fprintf(stderr, "ACMemoryMappedFileInit Error: Could not read file at %s\n", filePathCharacters);
		CFRelease(memory);
		return NULL;
	}
	
	// read/mmap the file
	
	switch (__CFBitfieldGetValue(flags,1,0))
	{
		case kACMemoryMappedFileMutable:
			;
			void * fileBytes = malloc(file_stat.st_size);
			if (read(fd,fileBytes,file_stat.st_size) == -1)
			{
				fprintf(stderr, "ACMemoryMappedFileInit Error: Read fail");
				CFRelease(memory);
				return NULL;
			}
			memory->bytes = fileBytes;
			break;
			
		case kACMemoryMappedFileImmutable:
			memory->bytes = mmap(0,file_stat.st_size,PROT_READ,MAP_PRIVATE,fd,0);
			if (memory->bytes == MAP_FAILED)
			{
				fprintf(stderr, "ACMemoryMappedFileInit Error: Mmap fail");
				CFRelease(memory);
				return NULL;
			}
			break;
	}
	
	return memory;
}

ACMemoryMappedFileRef ACMemoryMappedFileCreate(CFStringRef filePath)
{
	return __ACMemoryMappedFileInit(NULL, kACMemoryMappedFileImmutable, filePath);
}

/* Not really a *true* memory mapped file, just here to facilitate an abstraction between mutable/immutable (S)ArtFiles 
   Will copy the file contents at the path into a buffer so file path must exist */
ACMutableMemoryMappedFileRef ACMemoryMappedFileCreateMutable(CFStringRef existingFilePath)
{
	return __ACMemoryMappedFileInit(NULL, kACMemoryMappedFileMutable, existingFilePath);
}


CFStringRef ACMemoryMappedFileGetPath(ACMemoryMappedFileRef mf)
{
	return mf->path;
}

CFIndex ACMemoryMappedFileGetSize(ACMemoryMappedFileRef mf)
{
	return mf->size;
}


bool ACMutableMemoryMappedFileSetSize(ACMutableMemoryMappedFileRef mmf, CFIndex newSize)
{
	switch (__ACMemoryMappedFileGetType(mmf))
	{
		case kACMemoryMappedFileImmutable:
			return false;
		case kACMemoryMappedFileMutable:
			;
			void * newBytes = realloc(mmf->bytes, newSize);
			if (!newBytes)
				return false;
			mmf->bytes = newBytes;
			mmf->size = newSize;
			return true;
		default:
			return false;
	}
}


uint8_t * ACMemoryMappedFileGetBytePtr(ACMemoryMappedFileRef mf)
{
	return mf->bytes;
}


