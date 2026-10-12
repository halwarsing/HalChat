#include <HalFileSystem.h>

#define HFSNULL (void*)0

ui32 HalFileSystem_initWithCustomSize(HalFileSystem* hfs,ui64(*read)(void*,ui64,void*),ui64(*write)(void* buf,ui64 size,void* p),
    void(*seek)(void*,ui64,ui64),ui64(*tell)(void*),i32(*eof)(void*),void(*flush)(void*),void*(*malloc)(ui64),void*(*calloc)(ui64,ui64),
    void*(*realloc)(void*,ui64),void(*free)(void*),ui64 sizeDiskBlock,ui64 sizeDiskName,ui8* zeroDiskBlock,ui64 sizeFileDirectoryBlock,ui64 sizeFileDirectoryName,
    ui8* zeroFileDirectoryBlock,ui64 sizeDataBlock,ui8* dataBlock,ui8* additional,HalStringFormatChar* diskname) {
    ui64 i;

    if (sizeDiskBlock<HalFileSystemDiskSize||sizeFileDirectoryBlock<HalFileSystemFileSize||sizeFileDirectoryBlock<HalFileSystemDirectorySize){return 1;}

    hfs->sizeDiskBlock=sizeDiskBlock;
    hfs->sizeDiskName=sizeDiskName;
    i=hfs->sizeDiskName*sizeof(HalStringFormatChar)+HalFileSystemDiskSize;
    if (i>hfs->sizeDiskBlock){return 1;}
    hfs->sizeDiskAdditional=hfs->sizeDiskBlock-i;
    hfs->zeroDiskBlock=zeroDiskBlock;
    
    hfs->sizeFileDirectoryBlock=sizeFileDirectoryBlock;
    hfs->sizeFileDirectoryName=sizeFileDirectoryName;
    i=hfs->sizeFileDirectoryName*sizeof(HalStringFormatChar)+HalFileSystemFileSize;
    if (i>hfs->sizeFileDirectoryBlock){return 1;}
    hfs->sizeFileAdditional=hfs->sizeFileDirectoryBlock-i;
    i=hfs->sizeFileDirectoryName*sizeof(HalStringFormatChar)+HalFileSystemDirectorySize;
    if (i>hfs->sizeFileDirectoryBlock){return 1;}
    hfs->sizeDirectoryAdditional=hfs->sizeFileDirectoryBlock-i;
    hfs->zeroFileDirectoryBlock=zeroFileDirectoryBlock;

    hfs->read=read;
    hfs->write=write;
    hfs->seek=seek;
    hfs->tell=tell;
    hfs->eof=eof;
    hfs->flush=flush;

    hfs->malloc=malloc;
    hfs->calloc=calloc;
    hfs->realloc=realloc;
    hfs->free=free;

    hfs->version=1;

    hfs->maxsizememory=0;
    hfs->maxcountfiles=0;

    hfs->additional_information=additional;
    hfs->diskname=diskname;

    hfs->cursizememory=0;
    hfs->curcountfiles=0;
    hfs->ignore=0;

    hfs->startIndex=0;
    hfs->startIndexFiles=0;
    hfs->disk=HFSNULL;

    hfs->sizeDataBlock=sizeDataBlock;
    hfs->dataBlock=dataBlock;

    return 0;
}

ui32 HalFileSystem_init(HalFileSystem* hfs,ui64(*read)(void*,ui64,void*),ui64(*write)(void* buf,ui64 size,void* p),
    void(*seek)(void*,ui64,ui64),ui64(*tell)(void*),i32(*eof)(void*),void(*flush)(void*),void*(*malloc)(ui64),void*(*calloc)(ui64,ui64),
    void*(*realloc)(void*,ui64),void(*free)(void*))
{
    return HalFileSystem_initWithCustomSize(hfs,read,write,seek,tell,eof,flush,malloc,calloc,realloc,free,HalFileSystemDefaultBlockSize,HalFileSystemDefaultNameSize,(ui8*)calloc(1,HalFileSystemDefaultBlockSize),HalFileSystemDefaultBlockSize,HalFileSystemDefaultNameSize,(ui8*)calloc(1,HalFileSystemDefaultBlockSize),HalFileSystemDefaultBlockSize,(ui8*)calloc(1,HalFileSystemDefaultBlockSize),(ui8*)calloc(1,HalFileSystemDefaultAdditionalSize),(HalStringFormatChar*)calloc(HalFileSystemDefaultNameSize,sizeof(HalStringFormatChar)));
}

