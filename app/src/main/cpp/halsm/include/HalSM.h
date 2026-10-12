#ifndef HALSM_H
#define HALSM_H

#ifdef __cplusplus
extern "C" {
#endif

/*0.1.8
Новая система выполнения функции предусматривает приведение всех строк кода в формат инструкции.
И взависимости от условий внутри класса или функции будет сохраняться в тело функций или сразу исполняться.
Это поможет не только стандартизировать выполнение, тем самым помочь в управлении и улучшении кода, но и также ускорение выполнения и возможность описание кода в виде бинарного файла, что делает его более статичным и исполняемым.
Для этого будет добавлены две функции:
1) HalSMCompiler_executeOP(OP)
2) HalSMCompiler_getOP(String text)
Новая структура -> HalSMOP -> HalSM Operation

Смысл ещё в операциях в том что все аргументы также будут превращены в инструкции и тем самым позволяя убрать сильную непредсказуемость кода, но останется всё тоже самое что до этого, просто станет лучше производительность.

Введена полная система отслеживания ошибок:
1) Вывод строки
2) Вывод номера строки из исходного кода
*/

#include <HalStringFormat.h>
#include <HalDefines.h>

#define NOTHALSMNULLPOINTER ((void*)0)


typedef struct HalSMNull {unsigned char c;} HalSMNull;

typedef struct HalSMErrorIndificator {
    HalStringFormatChar* name;
    ui64 line;
    void* prev;
} HalSMErrorIndificator;

typedef struct HalSMError {
    HalSMErrorIndificator* ind;
    HalStringFormatChar* error;
} HalSMError;

typedef enum HalSMSystemFunction {
    HalSMSystemFunction_input,
    HalSMSystemFunction_reversed,
    HalSMSystemFunction_range,
    HalSMSystemFunction_print,
    HalSMSystemFunction_exit,
    HalSMSystemFunction_for,
    HalSMSystemFunction_while,
    HalSMSystemFunction_if,
    HalSMSystemFunction_elif,
    HalSMSystemFunction_else,
    HalSMSystemFunction_increment,
    HalSMSystemFunction_decrement,
    HalSMSystemFunction_do,
    HalSMSystemFunction_replace,
    HalSMSystemFunction_index,
    HalSMSystemFunction_rindex,
    HalSMSystemFunction_split,
    HalSMSystemFunction_substring,

    //ARRAY
    HalSMSystemFunction_append,
    HalSMSystemFunction_len,
    HalSMSystemFunction_clear,
    HalSMSystemFunction_insert,
    HalSMSystemFunction_pop,
    HalSMSystemFunction_remove,
    HalSMSystemFunction_reverse,
    HalSMSystemFunction_copy,
    HalSMSystemFunction_appendArray,
    HalSMSystemFunction_slice,

    //Dict
    HalSMSystemFunction_appendDict,
    HalSMSystemFunction_indexByKey,
    HalSMSystemFunction_indexByValue,
    HalSMSystemFunction_set,

    //Math
    HalSMSystemFunction_sqrt,
    HalSMSystemFunction_int,
    HalSMSystemFunction_double,
    HalSMSystemFunction_str,
    HalSMSystemFunction_floor,
    HalSMSystemFunction_ceil,
    HalSMSystemFunction_round,
    HalSMSystemFunction_abs,

    //String
    HalSMSystemFunction_join,

    //File
    HalSMSystemFunction_fopen,

    //Convert
    HalSMSystemFunction_array,

    //System
    HalSMSystemFunction_dir,
    HalSMSystemFunction_keyByIndex,
    HalSMSystemFunction_valueByIndex,
    HalSMSystemFunction_dict,
    HalSMSystemFunction_pow,
    HalSMSystemFunction_bin,
    HalSMSystemFunction_hex,
    HalSMSystemFunction_type
} HalSMSystemFunction;

typedef enum HalSMVariableType {
    HalSMVariableType_int,
    HalSMVariableType_double,
    HalSMVariableType_char,
    HalSMVariableType_void,
    HalSMVariableType_HalSMArray,
    HalSMVariableType_str,
    HalSMVariableType_int_array,
    HalSMVariableType_HalSMFunctionC,
    HalSMVariableType_HalSMClassC,
    HalSMVariableType_HalSMRunClassC,
    HalSMVariableType_HalSMSetArg,
    HalSMVariableType_HalSMError,
    HalSMVariableType_HalSMNull,
    HalSMVariableType_HalSMRunFunc,
    HalSMVariableType_HalSMRunFuncC,
    HalSMVariableType_HalSMLocalFunction,
    HalSMVariableType_HalSMCModule,
    HalSMVariableType_HalSMModule,
    HalSMVariableType_HalSMCompiler,
    HalSMVariableType_HalSMRunClass,
    HalSMVariableType_HalSMDoubleGet,
    HalSMVariableType_HalSMClass,
    HalSMVariableType_HalSMVar,
    HalSMVariableType_HalSMMult,
    HalSMVariableType_HalSMDivide,
    HalSMVariableType_HalSMPlus,
    HalSMVariableType_HalSMMinus,
    HalSMVariableType_HalSMEqual,
    HalSMVariableType_HalSMNotEqual,
    HalSMVariableType_HalSMMore,
    HalSMVariableType_HalSMLess,
    HalSMVariableType_HalSMBool,
    HalSMVariableType_HalSMDict,
    HalSMVariableType_HalSMSetVar,
    HalSMVariableType_HalSMReturn,
    HalSMVariableType_HalSMFunctionCTypeDef,
    HalSMVariableType_HalSMFunctionArray,
    HalSMVariableType_unsigned_int,
    HalSMVariableType_custom,
    HalSMVariableType_HalSMMoreEqual,
    HalSMVariableType_HalSMLessEqual,
    HalSMVariableType_HalSMShift,
    HalSMVariableType_HalSMPow,
    HalSMVariableType_HalSMModulo,
    HalSMVariableType_HalSMAND,
    HalSMVariableType_HalSMOR,
    HalSMVariableType_HalSMNegate,
    HalSMVariableType_HalSMNegativeSign,
    //0.1.5
    HalSMVariableType_HalSMGOTO,
    HalSMVariableType_HalSMGOTOSector,
    HalSMVariableType_HalSMInlineIf, //if(a>b) 1 else 0
    HalSMVariableType_HalSMInlineElse,
    HalSMVariableType_HalSMBreak,
    HalSMVariableType_HalSMXOR,
    HalSMVariableType_HalSMBAND,
    HalSMVariableType_HalSMBOR,
    HalSMVariableType_HalSMBNOT,
    //0.1.8
    HalSMVariableType_fv,
    HalSMVariableType_HalSMOP
} HalSMVariableType;

static const HalStringFormatStr* HalSMVariableTypeName=(HalStringFormatStr[]){
    I2H(((ui32[]){0x81240000,0x1380000,0x1500000,0x0})),
    I2H(((ui32[]){0x1100000,0x813c0000,0x81540000,0x1080000,0x1300000,0x81140000,0x0})),
    I2H(((ui32[]){0x10c0000,0x1200000,0x81040000,0x1480000,0x0})),
    I2H(((ui32[]){0x1580000,0x813c0000,0x81240000,0x1100000,0x0})),
    I2H(((ui32[]){0x81040000,0x1480000,0x1480000,0x81040000,0x81640000,0x0})),
    I2H(((ui32[]){0x14c0000,0x1500000,0x1480000,0x0})),
    I2H(((ui32[]){0xc1240000,0x1380000,0x1500000,0x800000,0xc1040000,0x1480000,0x1480000,0x81040000,0x81640000,0x0})),
    I2H(((ui32[]){0x41180000,0x81540000,0x1380000,0x10c0000,0x1500000,0x81240000,0x813c0000,0x1380000,0x410c0000,0x0})),
    I2H(((ui32[]){0x410c0000,0x1300000,0x81040000,0x14c0000,0x14c0000,0x410c0000,0x0})),
    I2H(((ui32[]){0x41480000,0x81540000,0x1380000,0x410c0000,0x1300000,0x81040000,0x14c0000,0x14c0000,0x410c0000,0x0})),
    I2H(((ui32[]){0x414c0000,0x81140000,0x1500000,0xc1040000,0x1480000,0x11c0000,0x0})),
    I2H(((ui32[]){0xc1140000,0x1480000,0x1480000,0x813c0000,0x1480000,0x0})),
    I2H(((ui32[]){0x41380000,0x81540000,0x1300000,0x1300000,0x0})),
    I2H(((ui32[]){0x41480000,0x81540000,0x1380000,0x41180000,0x81540000,0x1380000,0x10c0000,0x0})),
    I2H(((ui32[]){0x41480000,0x81540000,0x1380000,0x41180000,0x81540000,0x1380000,0x10c0000,0x410c0000,0x0})),
    I2H(((ui32[]){0x41300000,0x813c0000,0x10c0000,0x81040000,0x1300000,0x41180000,0x81540000,0x1380000,0x10c0000,0x1500000,0x81240000,0x813c0000,0x1380000,0x0})),
    I2H(((ui32[]){0x410c0000,0x41340000,0x813c0000,0x1100000,0x81540000,0x1300000,0x81140000,0x0})),
    I2H(((ui32[]){0x41340000,0x813c0000,0x1100000,0x81540000,0x1300000,0x81140000,0x0})),
    I2H(((ui32[]){0x41200000,0x81040000,0x1300000,0x414c0000,0x41340000,0x410c0000,0x813c0000,0x1340000,0x1400000,0x81240000,0x1300000,0x81140000,0x1480000,0x0})),
    I2H(((ui32[]){0x41480000,0x81540000,0x1380000,0x410c0000,0x1300000,0x81040000,0x14c0000,0x14c0000,0x0})),
    I2H(((ui32[]){0x41200000,0x81040000,0x1300000,0x414c0000,0x41340000,0x41100000,0x813c0000,0x81540000,0x1080000,0x1300000,0x81140000,0x411c0000,0x81140000,0x1500000,0x0})),
    I2H(((ui32[]){0x410c0000,0x1300000,0x81040000,0x14c0000,0x14c0000,0x0})),
    I2H(((ui32[]){0x41580000,0x81040000,0x1480000,0x0})),
    I2H(((ui32[]){0x41340000,0x81540000,0x1300000,0x1500000,0x0})),
    I2H(((ui32[]){0x41100000,0x81240000,0x1580000,0x81240000,0x1100000,0x81140000,0x0})),
    I2H(((ui32[]){0x41400000,0x1300000,0x81540000,0x14c0000,0x0})),
    I2H(((ui32[]){0x41340000,0x81240000,0x1380000,0x81540000,0x14c0000,0x0})),
    I2H(((ui32[]){0xc1140000,0x1440000,0x81540000,0x81040000,0x1300000,0x0})),
    I2H(((ui32[]){0x41380000,0x813c0000,0x1500000,0x800000,0xc1140000,0x1440000,0x81540000,0x81040000,0x1300000,0x0})),
    I2H(((ui32[]){0x41340000,0x813c0000,0x1480000,0x81140000,0x0})),
    I2H(((ui32[]){0x41300000,0x81140000,0x14c0000,0x14c0000,0x0})),
    I2H(((ui32[]){0x1080000,0x813c0000,0x813c0000,0x1300000,0x0})),
    I2H(((ui32[]){0x1100000,0x81240000,0x10c0000,0x1500000,0x0})),
    I2H(((ui32[]){0x414c0000,0x81140000,0x1500000,0x41580000,0x81040000,0x1480000,0x0})),
    I2H(((ui32[]){0x41480000,0x81140000,0x1500000,0x81540000,0x1480000,0x1380000,0x0})),
    I2H(((ui32[]){0x41200000,0x81040000,0x1300000,0x414c0000,0x41340000,0x41180000,0x81540000,0x1380000,0x10c0000,0x1500000,0x81240000,0x813c0000,0x1380000,0x410c0000,0x41500000,0x81640000,0x1400000,0x81140000,0x41100000,0x81140000,0x1180000,0x0})),
    I2H(((ui32[]){0x41200000,0x81040000,0x1300000,0x414c0000,0x41340000,0x41180000,0x81540000,0x1380000,0x10c0000,0x1500000,0x81240000,0x813c0000,0x1380000,0xc1040000,0x1480000,0x1480000,0x81040000,0x81640000,0x0})),
    I2H(((ui32[]){0x81540000,0x1380000,0x14c0000,0x81240000,0x11c0000,0x1380000,0x81140000,0x1100000,0x800000,0x81240000,0x1380000,0x1500000,0x0})),
    I2H(((ui32[]){0x10c0000,0x81540000,0x14c0000,0x1500000,0x813c0000,0x1340000,0x0})),
    I2H(((ui32[]){0x41340000,0x813c0000,0x1480000,0x81140000,0x800000,0xc1140000,0x1440000,0x81540000,0x81040000,0x1300000,0x0})),
    I2H(((ui32[]){0x41300000,0x81140000,0x14c0000,0x14c0000,0x800000,0xc1140000,0x1440000,0x81540000,0x81040000,0x1300000,0x0})),
    I2H(((ui32[]){0x414c0000,0x1200000,0x81240000,0x1180000,0x1500000,0x0})),
    I2H(((ui32[]){0x41400000,0x813c0000,0x15c0000,0x0})),
    I2H(((ui32[]){0x41340000,0x813c0000,0x1100000,0x81540000,0x1300000,0x813c0000,0x0})),
    I2H(((ui32[]){0xc1040000,0x41380000,0x41100000,0x0})),
    I2H(((ui32[]){0xc13c0000,0x41480000,0x0})),
    I2H(((ui32[]){0x41380000,0x81140000,0x11c0000,0x81040000,0x1500000,0x81140000,0x0})),
    I2H(((ui32[]){0x41380000,0x81140000,0x11c0000,0x81040000,0x1500000,0x81240000,0x1580000,0x81140000,0x800000,0x414c0000,0x81240000,0x11c0000,0x1380000,0x0})),
    I2H(((ui32[]){0x411c0000,0xc13c0000,0x41500000,0xc13c0000,0x0})),
    I2H(((ui32[]){0x411c0000,0xc13c0000,0x41500000,0xc13c0000,0x800000,0x414c0000,0x81140000,0x10c0000,0x1500000,0x813c0000,0x1480000,0x0})),
    I2H(((ui32[]){0xc1240000,0x1380000,0x1300000,0x81240000,0x1380000,0x81140000,0x800000,0x81240000,0x1180000,0x0})),
    I2H(((ui32[]){0xc1240000,0x1380000,0x1300000,0x81240000,0x1380000,0x81140000,0x800000,0x81140000,0x1300000,0x14c0000,0x81140000,0x0})),
    I2H(((ui32[]){0x41080000,0x1480000,0x81140000,0x81040000,0x12c0000,0x0})),
    I2H(((ui32[]){0x41600000,0xc13c0000,0x41480000,0x0})),
    I2H(((ui32[]){0x41080000,0xc1240000,0x41380000,0x800000,0xc1040000,0x41380000,0x41100000,0x0})),
    I2H(((ui32[]){0x41080000,0xc1240000,0x41380000,0x800000,0xc13c0000,0x41480000,0x0})),
    I2H(((ui32[]){0x41080000,0xc1240000,0x41380000,0x800000,0x41380000,0xc13c0000,0x41500000,0x0}))
};

typedef struct HalSMVariable {
    void* value;
    HalSMVariableType type;
} HalSMVariable;

typedef struct HalSMCompiler HalSMCompiler;

typedef struct DictElement {
    HalSMVariable* key;
    HalSMVariable* value;
} DictElement;

typedef struct DictElementForEach {
    HalSMVariable* key;
    HalSMVariable* value;
    ui64 index;
} DictElementForEach;

typedef struct Dict {
    ui64 size;
    DictElement** elements;
} Dict;

typedef struct HalSMArray {
    ui64 size;
    HalSMVariable** arr;
} HalSMArray;

//VARIABLE CUSTOM TYPE

typedef struct HalSMVariableTypeCustom {
    HalStringFormatChar* name;
    HalStringFormatChar*(*toString)(HalSMCompiler*,HalSMVariable*);
    HalSMVariable*(*add)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*sub)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*mul)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*div)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*pow)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*modulo)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*increment)(HalSMCompiler*,HalSMVariable*);
    HalSMVariable*(*decrement)(HalSMCompiler*,HalSMVariable*);
    unsigned char(*isEqual)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    unsigned char(*isMore)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    unsigned char(*isLess)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    unsigned char(*getBool)(HalSMCompiler*,HalSMVariable*);
    long long int(*indexVariableByName)(HalSMCompiler*,HalSMVariable*,HalStringFormatChar*);
    long long int(*indexFunctionByName)(HalSMCompiler*,HalSMVariable*,HalStringFormatChar*);
    HalSMVariable*(*getVariableByName)(HalSMCompiler*,HalSMVariable*,HalStringFormatChar*);
    HalSMVariable*(*getFunctionByName)(HalSMCompiler*,HalSMVariable*,HalStringFormatChar*);
    HalSMVariable*(*getItem)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*shiftRight)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*shiftLeft)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*convert)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*bxor)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*band)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*bor)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMVariable*(*bnot)(HalSMCompiler*,HalSMVariable*);
    Dict* vars;
    Dict* funcs;
} HalSMVariableTypeCustom;

