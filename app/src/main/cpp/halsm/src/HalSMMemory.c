#include <HalSM.h>
#include <stdio.h>

#define HEADER_SIZE sizeof(ui64)

void HalSMMemory_init(HalSMMemory* out,HalSMMemoryManagmentLibrary* mml) {
    out->free=0;
    out->size=0;
    out->table=(HalSMMemoryCell*)mml->malloc(0);
    out->mml=mml;
    out->firstFree=NOTHALSMNULLPOINTER;
}

void HalSMMemory_setCell(HalSMMemory* memory,ui64 nc,void* addr) {
    if(nc==HSMEM_ERR_ADDR)return;
    if(memory->table[nc].isFree==1) {
        HalSMMemoryCell* pr=memory->table[nc].prevFree;
        HalSMMemoryCell* nx=memory->table[nc].nextFree;
        if(pr!=NOTHALSMNULLPOINTER) {
            pr->nextFree=nx;
        }
        if(nx!=NOTHALSMNULLPOINTER) {
            nx->prevFree=pr;
        }
        if(memory->firstFree==&memory->table[nc]) {
            memory->firstFree=nx;
        }
    }
    //printf("MEMORY SET CELL: %p;%lld START\n",addr,nc);
    memory->table[nc]=(HalSMMemoryCell){
        .addr=addr,
        .isFree=0,
        .nextFree=NOTHALSMNULLPOINTER,
        .prevFree=NOTHALSMNULLPOINTER
    };
    
    //printf("MEMORY SET CELL: %p;%lld END\n",addr,nc);
}

ui64 HalSMMemory_setFreeCell(HalSMMemory* memory,void* addr) {
    if(memory->firstFree==NOTHALSMNULLPOINTER)return HSMEM_ERR_ADDR;
    ui64 i;
    HalSMMemoryCell* z;
    z=memory->firstFree;
    /*if(memory->firstFree==z) {
        if(z->nextFree!=NOTHALSMNULLPOINTER) {
            memory->firstFree=z->nextFree;
        } else {
            memory->firstFree=NOTHALSMNULLPOINTER;
        }
    }

    if(z->prevFree!=NOTHALSMNULLPOINTER&&z->nextFree!=NOTHALSMNULLPOINTER) {
        z->prevFree->nextFree=z->nextFree;
        z->nextFree->prevFree=z->prevFree;
    }*/
    memory->firstFree=z->nextFree;
    if(z->nextFree!=NOTHALSMNULLPOINTER) {
        z->nextFree->prevFree=NOTHALSMNULLPOINTER;
    }
    
    //printf("MEMORY SET FREECELL: %p START\n",addr);
    i=z-memory->table;
    //printf("Z: %lld;%p;%p;%d\n",i,z,memory->table,sizeof(HalSMMemoryCell));
    z->isFree=0;
    z->addr=addr;
    z->nextFree=NOTHALSMNULLPOINTER;
    z->prevFree=NOTHALSMNULLPOINTER;
    memory->free--;
    //printf("MEMORY SET FREECELL: %p END\n",addr);
    return i;
}

ui64 HalSMMemory_addNewCell(HalSMMemory* memory,void* addr) {
    ui64 i;
    if(memory->firstFree!=NOTHALSMNULLPOINTER) {
        return HalSMMemory_setFreeCell(memory,addr);;
    }
    //printf("MEMORY ADD CELL: %p START\n",addr);
    i=memory->size++;
    memory->table=memory->mml->realloc(memory->table,sizeof(HalSMMemoryCell)*memory->size);
    memory->table[i]=(HalSMMemoryCell){
        .addr=addr,
        .isFree=0,
        .nextFree=NOTHALSMNULLPOINTER,
        .prevFree=NOTHALSMNULLPOINTER
    };
    return i;
    //printf("MEMORY ADD CELL: %p END\n",addr);
}

