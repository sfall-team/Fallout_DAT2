#include "stdafx.h"
#include "dat2.h"

int OnDelete()
{
	CFileDescriptorBase descriptor;
	DWORD dwTotalDeleted = 0;

	for(INT_PTR j = 0; j < g_FileList.GetCount(); j++) {
		if (g_pDatFile->FindFirst(g_FileList[j]) != -1) {
			do {
				descriptor = (*g_pDatFile)[g_pDatFile->GetCurrentFileIndex()];
				printf("Deleting: %s", descriptor.m_strFileName);

				if (g_pDatFile->Delete()) {
					dwTotalDeleted++;
					printf("\n");
				}
				else {
					printf(" Error!!!\n");
					return 2;
				}
			} while(g_pDatFile->FindNext() != -1);
		}
	}

	printf("----------\n");
	printf("%u file(s) deleted\n", dwTotalDeleted);

	return 0;
}