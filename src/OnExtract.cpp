#include "stdafx.h"
#include "dat2.h"
#include "Util.h"

int OnExtract()
{
	CFileDescriptorBase descriptor;
	DWORD dwTotalExtracted = 0;
	CString strDirectory;
	CString strFileName;
	int nLastSlashPos;
	CString strOutFileName;
	CFile fileOut;

	// Create output directory
	if (!g_strOutFolder.IsEmpty()) {
		if (!ForceCreateDirectory(g_strOutFolder)) {
			printf("Error: Unable create output directory\n");
			return 3;
		}
	}

	for(INT_PTR j = 0; j < g_FileList.GetCount(); j++) {
		if (g_pDatFile->FindFirst(g_FileList[j]) != -1) {
			do {
				descriptor = (*g_pDatFile)[g_pDatFile->GetCurrentFileIndex()];
				printf("Extracting: %s", descriptor.m_strFileName);
				nLastSlashPos = descriptor.m_strFileName.ReverseFind('\\');
				strDirectory = descriptor.m_strFileName.Left(nLastSlashPos + 1);
				strFileName = descriptor.m_strFileName.Right(descriptor.m_strFileName.GetLength() - nLastSlashPos - 1);


				if (g_bExtractWithoutPaths) {
					strOutFileName = g_strOutFolder + strFileName;
				}
				else {
					if (!ForceCreateDirectory(g_strOutFolder + strDirectory)) {
						return 3;
					}

					strOutFileName = g_strOutFolder + strDirectory + strFileName;
				}

				if (fileOut.Open(strOutFileName, CFile::modeCreate | CFile::modeWrite)) {
					CArchive arOut(&fileOut, CArchive::store);

					if (g_pDatFile->Inflate(arOut)) {
						dwTotalExtracted++;
						printf("\n");
					}
					else {
						printf(" Error!!!\n");
						return 3;
					}
				}
				else {
					printf(" Error!!! Unable open out file %s\n", strOutFileName);
					return 3;
				}

				fileOut.Close();
			} while(g_pDatFile->FindNext() != -1);
		}
	}

	printf("----------\n");
	printf("%u file(s) extracted\n", dwTotalExtracted);
	return 0;
}