ui32 HalFileSystem_create(HalFileSystem* hfs,void* disk,HalStringFormatChar* diskname,ui64 maxsizememory,ui32 maxcountfiles,ui64 ignore,ui64 startIndex)
{
    ui64 disknamelen=HalStringFormat_length(diskname);
    ui64 i,l;
    if (disknamelen>hfs->sizeDiskName){return 1;}
    hfs->disk=disk;
    hfs->maxsizememory=maxsizememory;
    hfs->maxcountfiles=maxcountfiles;
    hfs->diskname=diskname;
    hfs->cursizememory=0;
    hfs->curcountfiles=0;
    hfs->ignore=ignore;
    hfs->startIndex=startIndex;
    hfs->startIndexFiles=hfs->startIndex+hfs->ignore+hfs->sizeDiskBlock;
    hfs->seek(hfs->disk,hfs->startIndex,0);
    hfs->write((char[]){'H','F','S'},3,hfs->disk);
    hfs->write(&hfs->version,4,hfs->disk);
    hfs->write(&hfs->maxsizememory,8,hfs->disk);
    hfs->write(&hfs->maxcountfiles,8,hfs->disk);
    hfs->write(&hfs->ignore,8,hfs->disk);
    hfs->write(&hfs->sizeDiskBlock,8,hfs->disk);
    hfs->write(&hfs->sizeDiskName,8,hfs->disk);
    hfs->write(&hfs->sizeFileDirectoryBlock,8,hfs->disk);
    hfs->write(&hfs->sizeFileDirectoryName,8,hfs->disk);
    hfs->write(hfs->zeroDiskBlock,hfs->sizeDiskAdditional,hfs->disk);
    hfs->write(hfs->diskname,disknamelen*sizeof(HalStringFormatChar),hfs->disk);
    hfs->write(hfs->zeroDiskBlock,(hfs->sizeDiskName-disknamelen)*sizeof(HalStringFormatChar),hfs->disk);
    for (i=0;i<hfs->sizeFileDirectoryBlock;i++){hfs->zeroFileDirectoryBlock[i]=0;}
    for (i=0,l=hfs->maxcountfiles;i<l;i++) {
        hfs->write(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
    }

    for (i=0,l=hfs->maxsizememory/hfs->sizeDiskBlock;i<l;i++) {
        hfs->write(hfs->zeroDiskBlock,hfs->sizeDiskBlock,hfs->disk);
    }

    hfs->write(hfs->zeroDiskBlock,hfs->maxsizememory%hfs->sizeDiskBlock,hfs->disk);
    return 0;
}

ui32 HalFileSystem_open(HalFileSystem* hfs,void* disk,ui64 startIndex)
{
    ui64 i;
    char indhfs[3]={0};
    hfs->disk=disk;
    hfs->startIndex=startIndex;
    hfs->seek(hfs->disk,hfs->startIndex,0);
    hfs->read(indhfs,3,hfs->disk);
    if (indhfs[0]!='H'||indhfs[1]!='F'||indhfs[2]!='S'){return 1;}
    hfs->read(&hfs->version,4,hfs->disk);
    hfs->read(&hfs->maxsizememory,8,hfs->disk);
    hfs->read(&hfs->maxcountfiles,8,hfs->disk);
    hfs->read(&hfs->ignore,8,hfs->disk);
    hfs->read(&hfs->sizeDiskBlock,8,hfs->disk);
    hfs->read(&hfs->sizeDiskName,8,hfs->disk);
    hfs->read(&hfs->sizeFileDirectoryBlock,8,hfs->disk);
    hfs->read(&hfs->sizeFileDirectoryName,8,hfs->disk);
    hfs->sizeDiskAdditional=hfs->sizeDiskBlock-hfs->sizeDiskName*sizeof(HalStringFormatChar)-HalFileSystemDiskSize;
    hfs->sizeFileAdditional=hfs->sizeFileDirectoryBlock-hfs->sizeFileDirectoryName*sizeof(HalStringFormatChar)-HalFileSystemFileSize;
    hfs->sizeDirectoryAdditional=hfs->sizeFileDirectoryBlock-hfs->sizeFileDirectoryName*sizeof(HalStringFormatChar)-HalFileSystemDirectorySize;
    hfs->startIndexFiles=hfs->startIndex+hfs->ignore+hfs->sizeDiskBlock;

    hfs->additional_information=(ui8*)hfs->malloc(hfs->sizeDiskAdditional);
    hfs->read(hfs->additional_information,hfs->sizeDiskAdditional,hfs->disk);
    hfs->diskname=(HalStringFormatChar*)hfs->malloc(hfs->sizeDiskName*sizeof(HalStringFormatChar));
    hfs->read(hfs->diskname,hfs->sizeDiskName*sizeof(HalStringFormatChar),hfs->disk);
    hfs->zeroDiskBlock=hfs->calloc(hfs->sizeDiskBlock,sizeof(ui8));
    hfs->zeroFileDirectoryBlock=hfs->calloc(hfs->sizeFileDirectoryBlock,sizeof(ui8));
    hfs->curcountfiles=HalFileSystem_getCurrentCountFiles(hfs);
    hfs->cursizememory=HalFileSystem_getCurrentSizeMemory(hfs);

    return 0;
}

ui8 HalFileSystem_getFreeIndex(HalFileSystem* hfs,ui64* indexOut)
{
    if (hfs->curcountfiles>=hfs->maxcountfiles){return 1;}
    ui64 i,j;
    i=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while(i<hfs->maxcountfiles) {
        for (j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (((hfs->zeroFileDirectoryBlock[0]>>3)&1)==0) {
            *indexOut=i;
            return 0;
        }
        i++;
    }
    return 1;
}

i64 HalFileSystem_indexStr(HalStringFormatChar* str,HalStringFormatChar f)
{
    ui64 i,l;
    l=HalStringFormat_length(str);
    for (i=0;i<l;i++) {
        if (HSFCharCompare(str[i],f)) {
            return i;
        }
    }
    return -1;
}

ui8 HalFileSystem_getIndexDirectoryByPath(HalFileSystem* hfs,ui64* indexOut,HalStringFormatChar* path,ui8 isNotDelete)
{
    if (HSFCompare(path,(HalStringFormatChar[]){{0,0,0}})||HSFCompare(path,(HalStringFormatChar[]){{0,47,0},{0,0,0}})){*indexOut=hfs_main_directory_index;return 0;}
    ui64 index=hfs_main_directory_index;
    i64 indStr;
    HalStringFormatChar* pp=path;
    HalStringFormatChar* pathName=hfs->calloc(hfs->sizeFileDirectoryName+1,sizeof(HalStringFormatChar));
    ui64 i,k,l;
    ui8 isF;
    while (1) {
        indStr=HalFileSystem_indexStr(pp,(HalStringFormatChar){0,47,0});
        if (indStr>-1) {
            if (indStr>hfs->sizeFileDirectoryName){return 1;}
            for (i=0;i<=hfs->sizeFileDirectoryName;i++){pathName[i]=(HalStringFormatChar){0,0,0};}
            for (i=0;i<indStr;i++){pathName[i]=*pp++;}
            pp++;
        } else {
            l=HalStringFormat_length(pp);
            if (l>hfs->sizeFileDirectoryName){return 1;}
            for (i=0;i<=hfs->sizeFileDirectoryName;i++){pathName[i]=(HalStringFormatChar){0,0,0};}
            for (i=0;i<l;i++){pathName[i]=*pp++;}
        }
        
        if (HSFCharCompare(pathName[0],(HalStringFormatChar){0,0,0})) {
            return 1;
        }

        isF=0;
        k=0;
        hfs->seek(hfs->disk,hfs->startIndexFiles,0);
        while (k<hfs->maxcountfiles) {
            for (i=0;i<hfs->sizeFileDirectoryBlock;i++){hfs->zeroFileDirectoryBlock[i]=0;}
            hfs->read(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
            if (((hfs->zeroFileDirectoryBlock[0]>>4)&1)==1) {
                if (indStr>-1) {
                    if (((hfs->zeroFileDirectoryBlock[0]>>3)&1)==1) {
                        if ((hfs->zeroFileDirectoryBlock[0]&1)==0&&HSFCompare(pathName,(HalStringFormatChar*)&hfs->zeroFileDirectoryBlock[hfs->sizeFileDirectoryBlock-hfs->sizeFileDirectoryName*sizeof(HalStringFormatChar)])&&(index==hfs_main_directory_index||index==*(ui64*)&hfs->zeroFileDirectoryBlock[1])) {
                            index=k;
                            isF=1;
                            break;
                        }
                    }
                } else {
                    if (((hfs->zeroFileDirectoryBlock[0]>>3)&1)==isNotDelete) {
                        if ((hfs->zeroFileDirectoryBlock[0]&1)==0) {
                            if (HSFCompare(pathName,(HalStringFormatChar*)&hfs->zeroFileDirectoryBlock[hfs->sizeFileDirectoryBlock-hfs->sizeFileDirectoryName*sizeof(HalStringFormatChar)])&&(index==hfs_main_directory_index||index==*(ui64*)&hfs->zeroFileDirectoryBlock[1])) {
                                *indexOut=k;
                                return 0;
                            }
                        }
                    }
                }
            }
            k++;
        }
        if (isF==0){break;}
    }
    return 1;
}

ui8 HalFileSystem_getIndexFileByPath(HalFileSystem* hfs,ui64* indexOut,HalStringFormatChar* path,ui8 isNotDelete)
{
    if (HSFCompare(path,(HalStringFormatChar[]){{0,0,0}})||HSFCompare(path,(HalStringFormatChar[]){{0,47,0},{0,0,0}})){return 1;}
    ui64 index=hfs_main_directory_index;
    i64 indStr;
    HalStringFormatChar* pp=path;
    HalStringFormatChar* pathName=hfs->calloc(hfs->sizeFileDirectoryName+1,sizeof(HalStringFormatChar));
    ui64 i,l,k;
    ui8 isF;
    while (1) {
        indStr=HalFileSystem_indexStr(pp,(HalStringFormatChar){0,47,0});
        if (indStr>-1) {
            if (indStr>hfs->sizeFileDirectoryName){return 1;}
            for (i=0;i<=hfs->sizeFileDirectoryName;i++){pathName[i]=(HalStringFormatChar){0,0,0};}
            for (i=0;i<indStr;i++){pathName[i]=*pp++;}
            pp++;
        } else {
            l=HalStringFormat_length(pp);
            if (l>hfs->sizeFileDirectoryName){return 1;}
            for (i=0;i<=hfs->sizeFileDirectoryName;i++){pathName[i]=(HalStringFormatChar){0,0,0};}
            for (i=0;i<l;i++){pathName[i]=*pp++;}
        }
        
        if (HSFCharCompare(pathName[0],(HalStringFormatChar){0,0,0})) {
            return 1;
        }

        isF=0;
        k=0;
        hfs->seek(hfs->disk,hfs->startIndexFiles,0);
        while (k<hfs->maxcountfiles) {
            for (i=0;i<hfs->sizeFileDirectoryBlock;i++){hfs->zeroFileDirectoryBlock[i]=0;}
            hfs->read(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
            if (((hfs->zeroFileDirectoryBlock[0]>>4)&1)==1) {
                if (indStr>-1) {
                    if (((hfs->zeroFileDirectoryBlock[0]>>3)&1)==1) {
                        if ((hfs->zeroFileDirectoryBlock[0]&1)==0&&HSFCompare(pathName,(HalStringFormatChar*)&hfs->zeroFileDirectoryBlock[hfs->sizeFileDirectoryBlock-hfs->sizeFileDirectoryName*sizeof(HalStringFormatChar)])&&(index==hfs_main_directory_index||index==*(ui64*)&hfs->zeroFileDirectoryBlock[1])) {
                            index=k;
                            isF=1;
                            break;
                        }
                    }
                } else {
                    if (((hfs->zeroFileDirectoryBlock[0]>>3)&1)==isNotDelete) {
                        if ((hfs->zeroFileDirectoryBlock[0]&1)==1) {
                            if (HSFCompare(pathName,(HalStringFormatChar*)&hfs->zeroFileDirectoryBlock[hfs->sizeFileDirectoryBlock-hfs->sizeFileDirectoryName*sizeof(HalStringFormatChar)])&&(index==hfs_main_directory_index||index==*(ui64*)&hfs->zeroFileDirectoryBlock[17])) {
                                *indexOut=k;
                                return 0;
                            }
                        }
                    }
                }
            }
            k++;
        }
        if (isF==0){break;}
    }
    return 1;
}

ui8 HalFileSystem_getFreeMemory(HalFileSystem* hfs,ui64 size,ui64* addr,ui8 isAllocate,ui64 ignore)
{
    /*ui64 start=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    ui64 i,j;
    ui8 temp[hfs_size_entry]={0};
    for (i=0;i<hfs->curcountfiles;i++) {
        for (j=0;j<hfs_size_entry;j++){temp[j]=0;}
        hfs->read(temp,hfs_size_entry,hfs->disk);
        if ((temp[0]&1)==0||((temp[0]>>3)&1)==1||(*(ui64*)&temp[9])==0){continue;}
        if ((*(ui64*)&temp[9])-start>=size){*addr=start;return 0;}
        start=(*(ui64*)&temp[9])+(*(ui64*)&temp[1]);
    }
    if (hfs->maxsizememory-start>=size) {
        *addr=start;
        return 0;
    }
    return 1;*/

    //Выделение памяти (полу-блочный метод (т. к. используются блоки файлов))
    //Проверяется все файлы и если есть удалённые, то вычитает из него нужную память и выделяет на новый файл
    //Если нет удалённых, то берёт максимальный адрес занятой памяти (если хватает места)
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    ui64 i,j,m;
    m=0;
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->zeroFileDirectoryBlock;

    for (i=0;i<hfs->maxcountfiles;i++) {
        for (j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (file->attributes.info.isFile==1) {
            if ((file->attributes.info.isNotDelete==0||file->attributes.info.isExist==0||i==ignore)&&size<=file->size) {
                if (isAllocate) {
                    hfs->seek(hfs->disk,hfs->startIndexFiles+i*hfs->sizeFileDirectoryBlock+9,0);
                    j=file->size-size;
                    hfs->write(&j,8,hfs->disk);
                }
                *addr=file->startAddress+j;
                return 0;
            }
            if (i!=ignore) {
                j=file->startAddress+file->size;
                if (j>m){m=j;}
            }
        }
    }
    if (size>hfs->maxsizememory-m) {
        return 1;
    }
    *addr=m;
    return 0;
}

ui32 HalFileSystem_addFileByIndex(HalFileSystem* hfs,ui64 path,HalStringFormatChar* name,ui8 mode,ui64 time,ui64* out)
{
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 index,i;
    if (path!=hfs_main_directory_index&&(HalFileSystem_getDirectoryByIndex(hfs,path,dir)>0||dir->attributes.info.isNotDelete==0)){return 1;}
    if (HalStringFormat_length(name)>hfs->sizeFileDirectoryName||HalStringFormat_length(name)==0){return 2;}
    if (HalFileSystem_getFileByNameAndPath(hfs,path,name,file)==0){return 3;}
    if (HalFileSystem_getFreeIndex(hfs,&index)){return 4;}
    hfs->curcountfiles++;
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
    for(i=0;i<hfs->sizeFileDirectoryBlock;i++){hfs->zeroFileDirectoryBlock[i]=0;}
    hfs->zeroFileDirectoryBlock[0]=1+((mode&1)<<1)+(((mode>>1)&1)<<2)+(1<<3)+(1<<4)+(((mode>>2)&1)<<5)+(((mode>>3)&1)<<6)+(((mode>>4)&1)<<7);
    hfs->write(hfs->zeroFileDirectoryBlock,1,hfs->disk);
    hfs->zeroFileDirectoryBlock[0]=0;
    hfs->write(hfs->zeroFileDirectoryBlock,16,hfs->disk);
    hfs->write(&path,8,hfs->disk);
    hfs->write(&time,8,hfs->disk);
    hfs->write(&time,8,hfs->disk);
    hfs->write(hfs->zeroFileDirectoryBlock,hfs->sizeFileAdditional,hfs->disk);
    hfs->write(name,HalStringFormat_length(name)*sizeof(HalStringFormatChar),hfs->disk);
    hfs->write(hfs->zeroFileDirectoryBlock,(hfs->sizeFileDirectoryName-HalStringFormat_length(name))*sizeof(HalStringFormatChar),hfs->disk);
    if (out!=HFSNULL){*out=index;}
    return 0;
}

ui32 HalFileSystem_addFileWithOut(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* name,ui8 mode,ui64 time,ui64* out)
{
    ui64 directoryIndex;
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&directoryIndex,path,1)){return 1;}
    return HalFileSystem_addFileByIndex(hfs,directoryIndex,name,mode,time,out);
}

ui32 HalFileSystem_addFile(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* name,ui8 mode,ui64 time)
{
    ui64 directoryIndex;
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&directoryIndex,path,1)){return 1;}
    return HalFileSystem_addFileByIndex(hfs,directoryIndex,name,mode,time,HFSNULL);
}

ui32 HalFileSystem_addDirectoryByIndex(HalFileSystem* hfs,ui64 path,HalStringFormatChar* name,ui8 mode,ui64 time,ui64* out)
{
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock),*oldDir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 index,i;
    if (path!=hfs_main_directory_index&&(HalFileSystem_getDirectoryByIndex(hfs,path,dir)>0||dir->attributes.info.isNotDelete==0)){return 1;}
    if (HalStringFormat_length(name)>hfs->sizeFileDirectoryName||HalStringFormat_length(name)==0){return 2;}
    if (HalFileSystem_getDirectoryByNameAndPath(hfs,path,name,oldDir)==0){return 3;}
    if (HalFileSystem_getFreeIndex(hfs,&index)){return 4;}
    hfs->curcountfiles++;
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
    for (i=0;i<hfs->sizeFileDirectoryBlock;i++){hfs->zeroFileDirectoryBlock[i]=0;}
    hfs->zeroFileDirectoryBlock[0]=0+((mode&1)<<1)+(((mode>>1)&1)<<2)+(1<<3)+(1<<4)+(((mode>>2)&1)<<5)+(((mode>>3)&1)<<6)+(((mode>>4)&1)<<7);
    hfs->write(hfs->zeroFileDirectoryBlock,1,hfs->disk);
    hfs->zeroFileDirectoryBlock[0]=0;
    hfs->write(&path,8,hfs->disk);
    hfs->write(&time,8,hfs->disk);
    hfs->write(&time,8,hfs->disk);
    hfs->write(hfs->zeroFileDirectoryBlock,hfs->sizeDirectoryAdditional,hfs->disk);
    hfs->write(name,HalStringFormat_length(name)*sizeof(HalStringFormatChar),hfs->disk);
    hfs->write(hfs->zeroFileDirectoryBlock,(hfs->sizeFileDirectoryName-HalStringFormat_length(name))*sizeof(HalStringFormatChar),hfs->disk);
    if (out!=HFSNULL){*out=index;}
    return 0;
}

