// DatFile1.cpp : implementation file
//

#include "stdafx.h"
#include "DatFile1.h"
#include "LZ77C.h"
#include "Util.h"


#if defined(_UNICODE) || defined(UNICODE)
#error UNICODE not supported
#endif


// CFileList1::CFileTreeItem
CFileList1::CFileTreeItem::CFileTreeItem()
{
}

CFileList1::CFileTreeItem::CFileTreeItem(const CFileList1::CFileTreeItem& item)
{
	m_strDirectoryName = item.m_strDirectoryName;
	m_FileList.Copy(item.m_FileList);
}

CFileList1::CFileTreeItem& CFileList1::CFileTreeItem::operator = (const CFileList1::CFileTreeItem& item)
{
	if (&item != this) {
		m_strDirectoryName = item.m_strDirectoryName;
		m_FileList.Copy(item.m_FileList);
	}

	return (*this);
}

// CFileList1
IMPLEMENT_DYNAMIC(CFileList1, CObject)

CFileList1::CFileList1()
{
}

CFileList1::~CFileList1()
{
}

void CFileList1::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		// Directories count
		DWORD dwDirCount = DWORD(m_FileTree.GetSize());
		WriteMSBDWord(ar, dwDirCount);

		// Unknown fields
		WriteMSBDWord(ar, dwDirCount);
		WriteMSBDWord(ar, 0);
		WriteMSBDWord(ar, 0);

		// Directories names
		CString strDirName;

		for(DWORD i = 0; i < dwDirCount; i++) {
			strDirName = m_FileTree[i].m_strDirectoryName;

			if (strDirName.IsEmpty()) {
				strDirName = ".";
			}

			WriteString(ar, strDirName);
		}

		// Directories descriptors
		for(DWORD i = 0; i < dwDirCount; i++) {
			// FileCount
			DWORD dwFileCount = DWORD(m_FileTree[i].m_FileList.GetSize());

			WriteMSBDWord(ar, dwFileCount);

			// Unknown fields
			WriteMSBDWord(ar, dwFileCount);
			WriteMSBDWord(ar, 16);
			WriteMSBDWord(ar, 0);

			// File descriptors
			CString strFileName;
			CFileDescriptorBase descriptor;
			DWORD dwFlags;
			DWORD dwPackedFileLength;

			for(DWORD j = 0; j < dwFileCount; j++) {
				descriptor = m_FileTree[i].m_FileList[j];
				strFileName = ExtractFileName(descriptor.m_strFileName);

				WriteString(ar, strFileName);

				if (descriptor.m_nType == 1) {
					dwFlags = 0x40;
					dwPackedFileLength = descriptor.m_dwPackedFileLength;
				}
				else {
					dwFlags = 0x20;
					dwPackedFileLength = 0;
				}

				WriteMSBDWord(ar, dwFlags);

				WriteMSBDWord(ar, descriptor.m_dwOffset);
				WriteMSBDWord(ar, descriptor.m_dwFileLength);
				WriteMSBDWord(ar, dwPackedFileLength);
			}
		}
	}
	else {
		m_FileTree.RemoveAll();

		// Directories count
		DWORD dwDirCount;

		if (ReadMSBDWord(ar, dwDirCount) != sizeof(DWORD)) {
			AfxThrowUserException();
		}

		// Unknown fields
		DWORD dwUnknown;

		for(DWORD u = 0; u < 3; u++) {
			if (ReadMSBDWord(ar, dwUnknown) != sizeof(DWORD)) {
				AfxThrowUserException();
			}
		}

		// Directories names
		CFileTreeItem directory;
		CString strDirName;


		for(DWORD i = 0; i < dwDirCount; i++) {
			if (!ReadString(ar, strDirName)) {
				AfxThrowUserException();
			}

			if (strDirName == ".") {
				strDirName = "";
			}

			directory.m_strDirectoryName = strDirName;
			m_FileTree.Add(directory);
		}

		// Directories descriptors
		for(DWORD i = 0; i < dwDirCount; i++) {
			// FileCount
			DWORD dwFileCount;

			if (ReadMSBDWord(ar, dwFileCount) != sizeof(DWORD)) {
				AfxThrowUserException();
			}

			// Unknown fields
			for(DWORD u = 0; u < 3; u++) {
				if (ReadMSBDWord(ar, dwUnknown) != sizeof(DWORD)) {
					AfxThrowUserException();
				}
			}

			// File descriptors
			CFileDescriptorBase descriptor;
			DWORD dwFlags;

			for(DWORD j = 0; j < dwFileCount; j++) {
				if (!ReadString(ar, descriptor.m_strFileName)) {
					AfxThrowUserException();
				}

				if (ReadMSBDWord(ar, dwFlags) != sizeof(DWORD)) {
					AfxThrowUserException();
				}

				if (ReadMSBDWord(ar, descriptor.m_dwOffset) != sizeof(DWORD)) {
					AfxThrowUserException();
				}

				if (ReadMSBDWord(ar, descriptor.m_dwFileLength) != sizeof(DWORD)) {
					AfxThrowUserException();
				}

				if (ReadMSBDWord(ar, descriptor.m_dwPackedFileLength) != sizeof(DWORD)) {
					AfxThrowUserException();
				}

				if (dwFlags == 0x40) {
					descriptor.m_nType = 1;
				}
				else {
					descriptor.m_nType = 0;
					descriptor.m_dwPackedFileLength = descriptor.m_dwFileLength;
				}

				if (!m_FileTree[i].m_strDirectoryName.IsEmpty()) {
					descriptor.m_strFileName = m_FileTree[i].m_strDirectoryName + "\\" + descriptor.m_strFileName;
				}

				m_FileTree[i].m_FileList.Add(descriptor);
			}
		}
	}
}