typedef struct HalSMVariableCustom {
    HalSMVariableTypeCustom* type;
    Dict* vars;
    Dict* funcs;
    void* value;
} HalSMVariableCustom;




//0.1.8
//Operations
typedef struct HalSMFindValue {
    HalStringFormatStr name;
} HalSMFindValue;

//VARIABLE CUSTOM TYPE

typedef enum HalSMFunctionArrayType {
    HalSMFunctionArrayType_function,
    HalSMFunctionArrayType_array,
    HalSMFunctionArrayType_var,
    HalSMFunctionArrayType_dict
} HalSMFunctionArrayType;

typedef struct HalSMFunctionArray {
    HalSMArray* args;
    HalSMFunctionArrayType type;
} HalSMFunctionArray;



typedef struct HalSMFileSystemLibrary {
    unsigned char(*isExistsDir)(HalStringFormatChar*);
    unsigned char(*isExistsFile)(HalStringFormatChar*);
    HalSMArray*(*getListFiles)(HalSMCompiler*,HalStringFormatChar*);
    void*(*openFile)(HalStringFormatChar*,HalStringFormatChar*);
    void(*closeFile)(void*);
    ui64(*read)(void* buf,ui64 size,ui64 count,void* p);
    ui64(*write)(void* buf,ui64 size,ui64 count,void* p);
    void(*puts)(void*,HalStringFormatChar*);
    void(*seek)(void*,ui64,ui64);
    ui64(*tell)(void*);
    void(*rewind)(void*);
    void(*flush)(void*);
    int(*eof)(void*);
} HalSMFileSystemLibrary;

