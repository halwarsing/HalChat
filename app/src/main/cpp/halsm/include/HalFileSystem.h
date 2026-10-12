#ifndef HALFILESYSTEM_H
#define HALFILESYSTEM_H

#include <HalStringFormat.h>
#include <HalDefines.h>

/*
INDEFICATION HFS (Halwarsing File System) WITHOUT BOOTLOADER
HFS (3 bytes) - indefication of HFS
int 4 bytes - version HFS
long 8 bytes - max size memory
long 8 bytes - max count files and directories
long 8 bytes - ignore
long 8 bytes - sizeDiskBlock
long 8 bytes - sizeDiskName
long 8 bytes - sizeFileDirectoryBlock
long 8 bytes - sizeFileDirectoryName
3033 bytes - not used
1000 bytes - name of disk HalStringFormat (4 bytes for one symbol - 250 symbols)
Total: 4096 bytes
*/

/*
INDEFICATION FILE
1 byte (bits information) - Indefication file
    1. is file
    2. addition parameter
    3. is read only
    4. is not delete
    5. is exist
    6-8. additional parameters
long 8 bytes - start address
long 8 bytes - current size bytes of used memory
long 8 bytes - directory index
long 8 bytes - last edit time
long 8 bytes - created time
3055 bytes - not used
1000 bytes - name of file HalStringFormat (4 bytes for one symbol - 250 symbols)
Total 4096 bytes
*/

/*
INDEFICATION DIRECTORY
1 byte (bits information) - Indefication directory
    1. is file
    2. addition parameter
    3. is read only
    4. is not delete
    5. is exist
    6-8. additional parameters
long 8 bytes - directory index
long 8 bytes - last edit time
long 8 bytes - created time
3071 bytes - not used
1000 bytes - name of directory HalStringFormat (4 bytes for one symbol - 250 symbols)
Total 4096 bytes
*/

#define hfs_main_directory_index 0xffffffffffffffffULL

struct HalFileSystem {
    ui64(*read)(void*,ui64,void*);
    ui64(*write)(void* buf,ui64 size,void* p);
    void(*seek)(void*,ui64,ui64);
    ui64(*tell)(void*);
    i32(*eof)(void*);
    void(*flush)(void*);
    void*(*malloc)(ui64);
    void*(*calloc)(ui64,ui64);
    void*(*realloc)(void*,ui64);
    void(*free)(void*);
    ui32 version;
    ui64 maxsizememory;
    ui64 maxcountfiles;
    HalStringFormatChar* diskname;
    ui64 cursizememory;
    ui64 curcountfiles;
    ui64 ignore;
    ui64 startIndex;
    ui64 startIndexFiles;
    void* disk;
    ui8* additional_information;
    ui64 sizeDiskBlock;
    ui64 sizeDiskName;
    ui64 sizeDiskAdditional;
    ui64 sizeFileDirectoryBlock;
    ui64 sizeFileDirectoryName;
    ui64 sizeFileAdditional;
    ui64 sizeDirectoryAdditional;
    ui64 sizeDataBlock;
    ui8* zeroDiskBlock;
    ui8* zeroFileDirectoryBlock;
    ui8* dataBlock;
} __attribute__((__packed__));

typedef struct HalFileSystem HalFileSystem;

struct HalFileSystemFile {
    union {
        struct {
            ui8 isFile:1;
            ui8 addition_parameter:1;
            ui8 isReadOnly:1;
            ui8 isNotDelete:1;
            ui8 isExist:1;
            ui8 additional_parameters:3;
        } info;
        ui8 byte;
    } attributes;
    ui64 startAddress;
    ui64 size;
    ui64 directory_index;
    ui64 last_edit_time;
    ui64 created_time;
    HalStringFormatChar name[];
} __attribute__((__packed__));

typedef struct HalFileSystemFile HalFileSystemFile;