ui32 HalFileSystem_addDirectoryWithOut(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* name,ui8 mode,ui64 time,ui64* out)
{
    ui64 directoryIndex;
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&directoryIndex,path,1)){return 1;}
    return HalFileSystem_addDirectoryByIndex(hfs,directoryIndex,name,mode,time,out);
}

ui32 HalFileSystem_addDirectory(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* name,ui8 mode,ui64 time)
{
    ui64 directoryIndex;
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&directoryIndex,path,1)){return 1;}
    return HalFileSystem_addDirectoryByIndex(hfs,directoryIndex,name,mode,time,HFSNULL);
}

ui32 HalFileSystem_writeFileByIndex(HalFileSystem* hfs,ui64 index,ui8* data,ui64 size,ui64 time)
{
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 freeMemory,freeMemoryAddress,indexData,indexEndOffsetSize;
    if (HalFileSystem_getFileByIndex(hfs,index,file)>0){return 1;}
    if (HalFileSystem_getFreeMemory(hfs,size,&freeMemory,1,index)) {
        freeMemory=0;
        size=0;
        hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock+1,0);
        hfs->write(&freeMemory,8,hfs->disk);
        hfs->write(&size,8,hfs->disk);
        hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock+25,0);
        hfs->write(&time,8,hfs->disk);
        return 2;
    }
    
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock+1,0);
    hfs->write(&freeMemory,8,hfs->disk);
    hfs->write(&size,8,hfs->disk);
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock+25,0);
    hfs->write(&time,8,hfs->disk);

    freeMemoryAddress=hfs->startIndexFiles+hfs->maxcountfiles*hfs->sizeFileDirectoryBlock+freeMemory;
    //freeMemoryOffsetSize=hfs_size_sector-freeMemoryAddress%hfs_size_sector;
    hfs->seek(hfs->disk,freeMemoryAddress,0);
    hfs->write(data,size,hfs->disk);
    hfs->cursizememory-=file->size;
    hfs->cursizememory+=size;
    return 0;
}
ui32 HalFileSystem_writeFile(HalFileSystem* hfs,HalStringFormatChar* path,ui8* data,ui64 size,ui64 time)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)>0){return 1;}
    return HalFileSystem_writeFileByIndex(hfs,index,data,size,time);
}