INT_PTR CFileList1::GetSize() const
{
	INT_PTR nResult = 0;

	for(INT_PTR i = 0; i < m_FileTree.GetSize(); i++) {
		nResult += m_FileTree[i].m_FileList.GetSize();
	}

	return nResult;
}

DWORD CFileList1::GetLength()
{
	DWORD dwResult = sizeof(DWORD) * 4;
	CString strDirectoryName;

	for(INT_PTR i = 0; i < m_FileTree.GetSize(); i++) {
		strDirectoryName = m_FileTree[i].m_strDirectoryName;

		if (strDirectoryName.IsEmpty()) {
			strDirectoryName = ".";
		}

		dwResult += strDirectoryName.GetLength() + 1;
		dwResult += sizeof(DWORD) * 4;

		for(INT_PTR j = 0; j < m_FileTree[i].m_FileList.GetSize(); j++) {
			dwResult += ExtractFileName(m_FileTree[i].m_FileList[j].m_strFileName).GetLength() + 1;
			dwResult += sizeof(DWORD) * 4;
		}
	}

	return dwResult;
}


INT_PTR CFileList1::Add(const CFileDescriptorBase& newElement)
{
	CString strDirectoryName = ExtractFilePath(newElement.m_strFileName, FALSE);
	CString strFileName = ExtractFileName(newElement.m_strFileName);

	if (!strDirectoryName.IsEmpty()) {
		strDirectoryName = strDirectoryName.Left(strDirectoryName.GetLength() - 1);
	}

	// ========= Directory =========
	INT_PTR nDirectoryIndex;
	CFileTreeItem directory;

	directory.m_strDirectoryName = strDirectoryName;

	// No elements
	if (m_FileTree.IsEmpty()) {
		nDirectoryIndex = m_FileTree.Add(directory);
	}
	else if (MakeLower(strDirectoryName) <
		MakeLower(m_FileTree[0].m_strDirectoryName)) {
		// Less than first
		m_FileTree.InsertAt(0, directory);
		nDirectoryIndex = 0;
	}
	else if (MakeLower(strDirectoryName) >
			 MakeLower(m_FileTree[m_FileTree.GetUpperBound()].m_strDirectoryName)) {
		// Greater than last
		nDirectoryIndex = m_FileTree.Add(directory);
	}
	else if (MakeLower(strDirectoryName) ==
			 MakeLower(m_FileTree[0].m_strDirectoryName)) {
		// Equal first
		nDirectoryIndex = 0;
	}
	else if (MakeLower(strDirectoryName) ==
			 MakeLower(m_FileTree[m_FileTree.GetUpperBound()].m_strDirectoryName)) {
		// Equal last
		nDirectoryIndex = m_FileTree.GetUpperBound();
	}
	else {
		// Other
		BOOL bNeedInsert = TRUE;
		INT_PTR nLeft = 0;
		INT_PTR nRight = m_FileTree.GetUpperBound();
		INT_PTR nCurrent;

		while(nRight - nLeft > 1) {
			nCurrent = (nRight + nLeft) >> 1; // divide by 2

			if (MakeLower(strDirectoryName) == MakeLower(m_FileTree[nCurrent].m_strDirectoryName)) {
				bNeedInsert = FALSE;
				break;
			}
			else if (MakeLower(strDirectoryName) < MakeLower(m_FileTree[nCurrent].m_strDirectoryName)) {
				nRight = nCurrent;
			}
			else {
				nLeft = nCurrent;
			}
		}

		if (bNeedInsert) {
			m_FileTree.InsertAt(nLeft + 1, directory);
			nDirectoryIndex = nLeft + 1;
		}
		else {
			nDirectoryIndex = nCurrent;
		}
	}

	// ========= File =========
	INT_PTR nFileIndex;


	if (m_FileTree[nDirectoryIndex].m_FileList.IsEmpty()) {
		// No elements
		nFileIndex = m_FileTree[nDirectoryIndex].m_FileList.Add(newElement);
	}
	else if (MakeLower(newElement.m_strFileName) < 
			 MakeLower(m_FileTree[nDirectoryIndex].m_FileList[0].m_strFileName)) {
		// Less than first
		m_FileTree[nDirectoryIndex].m_FileList.InsertAt(0, newElement);
		nFileIndex = 0;
	}
	else if (MakeLower(newElement.m_strFileName) > 
			 MakeLower(m_FileTree[nDirectoryIndex].m_FileList[m_FileTree[nDirectoryIndex].m_FileList.GetUpperBound()].m_strFileName)) {
		// Greater than last
		nFileIndex = m_FileTree[nDirectoryIndex].m_FileList.Add(newElement);
	}
	else if (MakeLower(newElement.m_strFileName) ==
			 MakeLower(m_FileTree[nDirectoryIndex].m_FileList[0].m_strFileName)) {
		// Equal first
		m_FileTree[nDirectoryIndex].m_FileList[0] = newElement;
		nFileIndex = 0;
	}
	else if (MakeLower(newElement.m_strFileName) ==
			 MakeLower(m_FileTree[nDirectoryIndex].m_FileList[m_FileTree[nDirectoryIndex].m_FileList.GetUpperBound()].m_strFileName)) {
		// Equal last
		m_FileTree[nDirectoryIndex].m_FileList[m_FileTree[nDirectoryIndex].m_FileList.GetUpperBound()] = newElement;
		nFileIndex = m_FileTree[nDirectoryIndex].m_FileList.GetUpperBound();
	}
	else {
		// Other
		BOOL bNeedInsert = TRUE;
		INT_PTR nLeft = 0;
		INT_PTR nRight = m_FileTree[nDirectoryIndex].m_FileList.GetUpperBound();
		INT_PTR nCurrent;
		
		while(nRight - nLeft > 1) {
			nCurrent = (nRight + nLeft) >> 1; // divide by 2

			if (MakeLower(newElement.m_strFileName) == MakeLower(m_FileTree[nDirectoryIndex].m_FileList[nCurrent].m_strFileName)) {
				m_FileTree[nDirectoryIndex].m_FileList[nCurrent] = newElement;
				bNeedInsert = FALSE;
				break;
			}
			else if (MakeLower(newElement.m_strFileName) < MakeLower(m_FileTree[nDirectoryIndex].m_FileList[nCurrent].m_strFileName)) {
				nRight = nCurrent;
			}
			else {
				nLeft = nCurrent;
			}
		}

		if (bNeedInsert) {
			m_FileTree[nDirectoryIndex].m_FileList.InsertAt(nLeft + 1, newElement);
			nFileIndex = nLeft + 1;
		}
		else {
			nFileIndex = nCurrent;
		}
	}

	INT_PTR nResult = 0;

	for(INT_PTR i = 0; i < nDirectoryIndex; i++) {
		nResult += m_FileTree[nDirectoryIndex].m_FileList.GetSize();
	}

	nResult += nFileIndex;

	return nResult;
}