typedef struct HalSMMemoryManagmentLibrary {
    void*(*malloc)(ui64);
    void*(*calloc)(ui64,ui64);
    void*(*realloc)(void*,ui64);
    void(*free)(void*);
} HalSMMemoryManagmentLibrary;

typedef struct HalSMSystemLibrary {
    void(*exit)(int);
} HalSMSystemLibrary;



//HalSMMemory

#define HSMEM_ERR_ADDR 0xFFFFFFFFFFFFFFFFULL

#pragma pack(push, 1)
typedef struct HalSMMemoryCell HalSMMemoryCell;

typedef struct HalSMMemoryCell {
    void* addr;
    ui8 isFree;
    HalSMMemoryCell* nextFree;
    HalSMMemoryCell* prevFree;
} HalSMMemoryCell;
#pragma pack(pop)

typedef struct HalSMMemory {
    HalSMMemoryManagmentLibrary* mml;
    HalSMMemoryCell* table;
    ui64 size;
    ui64 free;
    HalSMMemoryCell* firstFree;
} HalSMMemory;

//HalSM

typedef struct HalSMInteger {
    unsigned char negative;
    unsigned char* value;
    ui64 size;
} HalSMInteger;

typedef struct HalSMDouble {
    unsigned char negative;
    HalSMInteger* a;
    HalSMInteger* b;
} HalSMDouble;

typedef struct HalSMCalculateVars {
    HalStringFormatChar* version;
    HalStringFormatChar*(*addStr)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMInteger*(*addInt)(HalSMMemory*,HalSMVariable*,HalSMVariable*);
    HalSMDouble*(*addDouble)(HalSMMemory*,HalSMVariable*,HalSMVariable*);
    HalStringFormatChar*(*subStr)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMInteger*(*subInt)(HalSMMemory*,HalSMVariable*,HalSMVariable*);
    HalSMDouble*(*subDouble)(HalSMMemory*,HalSMVariable*,HalSMVariable*);
    HalStringFormatChar*(*mulStr)(HalSMMemory*,HalSMVariable*,HalSMVariable*);
    HalSMInteger*(*mulInt)(HalSMMemory*,HalSMVariable*,HalSMVariable*);
    HalSMDouble*(*mulDouble)(HalSMMemory*,HalSMVariable*,HalSMVariable*);
    HalStringFormatChar*(*divStr)(HalSMCompiler*,HalSMVariable*,HalSMVariable*);
    HalSMInteger*(*divInt)(HalSMMemory*,HalSMVariable*,HalSMVariable*);
    HalSMDouble*(*divDouble)(HalSMMemory*,HalSMVariable*,HalSMVariable*);
} HalSMCalculateVars;

typedef struct HalSMCompiler {
    HalStringFormatChar* versionName;
    ui64 version;
    HalStringFormatChar* path;
    void* addition_data;
    Dict* functions;
    Dict* sys_modules;
    HalSMCalculateVars calcVars;
    ui64 line;
    Dict* sys_variables;
    Dict* variables;
    Dict* modules;
    Dict* localFunctions;
    Dict* classes;
    Dict* groupFunctions;
    void(*print)(HalStringFormatChar*);
    void(*printErrorf)(HalStringFormatChar*);
    HalStringFormatChar*(*inputf)(HalStringFormatChar*);
    HalSMVariable*(*getVariable)(HalStringFormatChar*);
    HalSMSystemLibrary* systemlibrary;
    HalSMFileSystemLibrary* filesystemlibrary;
    Dict* sys_custom_variables;
    Dict* GOTOSectors;
    ui64 indexl;
    unsigned char isRun;
    unsigned char isClose;
    ui64 countLocalFunctions;
    HalSMVariable*(*HalSMLoadModule)(HalSMCompiler*,HalStringFormatChar*);
    HalSMErrorIndificator* ind;
    HalSMErrorIndificator* curInd;

    //0.1.7 Clear memory every day!
    HalSMMemory* memory;

    //0.1.8 Salsa
    //Detection errors and print code
    HalSMArray* allLines;
} HalSMCompiler;

typedef enum HalSMOPType {
    HalSMOPType_nothing=0,
    HalSMOPType_reserved0=1,
    HalSMOPType_value=2,
    HalSMOPType_runFunction=3,
    HalSMOPType_runClass=4,
    HalSMOPType_setVar=5,
    HalSMOPType_add=6,
    HalSMOPType_sub=7,
    HalSMOPType_mul=8,
    HalSMOPType_div=9,
    HalSMOPType_inc=10,
    HalSMOPType_dec=11,
} HalSMOPType;

typedef struct HalSMOP {
    //Type of operation:
    //0: nothing
    //1: reserved
    //2: reserved
    //3: run function
    //4: run class
    //5: set var
    HalSMOPType type;
    HalSMArray* context;
    HalSMArray* args;
} HalSMOP;

typedef HalSMVariable*(*HalSMFunctionCTypeDef)(HalSMCompiler*,HalSMArray*,Dict*);
typedef HalSMVariable*(*HalSMFunctionCTypeDefWithData)(void*,HalSMCompiler*,HalSMArray*,Dict*);

typedef struct HalSMFunctionC {
    void** func;
    HalSMCompiler* hsmc;
    unsigned char type; //0 - standard, 1 - with data
    void* data;
} HalSMFunctionC;

typedef struct HalSMRunClassC {
    HalStringFormatChar* name;
    Dict* vrs;
    Dict* funcs;
} HalSMRunClassC;

typedef struct HalSMClassC {
    Dict* vrs;
    Dict* funcs;
    HalStringFormatChar* name;
} HalSMClassC;

typedef struct HalSMCModule {
    Dict* lfuncs;
    Dict* vrs;
    Dict* classes;
    HalStringFormatChar* name;
} HalSMCModule;

typedef struct HalSMCModule_entry {
    HalStringFormatChar* name;
    HalStringFormatChar* description;
    HalStringFormatChar* version;
    HalStringFormatChar* author;
    Dict* classes;
    Dict* lfuncs;
    Dict* vrs;
} HalSMCModule_entry;

typedef struct HalSMModule {
    HalStringFormatChar* name;
    HalSMCompiler* hsmc;
} HalSMModule;

typedef struct HalSMLocalFunction {
    HalSMCompiler* hsmc;
    HalStringFormatChar* name;
    ui64 id;
    HalSMArray* args;
    HalSMArray* func;
    Dict* vars;
    HalSMVariable* module;
} HalSMLocalFunction;

typedef struct HalSMVar {
    HalSMVariable* name;
} HalSMVar;

typedef struct HalSMPlus {unsigned char c;} HalSMPlus;
typedef struct HalSMMinus {unsigned char c;} HalSMMinus;
typedef struct HalSMMult {unsigned char c;} HalSMMult;
typedef struct HalSMDivide {unsigned char c;} HalSMDivide;
typedef struct HalSMPow {unsigned char c;} HalSMPow;
typedef struct HalSMModulo {unsigned char c;} HalSMModulo;

typedef struct HalSMEqual {unsigned char c;} HalSMEqual;
typedef struct HalSMNotEqual {unsigned char c;} HalSMNotEqual;
typedef struct HalSMMore {unsigned char c;} HalSMMore;
typedef struct HalSMLess {unsigned char c;} HalSMLess;
typedef struct HalSMMoreEqual {unsigned char c;} HalSMMoreEqual;
typedef struct HalSMLessEqual {unsigned char c;} HalSMLessEqual;
typedef struct HalSMAND {unsigned char c;} HalSMAND;
typedef struct HalSMOR {unsigned char c;} HalSMOR;
typedef struct HalSMNegate {unsigned char c;} HalSMNegate;
typedef struct HalSMNegativeSign {unsigned char c;} HalSMNegativeSign;

typedef struct HalSMShift {unsigned char c;} HalSMShift;
typedef struct HalSMInlineIf {unsigned char c;} HalSMInlineIf;
typedef struct HalSMInlineElse {unsigned char c;} HalSMInlineElse;
typedef struct HalSMBreak {unsigned char c;} HalSMBreak;
typedef struct HalSMVoid {unsigned char c;} HalSMVoid;
typedef struct HalSMXOR {unsigned char c;} HalSMXOR;
typedef struct HalSMBAND {unsigned char c;} HalSMBAND;
typedef struct HalSMBOR {unsigned char c;} HalSMBOR;
typedef struct HalSMBNOT {unsigned char c;} HalSMBNOT;

typedef struct HalSMSetArg {
    HalSMVariable* name;
    HalSMVariable* value;
} HalSMSetArg;

typedef struct HalSMRunFuncC {
    HalSMFunctionC* func;
    HalSMArray* args;
    HalSMArray* pointer;
} HalSMRunFuncC;

typedef struct HalSMRunFunc {
    HalSMLocalFunction* func;
    HalSMArray* args;
    HalSMArray* pointer;
} HalSMRunFunc;

typedef struct HalSMRunClass {
    HalStringFormatChar* name;
    Dict* funcs;
    Dict* vars;
} HalSMRunClass;

typedef struct HalSMClass {
    HalStringFormatChar* name;
    Dict* funcs;
    Dict* vars;
} HalSMClass;

