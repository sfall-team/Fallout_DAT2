#pragma once

#include "DatFileBase.h"

// CFileList1
class CFileList1 : public CObject {
public:
	CFileList1();
	virtual ~CFileList1();

	DECLARE_DYNAMIC(CFileList1)

	virtual void Serialize(CArchive& ar);
	INT_PTR GetSize() const;

	DWORD GetLength();

	INT_PTR Add(const CFileDescriptorBase& newElement);
	void RemoveAt(INT_PTR nIndex);

	CFileDescriptorBase& operator [] (INT_PTR nIndex);
	CFileDescriptorBase  operator [] (INT_PTR nIndex) const;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	// Assignment and copy not allowed
	CFileList1(const CFileList1&);
	CFileList1& operator = (const CFileList1&);

private:
	// CFileDescriptorBaseArray
	typedef CArray<CFileDescriptorBase, const CFileDescriptorBase&> CFileDescriptorBaseArray;

	// CFileTreeItem
	class CFileTreeItem {
	public:
		CFileTreeItem();
		CFileTreeItem(const CFileTreeItem& item);

	public:
		CFileTreeItem& operator = (const CFileTreeItem& item);

	public:
		CString m_strDirectoryName;
		CFileDescriptorBaseArray m_FileList;
	};

	// CFileTree
	typedef CArray<CFileTreeItem, const CFileTreeItem&> CFileTree;

	CFileTree m_FileTree;
};

// CDatFile1
class CDatFile1 : public CDatFileBase {
public:
	CDatFile1();
	virtual ~CDatFile1();

	DECLARE_DYNAMIC(CDatFile1)

	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags);
	virtual void Close();
	virtual void Flush();

	virtual INT_PTR GetFileCount();
	virtual CFileDescriptorBase operator [] (INT_PTR nIndex) const;

	virtual INT_PTR FindNext();

	virtual BOOL Inflate(CArchive& ar);
	virtual BOOL Deflate(CArchive& ar, DWORD dwLength, LPCTSTR lpszName, int level);
	virtual BOOL Delete();
	virtual BOOL Shrink(void (*pCallback)(INT_PTR nIndex));

private:
	void FreeSpaceForCatalog();

private:
	// Restricted operations
	CDatFile1(const CDatFile1&);
	CDatFile1& operator = (const CDatFile1&);
	virtual void Serialize(CArchive&) {};

private:
	CFileList1 m_FileList;
};