ui64 HalSMMemory_getNCByAddr(HalSMMemory* memory,void* addr) {
    ui64 i;
    HalSMMemoryCell* p,cell;
    p=memory->table;
    for(i=0;i<memory->size;i++,p++) {
        cell=*p;
        if(cell.addr==addr)return i;
    }
    return HSMEM_ERR_ADDR;
}

void HalSMMemory_freeCell(HalSMMemory* memory,ui64 nc) {
    if(nc==HSMEM_ERR_ADDR)return;
    HalSMMemoryCell* z;
    z=&memory->table[nc];
    //printf("MEMORY FREE CELL: %lld;%lld START\n",nc,memory->size);
    z->addr=NOTHALSMNULLPOINTER;
    z->nextFree=NOTHALSMNULLPOINTER;
    z->isFree=1;
    memory->free++;
    
    if(memory->firstFree==NOTHALSMNULLPOINTER){
        z->prevFree=NOTHALSMNULLPOINTER;
        memory->firstFree=&memory->table[nc];
        return;
    }
    HalSMMemoryCell* cell=memory->firstFree;
    while(cell->nextFree!=NOTHALSMNULLPOINTER) {
        cell=cell->nextFree;
    }
    cell->nextFree=z;
    z->prevFree=cell;
    
    //printf("MEMORY FREE CELL: %lld END\n",nc);
}

void HalSMMemory_freeSys(HalSMMemory* memory) {
    ui64 i;
    HalSMMemoryCell* p,cell;
    p=memory->table;
    for(i=0;i<memory->size;i++,p++) {
        cell=*p;
        if(cell.isFree==0){
            // HalSMMemory_malloc returns the address after its ui64 bookkeeping
            // header. Free the actual allocation, not the shifted user pointer.
            memory->mml->free((void*)((char*)cell.addr-HEADER_SIZE));
        }
    }
    memory->mml->free(memory->table);
    memory->table=memory->mml->malloc(0);
    memory->size=0;
    memory->free=0;
    memory->firstFree=NOTHALSMNULLPOINTER;
}

void* HalSMMemory_malloc(HalSMMemory* memory,ui64 size) {
    void* raw=memory->mml->malloc(size+HEADER_SIZE);
    if(!raw)return NOTHALSMNULLPOINTER;
    //printf("malloc %lld:%p\n",size,raw);
    void* p=(void*)((char*)raw+HEADER_SIZE);

    *(ui64*)raw=HalSMMemory_addNewCell(memory,p);
    return p;
}

void* HalSMMemory_realloc(HalSMMemory* memory,void* p,ui64 newsize) {
    if(!p)return HalSMMemory_malloc(memory,newsize);
    void* raw=(void*)((char*)p - HEADER_SIZE);

    //ui64 nc=HalSMMemory_getNCByAddr(memory,p);
    ui64 nc=*((ui64*)raw);
    void* o=memory->mml->realloc(raw,newsize+HEADER_SIZE);
    if(!o)return NOTHALSMNULLPOINTER;
    //printf("realloc %lld:%p\n",newsize,o);
    void* newo=(void*)((char*)o+HEADER_SIZE);
    HalSMMemory_setCell(memory,nc,newo);
    *((ui64*)o)=nc;
    return newo;
}

void HalSMMemory_free(HalSMMemory* memory,void* p) {
    //printf("free %p\n",p);
    if(!p)return;
    void* raw=(void*)((char*)p-HEADER_SIZE);

    ui64 nc=*((ui64*)raw);
//HalSMMemory_getNCByAddr(memory,p)
    HalSMMemory_freeCell(memory,nc);
    memory->mml->free(raw);
}

void* HalSMMemory_calloc(HalSMMemory* memory,ui64 count,ui64 size) {
    void* raw=memory->mml->calloc(1,size*count+HEADER_SIZE);
    if(!raw)return NOTHALSMNULLPOINTER;
    //printf("calloc %lld;%lld:%p\n",count,size,raw);
    void* p=(void*)((char*)raw+HEADER_SIZE);

    *(ui64*)raw=HalSMMemory_addNewCell(memory,p);
    return p;
}
