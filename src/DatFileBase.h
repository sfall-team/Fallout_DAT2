#pragma once

// CFileDescriptorBase
class CFileDescriptorBase : public CObject {
public:
	CFileDescriptorBase();
	CFileDescriptorBase(const CFileDescriptorBase& item);
	virtual ~CFileDescriptorBase();

	DECLARE_DYNAMIC(CFileDescriptorBase)

	virtual void Serialize(CArchive& ar);

	CFileDescriptorBase& operator = (const CFileDescriptorBase& item);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	CString m_strFileName;
	BYTE  m_nType;
	DWORD m_dwFileLength;
	DWORD m_dwPackedFileLength;
	DWORD m_dwOffset;
};


// CDatFileBase
class CDatFileBase : public CObject {
public:
	CDatFileBase();
	virtual ~CDatFileBase();

	DECLARE_DYNAMIC(CDatFileBase)

	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags) = 0;
	virtual void Close() = 0;
	virtual void Flush() = 0;

	DWORD GetLength();

	virtual INT_PTR GetFileCount() = 0;
	virtual CFileDescriptorBase operator [](INT_PTR nIndex) const = 0;

	INT_PTR FindFirst(LPCTSTR lpszPattern);
	virtual INT_PTR FindNext() = 0;
	INT_PTR GetCurrentFileIndex();
	CString GetFindFilePattern();

	virtual BOOL Inflate(CArchive& ar) = 0;
	virtual BOOL Deflate(CArchive& ar, DWORD dwLength, LPCTSTR lpszName, int level) = 0;
	virtual BOOL Delete() = 0;
	virtual BOOL Shrink(void (*pCallback)(INT_PTR nIndex)) = 0;

protected:
	BOOL MatchPattern(LPCTSTR lpszString, LPCTSTR lpszPattern, BOOL bCaseSensitive);
	char ConvertCase(char c, BOOL bCaseSensetive);

protected:
	CString m_strPattern;
	CFile m_Dat;

	BOOL m_bNeedFlush;
	INT_PTR m_nCurrent;
};


// Utility function
CString MakeLower(CString str);