ui32 HalFileSystem_writeFileWithoutAllocateByIndex(HalFileSystem* hfs,ui64 index,ui8* data,ui64 offset,ui64 size,ui64 time)
{
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 freeMemory,freeMemoryAddress,indexData,sizec;
    if (HalFileSystem_getFileByIndex(hfs,index,file)>0){return 1;}
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock+1,0);
    hfs->read(&freeMemory,8,hfs->disk);
    hfs->read(&sizec,8,hfs->disk);
    hfs->read(&indexData,8,hfs->disk);
    hfs->write(&time,8,hfs->disk);
    if (size+offset>sizec){size=sizec;}
    else if (sizec==0){return 2;}

    freeMemoryAddress=hfs->startIndexFiles+hfs->maxcountfiles*hfs->sizeFileDirectoryBlock+freeMemory;
    hfs->seek(hfs->disk,freeMemoryAddress+offset,0);
    hfs->write(data,size,hfs->disk);
    //if (sizea>sizec){hfs->write(&data[size*count],siz-sizec,hfs->disk);}
    return 0;
}

ui32 HalFileSystem_writeFileWithoutAllocate(HalFileSystem* hfs,HalStringFormatChar* path,ui8* data,ui64 offset,ui64 size,ui64 time)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)>0){return 1;}
    return HalFileSystem_writeFileWithoutAllocateByIndex(hfs,index,data,offset,size,time);
}

ui32 HalFileSystem_writeFileWithReallocateByIndex(HalFileSystem* hfs,ui64 index,ui8* data,ui64 offset,ui64 size,ui64 time)
{
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 freeMemory,freeMemoryAddress,indexData,sizec,i,l,p0,p1;
    if (HalFileSystem_getFileByIndex(hfs,index,file)>0){return 1;}
    freeMemory=offset+size;
    if (freeMemory>file->size) {
        if (HalFileSystem_allocateMemoryByIndex(hfs,index,freeMemory,&freeMemoryAddress)>0){return 2;}
        if (file->size>0&&freeMemoryAddress!=file->startAddress) {
            l=file->size/hfs->sizeDataBlock;
            p0=hfs->startIndexFiles+hfs->maxcountfiles*hfs->sizeFileDirectoryBlock+file->startAddress;
            p1=hfs->startIndexFiles+hfs->maxcountfiles*hfs->sizeFileDirectoryBlock+freeMemoryAddress;
            for (i=0;i<l;i++,p0+=hfs->sizeDataBlock,p1+=hfs->sizeDataBlock) {
                hfs->seek(hfs->disk,p0,0);
                hfs->read(hfs->dataBlock,hfs->sizeDataBlock,hfs->disk);
                hfs->seek(hfs->disk,p1,0);
                hfs->write(hfs->dataBlock,hfs->sizeDataBlock,hfs->disk);
            }

            if (file->size%hfs->sizeDataBlock) {
                hfs->seek(hfs->disk,p0,0);
                hfs->read(hfs->dataBlock,file->size%hfs->sizeDataBlock,hfs->disk);
                hfs->seek(hfs->disk,p1,0);
                hfs->write(hfs->dataBlock,file->size%hfs->sizeDataBlock,hfs->disk);
            }
        }
    }
    return HalFileSystem_writeFileWithoutAllocateByIndex(hfs,index,data,offset,size,time);
}

