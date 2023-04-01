// DatFile.cpp : implementation file
//

#include "stdafx.h"
#include "DatFileBase.h"
#include ".\datfilebase.h"

#if defined(_UNICODE) || defined(UNICODE)
#error UNICODE not supported
#endif

// Utility function
CString MakeLower(CString str)
{
	return str.MakeLower();
}

// CFileDescriptorBase
IMPLEMENT_DYNAMIC(CFileDescriptorBase, CObject)

CFileDescriptorBase::CFileDescriptorBase() :
	m_strFileName(""),
	m_nType(0),
	m_dwFileLength(0),
	m_dwPackedFileLength(0),
	m_dwOffset(0)
{
}

CFileDescriptorBase::CFileDescriptorBase(const CFileDescriptorBase& item) :
	m_strFileName(item.m_strFileName),
	m_nType(item.m_nType),
	m_dwFileLength(item.m_dwFileLength),
	m_dwPackedFileLength(item.m_dwPackedFileLength),
	m_dwOffset(item.m_dwOffset)
{
}

CFileDescriptorBase::~CFileDescriptorBase()
{
}

CFileDescriptorBase& CFileDescriptorBase::operator = (const CFileDescriptorBase& item)
{
	if (&item != this) {
		m_strFileName = item.m_strFileName;
		m_nType = item.m_nType;
		m_dwFileLength = item.m_dwFileLength;
		m_dwPackedFileLength = item.m_dwPackedFileLength;
		m_dwOffset = item.m_dwOffset;
	}

	return (*this);
}

void CFileDescriptorBase::Serialize(CArchive& ar)
{
	ASSERT(FALSE);
}

#ifdef _DEBUG
void CFileDescriptorBase::AssertValid() const
{
	CObject::AssertValid();
	ASSERT(!m_strFileName.IsEmpty());
	ASSERT(m_nType <= 1);
	ASSERT(m_dwFileLength != 0);
	ASSERT(m_dwPackedFileLength != 0);
	ASSERT(m_dwPackedFileLength >= m_dwFileLength);
}

void CFileDescriptorBase::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	dc << "File name length:\t" << m_strFileName.GetLength() << "\n";
	dc << "File name:\t\t\t" << m_strFileName << "\n";
	dc << "Type:\t\t\t\t" << m_nType << "\n";
	dc << "File length:\t\t\t" << m_dwFileLength << " (";
	dc.DumpAsHex(m_dwFileLength);
	dc << ")\n";
	dc << "Packed file length:\t" << m_dwPackedFileLength << " (";
	dc.DumpAsHex(m_dwPackedFileLength);
	dc << ")\n";
	dc << "Offset:\t\t\t\t"  << m_dwOffset << " (";
	dc.DumpAsHex(m_dwOffset);
	dc << ")\n";
}
#endif

// CDatFileBase
IMPLEMENT_DYNAMIC(CDatFileBase, CObject)

CDatFileBase::CDatFileBase() :
	m_strPattern("*"),
	m_bNeedFlush(FALSE),
	m_nCurrent(-1)
{
}

CDatFileBase::~CDatFileBase()
{
}

DWORD CDatFileBase::GetLength(void)
{
	return DWORD(m_Dat.GetLength());
}

INT_PTR CDatFileBase::FindFirst(LPCTSTR lpszPattern)
{
	m_strPattern = lpszPattern;
	m_nCurrent = -1;
	return FindNext();
}

INT_PTR CDatFileBase::GetCurrentFileIndex()
{
	return m_nCurrent;
}

CString CDatFileBase::GetFindFilePattern()
{
	return m_strPattern;
}

BOOL CDatFileBase::MatchPattern(LPCTSTR lpszString, LPCTSTR lpszPattern, BOOL bCaseSensitive)
{
	char c, p;

	for(; ;) {
		switch(p = ConvertCase(*lpszPattern++, bCaseSensitive)) {
			case 0:								// end of lpszPattern
				return *lpszString ? FALSE : TRUE;	// if end of lpszString TRUE

			case '*':
				while (*lpszString) {				// match zero or more char
					if (MatchPattern(lpszString++, lpszPattern, bCaseSensitive))
						return TRUE; 
				}

				return MatchPattern (lpszString, lpszPattern, bCaseSensitive );

			case '?':
				if (*lpszString++ == 0)				// match any one char
					return FALSE;				// not end of lpszString

				break;

			default:
				c = ConvertCase(*lpszString++, bCaseSensitive);

				if (c != p)						// check for exact char
					return FALSE;				// not a match

				break;
		}
	}
}

char CDatFileBase::ConvertCase(char c, BOOL bCaseSensetive)
{
	return  bCaseSensetive ? c : char(CharUpper(LPTSTR(c)));
}