void CFileList1::RemoveAt(INT_PTR nIndex)
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetSize());
	
	for(INT_PTR nDirectoryIndex = 0; nDirectoryIndex < m_FileTree.GetSize(); nDirectoryIndex++) {
		if (nIndex < m_FileTree[nDirectoryIndex].m_FileList.GetSize()) {
			m_FileTree[nDirectoryIndex].m_FileList.RemoveAt(nIndex);

			if (m_FileTree[nDirectoryIndex].m_FileList.IsEmpty()) {
				m_FileTree.RemoveAt(nDirectoryIndex);
			}

			break;
		}
		else {
			nIndex -= m_FileTree[nDirectoryIndex].m_FileList.GetSize();
		}
	}
}

CFileDescriptorBase& CFileList1::operator [] (INT_PTR nIndex)
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetSize());
	
	for(INT_PTR nDirectoryIndex = 0; nDirectoryIndex < m_FileTree.GetSize(); nDirectoryIndex++) {
		if (nIndex < m_FileTree[nDirectoryIndex].m_FileList.GetSize()) {
			return m_FileTree[nDirectoryIndex].m_FileList[nIndex];
		}
		else {
			nIndex -= m_FileTree[nDirectoryIndex].m_FileList.GetSize();
		}
	}

	// Fake return (avoid warning)
	return m_FileTree[0].m_FileList[0];
}