typedef struct HalSMDoubleGet {
    HalStringFormatChar* st;
} HalSMDoubleGet;

typedef struct HalSMSetVar {
    HalSMVariable* name;
    HalStringFormatChar* value;
    HalSMVariable* pointer;
} HalSMSetVar;

typedef struct HalSMReturn {
    HalStringFormatChar* value;
    ui64 idLocalFunction;
} HalSMReturn;

extern HalSMInteger HalSMIntegerZero;
extern HalSMInteger HalSMIntegerOne;
extern HalSMInteger HalSMIntegerTwo;
extern HalSMInteger HalSMIntegerThree;
extern HalSMInteger HalSMIntegerTen;
extern HalSMInteger HalSMIntegerThousand;
extern HalSMDouble HalSMDoubleOne;
extern HalSMDouble HalSMDoubleZero;
extern HalSMDouble HalSMDoubleTwo;
extern HalSMDouble HalSMDoubleThree;
extern HalSMDouble HalSMDoubleTen;

static HalSMVariable HALSMNULLVAR={NOTHALSMNULLPOINTER,HalSMVariableType_HalSMNull};

HalSMNull* HalSMNull_init(HalSMCompiler* hsmc);
HalSMError* HalSMError_init(HalSMCompiler* hsmc,HalSMErrorIndificator* ind,HalStringFormatChar* error);
HalSMErrorIndificator* HalSMErrorIndificator_init(HalSMCompiler* hsmc,HalStringFormatChar* name,ui64 line,void* prev);

HalSMArray* HalSMArray_init(HalSMMemory* memory);
HalSMArray* HalSMArray_init_with_elements(HalSMMemory* memory,HalSMVariable** arr,ui64 size);
HalSMArray* HalSMArray_split_str(HalSMMemory* memory,HalStringFormatChar* str,HalStringFormatChar* spl);
void HalSMArray_add(HalSMMemory* memory,HalSMArray* harr,HalSMVariable* value);
void HalSMArray_set(HalSMArray* harr,HalSMVariable* value,ui64 index);
void HalSMArray_remove(HalSMMemory* memory,HalSMArray* harr,ui64 index);
void HalSMArray_appendArray(HalSMMemory* memory,HalSMArray* harr,HalSMArray* t);
void HalSMArray_insert(HalSMMemory* memory,HalSMArray* harr,HalSMVariable* value,ui64 index);
HalSMVariable* HalSMArray_get(HalSMArray* harr,ui64 index);
HalSMArray* HalSMArray_reverse(HalSMMemory* memory,HalSMArray* harr);
HalStringFormatChar* HalSMArray_join_str(HalSMMemory* memory,HalSMArray* harr,HalStringFormatChar* join);
HalStringFormatChar* HalSMArray_to_print(HalSMCompiler* hsmc,HalSMArray* harr);
HalStringFormatChar* HalSMArray_chars_to_str(HalSMMemory* memory,HalSMArray* harr);
HalSMArray* HalSMArray_slice(HalSMMemory* memory,HalSMArray* harr,ui64 s,ui64 e);
unsigned char HalSMArray_compare(HalSMCompiler* hsmc,HalSMArray* harr,HalSMArray* barr);
HalSMArray* HalSMArray_from_str(HalSMMemory* memory,HalStringFormatChar* str,ui64 size);
HalSMArray* HalSMArray_copy(HalSMMemory* memory,HalSMArray* harr);
void HalSMArray_free(HalSMMemory* memory,HalSMArray* harr);
long long int HalSMArray_find(HalSMCompiler* hsmc,HalSMArray* harr,HalSMVariable* value);
long long int HalSMArray_index(HalSMCompiler* hsmc,HalSMArray* harr,HalSMVariable* value);
long long int HalSMArray_right_index(HalSMCompiler* hsmc,HalSMArray* harr,HalSMVariable* value);
void HalSMArray_clear(HalSMMemory* memory,HalSMArray* harr);

HalSMCalculateVars HalSMCalculateVars_init();
HalStringFormatChar* HalSMCalculateVars_addStr(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1);
HalStringFormatChar* HalSMCalculateVars_subStr(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1);
HalStringFormatChar* HalSMCalculateVars_mulStr(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1);
HalStringFormatChar* HalSMCalculateVars_divStr(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1);
HalSMInteger* HalSMCalculateVars_addInt(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1);
HalSMInteger* HalSMCalculateVars_subInt(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1);
HalSMInteger* HalSMCalculateVars_mulInt(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1);
HalSMInteger* HalSMCalculateVars_divInt(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1);
HalSMDouble* HalSMCalculateVars_addDouble(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1);
HalSMDouble* HalSMCalculateVars_subDouble(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1);
HalSMDouble* HalSMCalculateVars_mulDouble(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1);
HalSMDouble* HalSMCalculateVars_divDouble(HalSMMemory* memory,HalSMVariable* v0,HalSMVariable* v1);