ui32 HalFileSystem_writeFileWithReallocate(HalFileSystem* hfs,HalStringFormatChar* path,ui8* data,ui64 offset,ui64 size,ui64 time)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)>0){return 1;}
    return HalFileSystem_writeFileWithReallocateByIndex(hfs,index,data,offset,size,time);
}

ui32 HalFileSystem_readFileByIndex(HalFileSystem* hfs,ui64 index,ui8 *data,ui64 offset,ui64 size)
{
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 memory,memoryAddress,indexData,sizec;
    if (HalFileSystem_getFileByIndex(hfs,index,file)>0){return 1;}
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock+1,0);
    hfs->read(&memory,8,hfs->disk);
    hfs->read(&sizec,8,hfs->disk);
    hfs->read(&indexData,8,hfs->disk);
    if (offset>=sizec||sizec==0){return 0;}
    if (offset+size>sizec){size=sizec-offset;}
    hfs->seek(hfs->disk,hfs->startIndexFiles+hfs->maxcountfiles*hfs->sizeFileDirectoryBlock+memory+offset,0);
    hfs->read(data,size,hfs->disk);
    return 0;
}

ui32 HalFileSystem_readFile(HalFileSystem* hfs,HalStringFormatChar* path,ui8 *data,ui64 offset,ui64 size)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)){return 1;}
    return HalFileSystem_readFileByIndex(hfs,index,data,offset,size);
}

ui32 HalFileSystem_removeByIndex(HalFileSystem* hfs,ui64 index,ui64 time)
{
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    if (HalFileSystem_getFileByIndex(hfs,index,file)==0) {
        hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
        file->attributes.info.isNotDelete=0;
        file->attributes.info.isExist=0;
        file->last_edit_time=time;
        hfs->write(file,hfs->sizeFileDirectoryBlock,hfs->disk);
        hfs->curcountfiles--;
        hfs->cursizememory-=file->size;
        return 0;
    } else if (HalFileSystem_getDirectoryByIndex(hfs,index,dir)==0) {
        hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
        dir->attributes.info.isNotDelete=0;
        dir->attributes.info.isExist=0;
        dir->last_edit_time=time;
        hfs->write(dir,hfs->sizeFileDirectoryBlock,hfs->disk);
        hfs->curcountfiles=HalFileSystem_getCurrentCountFiles(hfs);
        hfs->cursizememory=HalFileSystem_getCurrentSizeMemory(hfs);
        return 0;
    }
    return 1;
}
ui32 HalFileSystem_recoveryByIndex(HalFileSystem* hfs,ui64 index,ui64 time)
{
    
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    if (HalFileSystem_getFileByIndex(hfs,index,file)==0) {
        hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
        file->attributes.info.isNotDelete=1;
        file->last_edit_time=time;
        hfs->write(file,hfs->sizeFileDirectoryBlock,hfs->disk);
        hfs->curcountfiles++;
        hfs->cursizememory=file->size;
        return 0;
    } else if (HalFileSystem_getDirectoryByIndex(hfs,index,dir)==0) {
        hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
        dir->attributes.info.isNotDelete=1;
        dir->last_edit_time=time;
        hfs->write(dir,hfs->sizeFileDirectoryBlock,hfs->disk);
        hfs->curcountfiles=HalFileSystem_getCurrentCountFiles(hfs);
        hfs->cursizememory=HalFileSystem_getCurrentSizeMemory(hfs);
        return 0;
    }
    return 1;
}

ui32 HalFileSystem_renameByIndex(HalFileSystem* hfs,ui64 index,HalStringFormatChar* newname,ui64 time)
{

    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    HalStringFormatChar* name;
    ui64 j,l;
    l=HalStringFormat_length(newname);
    if (l>hfs->sizeFileDirectoryName){return 2;}
    if (HalFileSystem_getFileByIndex(hfs,index,file)==0) {
        hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
        name=(HalStringFormatChar*)(((ui8*)file->name)+hfs->sizeFileAdditional);
        for (j=0;j<hfs->sizeFileDirectoryName;j++) {
            name[j]=(HalStringFormatChar){0,0,0};
        }
        for (j=0;j<l;j++) {
            name[j]=newname[j];
        }
        file->last_edit_time=time;
        hfs->write(file,hfs->sizeFileDirectoryBlock,hfs->disk);
        return 0;
    } else if (HalFileSystem_getDirectoryByIndex(hfs,index,dir)==0) {
        hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
        name=(HalStringFormatChar*)(((ui8*)dir->name)+hfs->sizeDirectoryAdditional);
        for (j=0;j<hfs->sizeFileDirectoryName;j++) {
            name[j]=(HalStringFormatChar){0,0,0};
        }
        for (j=0;j<l;j++) {
            name[j]=newname[j];
        }
        dir->last_edit_time=time;
        hfs->write(dir,hfs->sizeFileDirectoryBlock,hfs->disk);
        return 0;
    }
    return 1;
}

ui32 HalFileSystem_moveByIndex(HalFileSystem* hfs,ui64 index,ui64 newpath,ui64 time)
{
    
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    if (newpath!=hfs_main_directory_index&&(HalFileSystem_getDirectoryByIndex(hfs,newpath,dir)>0||dir->attributes.info.isNotDelete==0)){return 2;}
    if (HalFileSystem_getFileByIndex(hfs,index,file)==0) {
        hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
        file->directory_index=newpath;
        file->last_edit_time=time;
        hfs->write(file,hfs->sizeFileDirectoryBlock,hfs->disk);
        return 0;
    } else if (HalFileSystem_getDirectoryByIndex(hfs,index,dir)==0) {
        hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
        dir->directory_index=newpath;
        dir->last_edit_time=time;
        hfs->write(dir,hfs->sizeFileDirectoryBlock,hfs->disk);
        return 0;
    }
    return 1;
}

ui32 HalFileSystem_removeFile(HalFileSystem* hfs,HalStringFormatChar* path,ui64 time)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)>0){return 1;}
    return HalFileSystem_removeByIndex(hfs,index,time);
}

ui32 HalFileSystem_recoveryFile(HalFileSystem* hfs,HalStringFormatChar* path,ui64 time)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,0)>0){return 1;}
    return HalFileSystem_recoveryByIndex(hfs,index,time);
}

ui32 HalFileSystem_removeDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64 time)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&index,path,1)>0){return 1;}
    return HalFileSystem_removeByIndex(hfs,index,time);
}

ui32 HalFileSystem_recoveryDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64 time)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&index,path,0)>0){return 1;}
    return HalFileSystem_recoveryByIndex(hfs,index,time);
}

ui32 HalFileSystem_renameFile(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* newname,ui64 time)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)>0){return 1;}
    return HalFileSystem_renameByIndex(hfs,index,newname,time);
}

ui32 HalFileSystem_renameDirectory(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* newname,ui64 time)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&index,path,1)>0){return 1;}
    return HalFileSystem_renameByIndex(hfs,index,newname,time);
}

ui32 HalFileSystem_moveFile(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* newpath,ui64 time)
{
    ui64 index,newIndex;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)>0){return 1;}
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&newIndex,newpath,1)>0){return 3;}
    return HalFileSystem_moveByIndex(hfs,index,newIndex,time);
}

ui32 HalFileSystem_moveDirectory(HalFileSystem* hfs,HalStringFormatChar* path,HalStringFormatChar* newpath,ui64 time)
{
    ui64 index,newIndex;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&index,path,1)>0){return 1;}
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&newIndex,newpath,1)>0){return 3;}
    return HalFileSystem_moveByIndex(hfs,index,newIndex,time);
}