struct HalFileSystemDirectory {
    union {
        struct {
            ui8 isFile:1;
            ui8 addition_parameter:1;
            ui8 isReadOnly:1;
            ui8 isNotDelete:1;
            ui8 isExist:1;
            ui8 additional_parameters:3;
        } info;
        ui8 byte;
    } attributes;
    ui64 directory_index;
    ui64 last_edit_time;
    ui64 created_time;
    HalStringFormatChar name[];
} __attribute__((__packed__));

typedef struct HalFileSystemDirectory HalFileSystemDirectory;

#define HalFileSystemDiskSize 63U
#define HalFileSystemFileSize 41U
#define HalFileSystemDirectorySize 25U
#define HalFileSystemFileDirectorySize (HalFileSystemFileSize>HalFileSystemDirectorySize?HalFileSystemFileSize:HalFileSystemDirectorySize)

#define HalFileSystemDefaultBlockSize 4096U
#define HalFileSystemDefaultNameSize 250U
#define HalFileSystemDefaultAdditionalSize (HalFileSystemDefaultBlockSize-HalFileSystemDefaultNameSize-HalFileSystemDiskSize)

ui32 HalFileSystem_initWithCustomSize(HalFileSystem* hfs,ui64(*read)(void*,ui64,void*),ui64(*write)(void* buf,ui64 size,void* p),
    void(*seek)(void*,ui64,ui64),ui64(*tell)(void*),i32(*eof)(void*),void(*flush)(void*),void*(*malloc)(ui64),void*(*calloc)(ui64,ui64),
    void*(*realloc)(void*,ui64),void(*free)(void*),ui64 sizeDiskBlock,ui64 sizeDiskName,ui8* zeroDiskBlock,ui64 sizeFileDirectoryBlock,ui64 sizeFileDirectoryName,
    ui8* zeroFileDirectoryBlock,ui64 sizeDataBlock,ui8* dataBlock,ui8* additional,HalStringFormatChar* diskname);

ui32 HalFileSystem_init(HalFileSystem* hfs,ui64(*read)(void*,ui64,void*),ui64(*write)(void* buf,ui64 size,void* p),
    void(*seek)(void*,ui64,ui64),ui64(*tell)(void*),i32(*eof)(void*),void(*flush)(void*),void*(*malloc)(ui64),void*(*calloc)(ui64,ui64),
    void*(*realloc)(void*,ui64),void(*free)(void*));

ui32 HalFileSystem_create(HalFileSystem* hfs,void* disk,HalStringFormatChar* diskname,ui64 maxsizememory,ui32 maxcountfiles,ui64 ignore,ui64 startIndex);

ui32 HalFileSystem_open(HalFileSystem* hfs,void* disk,ui64 startIndex);

ui8 HalFileSystem_getFreeIndex(HalFileSystem* hfs,ui64* indexOut);
i64 HalFileSystem_indexStr(HalStringFormatChar* str,HalStringFormatChar f);

ui8 HalFileSystem_getIndexDirectoryByPath(HalFileSystem* hfs,ui64* indexOut,HalStringFormatChar* path,ui8 isNotDelete);
ui8 HalFileSystem_getIndexFileByPath(HalFileSystem* hfs,ui64* indexOut,HalStringFormatChar* path,ui8 isNotDelete);

ui8 HalFileSystem_getFreeMemory(HalFileSystem* hfs,ui64 size,ui64* addr,ui8 isAllocate,ui64 ignore);

ui32 HalFileSystem_addFileByIndex(HalFileSystem* hfs,ui64 index,HalStringFormatChar* name,ui8 mode,ui64 time,ui64* out);
ui32 HalFileSystem_addFileWithOut(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* name,ui8 mode,ui64 time,ui64* out);
ui32 HalFileSystem_addFile(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* name,ui8 mode,ui64 time);

ui32 HalFileSystem_addDirectoryByIndex(HalFileSystem* hfs,ui64 index,HalStringFormatChar* name,ui8 mode,ui64 time,ui64* out);
ui32 HalFileSystem_addDirectoryWithOut(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* name,ui8 mode,ui64 time,ui64* out);
ui32 HalFileSystem_addDirectory(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* name,ui8 mode,ui64 time);

