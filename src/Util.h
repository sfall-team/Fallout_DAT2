#pragma once

CString ExtractFileName(const CString& strFileName);
CString ExtractFilePath(const CString& strFileName, BOOL bIncludeDrive = TRUE);
BOOL DirectoryExist(const CString& strDirectory);
BOOL ForceCreateDirectory(CString strDirectory);

UINT ReadMSBWord(CArchive& ar, WORD& wValue);
UINT ReadMSBDWord(CArchive& ar, DWORD& dwValue);

UINT ReadMSBWord(CFile& file, WORD& wValue);
UINT ReadMSBDWord(CFile& file, DWORD& dwValue);

BOOL ReadString(CArchive& ar, CString& strString);
BOOL ReadString(CFile& file, CString& strString);

void WriteMSBWord(CArchive& ar, WORD wValue);
void WriteMSBDWord(CArchive& ar, DWORD dwValue);

void WriteMSBWord(CFile& file, WORD wValue);
void WriteMSBDWord(CFile& file, DWORD dwValue);

void WriteString(CArchive& ar, CString& strString);
void WriteString(CFile& file, CString& strString);
