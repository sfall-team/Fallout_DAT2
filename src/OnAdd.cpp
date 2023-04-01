#include "stdafx.h"
#include "dat2.h"
#include "Util.h"

extern CString g_strTargetDir;

DWORD g_dwTotalAdded;

BOOL ProcessTree(CString strPattern, CString strRoot, BOOL bRecursive)
{
	BOOL bResult = TRUE;

	CString strPath = ExtractFilePath(strPattern);
	CString strFilePattern = ExtractFileName(strPattern);

	WIN32_FIND_DATA fd;
	HANDLE hSearchHandle = FindFirstFile(strPattern, &fd);

	if (hSearchHandle != INVALID_HANDLE_VALUE) {
		do {
			if (CString(fd.cFileName) != CString(_T(".")) && 
				CString(fd.cFileName) != CString(_T(".."))) {

				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (bRecursive) {
						ProcessTree(strPath + fd.cFileName + "\\" + strFilePattern,
									strRoot.IsEmpty() ? fd.cFileName : strRoot + "\\" + fd.cFileName, // + _T("\\"),
									bRecursive);
					}
				}
				else {
					CFile fileIn;

					if (fileIn.Open(strPath + fd.cFileName, CFile::modeRead | CFile::shareDenyWrite)) {
						CArchive arIn(&fileIn, CArchive::load);
						CString strFileDescriptorName = g_strTargetDir;
						strFileDescriptorName += strRoot.IsEmpty() ? fd.cFileName : strRoot + _T("\\") + fd.cFileName;

						printf("Adding: %s", strFileDescriptorName);
						
						if (g_pDatFile->Deflate(arIn, DWORD(fileIn.GetLength()), 
											  strFileDescriptorName, g_nCompressMethod)) {
							printf("\n");
							g_dwTotalAdded++;
						}
						else {
							printf(" Error!!!\n");
							bResult = FALSE;
							break;
						}
					}
					else {
						bResult = FALSE;
						break;
					}
				}
			}
		} while(FindNextFile(hSearchHandle, &fd));

		FindClose(hSearchHandle);
		return bResult;
	}
	else  {
		return FALSE;
	}
}

int OnAdd()
{
	g_dwTotalAdded = 0;

	if (g_pDatFile->Open(g_strDatFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite | CFile::shareDenyWrite)) {
		for(INT_PTR j = 0; j < g_FileList.GetCount(); j++) {
			CString strRoot = ExtractFilePath(g_FileList[j], FALSE);

			if (strRoot[0] == '\\') {
				strRoot = strRoot.Right(strRoot.GetLength() - 1);
			}

			if (strRoot.Right(1) == '\\') {
				strRoot = strRoot.Left(strRoot.GetLength() - 1);
			}

			if (!ProcessTree(g_FileList[j], strRoot, g_bRecurseIntoDir)) {
				return 4;
			}
		}

		printf("----------\n");
		printf("%u file(s) added\n", g_dwTotalAdded);

		return 0;
	}
	else {
		printf("Error: Unable open file %s\n", LPCTSTR(g_strDatFileName));
		return 1;
	}
}