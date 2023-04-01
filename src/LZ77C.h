#ifndef LZ77C_H
#define LZ77C_H

void LZSSEncodeBuffer(BYTE* buffer, DWORD dwInputLength, CFile& fileOut);
void LZSSDecodeBuffer(BYTE* buffer, DWORD dwInputLength, CArchive& arOut);

#endif /* LZ77C_H */
