
#include <cstdio>
// #include <cstring>
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
	const int bpp = 32;				//32 bit of one px color
	const int colorByteCount = bpp / 8;

	const int bufHeadSize = 14 + 40;
	const int rowCount = (gbkRowEnd - gbkRowStart + 1);
	const int colCount = (gbkColEnd - gbkColStart + 1);
	const int dataSize = rowCount * colCount * (fontGridSize * fontGridSize) * colorByteCount;

	// laod from bin file

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




	// load from bmp file

	// FILE *file = fopen("/3ds/emus/gbk.bmp", "rb");
	// if (file == NULL) return;

	// // seek to end of file
	// fseek(file, 0, SEEK_END);

	// // file pointer tells us the size
	// off_t size = ftell(file);
	// if (size != (bufHeadSize + dataSize)) {
	// 	fclose(file);
	// 	return;
	// }

	// // seek back to start
	// fseek(file, 0, SEEK_SET);

	// //read head
	// uint8 bufHead[bufHeadSize];
	// off_t bytesRead = fread(bufHead, sizeof(uint8), bufHeadSize, file);
	// if (bufHeadSize != bytesRead) {
	// 	fclose(file);
	// 	return;
	// }

	// // ushort dataOffset = (bufHead[11] << 8) + bufHead[10];

	// //output buffer temp
	// const int rstBufSize = rowCount * colCount * (fontGridSize * fontGridSize);
	// // uint8 fontGBKTmp[rstBufSize];
	// fontGBK = (uint8 *)malloc(rstBufSize);

	// //read data
	// const int bufSize = fontGridSize * fontGridSize * colCount * colorByteCount;
	// const int bufPxWidth = fontGridSize * colCount * colorByteCount;
	// // uint8 buf[bufSize];
	// // uint8 buf[1024];
	// uint8 *buf = (uint8 *)malloc(bufSize);
	// // erg each word
	// for (int i = gbkRowStart; i <= gbkRowEnd; ++i) {
	// 	off_t bytesRead = fread(buf, sizeof(uint8), bufSize, file);
	// 	if (bufSize != bytesRead) {
	// 		free(buf);
	// 		free(fontGBK);
	// 		fclose(file);
	// 		fontGBK = NULL;
	// 		return;
	// 	}
	// 	int fontPxCount = fontGridSize * fontGridSize;

	// 	for (int j = gbkColStart; j <= gbkColEnd; ++j) {
	// 		int rstStartIdx = ((i - gbkRowStart) * colCount + (j - gbkColStart)) * (fontGridSize * fontGridSize);
	// 		int fontStartIdx = (j - gbkColStart) * fontGridSize * colorByteCount;
	// 		// erg pixel of one word
	// 		for (int r = fontGridSize - 1; r >= 0; --r) {
	// 			int rStartIdx = r * bufPxWidth;
	// 			for (int c = 0; c < fontGridSize; ++c) {
	// 				int idx = rStartIdx + fontStartIdx + c * 4;
	// 				int rstIdx = rstStartIdx + (fontGridSize - 1 - r) * fontGridSize + c;
	// 				fontGBK[rstIdx] = buf[idx] * 8 / 255;
	// 			}
	// 		}
	// 	}
	// }

	// free(buf);
	// fclose(file);
}
