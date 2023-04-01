#pragma once

#include "DatFileBase.h"

// CFileDescriptor2
class CFileDescriptor2 : public CFileDescriptorBase {
public:
	CFileDescriptor2();
	CFileDescriptor2(const CFileDescriptor2& item);
	virtual ~CFileDescriptor2();

	DECLARE_DYNAMIC(CFileDescriptor2)

	virtual void Serialize(CArchive& ar);
	int GetSize() const;

	CFileDescriptor2& operator = (const CFileDescriptor2& item);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};


// CFileList2
class CFileList2 : public CObject {
public:
	CFileList2();
	virtual ~CFileList2();

	DECLARE_DYNAMIC(CFileList2)

	virtual void Serialize(CArchive& ar);
	INT_PTR GetSize() const;

	INT_PTR Add(const CFileDescriptor2& newElement);
	void RemoveAt(INT_PTR nIndex);

	CFileDescriptor2& operator [] (INT_PTR nIndex);
	CFileDescriptor2  operator [] (INT_PTR nIndex) const;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	// Assignment and copy not allowed
	CFileList2(const CFileList2&);
	CFileList2& operator = (const CFileList2&);

private:
	typedef CArray<CFileDescriptor2> CFileDescriptor2Array;

	CFileDescriptor2Array m_FileList;
};

// CDatFile2
class CDatFile2 : public CDatFileBase {
public:
	CDatFile2();
	virtual ~CDatFile2();

	DECLARE_DYNAMIC(CDatFile2)

	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags);
	virtual void Close();
	virtual void Flush();

	virtual INT_PTR GetFileCount();
	virtual CFileDescriptorBase operator [](INT_PTR nIndex) const;

	virtual INT_PTR FindNext();

	virtual BOOL Inflate(CArchive& ar);
	virtual BOOL Deflate(CArchive& ar, DWORD dwLength, LPCTSTR lpszName, int level);
	virtual BOOL Delete();

	virtual BOOL Shrink(void (*pCallback)(INT_PTR nIndex));

private:
	// Restricted operations
	CDatFile2(const CDatFile2&);
	CDatFile2& operator = (const CDatFile2&);
	virtual void Serialize(CArchive&) {};

private:
	CFileList2 m_FileList;
};
