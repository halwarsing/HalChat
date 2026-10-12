#ifndef HALSMEXECUTABLE_H
#define HALSMEXECUTABLE_H

#include <HalSM.h>
#include <HalFileSystem.h>

typedef struct HalSMExecutable {
    HalFileSystem* hfs;
    unsigned char version[3];
    HalStringFormatChar* mainFile;
} HalSMExecutable;

void HalSMExecutable_init(HalSMExecutable* hsme,HalFileSystem* hfs);
unsigned int HalSMExecutable_load(HalSMExecutable* hsme,HalSMCompiler* hsmc);
void HalSMExecutable_run(HalSMExecutable* hsme,HalSMCompiler* hsmc);

#endif