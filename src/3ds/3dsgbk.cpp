#include <cstdio>
#include <cstdlib>

#include <stdio.h>
#include <3ds.h>

#include "3dstypes.h"

uint8 *fontGBK = NULL;

void gbk3dsLoadGBKImage() {
	const int fontGridSize = 16;
	const int gbkRowStart = 0xa1;
	const int gbkRowEnd = 0xf7;
	const int gbkColStart = 0xa1;
	const int gbkColEnd = 0xfe;
	const int bpp = 32;	// 32 bit of one px color
	const int colorByteCount = bpp / 8;

	const int bufHeadSize = 14 + 40;
	const int rowCount = (gbkRowEnd - gbkRowStart + 1);
	const int colCount = (gbkColEnd - gbkColStart + 1);
	const int dataSize = rowCount * colCount * (fontGridSize * fontGridSize) * colorByteCount;

	// load from bin file
	FILE *file = fopen("/emus3ds/gbk.bin", "rb");
	if (file == NULL) return;
	fseek(file, 0, SEEK_END);
	off_t size = ftell(file);

	const int rstBufSize = rowCount * colCount * (fontGridSize * fontGridSize);
	if (size != rstBufSize) {
		fclose(file);
		return;
	}

	fseek(file, 0, SEEK_SET);
	fontGBK = (uint8 *)malloc(rstBufSize);
	off_t bytesRead = fread(fontGBK, sizeof(uint8), rstBufSize, file);

	fclose(file);
	return;
}