CFileDescriptorBase CFileList1::operator [] (INT_PTR nIndex) const
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetSize());

	for(INT_PTR nDirectoryIndex = 0; nDirectoryIndex < m_FileTree.GetSize(); nDirectoryIndex++) {
		if (nIndex < m_FileTree[nDirectoryIndex].m_FileList.GetSize()) {
			return m_FileTree[nDirectoryIndex].m_FileList[nIndex];
		}
		else {
			nIndex -= m_FileTree[nDirectoryIndex].m_FileList.GetSize();
		}
	}

	// Fake return (avoid warning)
	return m_FileTree[0].m_FileList[0];
}

#ifdef _DEBUG
void CFileList1::AssertValid() const
{
	CObject::AssertValid();
	m_FileTree.AssertValid();
	
	for(INT_PTR i = 0; i < GetSize(); i++) {
		(operator [] (i)).AssertValid();
	}
}

template<>
void AFXAPI DumpElements<CFileDescriptorBase>(CDumpContext& dc, const CFileDescriptorBase* pElements, INT_PTR nCount)
{
	ASSERT(nCount == 0 ||
		   AfxIsValidAddress(pElements, (size_t)nCount * sizeof(CFileDescriptorBase), FALSE));

	for(INT_PTR i = 0; i < nCount; i++) {
		dc << "Element " << i << ":\n";
		pElements[i].Dump(dc);
	}
}

