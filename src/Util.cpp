#include "stdafx.h"
#include "Util.h"

CString ExtractFilePath(const CString& strFileName, BOOL bIncludeDrive)
{
	CString strDrive;
	CString strDirectory;
	LPTSTR lpszDrive = strDrive.GetBufferSetLength(_MAX_DRIVE);
	LPTSTR lpszDirectory = strDirectory.GetBufferSetLength(_MAX_DIR);
	_splitpath(strFileName, lpszDrive, lpszDirectory, NULL, NULL);
	strDrive.ReleaseBuffer();
	strDirectory.ReleaseBuffer();

	if (bIncludeDrive) {
		return (strDrive + strDirectory);
	}
	else {
		return strDirectory;
	}
}

CString ExtractFileName(const CString& strFileName)
{
	CString strName;
	CString strExtension;
	LPTSTR lpszName = strName.GetBufferSetLength(_MAX_FNAME);
	LPTSTR lpszExtension = strExtension.GetBufferSetLength(_MAX_EXT);
	_splitpath(strFileName, NULL, NULL, lpszName, lpszExtension);
	strName.ReleaseBuffer();
	strExtension.ReleaseBuffer();
	return (strName + strExtension);
}

BOOL DirectoryExist(const CString& strDirectory)
{
	DWORD dwResult = ::GetFileAttributes(strDirectory);
	return ((dwResult != INVALID_FILE_ATTRIBUTES) && 
			((dwResult & FILE_ATTRIBUTE_DIRECTORY) != 0));
}

BOOL ForceCreateDirectory(CString strDirectory)
{
	if (strDirectory.GetLength() == 0) {
		return TRUE;
	}

	if (strDirectory.Right(1) == _T("\\")) {
		strDirectory = strDirectory.Left(strDirectory.GetLength() - 1);
	}

	if ((strDirectory.GetLength() < 3) ||
		DirectoryExist(strDirectory) ||
		(ExtractFilePath(strDirectory) == strDirectory)) {
		return TRUE;
	}

	return (ForceCreateDirectory(ExtractFilePath(strDirectory)) && 
			::CreateDirectory(strDirectory, NULL));
}

UINT ReadMSBWord(CArchive& ar, WORD& wValue)
{
	BYTE* pBuffer = reinterpret_cast<BYTE*>(&wValue);
	return (ar.Read(pBuffer + 1, 1) + ar.Read(pBuffer, 1));
}

UINT ReadMSBDWord(CArchive& ar, DWORD& dwValue)
{
	BYTE* pBuffer = reinterpret_cast<BYTE*>(&dwValue);
	return (ar.Read(pBuffer + 3, 1) + ar.Read(pBuffer + 2, 1) +
			ar.Read(pBuffer + 1, 1) + ar.Read(pBuffer, 1));
}

UINT ReadMSBWord(CFile& file, WORD& wValue)
{
	BYTE* pBuffer = reinterpret_cast<BYTE*>(&wValue);
	return (file.Read(pBuffer + 1, 1) + file.Read(pBuffer, 1));
}

UINT ReadMSBDWord(CFile& file, DWORD& dwValue)
{
	BYTE* pBuffer = reinterpret_cast<BYTE*>(&dwValue);
	return (file.Read(pBuffer + 3, 1) + file.Read(pBuffer + 2, 1) +
			file.Read(pBuffer + 1, 1) + file.Read(pBuffer, 1));
}

BOOL ReadString(CArchive& ar, CString& strString)
{
	BYTE nStrLength;

	if (ar.Read(&nStrLength, 1) != 1) {
		return FALSE;
	}

	LPTSTR lpszString = strString.GetBufferSetLength(nStrLength + 1);
	DWORD dwRead = ar.Read(lpszString, nStrLength);
	strString.ReleaseBuffer(nStrLength);

	if (dwRead != nStrLength) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

BOOL ReadString(CFile& file, CString& strString)
{
	BYTE nStrLength;

	if (file.Read(&nStrLength, 1) != 1) {
		return FALSE;
	}

	LPTSTR lpszString = strString.GetBufferSetLength(nStrLength + 1);
	DWORD dwRead = file.Read(lpszString, nStrLength);
	strString.ReleaseBuffer(nStrLength);

	if (dwRead != nStrLength) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

void WriteMSBWord(CArchive& ar, WORD wValue)
{
	BYTE* pBuffer = reinterpret_cast<BYTE*>(&wValue);

	ar.Write(pBuffer + 1, 1);
	ar.Write(pBuffer, 1);
}

void WriteMSBDWord(CArchive& ar, DWORD dwValue)
{
	BYTE* pBuffer = reinterpret_cast<BYTE*>(&dwValue);

	ar.Write(pBuffer + 3, 1);
	ar.Write(pBuffer + 2, 1);
	ar.Write(pBuffer + 1, 1);
	ar.Write(pBuffer, 1);
}

void WriteMSBWord(CFile& file, WORD wValue)
{
	BYTE* pBuffer = reinterpret_cast<BYTE*>(&wValue);

	file.Write(pBuffer + 1, 1);
	file.Write(pBuffer, 1);
}

void WriteMSBDWord(CFile& file, DWORD dwValue)
{
	BYTE* pBuffer = reinterpret_cast<BYTE*>(&dwValue);

	file.Write(pBuffer + 3, 1);
	file.Write(pBuffer + 2, 1);
	file.Write(pBuffer + 1, 1);
	file.Write(pBuffer, 1);
}

void WriteString(CArchive& ar, CString& strString)
{
	BYTE nStrLength = BYTE(strString.GetLength());

	ar.Write(&nStrLength, 1);
	ar.Write(LPCTSTR(strString), nStrLength);
}

void WriteString(CFile& file, CString& strString)
{
	BYTE nStrLength = BYTE(strString.GetLength());

	file.Write(&nStrLength, 1);
	file.Write(LPCTSTR(strString), nStrLength);
}