ui8 HalFileSystem_readAttributesFileByIndex(HalFileSystem* hfs,ui64 index,ui8* out)
{
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    if (HalFileSystem_getFileByIndex(hfs,index,file)>0){return 1;}
    *out=file->attributes.byte;
    return 0;
}

ui8 HalFileSystem_readAttributesFile(HalFileSystem* hfs,HalStringFormatChar* path,ui8* out)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)>0){return 1;}
    return HalFileSystem_readAttributesFileByIndex(hfs,index,out);
}

ui8 HalFileSystem_readAttributesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui8* out)
{
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    if (HalFileSystem_getDirectoryByIndex(hfs,index,dir)>0){return 1;}
    *out=dir->attributes.byte;
    return 0;
}

ui8 HalFileSystem_readAttributesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui8* out)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&index,path,1)>0){return 1;}
    return HalFileSystem_readAttributesDirectoryByIndex(hfs,index,out);
}

ui8 HalFileSystem_writeAttributesFileByIndex(HalFileSystem* hfs,ui64 index,ui8 attr,ui64 time)
{
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    if (HalFileSystem_getFileByIndex(hfs,index,file)>0){return 1;}
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
    file->attributes.byte=attr;
    file->last_edit_time=time;
    hfs->write(file,hfs->sizeFileDirectoryBlock,hfs->disk);
    return 0;
}

ui8 HalFileSystem_writeAttributesFile(HalFileSystem* hfs,HalStringFormatChar* path,ui8 attr,ui64 time)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)){return 1;}
    return HalFileSystem_writeAttributesFileByIndex(hfs,index,attr,time);
}

ui8 HalFileSystem_writeAttributesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui8 attr,ui64 time)
{
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    if (HalFileSystem_getDirectoryByIndex(hfs,index,dir)>0){return 1;}
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
    dir->attributes.byte=attr;
    dir->last_edit_time=time;
    hfs->write(dir,hfs->sizeFileDirectoryBlock,hfs->disk);
    return 0;
}

ui8 HalFileSystem_writeAttributesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui8 attr,ui64 time)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&index,path,1)){return 1;}
    return HalFileSystem_writeAttributesDirectoryByIndex(hfs,index,attr,time);
}

ui8 HalFileSystem_readAdditionalInformationFileByIndex(HalFileSystem* hfs,ui64 index,ui8* out)
{
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 j;
    ui8* p;
    if (HalFileSystem_getFileByIndex(hfs,index,file)>0){return 1;}
    p=(ui8*)file->name;
    for (j=0;j<hfs->sizeFileAdditional;j++){*out++=*p++;}
    return 0;
}

ui8 HalFileSystem_readAdditionalInformationFile(HalFileSystem* hfs,HalStringFormatChar* path,ui8* out)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)){return 1;}
    return HalFileSystem_readAdditionalInformationFileByIndex(hfs,index,out);
}


ui8 HalFileSystem_readAdditionalInformationDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui8* out)
{
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 j;
    ui8* p;
    if (HalFileSystem_getDirectoryByIndex(hfs,index,dir)>0){return 1;}
    p=(ui8*)dir->name;
    for (j=0;j<hfs->sizeDirectoryAdditional;j++){*out++=*p++;}
    return 0;
}

ui8 HalFileSystem_readAdditionalInformationDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui8* out)
{   
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)){return 1;}
    return HalFileSystem_readAdditionalInformationDirectoryByIndex(hfs,index,out);
}

ui8 HalFileSystem_writeAdditionalInformationFileByIndex(HalFileSystem* hfs,ui64 index,ui8* data,ui64 time)
{
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 j;
    ui8* p;
    if (HalFileSystem_getFileByIndex(hfs,index,file)>0){return 1;}
    p=(ui8*)file->name;
    for (j=0;j<hfs->sizeFileAdditional;j++){*p++=*data++;}
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
    hfs->write(file,hfs->sizeFileDirectoryBlock,hfs->disk);
    return 0;
}

ui8 HalFileSystem_writeAdditionalInformationFile(HalFileSystem* hfs,HalStringFormatChar* path,ui8* data,ui64 time)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)>0){return 1;}
    return HalFileSystem_writeAdditionalInformationFileByIndex(hfs,index,data,time);
}

ui8 HalFileSystem_writeAdditionalInformationDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui8* data,ui64 time)
{
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 j;
    ui8* p;
    if (HalFileSystem_getDirectoryByIndex(hfs,index,dir)>0){return 1;}
    p=(ui8*)dir->name;
    for (j=0;j<hfs->sizeDirectoryAdditional;j++){*p++=*data++;}
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
    hfs->write(dir,hfs->sizeFileDirectoryBlock,hfs->disk);
    return 0;
}

ui8 HalFileSystem_writeAdditionalInformationDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui8* data,ui64 time)
{   
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)>0){return 1;}
    return HalFileSystem_writeAdditionalInformationDirectoryByIndex(hfs,index,data,time);
}

ui8 HalFileSystem_getCountFilesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui64* out,ui8 isNotDelete)
{
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 i,j,o;
    if (index!=hfs_main_directory_index&&HalFileSystem_getDirectoryByIndex(hfs,index,dir)>0){return 1;}
    i=0;
    o=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while(i<hfs->maxcountfiles) {
        for (j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (((hfs->zeroFileDirectoryBlock[0]>>4)&1)==1&&((hfs->zeroFileDirectoryBlock[0]>>3)&1)==isNotDelete&&(hfs->zeroFileDirectoryBlock[0]&1)==1&&index==*(ui64*)&hfs->zeroFileDirectoryBlock[17]) {
            o++;
        }
        i++;
    }
    *out=o;
    return 0;
}

ui8 HalFileSystem_getCountFilesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out,ui8 isNotDelete)
{
    ui64 index;
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&index,path,isNotDelete)){return 1;}
    return HalFileSystem_getCountDirectoriesDirectoryByIndex(hfs,index,out,isNotDelete);
}

ui8 HalFileSystem_getCountDirectoriesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui64* out,ui8 isNotDelete)
{
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 i,j,o;
    if (index!=hfs_main_directory_index&&HalFileSystem_getDirectoryByIndex(hfs,index,dir)>0){return 1;}
    i=0;
    o=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while(i<hfs->maxcountfiles) {
        for (j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (((hfs->zeroFileDirectoryBlock[0]>>4)&1)==1&&((hfs->zeroFileDirectoryBlock[0]>>3)&1)==isNotDelete&&(hfs->zeroFileDirectoryBlock[0]&1)==0&&index==*(ui64*)&hfs->zeroFileDirectoryBlock[1]) {
            o++;
        }
        i++;
    }
    *out=o;
    return 0;
}

ui8 HalFileSystem_getCountDirectoriesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out,ui8 isNotDelete)
{
    ui64 index;
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&index,path,isNotDelete)){return 1;}
    return HalFileSystem_getCountDirectoriesDirectoryByIndex(hfs,index,out,isNotDelete);
}