void CFileList1::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	dc.SetDepth(1);
	m_FileTree.Dump(dc);
}
#endif


// CDatFile1
IMPLEMENT_DYNAMIC(CDatFile1, CObject)

CDatFile1::CDatFile1()
{
}

CDatFile1::~CDatFile1()
{
	Close();
}

BOOL CDatFile1::Open(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	if (!m_Dat.Open(lpszFileName, nOpenFlags)) {
		return FALSE;
	}

	if (m_Dat.GetLength() == 0) {
		return TRUE;
	}

	CArchive arFileList(&m_Dat, CArchive::load);

	TRY {
		m_FileList.Serialize(arFileList);
	}

	CATCH(CUserException, e) {
		return FALSE;
	}

	END_CATCH

	return TRUE;
}

void CDatFile1::Close()
{
	Flush();

	if (m_Dat.m_hFile != CFile::hFileNull) {
		m_Dat.Close();
	}
}

void CDatFile1::Flush()
{
	if (m_bNeedFlush) {
		FreeSpaceForCatalog();
		m_Dat.SeekToBegin();
		CArchive arFileList(&m_Dat, CArchive::store);
		m_FileList.Serialize(arFileList);
		arFileList.Flush();
		m_bNeedFlush = FALSE;
	}
}

INT_PTR CDatFile1::GetFileCount()
{
	return m_FileList.GetSize();
}

CFileDescriptorBase CDatFile1::operator [](INT_PTR nIndex) const
{
	return m_FileList[nIndex];
}

INT_PTR CDatFile1::FindNext()
{
	for(++m_nCurrent; m_nCurrent < m_FileList.GetSize(); m_nCurrent++) {
		if (MatchPattern(m_FileList[m_nCurrent].m_strFileName, m_strPattern, FALSE)) {
			break;
		}
	}

	if (m_nCurrent == m_FileList.GetSize()) {
		m_nCurrent = -1;
	}

	return m_nCurrent;
}

BOOL CDatFile1::Inflate(CArchive& ar)
{
	if (m_nCurrent == -1) {
		return FALSE;
	}

	CFileDescriptorBase descriptor = m_FileList[m_nCurrent];

	m_Dat.Seek(descriptor.m_dwOffset, CFile::begin);

	const DWORD c_dwBufferSize = 65536;
	BYTE buffer[c_dwBufferSize];

	DWORD dwBytesLeft = descriptor.m_dwPackedFileLength;
	DWORD dwBytesToRead;
	DWORD dwRead;
	DWORD dwBytesOut = 0;

	// Uncompressed file
	if (descriptor.m_nType == 0) {
		do {
			dwBytesToRead = (dwBytesLeft < c_dwBufferSize) ? dwBytesLeft : c_dwBufferSize;
			dwRead = m_Dat.Read(buffer, dwBytesToRead);

			if (dwRead != dwBytesToRead) {
				return FALSE;
			}

			ar.Write(buffer, dwBytesToRead);
			dwBytesOut += dwRead;
		} while((dwBytesLeft -= dwRead ) > 0);

		return TRUE;
	}

	BOOL bResult = TRUE;
	BOOL bEndLoop = FALSE;

	do {
		WORD wBlockDescriptor;

		if (ReadMSBWord(m_Dat, wBlockDescriptor) != sizeof(WORD)) {
			bResult = FALSE;
			break;
		}

		dwBytesToRead = wBlockDescriptor & 0x7FFF;

		dwRead = m_Dat.Read(buffer, dwBytesToRead);

		if (dwRead != dwBytesToRead) {
			bResult = FALSE;
			break;
		}

		if (wBlockDescriptor & 0x8000) {
			// Uncompressed block
			ar.Write(buffer, dwRead);
		}
		else {
			// Compressed block
			LZSSDecodeBuffer(buffer, dwRead, ar);
		}
	} while((dwBytesLeft -= dwRead + sizeof(WORD)) > 0);

	return bResult;
}

