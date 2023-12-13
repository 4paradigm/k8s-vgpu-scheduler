#ifndef __CUDA_ADDITION_FUNC_H__
#define __CUDA_ADDITION_FUNC_H__

#include <cuda.h>

typedef int (*ExportedFunction)();

#endif

typedef struct {
    uint16_t kind;
    uint16_t version;
    uint32_t header_size;
    uint32_t padded_payload_size;
    uint32_t unknown0; // check if it's written into separately
    uint32_t payload_size;
    uint32_t unknown1;
    uint32_t unknown2;
    uint32_t sm_version;
    uint32_t bit_width;
    uint32_t unknown3;
    uint64_t unknown4;
    uint64_t unknown5;
    uint64_t uncompressed_payload;
} FatbinFileHeader;


typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t header_size;
    uint64_t file_size;
} FatbinHeader;

typedef struct {
    uint32_t magic;
    uint32_t version;
    FatbinHeader *data;
    void *filename_or_fatbins;
} FatbincWrapper;
