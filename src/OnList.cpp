#include "stdafx.h"
#include "dat2.h"

int OnList()
{
	CFileDescriptorBase descriptor;
	DWORD dwTotalSize = 0;

	printf("   Length      Packed     Type   Name\n");
	printf(" ---------- ----------- -------- ---------------\n");

	for(INT_PTR i = 0; i < g_pDatFile->GetFileCount(); i++) {
		descriptor = (*g_pDatFile)[i];
		dwTotalSize += descriptor.m_dwFileLength;
		printf("%11u %11u  %s  %s\n",
				descriptor.m_dwFileLength,
				descriptor.m_dwPackedFileLength,
				(descriptor.m_nType) ? "Packed" : "Stored",
				descriptor.m_strFileName);
	}

	printf(" ----------                      ---------------\n");
	printf("%11u                        %u file(s)\n",
			dwTotalSize,
			DWORD(g_pDatFile->GetFileCount()));

	return 0;
}