BOOL CDatFile1::Deflate(CArchive& ar, DWORD dwLength, LPCTSTR lpszName, int level)
{
	ULONGLONG ullDatLength = m_Dat.SeekToEnd();

	// File descriptor
	CFileDescriptorBase descriptor;
	descriptor.m_dwFileLength = dwLength;
	descriptor.m_strFileName = lpszName;
	descriptor.m_dwOffset = DWORD(ullDatLength);

	// Buffers
	const DWORD c_dwBufferSize = 65536;
	BYTE buffer[c_dwBufferSize];
//	BYTE outBuffer[c_dwBufferSize];

	DWORD dwBytesLeft = dwLength;
	DWORD dwBytesToRead;
	DWORD dwRead;
	DWORD dwBytesOut = 0;

	// Flags
	BOOL bResult = TRUE;
	BOOL bUseInputArchive = TRUE;

	// Temporary file
	char lpszTempFileName[MAX_PATH];
	CFile fileTemp;
	BOOL bDeleteTempFile = FALSE;

	// Try compress file
	if (level != 0) {
		// Save position in input archive
		ar.Flush();
		ULONGLONG ullArPosition = ar.GetFile()->GetPosition();

		// Create temporary file
		char lpszTempPath[MAX_PATH - 14];
		HANDLE hTempFile;

		GetTempPath(MAX_PATH - 14, lpszTempPath);
		GetTempFileName(lpszTempPath, "DAT", 0, lpszTempFileName);

		hTempFile = ::CreateFile(lpszTempFileName,
									GENERIC_READ | GENERIC_WRITE,
									FILE_SHARE_READ,
									NULL,
									CREATE_ALWAYS,
									FILE_ATTRIBUTE_TEMPORARY,
									NULL);

		if (hTempFile == INVALID_HANDLE_VALUE) {
			return FALSE;
		}

		fileTemp.m_hFile = hTempFile;
		bDeleteTempFile = TRUE;

		// Deflate file
		const DWORD c_dwBlockSize = 0x4000;
		WORD wBlockDescriptor = 0;
		ULONGLONG ullBlockDescPos;

		do {
			// Read block of data
			dwBytesToRead = (dwBytesLeft < c_dwBlockSize) ? dwBytesLeft : c_dwBlockSize;

			dwRead = ar.Read(buffer, dwBytesToRead);

			if (dwRead != dwBytesToRead) {
				bResult = FALSE;
				break;
			}

			// Write descriptor placeholder
			ullBlockDescPos = fileTemp.GetPosition();
			WriteMSBWord(fileTemp, wBlockDescriptor);

			// Compress block
			LZSSEncodeBuffer(buffer, dwRead, fileTemp);

			// Check length of compressed block
			DWORD dwBlockLength = DWORD(fileTemp.GetPosition()) - DWORD(ullBlockDescPos) - sizeof(WORD);

			// More than uncompressed
			if (dwBlockLength > dwRead) {
				fileTemp.SetLength(ullBlockDescPos + sizeof(WORD));
				fileTemp.SeekToEnd();
				fileTemp.Write(buffer, dwRead);
				wBlockDescriptor = WORD(dwRead) | 0x8000;
			}
			else {
				wBlockDescriptor = WORD(dwBlockLength);
			}

			// Patch descriptor
			fileTemp.Seek(ullBlockDescPos, CFile::begin);
			WriteMSBWord(fileTemp, wBlockDescriptor);
			fileTemp.SeekToEnd();
		} while((dwBytesLeft -= dwRead) > 0);

		fileTemp.SeekToBegin();

		// Check compression result
		if (!bResult) {
			fileTemp.Close();
			CFile::Remove(lpszTempFileName);
			return FALSE;
		}

		// Check length
		ULONGLONG ullTempFileLength = fileTemp.GetLength();

		if (ullTempFileLength < dwLength) {
			bUseInputArchive = FALSE;
			descriptor.m_dwPackedFileLength = DWORD(ullTempFileLength);
			descriptor.m_nType = 1;
			dwBytesLeft = DWORD(ullTempFileLength);
		}
		else {
			ar.Flush();
			ar.GetFile()->Seek(ullArPosition, CFile::begin);
			descriptor.m_dwPackedFileLength = dwLength;
			descriptor.m_nType = 0;
			dwBytesLeft = dwLength;
		}
	}
	else {
		descriptor.m_dwPackedFileLength = dwLength;
		descriptor.m_nType = 0;
	}

	// Set source archive
	CArchive arTemp(&fileTemp, CArchive::load);		// Temporary archive
	CArchive* pArSource;

	if (bUseInputArchive) {
		pArSource = &ar;
	}
	else {
		pArSource = &arTemp;
	}

	// Write data
	TRY {
		m_Dat.SeekToEnd();

		do {
			dwBytesToRead = (dwBytesLeft < c_dwBufferSize) ? dwBytesLeft : c_dwBufferSize;
			dwRead = pArSource->Read(buffer, dwBytesToRead);

			if (dwRead != dwBytesToRead) {
				bResult = FALSE;
				break;
			}

			m_Dat.Write(buffer, dwBytesToRead);
			dwBytesOut += dwRead;
		} while((dwBytesLeft -= dwRead ) > 0);

		if (bResult) {
			// Add descriptor
			m_FileList.Add(descriptor);
			m_bNeedFlush = TRUE;
		}
		else {
			// Restore dat-file length on error
			m_Dat.SetLength(ullDatLength);
		}

		// Destroy temporary file
		if (bDeleteTempFile) {
			arTemp.Abort();
			fileTemp.Close();
			CFile::Remove(lpszTempFileName);
		}

		return bResult;
	}

	CATCH_ALL(e) {
		m_Dat.SetLength(ullDatLength);
		THROW_LAST();
	}

	END_CATCH_ALL
}