ui8 HalFileSystem_getCountFilesAndDirectoriesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui64* out,ui8 isNotDelete)
{
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 i,j,o;
    if (index!=hfs_main_directory_index&&HalFileSystem_getDirectoryByIndex(hfs,index,dir)>0){return 1;}
    i=0;
    o=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while(i<hfs->maxcountfiles) {
        for (j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (((hfs->zeroFileDirectoryBlock[0]>>4)&1)==1&&((hfs->zeroFileDirectoryBlock[0]>>3)&1)==isNotDelete&&(((hfs->zeroFileDirectoryBlock[0]&1)==0&&index==*(ui64*)&hfs->zeroFileDirectoryBlock[1])||((hfs->zeroFileDirectoryBlock[0]&1)==1&&index==*(ui64*)&hfs->zeroFileDirectoryBlock[17]))) {
            o++;
        }
        i++;
    }
    *out=o;
    return 0;
}

ui8 HalFileSystem_getCountFilesAndDirectoriesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out,ui8 isNotDelete)
{
    ui64 index;
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&index,path,isNotDelete)){return 1;}
    return HalFileSystem_getCountFilesAndDirectoriesDirectoryByIndex(hfs,index,out,isNotDelete);
}

ui8 HalFileSystem_getListFilesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui64* out,ui64* count,ui64 maxout,ui8 isNotDelete)
{
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 i,j,o;
    if (maxout==0){*count=0;return 0;}
    if (index!=hfs_main_directory_index&&HalFileSystem_getDirectoryByIndex(hfs,index,dir)>0){return 1;}
    i=0;
    o=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while (i<hfs->maxcountfiles) {
        for (j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (((hfs->zeroFileDirectoryBlock[0]>>4)&1)==1&&((hfs->zeroFileDirectoryBlock[0]>>3)&1)==isNotDelete&&(hfs->zeroFileDirectoryBlock[0]&1)==1&&index==*(ui64*)&hfs->zeroFileDirectoryBlock[17]) {
            *out++=i;
            o++;
            if (o==maxout){*count=o;return 0;}
        }
        i++;
    }
    *count=o;
    return 0;
}

ui8 HalFileSystem_getListFilesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out,ui64* count,ui64 maxout,ui8 isNotDelete)
{
    ui64 index;
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&index,path,isNotDelete)){return 1;}
    return HalFileSystem_getListFilesDirectoryByIndex(hfs,index,out,count,maxout,isNotDelete);
}

ui8 HalFileSystem_getListDirectoriesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui64* out,ui64* count,ui64 maxout,ui8 isNotDelete)
{
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 i,j,o;
    if (maxout==0){*count=0;return 0;}
    if (index!=hfs_main_directory_index&&HalFileSystem_getDirectoryByIndex(hfs,index,dir)>0){return 1;}
    i=0;
    o=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while (i<hfs->maxcountfiles) {
        for (j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (((hfs->zeroFileDirectoryBlock[0]>>4)&1)==1&&((hfs->zeroFileDirectoryBlock[0]>>3)&1)==isNotDelete&&(hfs->zeroFileDirectoryBlock[0]&1)==0&&index==*(ui64*)&hfs->zeroFileDirectoryBlock[1]) {
            *out++=i;
            o++;
            if (o==maxout){*count=o;return 0;}
        }
        i++;
    }
    *count=o;
    return 0;
}

ui8 HalFileSystem_getListDirectoriesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out,ui64* count,ui64 maxout,ui8 isNotDelete)
{
    ui64 index;
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&index,path,isNotDelete)){return 1;}
    return HalFileSystem_getListDirectoriesDirectoryByIndex(hfs,index,out,count,maxout,isNotDelete);
}

ui8 HalFileSystem_getListFilesAndDirectoriesDirectoryByIndex(HalFileSystem* hfs,ui64 index,ui64* out,ui64* count,ui64 maxout,ui8 isNotDelete)
{
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 i,j,o;
    if (maxout==0){*count=0;return 0;}
    if (index!=hfs_main_directory_index&&HalFileSystem_getDirectoryByIndex(hfs,index,dir)>0){return 1;}
    i=0;
    o=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while (i<hfs->maxcountfiles) {
        for (j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (((hfs->zeroFileDirectoryBlock[0]>>4)&1)==1&&((hfs->zeroFileDirectoryBlock[0]>>3)&1)==isNotDelete&&(((hfs->zeroFileDirectoryBlock[0]&1)==0&&index==*(ui64*)&hfs->zeroFileDirectoryBlock[1])||((hfs->zeroFileDirectoryBlock[0]&1)==1&&index==*(ui64*)&hfs->zeroFileDirectoryBlock[17]))) {
            *out++=i;
            o++;
            if (o==maxout){*count=o;return 0;}
        }
        i++;
    }
    *count=o;
    return 0;
}

ui8 HalFileSystem_getListFilesAndDirectoriesDirectory(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out,ui64* count,ui64 maxout,ui8 isNotDelete)
{
    ui64 index;
    if (HalFileSystem_getIndexDirectoryByPath(hfs,&index,path,isNotDelete)){return 1;}
    return HalFileSystem_getListFilesAndDirectoriesDirectoryByIndex(hfs,index,out,count,maxout,isNotDelete);
}

void HalFileSystem_close(HalFileSystem* hfs)
{
    hfs->seek(hfs->disk,hfs->startIndex+3,0);
    hfs->write(&hfs->version,4,hfs->disk);
    hfs->write(&hfs->maxsizememory,8,hfs->disk);
    hfs->write(&hfs->maxcountfiles,8,hfs->disk);
    hfs->write(&hfs->ignore,8,hfs->disk);
    hfs->write(&hfs->sizeDiskBlock,8,hfs->disk);
    hfs->write(&hfs->sizeDiskName,8,hfs->disk);
    hfs->write(&hfs->sizeFileDirectoryBlock,8,hfs->disk);
    hfs->write(&hfs->sizeFileDirectoryName,8,hfs->disk);
    hfs->write(hfs->additional_information,hfs->sizeDiskAdditional,hfs->disk);
    hfs->write(hfs->diskname,HalStringFormat_length(hfs->diskname)*sizeof(HalStringFormatChar),hfs->disk);
    hfs->write(hfs->zeroFileDirectoryBlock,(hfs->sizeFileDirectoryName-HalStringFormat_length(hfs->diskname))*sizeof(HalStringFormatChar),hfs->disk);
    hfs->flush(hfs->disk);
    hfs->disk=HFSNULL;
}

ui64 HalFileSystem_getCurrentCountFiles(HalFileSystem* hfs)
{
    ui64 i,j,o;
    o=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    for (i=0;i<hfs->maxcountfiles;i++) {
        for(j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (((hfs->zeroFileDirectoryBlock[0]>>3)&1)==1){o++;}
    }
    return o;
}

ui64 HalFileSystem_getCurrentSizeMemory(HalFileSystem* hfs)
{
    
    ui64 i,j,o;
    o=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    for (i=0;i<hfs->maxcountfiles;i++) {
        for(j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(hfs->zeroFileDirectoryBlock,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (((hfs->zeroFileDirectoryBlock[0]>>3)&1)==1&&(hfs->zeroFileDirectoryBlock[0]&1)==1){o+=*(ui64*)&hfs->zeroFileDirectoryBlock[9];}
    }
    return o;
}

ui32 HalFileSystem_allocateMemoryByIndex(HalFileSystem* hfs,ui64 index,ui64 size,ui64* out)
{
    ui64 addr;
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    if (HalFileSystem_getFileByIndex(hfs,index,file)>0){return 1;}
    if (HalFileSystem_getFreeMemory(hfs,size,&addr,1,index)){return 2;}
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock+1,0);
    hfs->write(&addr,8,hfs->disk);
    hfs->write(&size,8,hfs->disk);
    hfs->cursizememory-=file->size;
    hfs->cursizememory+=size;
    if (out!=HFSNULL){*out=addr;}
    return 0;
}

ui32 HalFileSystem_allocateMemory(HalFileSystem* hfs,HalStringFormatChar* path,ui64 size,ui64* out)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)>0){return 1;}
    return HalFileSystem_allocateMemoryByIndex(hfs,index,size,out);
}

