#ifndef HALHASHC_H
#define HALHASHC_H

#define HALHASH_VERSION "0.0.2"
#define HALHASHC_VERSION "0.0.3"
//0.0.2 - Str2Hash по байтам UTF-8, без утечек памяти
//0.0.3 - без HalSM и HalStringFormat: вход и выход в байтах, память у каждого HalHash своя

//Хэширование Халварсинга, результат равен HalHash.js (HalNet/api) и HalHash.java.
//Глобальных переменных нет: один HalHash можно использовать из нескольких потоков,
//если его функции памяти потокобезопасны (malloc/calloc/free стандартной библиотеки - да).
typedef struct HalHash {
    void*(*malloc)(unsigned long long size);
    void*(*calloc)(unsigned long long count,unsigned long long size);
    void(*free)(void* p);
} HalHash;

/**
 * Задаёт функции памяти для HalHash
 *
 * @param hh HalHash
 * @param mallocFunc Выделение памяти
 * @param callocFunc Выделение памяти, заполненной нулями
 * @param freeFunc Освобождение памяти
 * @return void
 */
void HalHash_init(HalHash* hh,void*(*mallocFunc)(unsigned long long),void*(*callocFunc)(unsigned long long,unsigned long long),void(*freeFunc)(void*));

/**
 * Хэш строки UTF-8 (как Str2Hash в HalHash.js)
 *
 * @param hh HalHash
 * @param str Строка UTF-8, заканчивается нулём
 * @param length Длина хэша в hex-символах
 * @param count Количество кусков
 * @param maxIteration Предел итераций подгонки длины
 * @return Хэш (hex в нижнем регистре, заканчивается нулём), освобождать через hh->free
 */
char* HalHash_Str2Hash(HalHash* hh,const char* str,unsigned int length,unsigned int count,unsigned long long maxIteration);

/**
 * Байты в hex (как Bin2Hex в HalHash.js)
 *
 * @param hh HalHash
 * @param bytes Байты
 * @param size Количество байтов
 * @return hex в нижнем регистре, заканчивается нулём, освобождать через hh->free
 */
char* HalHash_Bin2Hex(HalHash* hh,const unsigned char* bytes,unsigned long long size);

/**
 * Хэш байтов (как Bin2Hash в HalHash.js)
 *
 * @param hh HalHash
 * @param bytes Байты, может быть 0 при size=0
 * @param size Количество байтов
 * @param length Длина хэша в hex-символах
 * @param count Количество кусков
 * @param maxIteration Предел итераций подгонки длины
 * @return Хэш (hex в нижнем регистре, заканчивается нулём), освобождать через hh->free
 */
char* HalHash_Bin2Hash(HalHash* hh,const unsigned char* bytes,unsigned long long size,unsigned int length,unsigned int count,unsigned long long maxIteration);

#endif