BOOL CDatFile1::Delete()
{
	if (m_nCurrent == -1) {
		return FALSE;
	}

	m_FileList.RemoveAt(m_nCurrent);
	m_nCurrent = -1;
	m_bNeedFlush = TRUE;

	return TRUE;
}


CDatFile1* g_pDatFile1ForSort;

int CompareDescriptorIndex1(const void* pIndex1, const void* pIndex2)
{
	INT_PTR nIndex1 = *((INT_PTR*)pIndex1);
	INT_PTR nIndex2 = *((INT_PTR*)pIndex2);

	CFileDescriptorBase descriptor1;
	CFileDescriptorBase descriptor2;

	descriptor1 = (*g_pDatFile1ForSort)[nIndex1];
	descriptor2 = (*g_pDatFile1ForSort)[nIndex2];

	if (descriptor1.m_dwOffset < descriptor2.m_dwOffset) {
		return -1;
	}
	else if (descriptor1.m_dwOffset > descriptor2.m_dwOffset) {
		return 1;
	}
	else {
		return 0;
	}
}

BOOL CDatFile1::Shrink(void (*pCallback)(INT_PTR nIndex))
{
	if (m_Dat.GetLength() == 0) {
		return FALSE;
	}

	// Sort files by offset
	INT_PTR nFileCount = GetFileCount();
	INT_PTR* pIndexArray = new INT_PTR[nFileCount];

	if (pIndexArray == NULL) {
		return FALSE;
	}

	for(INT_PTR i = 0; i < nFileCount; i++) {
		pIndexArray[i] = i;
	}

	g_pDatFile1ForSort = this;
	qsort(pIndexArray, nFileCount, sizeof(INT_PTR), CompareDescriptorIndex1);

	// Move files
	const DWORD c_dwBufferSize = 65536;
	BYTE buffer[c_dwBufferSize];

	DWORD dwWritePos = m_FileList.GetLength();
	DWORD dwShift;
	DWORD dwBytesToRead;
	DWORD dwBytesLeft;
	DWORD dwReadPos;
	DWORD dwRead;

	CFileDescriptorBase descriptor;

	for(INT_PTR i = 0; i < nFileCount; i++) {
		descriptor = (*this)[pIndexArray[i]];
		dwShift = descriptor.m_dwOffset - dwWritePos;

		if (dwShift > 0) {
			m_bNeedFlush = TRUE;

			if (pCallback) {
				pCallback(pIndexArray[i]);
			}

			dwBytesLeft = descriptor.m_dwPackedFileLength;
			dwReadPos = descriptor.m_dwOffset;

			descriptor.m_dwOffset = dwWritePos;

			do {
				dwBytesToRead = (dwBytesLeft < c_dwBufferSize) ? dwBytesLeft : c_dwBufferSize;

				m_Dat.Seek(dwReadPos, CFile::begin);
				dwRead = m_Dat.Read(buffer, dwBytesToRead);

				if (dwRead != dwBytesToRead) {
					delete [] pIndexArray;
					return FALSE;
				}

				m_Dat.Seek(dwWritePos, CFile::begin);
				m_Dat.Write(buffer, dwRead);
				dwWritePos += dwRead;
				dwReadPos += dwRead;
			} while((dwBytesLeft -= dwRead ) > 0);

			// Patch offsets
			m_FileList[pIndexArray[i]].m_dwOffset = descriptor.m_dwOffset;
		}
		else {
			dwWritePos += descriptor.m_dwPackedFileLength;
		}
	}

	m_Dat.SetLength(dwWritePos);
	delete [] pIndexArray;

	Flush();
	return TRUE;
}