ui32 HalFileSystem_writeFileByIndex(HalFileSystem* hfs,ui64 index,ui8* data,ui64 size,ui64 time);
ui32 HalFileSystem_writeFile(HalFileSystem* hfs,HalStringFormatChar* path,ui8* data,ui64 size,ui64 time);

ui32 HalFileSystem_writeFileWithoutAllocateByIndex(HalFileSystem* hfs,ui64 index,ui8* data,ui64 offset,ui64 size,ui64 time);
ui32 HalFileSystem_writeFileWithoutAllocate(HalFileSystem* hfs,HalStringFormatChar* path,ui8* data,ui64 offset,ui64 size,ui64 time);

ui32 HalFileSystem_writeFileWithReallocateByIndex(HalFileSystem* hfs,ui64 index,ui8* data,ui64 offset,ui64 size,ui64 time);
ui32 HalFileSystem_writeFileWithReallocate(HalFileSystem* hfs,HalStringFormatChar* path,ui8* data,ui64 offset,ui64 size,ui64 time);

ui32 HalFileSystem_readFileByIndex(HalFileSystem* hfs,ui64 index,ui8 *data,ui64 offset,ui64 size);
ui32 HalFileSystem_readFile(HalFileSystem* hfs,HalStringFormatChar* path,ui8 *data,ui64 offset,ui64 size);

ui32 HalFileSystem_removeByIndex(HalFileSystem* hfs,ui64 index,ui64 time);
ui32 HalFileSystem_recoveryByIndex(HalFileSystem* hfs,ui64 index,ui64 time);
ui32 HalFileSystem_renameByIndex(HalFileSystem* hfs,ui64 index,HalStringFormatChar* newname,ui64 time);
ui32 HalFileSystem_moveByIndex(HalFileSystem* hfs,ui64 index,ui64 newpath,ui64 time);

ui32 HalFileSystem_removeFile(HalFileSystem* hfs,HalStringFormatChar* path,ui64 time);
ui32 HalFileSystem_recoveryFile(HalFileSystem* hfs,HalStringFormatChar* path,ui64 time);
ui32 HalFileSystem_removeDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64 time);
ui32 HalFileSystem_recoveryDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64 time);
ui32 HalFileSystem_renameFile(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* newname,ui64 time);
ui32 HalFileSystem_renameDirectory(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* newname,ui64 time);
ui32 HalFileSystem_moveFile(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* newpath,ui64 time);
ui32 HalFileSystem_moveDirectory(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* newpath,ui64 time);

ui8 HalFileSystem_readAttributesFileByIndex(HalFileSystem* hfs,ui64 index,ui8* out);
ui8 HalFileSystem_readAttributesFile(HalFileSystem* hfs,HalStringFormatChar* path,ui8* out);

ui8 HalFileSystem_readAttributesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui8* out);
ui8 HalFileSystem_readAttributesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui8* out);

ui8 HalFileSystem_writeAttributesFileByIndex(HalFileSystem* hfs,ui64 index,ui8 attr,ui64 time);
ui8 HalFileSystem_writeAttributesFile(HalFileSystem* hfs,HalStringFormatChar* path,ui8 attr,ui64 time);

ui8 HalFileSystem_writeAttributesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui8 attr,ui64 time);
ui8 HalFileSystem_writeAttributesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui8 attr,ui64 time);

ui8 HalFileSystem_readAdditionalInformationFileByIndex(HalFileSystem* hfs,ui64 index,ui8* out);
ui8 HalFileSystem_readAdditionalInformationFile(HalFileSystem* hfs,HalStringFormatChar* path,ui8* out);

ui8 HalFileSystem_readAdditionalInformationDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui8* out);
ui8 HalFileSystem_readAdditionalInformationDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui8* out);

ui8 HalFileSystem_writeAdditionalInformationFileByIndex(HalFileSystem* hfs,ui64 index,ui8* data,ui64 time);
ui8 HalFileSystem_writeAdditionalInformationFile(HalFileSystem* hfs,HalStringFormatChar* path,ui8* data,ui64 time);