ui8 HalFileSystem_getSizeFileByIndex(HalFileSystem* hfs,ui64 index,ui64* out)
{
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    if (HalFileSystem_getFileByIndex(hfs,index,file)>0){return 1;}
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock+9,0);
    hfs->read(out,8,hfs->disk);
    return 0;
}

ui8 HalFileSystem_getSizeFile(HalFileSystem* hfs,HalStringFormatChar* path,ui64* out)
{
    ui64 index;
    if (HalStringFormat_length(path)==0){return 1;}
    if (HalFileSystem_getIndexFileByPath(hfs,&index,path,1)){return 1;}
    return HalFileSystem_getSizeFileByIndex(hfs,index,out);
}

ui8 HalFileSystem_getFileByIndex(HalFileSystem* hfs,ui64 index,HalFileSystemFile* out)
{
    if (index>hfs->maxcountfiles){return 1;}
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
    hfs->read(out,hfs->sizeFileDirectoryBlock,hfs->disk);
    if (out->attributes.info.isExist==0||out->attributes.info.isFile==0) {
        return 1;
    }
    return 0;
}

ui8 HalFileSystem_getDirectoryByIndex(HalFileSystem* hfs,ui64 index,HalFileSystemDirectory* out)
{
    if (index>hfs->maxcountfiles){return 1;}
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
    hfs->read(out,hfs->sizeFileDirectoryBlock,hfs->disk);
    if (out->attributes.info.isExist==0||out->attributes.info.isFile==1) {
        return 1;
    }
    return 0;
}

ui8 HalFileSystem_getFileOrDirectoryByIndex(HalFileSystem* hfs,ui64 index,void* out)
{
    if (index>hfs->maxcountfiles){return 1;}
    hfs->seek(hfs->disk,hfs->startIndexFiles+index*hfs->sizeFileDirectoryBlock,0);
    hfs->read(out,hfs->sizeFileDirectoryBlock,hfs->disk);
    ui8* temp=(ui8*)out;
    if (((temp[0]>>4)&1)==0) {
        return 1;
    }
    return 0;
}

ui8 HalFileSystem_getNotDeletedFileByIndex(HalFileSystem* hfs,ui64 index,HalFileSystemFile* out,ui64* outIndex)
{
    if (index>hfs->maxcountfiles){return 1;}
    HalFileSystemFile* file=(HalFileSystemFile*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 i,j,o;
    i=0;
    o=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while (i<hfs->maxcountfiles) {
        for (j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(file,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (file->attributes.info.isNotDelete==1&&file->attributes.info.isExist==1) {
            if (o==index) {
                if (file->attributes.info.isFile==0) {
                    return 2;
                }
                if(out!=HFSNULL){*out=*file;}
                *outIndex=i;
                return 0;
            }
            o++;
        }
        i++;
    }
    return 3;
}

ui8 HalFileSystem_getNotDeletedDirectoryByIndex(HalFileSystem* hfs,ui64 index,HalFileSystemDirectory* out,ui64* outIndex)
{
    if (index>hfs->maxcountfiles){return 1;}
    HalFileSystemDirectory* dir=(HalFileSystemDirectory*)hfs->calloc(1,hfs->sizeFileDirectoryBlock);
    ui64 i,j,o;
    i=0;
    o=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while (i<hfs->maxcountfiles) {
        for (j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(dir,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (dir->attributes.info.isNotDelete==1&&dir->attributes.info.isExist==1) {
            if (o==index) {
                if (dir->attributes.info.isFile==1) {
                    return 2;
                }
                if(out!=HFSNULL){*out=*dir;}
                *outIndex=i;
                return 0;
            }
            o++;
        }
        i++;
    }
    return 3;
}

ui8 HalFileSystem_getNotDeletedFileOrDirectoryByIndex(HalFileSystem* hfs,ui64 index,void* out,ui64* outIndex)
{
    if (index>hfs->maxcountfiles){return 1;}
    HalFileSystemFile* file=(HalFileSystemFile*)out;
    ui64 i,j,o;
    i=0;
    o=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while (i<hfs->maxcountfiles) {
        for (j=0;j<hfs->sizeFileDirectoryBlock;j++){hfs->zeroFileDirectoryBlock[j]=0;}
        hfs->read(file,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (file->attributes.info.isNotDelete==1&&file->attributes.info.isExist==1) {
            if (o==index) {
                *outIndex=i;
                return 0;
            }
            o++;
        }
        i++;
    }
    return 3;
}

ui8 HalFileSystem_getFileByNameAndPath(HalFileSystem* hfs,ui64 path,HalStringFormatChar* name,HalFileSystemFile* out)
{
    ui64 i;
    i=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while(i<hfs->maxcountfiles) {
        hfs->read(out,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (out->attributes.info.isExist==1&&out->attributes.info.isFile==1&&HSFCompare((HalStringFormatChar*)(((ui8*)out)+(hfs->sizeFileDirectoryBlock-hfs->sizeFileDirectoryName*sizeof(HalStringFormatChar))),name)) {
            return 0;
        }
        i++;
    }
    return 1;
}

ui8 HalFileSystem_getDirectoryByNameAndPath(HalFileSystem* hfs,ui64 path,HalStringFormatChar* name,HalFileSystemDirectory* out)
{
    ui64 i;
    i=0;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while(i<hfs->maxcountfiles) {
        hfs->read(out,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (out->attributes.info.isExist==1&&out->attributes.info.isFile==0&&HSFCompare((HalStringFormatChar*)(((ui8*)out)+(hfs->sizeFileDirectoryBlock-hfs->sizeFileDirectoryName*sizeof(HalStringFormatChar))),name)) {
            return 0;
        }
        i++;
    }
    return 1;

}

ui8 HalFileSystem_getFileOrDirectoryByNameAndPath(HalFileSystem* hfs,ui64 path,HalStringFormatChar* name,void* out)
{
    ui64 i;
    i=0;
    ui8* temp=(ui8*)out;
    hfs->seek(hfs->disk,hfs->startIndexFiles,0);
    while(i<hfs->maxcountfiles) {
        hfs->read(temp,hfs->sizeFileDirectoryBlock,hfs->disk);
        if (((temp[0]>>4)&1)==1&&HSFCompare((HalStringFormatChar*)&temp[hfs->sizeFileDirectoryBlock-hfs->sizeFileDirectoryName*sizeof(HalStringFormatChar)],name)) {
            return 0;
        }
        i++;
    }
    return 1;
}

HalStringFormatChar* HalFileSystem_getFileName(HalFileSystem* hfs,HalFileSystemFile* file)
{
    return (HalStringFormatChar*)(((ui8*)file->name)+hfs->sizeFileAdditional);
}

HalStringFormatChar* HalFileSystem_getDirectoryName(HalFileSystem* hfs,HalFileSystemDirectory* dir)
{
    return (HalStringFormatChar*)(((ui8*)dir->name)+hfs->sizeFileAdditional);
}