void CDatFile1::FreeSpaceForCatalog()
{
	if (m_FileList.GetSize() != 0) {
		DWORD dwCatalogSize = m_FileList.GetLength();

		// Find offset
		CFileDescriptorBase descriptor;
		DWORD dwMinimalOffset = m_FileList[0].m_dwOffset;

		for(INT_PTR i = 1; i < m_FileList.GetSize(); i++) {
			descriptor = m_FileList[i];

			if (dwMinimalOffset > descriptor.m_dwOffset) {
				dwMinimalOffset = descriptor.m_dwOffset;
			}
		}

		// Free space
		if (dwMinimalOffset < dwCatalogSize) {
			printf("Freeing space for catalog...\n");
			DWORD dwShift = dwCatalogSize - dwMinimalOffset;
			DWORD dwBytesLeft = DWORD(m_Dat.GetLength()) - dwMinimalOffset;
			DWORD dwTotalBytesToMove = dwBytesLeft;
			DWORD dwReadPos = DWORD(m_Dat.GetLength());
			DWORD dwWritePos;

			const DWORD c_dwBufferSize = 65536;
			BYTE buffer[c_dwBufferSize];
			DWORD dwBytesToRead;
			DWORD dwRead;

			do {
				dwBytesToRead = (dwBytesLeft < c_dwBufferSize) ? dwBytesLeft : c_dwBufferSize;
				dwReadPos -= dwBytesToRead;
				dwWritePos = dwReadPos + dwShift;

				m_Dat.Seek(dwReadPos, CFile::begin);
				dwRead = m_Dat.Read(buffer, dwBytesToRead);

				m_Dat.Seek(dwWritePos, CFile::begin);
				m_Dat.Write(buffer, dwRead);

				dwBytesLeft -= dwRead;
				printf("Moved: %d of %d byte(s)\r", dwTotalBytesToMove - dwBytesLeft, dwTotalBytesToMove);
			} while(dwBytesLeft > 0);

			// Patch offsets
			for(INT_PTR i = 0; i < m_FileList.GetSize(); i++) {
				m_FileList[i].m_dwOffset += dwShift;
			}
		}

		printf("\nSpace are freed.\n");
	}
}
