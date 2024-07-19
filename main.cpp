#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <string.h>

#include <tiny_text_renderer.h>

long read_font_file(const char* file_name, char** buffer) {
    FILE* file = fopen(file_name, "rb");
    if (!file) {
        fprintf(stderr, "Could not open file\n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *buffer = (char*)malloc(file_size);
    if (!*buffer) {
        fprintf(stderr, "Could not allocate buffer\n");
        fclose(file);
        return -1;
    }

    fread(*buffer, 1, file_size, file);
    fclose(file);

    return file_size;
}

void write_bitmap(const char* file_name, uint8_t* pixels, unsigned int width, unsigned int height) {
    #pragma pack(push,1)
    struct BmpHeader {
        char bitmapSignatureBytes[2] = {'B', 'M'};
        uint32_t sizeOfBitmapFile = 54;
        uint32_t reservedBytes = 0;
        uint32_t pixelDataOffset = 54;
    } bmpHeader;
    #pragma pack(pop)

    #pragma pack(push,1)
    struct BmpInfoHeader {
        uint32_t sizeOfThisHeader = 40;
        int32_t width = 0; // in pixels
        int32_t height = 0; // in pixels
        uint16_t numberOfColorPlanes = 1; // must be 1
        uint16_t colorDepth = 24;
        uint32_t compressionMethod = 0;
        uint32_t rawBitmapDataSize = 0; // generally ignored
        int32_t horizontalResolution = 0; // in pixel per meter
        int32_t verticalResolution = 0; // in pixel per meter
        uint32_t colorTableEntries = 0;
        uint32_t importantColors = 0;
    } bmpInfoHeader;
    #pragma pack(pop)

    bmpHeader.sizeOfBitmapFile += width * height * 3;
    bmpInfoHeader.width = width;
    bmpInfoHeader.height = -height;

    FILE* file = fopen(file_name, "wb");
    if (!file) {
        fprintf(stderr, "Could not open file\n");
        return;
    }

    fwrite(&bmpHeader, sizeof(bmpHeader), 1, file);
    fwrite(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, file);

    int stride = 4 * (((width * 3) + 3) / 4);
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            fputc(~pixels[i * width + j], file);
            fputc(~pixels[i * width + j], file);
            fputc(~pixels[i * width + j], file);
        }
        fwrite("\0\0\0", 1, stride - (width * 3), file);
    }

    fclose(file);
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <font.ttf> <size> \"text\"\n", argv[0]);
        return 1;
    }

    const char* file_name = argv[1];
    unsigned int size = atoi(argv[2]);
    const char* text = argv[3];

    char* font_data;
    long file_data_size = read_font_file(file_name, &font_data);

    if (file_data_size <= 0) {
        fprintf(stderr, "Failed to read font file: %s\n", file_name);
        return 1;
    }

    hb_font_t* font = create_font(font_data, file_data_size, size);

    unsigned int width, height, baseline;
    measure_text(font, text, &width, &height, &baseline);

    uint8_t* pixels = (uint8_t*)malloc(width * height);
    memset(pixels, 0, width * height);
    draw_text_on_buffer(font, text, 0, 0, width, height, pixels);

    write_bitmap("/tmp/output.bmp", pixels, width, height);

    destroy_font(font);

    printf("Width: %d, Height: %d, Baseline: %d\n", width, height, baseline);

    return 0;
}