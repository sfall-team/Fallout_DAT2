#include "stdafx.h"
#include "dat2.h"


DWORD dwTotalMoved = 0;

void OutInfo(INT_PTR nIndex)
{
	CFileDescriptorBase descriptor;
	descriptor = (*g_pDatFile)[nIndex];


	dwTotalMoved++;
	printf("Moving: %s\n", descriptor.m_strFileName);
}


int OnShrink()
{
	DWORD dwOldLength = g_pDatFile->GetLength();

	if (g_pDatFile->Shrink(OutInfo)) {
		printf("----------\n");
		printf("%u file(s) moved. Old size: %u bytes. New size: %u bytes \n", dwTotalMoved, dwOldLength, g_pDatFile->GetLength());
		return 0;
	}
	else {
		printf("Error: Unable shrink file\n");
		return 5;
	}
}