HalSMVariable* HalSMCompiler_input(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_reversed(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_range(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_print(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMArray* HalSMCompiler_get_print_text(HalSMCompiler* hsmc,HalSMArray* args,unsigned char isToPrint);
HalSMCompiler* HalSMCompiler_init(HalStringFormatChar* path,void(*printf)(HalStringFormatChar*),void(*printErrorf)(HalStringFormatChar*),HalStringFormatChar*(*inputf)(HalStringFormatChar*),HalSMVariable*(*getVariable)(HalStringFormatChar*),HalSMMemory* memory,HalSMSystemLibrary* systemlibrary,HalSMFileSystemLibrary* filesystemlibrary,HalSMVariable*(*HalSMLoadModule)(HalSMCompiler*,HalStringFormatChar*));
HalSMArray* HalSMCompiler_getLines(HalSMCompiler* hsmc,HalStringFormatChar* text);
HalStringFormatChar* HalSMCompiler_removeSpaces(HalSMMemory* memory,HalStringFormatChar* text);
HalStringFormatChar* HalSMCompiler_removeStartSpaces(HalSMMemory* memory,HalStringFormatChar* text);
HalStringFormatChar* HalSMCompiler_removeEndSpaces(HalSMMemory* memory,HalStringFormatChar* text);
void HalSMCompiler_ThrowError(HalSMCompiler* hsmc,HalSMError* err);
HalSMVariable* HalSMCompiler_getNameFunction(HalSMCompiler* hsmc,HalStringFormatChar* l);
HalSMVariable* HalSMCompiler_isSetVar(HalSMCompiler* hsmc,HalStringFormatChar* l,Dict* vars);
ui64 HalSMCompiler_getTabs(HalSMCompiler* hsmc,HalStringFormatChar* l);
unsigned char HalSMCompiler_isNull(HalStringFormatChar* text);
HalSMVariable* HalSMCompiler_compileList(HalSMCompiler* hsmc,unsigned char isConsole,HalSMVariable* module,HalSMArray* allLines);
HalSMVariable* HalSMCompiler_compile(HalSMCompiler* hsmc,HalStringFormatChar* text,unsigned char isConsole,HalSMVariable* module);
void HalSMCompiler_free(HalSMCompiler* hsmc);
void HalSMCompiler_exitApp(HalSMCompiler* hsmc);

//HalSM Operations
HalSMOP* HalSMOP_init(HalSMCompiler* hsmc,HalSMOPType type);
HalSMFindValue* HalSMFindValue_init(HalSMCompiler* hsmc, HalStringFormatStr name);
HalSMVariable* HalSMOP_findValue(HalSMCompiler* hsmc, HalSMVariable* context, HalSMFindValue* fv, Dict* vars);
HalSMVariable* HalSMOP_processVar(HalSMCompiler* hsmc,HalSMVariable* var, Dict* vars);
HalSMArray* HalSMOP_processArgs(HalSMCompiler* hsmc, HalSMArray* args, Dict* vars);
HalSMVariable* HalSMOP_processContext(HalSMCompiler* hsmc, HalSMArray* context, Dict* vars, HalSMVariable** module);
Dict* HalSMOP_processDict(HalSMCompiler* hsmc,Dict* args, Dict* vars);
HalSMOP* HalSMOP_compile(HalSMCompiler* hsmc,HalStringFormatStr line, Dict* vars);
HalSMVariable* HalSMOP_execute(HalSMCompiler* hsmc, HalSMOP* op, Dict* vars);

unsigned char HalSMCompiler_isGet(HalSMCompiler* hsmc,HalStringFormatChar* l,HalSMVariable* outvar,unsigned char isNewVar,Dict* vars);
HalSMVariable* HalSMCompiler_isGetU(HalSMCompiler* hsmc,HalStringFormatChar* l,unsigned char* isOut,unsigned char isNewVar,Dict* vars);
HalSMVariable* HalSMCompiler_isGetValue(HalSMCompiler* hsmc,HalSMVariable* obuff,unsigned char* isOut,unsigned char isNewVar,Dict* vars,unsigned char* isNotNullOut,HalSMVariable** moduleOut,HalSMVariable* out,HalStringFormatChar* type);
HalSMVariable* HalSMCompiler_getArrayByIndex(HalSMCompiler* hsmc,HalStringFormatChar* obuffs,HalSMVariable* out,unsigned char* isNotNullOut,Dict* vars);
void HalSMCompiler_executeArgs(HalSMCompiler* hsmc,HalSMArray** outArr,Dict* vars);
HalSMArray* HalSMCompiler_getArgs(HalSMCompiler* hsmc,HalStringFormatChar* l,Dict* vars);
void HalSMCompiler_getArgsValue(HalSMCompiler* hsmc,HalSMVariable* buffo,HalStringFormatChar* buffs,HalSMArray* out,Dict* vars);
void HalSMCompiler_getArgsSetVar(HalSMCompiler* hsmc,HalSMArray* value,HalSMVariable* pointer,Dict* vars);

void HalSMCompiler_getArgsFunction(HalSMCompiler* hsmc,HalSMArray* value,HalSMArray* out,Dict* vars);
Dict* HalSMCompiler_getArgsDict(HalSMCompiler* hsmc,HalStringFormatChar* l,Dict* vars);

HalStringFormatChar* HalSMCompiler_getPath(HalSMMemory* memory,HalStringFormatChar* a,HalStringFormatChar* b);
HalSMVariable* HalSMCompiler_isExistsPath(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_isExistsDir(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_isExistsFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_getListFiles(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_openFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_closeFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_readFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_putsFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_seekFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_tellFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_rewindFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_flushFile(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);

HalSMVariable* HalSMCompiler_additionVariables(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1);
HalSMVariable* HalSMCompiler_subtractionVariables(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1);
HalSMVariable* HalSMCompiler_multiplyVariables(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1);
HalSMVariable* HalSMCompiler_divideVariables(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1);
HalSMVariable* HalSMCompiler_incrementVariable(HalSMCompiler* hsmc,HalSMVariable* v);
HalSMVariable* HalSMCompiler_decrementVariable(HalSMCompiler* hsmc,HalSMVariable* v);
HalSMVariable* HalSMCompiler_increment(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_decrement(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);

HalSMVariable* HalSMCompiler_while(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);

HalSMLocalFunction* HalSMLocalFunction_init(HalSMCompiler* hsmc,HalStringFormatChar* name,HalStringFormatChar* args,HalSMVariable* module);
HalSMArray* HalSMLocalFunction_getArgs(HalSMCompiler* hsmc,HalStringFormatChar* args);
HalSMVariable* HalSMLocalFunction_run(HalSMLocalFunction* lf,HalSMArray* args,Dict* vars);

unsigned char HalSMCompiler_isMore(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
unsigned char HalSMCompiler_isLess(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);

HalSMVariable* HalSMCompiler_shift(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b,HalSMShift* shift);
HalSMVariable* HalSMCompiler_pow(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HalSMCompiler_powFunc(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_bin(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_hex(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_modulo(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);

unsigned char HalSMCompiler_getBool(HalSMCompiler* hsmc,HalSMVariable* v);
HalSMVariable* HalSMCompiler_negate(HalSMCompiler* hsmc,HalSMVariable* v);

HalSMVariable* HalSMCompiler_replace(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_index(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_rindex(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_split(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_substring(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_str_upper(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);

HalSMVariable* HalSMCompiler_add(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_append(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_count(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_clear(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_insert(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_pop(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_remove(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_reverse(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_copy(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_appendArray(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_slice(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);

HalSMVariable* HalSMCompiler_len(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);

HalSMVariable* HalSMCompiler_appendDict(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_indexByKey(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_indexByValue(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_keyByIndex(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_valueByIndex(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_set(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);

HalSMVariable* HalSMCompiler_sqrt(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_int(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_double(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_str(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_floor(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_ceil(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_round(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_abs(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_join(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_array(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_dir(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_dict(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HalSMCompiler_type(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);

unsigned char HalSMIsInt(HalSMMemory* memory,HalStringFormatChar* c);
unsigned char HalSMIsDouble(HalSMMemory* memory,HalStringFormatChar* c);
unsigned char HalSMIsHex(HalSMMemory* memory,HalStringFormatChar* c);
unsigned char HalSMIsBin(HalSMMemory* memory,HalStringFormatChar* c);

HalSMDoubleGet* HalSMDoubleGet_init(HalSMMemory* memory,HalStringFormatChar* st);

HalSMCModule* HalSMCModule_init(HalSMMemory* memory,HalStringFormatChar* name);
HalSMCModule* HalSMCModule_load(HalSMCompiler* hsmc,HalSMCModule_entry* entry);

HalSMModule* HalSMModule_init(HalSMMemory* memory,HalStringFormatChar* name,HalSMCompiler* hsmcmodule);

HalSMRunFunc* HalSMRunFunc_init(HalSMMemory* memory,HalSMLocalFunction* func,HalSMArray* args);

HalSMRunFuncC* HalSMRunFuncC_init(HalSMMemory* memory,HalSMFunctionC* func,HalSMArray* args);

HalSMClassC* HalSMClassC_init(HalSMMemory* memory,HalStringFormatChar* name);
HalSMRunClassC* HalSMClassC_run(HalSMCompiler* hsmc,HalSMClassC* classc,HalSMArray* args);

HalSMClass* HalSMClass_init(HalSMMemory* memory,HalStringFormatChar* name,Dict* vrs);
HalSMRunClass* HalSMClass_run(HalSMClass* clas,HalSMCompiler* hsmc,HalSMArray* args);

HalSMRunClass* HalSMRunClass_init(HalSMMemory* memory,HalStringFormatChar* name,Dict* vrs,Dict* funcs);
HalSMRunClass* HalSMRunClass__init__(HalSMRunClass* runclass,HalSMCompiler* hsmc,HalSMArray* args);

HalSMFunctionC* HalSMFunctionC_init(HalSMCompiler* hsmc,HalSMFunctionCTypeDef func);
HalSMFunctionC* HalSMFunctionC_initWithData(void* data,HalSMCompiler* hsmc,HalSMFunctionCTypeDefWithData func);
HalSMVariable* HalSMFunctionC_run(HalSMCompiler* hsmc,HalSMFunctionC* hfc,HalSMArray* args,Dict* vars);

HalSMRunClassC* HalSMRunClassC_init(HalSMMemory* memory,HalStringFormatChar* name,Dict* vrs,Dict* funcs);
HalSMRunClassC* HalSMRunClassC__init__(HalSMCompiler* hsmc,HalSMRunClassC* runclassc,HalSMArray* args);

HalSMVar* HalSMVar_init(HalSMMemory* memory,HalStringFormatChar* name);

HalSMSetArg* HalSMSetArg_init(HalSMMemory* memory,HalSMVariable* name);
HalSMSetArg* HalSMSetArg_initWithValue(HalSMMemory* memory,HalSMVariable* name,HalSMVariable* value);

HalSMReturn* HalSMReturn_init(HalSMMemory* memory,HalStringFormatChar* val,ui64 idLocalFunction);

Dict* DictInit(HalSMMemory* memory);
Dict* DictInitWithElements(HalSMMemory* memory,DictElement* elements[],ui64 size);
DictElement* DictElementInit(HalSMMemory* memory,HalSMVariable* key,HalSMVariable* value);
DictElement* DictElementFindByKey(HalSMCompiler* hsmc,Dict* dict,HalSMVariable* key);
DictElement* DictElementFindByValue(HalSMCompiler* hsmc,Dict* dict,HalSMVariable* value);
DictElement* DictElementFindByIndex(Dict* dict,ui64 index);
long long int DictElementIndexByKey(HalSMCompiler* hsmc,Dict* dict,HalSMVariable* key);
long long int DictElementIndexByValue(HalSMCompiler* hsmc,Dict* dict,HalSMVariable* value);
DictElement* DictElementCopy(HalSMMemory* memory,DictElement* elem);
void DictElementFree(HalSMMemory* memory,DictElement* elem);
void PutDictElementToDict(HalSMCompiler* hsmc,Dict* dict,DictElement* elem);
Dict* DictCopy(HalSMMemory* memory,Dict* dict);
unsigned char DictCompare(HalSMCompiler* hsmc,Dict* a,Dict* b);
void DictFree(HalSMMemory* memory,Dict* dict);
void DictFreeE(HalSMMemory* memory,Dict* dict);
void DictClear(HalSMMemory* memory,Dict* dict);
HalStringFormatChar* Dict_to_print(HalSMCompiler* hsmc,Dict* harr);
void DictAppendDict(HalSMMemory* memory,Dict* a,Dict* b);
void DictRemove(HalSMMemory* memory,Dict* a,ui64 index);

HalSMVariable* HalSMVariable_init(HalSMMemory* memory,void* value,HalSMVariableType type);
void HalSMVariable_AsVar(void* var,HalSMVariable* arg);
HalSMVariable* HalSMVariable_init_str(HalSMMemory* memory,HalStringFormatChar* str);
HalSMVariable* HalSMVariable_init_int(HalSMMemory* memory,HalSMInteger* n);
HalSMVariable* HalSMVariable_init_double(HalSMMemory* memory,HalSMDouble* d);
HalSMVariable* HalSMVariable_init_bool(HalSMMemory* memory,unsigned char b);
HalSMVariable* HalSMVariable_init_unsigned_int(HalSMMemory* memory,ui64 n);
HalSMVariable* HalSMVariable_init_char(HalSMMemory* memory,HalStringFormatChar c);
HalStringFormatChar* HalSMVariable_to_str(HalSMCompiler* hsmc,HalSMVariable* a,unsigned char isToPrint);
unsigned char HalSMVariable_Compare(HalSMCompiler* hsmc,HalSMVariable* v0,HalSMVariable* v1);
void HalSMVariable_free(HalSMMemory* memory,HalSMVariable* arg);
HalSMVariableCustom* HalSMVariableCustom_init(HalSMMemory* memory,HalSMVariableTypeCustom* type,void* value,Dict* vars,Dict* funcs);
HalSMVariableTypeCustom* HalSMVariableTypeCustom_init(HalSMMemory* memory,HalSMVariableTypeCustom type);

HalSMSetVar* HalSMSetVar_init(HalSMMemory* memory,HalStringFormatChar* name,HalStringFormatChar* value,HalSMVariable* pointer);

HalSMVariable* ParseHalSMVariableInt(HalSMCompiler* hsmc,HalStringFormatChar* c);
HalSMVariable* ParseHalSMVariableDouble(HalSMCompiler* hsmc,HalStringFormatChar* c);

long long int StringIndexOf(HalSMMemory* memory,HalStringFormatChar* c,HalStringFormatChar* f);
long long int StringLastIndexOf(HalSMMemory* memory,HalStringFormatChar* c,HalStringFormatChar* f);
HalStringFormatChar* SubString(HalSMMemory* memory,HalStringFormatChar* c,int start,int end);
HalStringFormatChar* SubStringHalSMInteger(HalSMMemory* memory,HalStringFormatChar* c,HalSMInteger* start,HalSMInteger* end);
HalStringFormatChar* ConcatenateStrings(HalSMMemory* memory,HalStringFormatChar* c,HalStringFormatChar* f);
HalStringFormatChar* StringReplace(HalSMMemory* memory,HalStringFormatChar* c,HalStringFormatChar* f,HalStringFormatChar* r);
unsigned char StringEndsWith(HalSMMemory* memory,HalStringFormatChar* c,HalStringFormatChar* f);
unsigned char StringStartsWith(HalSMMemory* memory,HalStringFormatChar* c,HalStringFormatChar* f);
void AdditionStrings(HalSMMemory* memory,HalStringFormatChar** c,HalStringFormatChar* f,ui64 sizec,ui64 sizef);



HalSMInteger* HalSMInteger_init(HalSMMemory* memory,unsigned char negative,unsigned char* value,ui64 size);
HalSMInteger* HalSMInteger_copy(HalSMMemory* memory,HalSMInteger* a);
HalSMInteger* HalSMInteger_FromUnsignedInteger(HalSMMemory* memory,unsigned int value);
HalSMInteger* HalSMInteger_FromSignedInteger(HalSMMemory* memory,signed int value);
HalSMInteger* HalSMInteger_FromUnsignedLongLongInteger(HalSMMemory* memory,ui64 value);
unsigned int HalSMInteger_ToUnsignedInteger(HalSMInteger* value);
ui64 HalSMInteger_ToUnsignedLongLongInteger(HalSMInteger* value);
HalSMInteger* HalSMInteger_parse(HalSMMemory* memory,HalStringFormatChar* str);
HalSMInteger* HalSMInteger_parseHex(HalSMMemory* memory,HalStringFormatChar* str,unsigned char isZeroX);
HalSMInteger* HalSMInteger_parseBin(HalSMMemory* memory,HalStringFormatChar* str);
void HalSMInteger_AddSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
HalSMInteger* HalSMInteger_Add(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
void HalSMInteger_SubSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
HalSMInteger* HalSMInteger_Sub(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
void HalSMInteger_MulSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
HalSMInteger* HalSMInteger_Mul(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
void HalSMInteger_DivSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b,HalSMInteger* remainder);
HalSMInteger* HalSMInteger_Div(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b,HalSMInteger* remainder);
HalSMInteger* HalSMInteger_negate(HalSMInteger* a);
HalSMInteger* HalSMInteger_absolute(HalSMInteger* a);
unsigned char HalSMInteger_isMore(HalSMInteger* a,HalSMInteger* b);
unsigned char HalSMInteger_isLess(HalSMInteger* a,HalSMInteger* b);
unsigned char HalSMInteger_isEqual(HalSMInteger* a,HalSMInteger* b);
HalSMInteger* HalSMInteger_getValueWithoutNull(HalSMMemory* memory,HalSMInteger* a);
char* HalSMInteger_Byte2Bits(HalSMMemory* memory,unsigned char byte);
HalStringFormatChar* HalSMInteger_Bytes2Bits(HalSMMemory* memory,unsigned char* bytes,ui64 size);
HalStringFormatChar* HalSMInteger_toString(HalSMMemory* memory,HalSMInteger* a,unsigned char isHex);
HalStringFormatChar* HalSMInteger_Byte2Hex(HalStringFormatChar* out,unsigned char x);
HalStringFormatChar* HalSMInteger_toStringBytes(HalSMMemory* memory,HalSMInteger* a);
unsigned char HalSMInteger_getBit(HalSMInteger* a,long long int byte,unsigned char bit);
void HalSMInteger_setBitSelf(HalSMMemory* memory,HalSMInteger* a,long long int byte,unsigned char bit,unsigned char value);
HalSMInteger* HalSMInteger_setBit(HalSMMemory* memory,HalSMInteger* a,long long int byte,unsigned char bit,unsigned char value);
void HalSMInteger_shiftRightSelf(HalSMMemory* memory,HalSMInteger* a,long long int bit);
void HalSMInteger_shiftLeftSelf(HalSMMemory* memory,HalSMInteger* a,long long int bit);
HalSMInteger* HalSMInteger_shiftRight(HalSMMemory* memory,HalSMInteger* a,long long int bit);
HalSMInteger* HalSMInteger_shiftLeft(HalSMMemory* memory,HalSMInteger* a,long long int bit);
HalSMInteger* HalSMInteger_clear(HalSMMemory* memory,HalSMInteger* a);
void HalSMInteger_PowSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
HalSMInteger* HalSMInteger_Pow(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
void HalSMInteger_Free(HalSMMemory* memory,HalSMInteger* a);
unsigned char HalSMInteger_ByteCTZ(unsigned char byte);
long long int HalSMInteger_ctz(HalSMInteger* a);
void HalSMInteger_BitORSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
HalSMInteger* HalSMInteger_BitOR(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
void HalSMInteger_BitXORSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
HalSMInteger* HalSMInteger_BitXOR(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
void HalSMInteger_BitANDSelf(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
HalSMInteger* HalSMInteger_BitAND(HalSMMemory* memory,HalSMInteger* a,HalSMInteger* b);
void HalSMInteger_BitNOTSelf(HalSMMemory* memory,HalSMInteger* a);
HalSMInteger* HalSMInteger_BitNOT(HalSMMemory* memory,HalSMInteger* a);
HalSMInteger* HalSMInteger_FromHalSMDouble(HalSMMemory* memory,HalSMDouble* a);
HalSMInteger* HalSMInteger_FloorHalSMDouble(HalSMMemory* memory,HalSMDouble* a);
HalSMInteger* HalSMInteger_CeilHalSMDouble(HalSMMemory* memory,HalSMDouble* a);
signed char HalSMInteger_compare(HalSMInteger* a,HalSMInteger* b);

HalSMDouble* HalSMDouble_init(HalSMMemory* memory,unsigned char negative,HalSMInteger* a,HalSMInteger* b);
HalSMDouble* HalSMDouble_copy(HalSMMemory* memory,HalSMDouble* a);
HalSMDouble* HalSMDouble_FromUnsignedInteger(HalSMMemory* memory,unsigned int value);
HalSMDouble* HalSMDouble_FromSignedInteger(HalSMMemory* memory,signed int value);
HalSMDouble* HalSMDouble_FromFloat(HalSMMemory* memory,float value);
HalSMDouble* HalSMDouble_FromDouble(HalSMMemory* memory,double value);
HalSMDouble* HalSMDouble_FromHalSMInteger(HalSMMemory* memory,HalSMInteger* value);
HalSMDouble* HalSMDouble_parse(HalSMMemory* memory,HalStringFormatChar* str);
unsigned char HalSMDouble_isMore(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b);
unsigned char HalSMDouble_isLess(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b);
unsigned char HalSMDouble_isEqual(HalSMDouble* a,HalSMDouble* b);
void HalSMDouble_AddSelf(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b);
HalSMDouble* HalSMDouble_Add(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b);
void HalSMDouble_SubSelf(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b);
HalSMDouble* HalSMDouble_Sub(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b);
void HalSMDouble_MulSelf(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b);
HalSMDouble* HalSMDouble_Mul(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b);
void HalSMDouble_DivSelf(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b);
HalSMDouble* HalSMDouble_Div(HalSMMemory* memory,HalSMDouble* a,HalSMDouble* b);
HalStringFormatChar* HalSMDouble_toString(HalSMMemory* memory,HalSMDouble* a,unsigned char isHex);
HalStringFormatChar* HalSMDouble_toStringRound(HalSMMemory* memory,HalSMDouble* a,ui64 round);
double HalSMDouble_ToDouble(HalSMMemory* memory,HalSMDouble* a);
float HalSMDouble_ToFloat(HalSMMemory* memory,HalSMDouble* a);
void HalSMDouble_SquareRootSelf(HalSMMemory* memory,HalSMDouble* a,HalSMInteger* q,ui64 crop);
HalSMDouble* HalSMDouble_SquareRoot(HalSMMemory* memory,HalSMDouble* a,HalSMInteger* q,ui64 crop);
void HalSMDouble_AbsSelf(HalSMMemory* memory,HalSMDouble* a);
HalSMDouble* HalSMDouble_Abs(HalSMMemory* memory,HalSMDouble* a);
void HalSMDouble_Free(HalSMMemory* memory,HalSMDouble* a);
void HalSMDouble_PowSelf(HalSMMemory* memory,HalSMDouble* a,HalSMInteger* b);
HalSMDouble* HalSMDouble_Pow(HalSMMemory* memory,HalSMDouble* a,HalSMInteger* b);
void HalSMDouble_RoundSelf(HalSMMemory* memory,HalSMDouble* a,HalSMInteger* b);
HalSMDouble* HalSMDouble_Round(HalSMMemory* memory,HalSMDouble* a,HalSMInteger* b);
void HalSMDouble_CropSelf(HalSMMemory* memory,HalSMDouble* a,ui64 crop);
HalSMInteger* HalSMDouble_gcdSelf(HalSMMemory* memory,HalSMInteger* u,HalSMInteger* v);
HalSMInteger* HalSMDouble_gcd(HalSMMemory* memory,HalSMInteger* u,HalSMInteger* v);
void HalSMDouble_clearSelf(HalSMMemory* memory,HalSMDouble* a);
HalSMDouble* HalSMDouble_clear(HalSMMemory* memory,HalSMDouble* a);

HalSMFunctionArray* HalSMFunctionArray_init(HalSMMemory* memory,HalSMArray* args,HalSMFunctionArrayType type);

//GOTO

typedef struct HalSMGOTO {
    HalStringFormatChar* sector;
} HalSMGOTO;

typedef struct HalSMGOTOSector {
    HalStringFormatChar* sector;
    ui64 index;
    HalSMLocalFunction* lf;
} HalSMGOTOSector;

HalSMGOTO* HalSMGOTO_init(HalSMMemory* memory,HalStringFormatChar* sector);
HalSMGOTOSector* HalSMGOTOSector_init(HalSMMemory* memory,HalStringFormatChar* sector,ui64 index,HalSMLocalFunction* lf);

//HalSM File Pointer

HalStringFormatChar* HFP_toString(HalSMCompiler* hsmc,HalSMVariable* v);
HalSMVariable* HFP_add(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_sub(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_mul(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_div(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_pow(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_modulo(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_increment(HalSMCompiler* hsmc,HalSMVariable* v);
HalSMVariable* HFP_decrement(HalSMCompiler* hsmc,HalSMVariable* v);
unsigned char HFP_isEqual(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
unsigned char HFP_isMore(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
unsigned char HFP_isLess(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
unsigned char HFP_getBool(HalSMCompiler* hsmc,HalSMVariable* v);
long long int HFP_indexVariableByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name);
long long int HFP_indexFunctionByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name);
HalSMVariable* HFP_getVariableByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name);
HalSMVariable* HFP_getFunctionByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name);
HalSMVariable* HFP_getItem(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_shiftRight(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_shiftLeft(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_convert(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_xor(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_and(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_or(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HFP_bnot(HalSMCompiler* hsmc,HalSMVariable* a);
HalSMVariable* HFP_close(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_read(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_get(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_write(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_put(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_eof(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_seek(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_tell(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_rewind(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_flush(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_readUTF8(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_writeUTF8(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_readHSF(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_writeHSF(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_readUTF16(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);
HalSMVariable* HFP_writeUTF16(HalSMCompiler* hsmc,HalSMArray* args,Dict* vars);

//HalSM Byte Array

typedef struct HalSMByteArray {
    unsigned char* bytes;
    ui64 size;
} HalSMByteArray;

HalStringFormatChar* HBA_toString(HalSMCompiler* hsmc,HalSMVariable* v);
HalSMVariable* HBA_add(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_sub(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_mul(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_div(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_pow(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_modulo(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_increment(HalSMCompiler* hsmc,HalSMVariable* v);
HalSMVariable* HBA_decrement(HalSMCompiler* hsmc,HalSMVariable* v);
unsigned char HBA_isEqual(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
unsigned char HBA_isMore(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
unsigned char HBA_isLess(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
unsigned char HBA_getBool(HalSMCompiler* hsmc,HalSMVariable* v);
long long int HBA_indexVariableByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name);
long long int HBA_indexFunctionByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name);
HalSMVariable* HBA_getVariableByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name);
HalSMVariable* HBA_getFunctionByName(HalSMCompiler* hsmc,HalSMVariable* v,HalStringFormatChar* name);
HalSMVariable* HBA_getItem(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_shiftRight(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_shiftLeft(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_convert(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_xor(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_and(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_or(HalSMCompiler* hsmc,HalSMVariable* a,HalSMVariable* b);
HalSMVariable* HBA_bnot(HalSMCompiler* hsmc,HalSMVariable* a);

//String

ui64 string_len(HalStringFormatChar* s);
HalSMInteger* string_len_HalSMInteger(HalSMMemory* memory,HalStringFormatChar* s);
HalStringFormatChar* string_cat(HalStringFormatChar* s1,HalStringFormatChar* s2);
HalStringFormatChar* string_cpy(HalStringFormatChar* s1,HalStringFormatChar* s2);
HalStringFormatChar* stringncpy(HalStringFormatChar* s1,HalStringFormatChar* s2,ui64 n);

void* memory_cpy(void* dst,const void* src,ui64 n);

//HalSMMemory

void HalSMMemory_setCell(HalSMMemory* memory,ui64 nc,void* addr);
ui64 HalSMMemory_setFreeCell(HalSMMemory* memory,void* addr);
ui64 HalSMMemory_addNewCell(HalSMMemory* memory,void* addr);
ui64 HalSMMemory_getNCByAddr(HalSMMemory* memory,void* addr);
void HalSMMemory_freeCell(HalSMMemory* memory,ui64 nc);
void HalSMMemory_freeSys(HalSMMemory* memory);

void HalSMMemory_init(HalSMMemory* out,HalSMMemoryManagmentLibrary* mml);
void* HalSMMemory_malloc(HalSMMemory* memory,ui64 size);
void* HalSMMemory_realloc(HalSMMemory* memory,void* p,ui64 newsize);
void HalSMMemory_free(HalSMMemory* memory,void* p);
void* HalSMMemory_calloc(HalSMMemory* memory,ui64 count,ui64 size);

//Defines

#define typevar(x) _Generic((x),HalStringFormatChar*:HalSMVariableType_char,HalSMVoid*:HalSMVariableType_void,HalSMInteger*:HalSMVariableType_int,int**:HalSMVariableType_int_array,\
HalSMDouble*:HalSMVariableType_double,HalSMArray*:HalSMVariableType_HalSMArray,HalStringFormatChar**:HalSMVariableType_str,HalSMFunctionC*:HalSMVariableType_HalSMFunctionC,\
HalSMRunClassC*:HalSMVariableType_HalSMRunClassC,HalSMSetArg*:HalSMVariableType_HalSMSetArg,HalSMError*:HalSMVariableType_HalSMError,\
HalSMNull*:HalSMVariableType_HalSMNull,HalSMRunFunc*:HalSMVariableType_HalSMRunFunc,HalSMLocalFunction*:HalSMVariableType_HalSMLocalFunction,\
HalSMCModule*:HalSMVariableType_HalSMCModule,HalSMModule*:HalSMVariableType_HalSMModule,HalSMClassC*:HalSMVariableType_HalSMClassC,\
HalSMCompiler*:HalSMVariableType_HalSMCompiler,HalSMRunClass*:HalSMVariableType_HalSMRunClass,HalSMDoubleGet*:HalSMVariableType_HalSMDoubleGet,\
HalSMClass*:HalSMVariableType_HalSMClass,HalSMVar*:HalSMVariableType_HalSMVar,HalSMPlus*:HalSMVariableType_HalSMPlus,\
HalSMMinus*:HalSMVariableType_HalSMMinus,HalSMMult*:HalSMVariableType_HalSMMult,HalSMDivide*:HalSMVariableType_HalSMDivide,\
HalSMEqual*:HalSMVariableType_HalSMEqual,HalSMNotEqual*:HalSMVariableType_HalSMNotEqual,HalSMMore*:HalSMVariableType_HalSMMore,\
HalSMLess*:HalSMVariableType_HalSMLess,unsigned char*:HalSMVariableType_HalSMBool,\
Dict*:HalSMVariableType_HalSMDict,HalSMReturn*:HalSMVariableType_HalSMReturn,HalSMSetVar*:HalSMVariableType_HalSMSetVar,\
HalSMFunctionCTypeDef*:HalSMVariableType_HalSMFunctionCTypeDef,HalSMFunctionArray*:HalSMVariableType_HalSMFunctionArray,\
ui64*:HalSMVariableType_unsigned_int,HalSMRunFuncC*:HalSMVariableType_HalSMRunFuncC,HalSMVariableCustom*:HalSMVariableType_custom,\
HalSMMoreEqual*:HalSMVariableType_HalSMMoreEqual,HalSMLessEqual*:HalSMVariableType_HalSMLessEqual,HalSMShift*:HalSMVariableType_HalSMShift,\
HalSMPow*:HalSMVariableType_HalSMPow,HalSMModulo*:HalSMVariableType_HalSMModulo,HalSMAND*:HalSMVariableType_HalSMAND,HalSMOR*:HalSMVariableType_HalSMOR,\
HalSMNegate*:HalSMVariableType_HalSMNegate,HalSMNegativeSign*:HalSMVariableType_HalSMNegativeSign,HalSMGOTO*:HalSMVariableType_HalSMGOTO,HalSMGOTOSector*:HalSMVariableType_HalSMGOTOSector,\
HalSMInlineIF*:HalSMVariableType_HalSMInlineIf,HalSMInlineElse*:HalSMVariableType_HalSMInlineElse,HalSMBreak*:HalSMVariableType_HalSMBreak,HalSMXOR*:HalSMVariableType_HalSMXOR,\
HalSMBAND*:HalSMVariableType_HalSMBAND,HalSMBOR*:HalSMVariableType_HalSMBOR,HalSMBNOT*:HalSMVariableType_HalSMBNOT,HalSMFindValue*:HalSMVariableType_fv,HalSMOP*:HalSMVariableType_HalSMOP)


#define HalSMVariable_auto(val) (HalSMVariable_init((val),typevar(val)))
#define HalSMVariable_AsVarAuto(var,arg) *(var)=*(__typeof__(*(var))*)(arg)->value;
#define HalSMVariable_GetValue(arg) ({\
    void* var;\
    if((arg)->type==HalSMVariableType_str){HalStringFormatChar* var;}\
    else if((arg)->type==HalSMVariableType_int){HalSMInteger var;}\
    else if((arg)->type==HalSMVariableType_char){HalStringFormatChar var;}\
    else if((arg)->type==HalSMVariableType_double){HalSMDouble var;}\
    else if((arg)->type==HalSMVariableType_HalSMArray){HalSMArray var;}\
    else if((arg)->type==HalSMVariableType_int_array){int* var;}\
    else if((arg)->type==HalSMVariableType_HalSMFunctionC){HalSMFunctionC var;}\
    else if((arg)->type==HalSMVariableType_HalSMRunClassC){HalSMRunClassC var;}\
    else if((arg)->type==HalSMVariableType_HalSMSetArg){HalSMSetArg var;}\
    else if((arg)->type==HalSMVariableType_HalSMError){HalSMError var;}\
    else if((arg)->type==HalSMVariableType_HalSMNull){HalSMNull var;}\
    else if((arg)->type==HalSMVariableType_HalSMRunFunc){HalSMRunFunc var;}\
    else if((arg)->type==HalSMVariableType_HalSMRunFuncC){HalSMRunFuncC var;}\
    else if((arg)->type==HalSMVariableType_HalSMLocalFunction){HalSMLocalFunction var;}\
    else if((arg)->type==HalSMVariableType_HalSMCModule){HalSMCModule var;}\
    else if((arg)->type==HalSMVariableType_HalSMModule){HalSMModule var;}\
    else if((arg)->type==HalSMVariableType_HalSMClassC){HalSMClassC var;}\
    else if((arg)->type==HalSMVariableType_HalSMCompiler){HalSMCompiler var;}\
    else if((arg)->type==HalSMVariableType_HalSMCompiler_source){HalSMCompiler* var;}\
    else if((arg)->type==HalSMVariableType_HalSMRunClassC_source){HalSMRunClassC* var;}\
    else if((arg)->type==HalSMVariableType_HalSMRunClass_source){HalSMRunClass* var;}\
    else if((arg)->type==HalSMVariableType_HalSMRunClass){HalSMRunClass var;}\
    else if((arg)->type==HalSMVariableType_HalSMDoubleGet){HalSMDoubleGet var;}\
    else if((arg)->type==HalSMVariableType_HalSMClass){HalSMClass var;}\
    else if((arg)->type==HalSMVariableType_HalSMVar){HalSMVar var;}\
    else if((arg)->type==HalSMVariableType_HalSMPlus){HalSMPlus var;}\
    else if((arg)->type==HalSMVariableType_HalSMMinus){HalSMMinus var;}\
    else if((arg)->type==HalSMVariableType_HalSMMult){HalSMMult var;}\
    else if((arg)->type==HalSMVariableType_HalSMDivide){HalSMDivide var;}\
    else if((arg)->type==HalSMVariableType_HalSMEqual){HalSMEqual var;}\
    else if((arg)->type==HalSMVariableType_HalSMNotEqual){HalSMNotEqual var;}\
    else if((arg)->type==HalSMVariableType_HalSMMore){HalSMMore var;}\
    else if((arg)->type==HalSMVariableType_HalSMLess){HalSMLess var;}\
    else if((arg)->type==HalSMVariableType_HalSMBool){unsigned char var;}\
    else if((arg)->type==HalSMVariableType_HalSMDict){Dict var;}\
    else if((arg)->type==HalSMVariableType_HalSMSetVar){HalSMSetVar var;}\
    else if((arg)->type==HalSMVariableType_HalSMReturn){HalSMReturn var;}\
    else if((arg)->type==HalSMVariableType_HalSMFunctionArray){HalSMFunctionArray var;}\
    else if((arg)->type==HalSMVariableType_unsigned_int){ui64 var;}\
    else if((arg)->type==HalSMVariableType_custom){HalSMVariableCustom var;}\
    else if((arg)->type==HalSMVariableType_HalSMMoreEqual){HalSMMoreEqual var;}\
    else if((arg)->type==HalSMVariableType_HalSMLessEqual){HalSMLessEqual var;}\
    else if((arg)->type==HalSMVariableType_HalSMShift){HalSMShift var;}\
    else if((arg)->type==HalSMVariableType_HalSMPow){HalSMPow var;}\
    else if((arg)->type==HalSMVariableType_HalSMModulo){HalSMModulo var;}\
    else if((arg)->type==HalSMVariableType_HalSMAND){HalSMAND var;}\
    else if((arg)->type==HalSMVariableType_HalSMOR){HalSMOR var;}\
    else if((arg)->type==HalSMVariableType_HalSMNegate){HalSMNegate var;}\
    else if((arg)->type==HalSMVariableType_HalSMNegativeSign){HalSMNegativeSign var;}\
    else if((arg)->type==HalSMVariableType_HalSMGOTO){HalSMGOTO var;}\
    else if((arg)->type==HalSMVariableType_HalSMGOTOSector){HalSMGOTOSector var;}\
    else if((arg)->type==HalSMVariableType_HalSMInlineIf){HalSMInlineIf var;}\
    else if((arg)->type==HalSMVariableType_HalSMInlineElse){HalSMInlineElse var;}\
    else if((arg)->type==HalSMVariableType_HalSMBreak){HalSMBreak var;}\
    else if((arg)->type==HalSMVariableType_HalSMXOR){HalSMXOR var;}\
    else if((arg)->type==HalSMVariableType_HalSMBAND){HalSMBAND var;}\
    else if((arg)->type==HalSMVariableType_HalSMBOR){HalSMBOR var;}\
    else if((arg)->type==HalSMVariableType_HalSMBNOT){HalSMBNOT var;}\
    else if((arg)->type==HalSMVariableType_fv){HalSMFindValue var;}\
    else if((arg)->type==HalSMVariableType_HalSMOP){HalSMOP var;}\
    __typeof__(var) out=*(__typeof__(var)*)(arg)->value;\
    out;\
})

#define HalSMVariable_FromValue(hsmmml,arg) ({\
    __typeof__((arg))* var=(__typeof__((arg))*)(hsmmml)->malloc(sizeof(__typeof__((arg))));\
    *var=(arg);\
    HalSMVariable_init(hsmmml,var,typevar(var));\
})

#define HalSMVariable_FromValueWithType(hsmmml,arg,type) ({\
    type* var=(type*)hsmmml->malloc(sizeof(type));\
    *var=(arg);\
    HalSMVariable_init(hsmmml,var,typevar(var));\
})

#define HalSMVariable_copy(hsmmml,arg) ({\
    __auto_type out=HalSMVariable_GetValue(arg);\
    __typeof__(out)* var=hsmmml->malloc(sizeof(__typeof__(out)));\
    *var=out;\
    HalSMVariable_init(hsmmml,var,(arg)->type);\
})

#define DictForEach(keyOutDictForEach,valueOutDictForEach,dict) \
    HalSMVariable* keyOutDictForEach=(dict)->elements[0]->key;HalSMVariable* valueOutDictForEach=(dict)->elements[0]->value;\
    for (ui64 indexDictForEach=0;indexDictForEach<(dict)->size;indexDictForEach++,keyOutDictForEach=(dict)->elements[indexDictForEach]->key,valueOutDictForEach=(dict)->elements[indexDictForEach]->value)

#define HalSMArrayForEach(elementHalSMArrayForEach,array) \
    HalSMVariable* elementHalSMArrayForEach=(array)->arr[0];\
    for (ui64 indexHalSMArrayForEach=0;indexHalSMArrayForEach<(array)->size;indexHalSMArrayForEach++,elementHalSMArrayForEach=(array)->arr[indexHalSMArrayForEach])

#define MathMin(a,b) ((a)<(b)?(a):(b))
#define MathMax(a,b) ((a)>(b)?(a):(b))
#define MathAbs(a) ((a)<0?-(a):(a))

#define MathCeilPos(a) ((a)-(int)(a)>0?(int)((a)+1):(int)(a))
#define MathCeilNeg(a) ((a)-(int)(a)<0?(int)((a)-1):(int)(a))
#define MathCeil(a) ((a)>0?MathCeilPos((a)):MathCeilNeg((a)))

#ifdef __cplusplus
}
#endif

#endif