ui8 HalFileSystem_writeAdditionalInformationDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui8* data,ui64 time);
ui8 HalFileSystem_writeAdditionalInformationDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui8* data,ui64 time);

ui8 HalFileSystem_getCountFilesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui64* out,ui8 isNotDelete);
ui8 HalFileSystem_getCountFilesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out,ui8 isNotDelete);

ui8 HalFileSystem_getCountDirectoriesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui64* out,ui8 isNotDelete);
ui8 HalFileSystem_getCountDirectoriesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out,ui8 isNotDelete);

ui8 HalFileSystem_getCountFilesAndDirectoriesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui64* out,ui8 isNotDelete);
ui8 HalFileSystem_getCountFilesAndDirectoriesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out,ui8 isNotDelete);

ui8 HalFileSystem_getListFilesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui64* out,ui64* count,ui64 maxout,ui8 isNotDelete);
ui8 HalFileSystem_getListFilesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out,ui64* count,ui64 maxout,ui8 isNotDelete);

ui8 HalFileSystem_getListDirectoriesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui64* out,ui64* count,ui64 maxout,ui8 isNotDelete);
ui8 HalFileSystem_getListDirectoriesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out,ui64* count,ui64 maxout,ui8 isNotDelete);

ui8 HalFileSystem_getListFilesAndDirectoriesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui64* out,ui64* count,ui64 maxout,ui8 isNotDelete);
ui8 HalFileSystem_getListFilesAndDirectoriesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out,ui64* count,ui64 maxout,ui8 isNotDelete);

void HalFileSystem_close(HalFileSystem* hfs);
ui64 HalFileSystem_getCurrentCountFiles(HalFileSystem* hfs);
ui64 HalFileSystem_getCurrentSizeMemory(HalFileSystem* hfs);
ui32 HalFileSystem_allocateMemoryByIndex(HalFileSystem* hfs,ui64 index,ui64 size,ui64* out);
ui32 HalFileSystem_allocateMemory(HalFileSystem* hfs,HalStringFormatChar* path,ui64 size,ui64* out);
ui8 HalFileSystem_getSizeFileByIndex(HalFileSystem* hfs,ui64 index,ui64* out);
ui8 HalFileSystem_getSizeFile(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out);

ui8 HalFileSystem_getFileByIndex(HalFileSystem* hfs,ui64 index,HalFileSystemFile* out);
ui8 HalFileSystem_getDirectoryByIndex(HalFileSystem* hfs,ui64 index,HalFileSystemDirectory* out);
ui8 HalFileSystem_getFileOrDirectoryByIndex(HalFileSystem* hfs,ui64 index,void* out);

ui8 HalFileSystem_getNotDeletedFileByIndex(HalFileSystem* hfs,ui64 index,HalFileSystemFile* out,ui64* outIndex);
ui8 HalFileSystem_getNotDeletedDirectoryByIndex(HalFileSystem* hfs,ui64 index,HalFileSystemDirectory* out,ui64* outIndex);
ui8 HalFileSystem_getNotDeletedFileOrDirectoryByIndex(HalFileSystem* hfs,ui64 index,void* out,ui64* outIndex);

ui8 HalFileSystem_getFileByNameAndPath(HalFileSystem* hfs,ui64 path,HalStringFormatChar* name,HalFileSystemFile* out);
ui8 HalFileSystem_getDirectoryByNameAndPath(HalFileSystem* hfs,ui64 path,HalStringFormatChar* name,HalFileSystemDirectory* out);
ui8 HalFileSystem_getFileOrDirectoryByNameAndPath(HalFileSystem* hfs,ui64 path,HalStringFormatChar* name,void* out);

HalStringFormatChar* HalFileSystem_getFileName(HalFileSystem* hfs,HalFileSystemFile* file);
HalStringFormatChar* HalFileSystem_getDirectoryName(HalFileSystem* hfs,HalFileSystemDirectory* dir);

#endif