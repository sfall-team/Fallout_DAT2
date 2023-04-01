// DatFile2.cpp : implementation file
//

#include "stdafx.h"
#include "DatFile2.h"

#include "zlib\zlib.h"

#if defined(_UNICODE) || defined(UNICODE)
#error UNICODE not supported
#endif


// CFileDescriptor2
IMPLEMENT_DYNAMIC(CFileDescriptor2, CFileDescriptorBase)

CFileDescriptor2::CFileDescriptor2()
{
}

CFileDescriptor2::CFileDescriptor2(const CFileDescriptor2& item) :
	CFileDescriptorBase(item)
{
}

CFileDescriptor2::~CFileDescriptor2()
{
}

CFileDescriptor2& CFileDescriptor2::operator = (const CFileDescriptor2& item)
{
	CFileDescriptorBase::operator = (item);
	return (*this);
}

void CFileDescriptor2::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		ASSERT(!m_strFileName.IsEmpty());

		DWORD dwFileNameLength = m_strFileName.GetLength();

		ar.Write(&dwFileNameLength, sizeof(dwFileNameLength));
		ar.WriteString(m_strFileName);
		ar.Write(&m_nType, sizeof(m_nType));
		ar.Write(&m_dwFileLength, sizeof(m_dwFileLength));
		ar.Write(&m_dwPackedFileLength, sizeof(m_dwPackedFileLength));
		ar.Write(&m_dwOffset, sizeof(m_dwOffset));
	}
	else {
		DWORD dwFileNameLength;
		LPTSTR lpszFileName;

		ar.Read(&dwFileNameLength, sizeof(dwFileNameLength));
		lpszFileName = m_strFileName.GetBufferSetLength(dwFileNameLength);
		ar.Read(lpszFileName, dwFileNameLength);
		m_strFileName.ReleaseBufferSetLength(dwFileNameLength);

		ar.Read(&m_nType,  sizeof(m_nType));
		ar.Read(&m_dwFileLength,  sizeof(m_dwFileLength));
		ar.Read(&m_dwPackedFileLength,  sizeof(m_dwPackedFileLength));
		ar.Read(&m_dwOffset,  sizeof(m_dwOffset));
	}
}

int CFileDescriptor2::GetSize() const
{
	return  sizeof(DWORD) + 
			m_strFileName.GetLength() + 
			sizeof(m_nType) +
			sizeof(m_dwFileLength) +
			sizeof(m_dwPackedFileLength) +
			sizeof(m_dwOffset);
}

#ifdef _DEBUG
void CFileDescriptor2::AssertValid() const
{
	CObject::AssertValid();
	ASSERT(!m_strFileName.IsEmpty());
	ASSERT(m_nType <= 1);
	ASSERT(m_dwFileLength != 0);
	ASSERT(m_dwPackedFileLength != 0);
	ASSERT(m_dwPackedFileLength >= m_dwFileLength);
}

void CFileDescriptor2::Dump(CDumpContext& dc) const
{
	CFileDescriptorBase::Dump(dc);
	dc << "Size of descriptor:\t" << GetSize() << " bytes\n";
}
#endif


// CFileList2
IMPLEMENT_DYNAMIC(CFileList2, CObject)

CFileList2::CFileList2()
{
}

CFileList2::~CFileList2()
{
}

void CFileList2::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
		DWORD dwFileCount = DWORD(m_FileList.GetSize());
		ar.Write(&dwFileCount, sizeof(dwFileCount));

		CFileDescriptor2 descriptor;
		DWORD dwListLength = 0;
		
		for(DWORD i = 0; i < dwFileCount; i++) {
			descriptor = m_FileList[i];
			descriptor.Serialize(ar);
			dwListLength += descriptor.GetSize();
		}

		dwListLength += sizeof(DWORD);
		ar.Write(&dwListLength, sizeof(dwListLength));
	}
	else {
		DWORD dwFileCount;
		ar.Read(&dwFileCount, sizeof(dwFileCount));
		m_FileList.SetSize(dwFileCount);

		CFileDescriptor2 descriptor;
		DWORD dwListLength;
		
		for(DWORD i = 0; i < dwFileCount; i++) {
			descriptor.Serialize(ar);
			m_FileList[i] = descriptor;
		}

		ar.Read(&dwListLength, sizeof(dwListLength));
	}
}

INT_PTR CFileList2::GetSize() const
{
	return m_FileList.GetSize();
}

INT_PTR CFileList2::Add(const CFileDescriptor2& newElement)
{
	// No elements
	if (m_FileList.GetSize() == 0) {
		return m_FileList.Add(newElement);
	}

	// Less than first
	if (MakeLower(newElement.m_strFileName) < 
		MakeLower(m_FileList[0].m_strFileName)) {
		m_FileList.InsertAt(0, newElement);
		return 0;
	}

	// Greater than last
	if (MakeLower(newElement.m_strFileName) > 
		MakeLower(m_FileList[m_FileList.GetUpperBound()].m_strFileName)) {
		m_FileList.Add(newElement);
		return m_FileList.GetUpperBound();
	}

	// Equally first
	if (MakeLower(newElement.m_strFileName) ==
		MakeLower(m_FileList[0].m_strFileName)) {
		m_FileList[0] = newElement;
		return 0;
	}

	// Equally last
	if (MakeLower(newElement.m_strFileName) ==
		MakeLower(m_FileList[m_FileList.GetUpperBound()].m_strFileName)) {
		m_FileList[m_FileList.GetUpperBound()] = newElement;
		return m_FileList.GetUpperBound();
	}

	// Other
	INT_PTR nLeft = 0;
	INT_PTR nRight = m_FileList.GetUpperBound();
	INT_PTR nCurrent;
	
	while(nRight - nLeft > 1) {
		nCurrent = (nRight + nLeft) >> 1; // divide by 2

		if (MakeLower(newElement.m_strFileName) == MakeLower(m_FileList[nCurrent].m_strFileName)) {
			m_FileList[nCurrent] = newElement;
			return nCurrent;
		}
		else if (MakeLower(newElement.m_strFileName) < MakeLower(m_FileList[nCurrent].m_strFileName)) {
			nRight = nCurrent;
		}
		else {
			nLeft = nCurrent;
		}
	}

	m_FileList.InsertAt(nLeft + 1, newElement);
	return (nLeft + 1);
}

void CFileList2::RemoveAt(INT_PTR nIndex)
{
	m_FileList.RemoveAt(nIndex);
}

CFileDescriptor2& CFileList2::operator [] (INT_PTR nIndex)
{
	return m_FileList[nIndex];
}

CFileDescriptor2  CFileList2::operator [] (INT_PTR nIndex) const
{
	return m_FileList[nIndex];
}

#ifdef _DEBUG
void CFileList2::AssertValid() const
{
	CObject::AssertValid();
	m_FileList.AssertValid();
	
	for(INT_PTR i = 0; i < m_FileList.GetSize(); i++) {
		m_FileList[i].AssertValid();
	}
}

template<>
void AFXAPI DumpElements<CFileDescriptor2>(CDumpContext& dc, const CFileDescriptor2* pElements, INT_PTR nCount)
{
	ASSERT(nCount == 0 ||
		   AfxIsValidAddress(pElements, (size_t)nCount * sizeof(CFileDescriptor2), FALSE));

	for(INT_PTR i = 0; i < nCount; i++) {
		dc << "Element " << i << ":\n";
		pElements[i].Dump(dc);
	}
}

void CFileList2::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
	dc.SetDepth(1);
	m_FileList.Dump(dc);
}
#endif


// CDatFile2
IMPLEMENT_DYNAMIC(CDatFile2, CObject)

CDatFile2::CDatFile2()
{
}

CDatFile2::~CDatFile2()
{
	Close();
}

BOOL CDatFile2::Open(LPCTSTR lpszFileName, UINT nOpenFlags)
{
	if (!m_Dat.Open(lpszFileName, nOpenFlags)) {
		return FALSE;
	}

	if (m_Dat.GetLength() == 0) {
		return TRUE;
	}

	DWORD dwDatLength;
	DWORD dwFileListLength;

	ULONGLONG ullFileLength = m_Dat.GetLength();

	if (ullFileLength < 12) {
		m_Dat.Close();
		return FALSE;
	}

	m_Dat.Seek(-8, CFile::end);
	m_Dat.Read(&dwFileListLength, sizeof(dwFileListLength));
	m_Dat.Read(&dwDatLength, sizeof(dwDatLength));

	if ((dwDatLength < 12) || (dwDatLength != ullFileLength)) {
		m_Dat.Close();
		return FALSE;
	}

	m_Dat.Seek(dwDatLength - dwFileListLength - 8, CFile::begin);

	CArchive arFileList(&m_Dat, CArchive::load);

	m_FileList.Serialize(arFileList);
	return TRUE;
}

void CDatFile2::Close()
{
	Flush();

	if (m_Dat.m_hFile != CFile::hFileNull) {
		m_Dat.Close();
	}
}

void CDatFile2::Flush()
{
	if (m_bNeedFlush) {
		ULONGLONG ullLength;

		TRY {
			ullLength = m_Dat.SeekToEnd();
			CArchive arFileList(&m_Dat, CArchive::store);
			m_FileList.Serialize(arFileList);
			arFileList.Flush();
			DWORD dwTotalLength = DWORD(m_Dat.GetLength()) + 4;
			m_Dat.Write(&dwTotalLength, sizeof(dwTotalLength));
			m_bNeedFlush = FALSE;
		}

		CATCH_ALL(e) {
			m_Dat.SetLength(ullLength);
			THROW_LAST();
		}

		END_CATCH_ALL
	}
}

INT_PTR CDatFile2::GetFileCount()
{
	return m_FileList.GetSize();
}

CFileDescriptorBase CDatFile2::operator [](INT_PTR nIndex) const
{
	return m_FileList[nIndex];
}

INT_PTR CDatFile2::FindNext()
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

BOOL CDatFile2::Inflate(CArchive& ar)
{
	if (m_nCurrent == -1) {
		return FALSE;
	}

	CFileDescriptorBase descriptor = m_FileList[m_nCurrent];

	m_Dat.Seek(descriptor.m_dwOffset, CFile::begin);

	const DWORD c_dwBufferSize = 65536;
	BYTE inBuffer[c_dwBufferSize];
	BYTE outBuffer[c_dwBufferSize];

	DWORD dwBytesLeft = descriptor.m_dwPackedFileLength;
	DWORD dwBytesToRead;
	DWORD dwRead;
	DWORD dwBytesOut = 0;

	// Uncompressed file
	if (descriptor.m_nType == 0) {
		do {
			dwBytesToRead = (dwBytesLeft < c_dwBufferSize) ? dwBytesLeft : c_dwBufferSize;
			dwRead = m_Dat.Read(inBuffer, dwBytesToRead);

			if (dwRead != dwBytesToRead) {
				return FALSE;
			}

			ar.Write(inBuffer, dwBytesToRead);
			dwBytesOut += dwRead;
		} while((dwBytesLeft -= dwRead ) > 0);

		return TRUE;
	}

	int rc;
	z_stream stream;

	stream.zalloc   = Z_NULL;
	stream.zfree    = Z_NULL;
	stream.opaque   = Z_NULL;
	stream.next_in  = Z_NULL;
	stream.avail_in = 0;

	rc = ::inflateInit(&stream);

	if (rc != Z_OK) {
		return FALSE;
	}

	stream.next_out  = outBuffer;
	stream.avail_out = c_dwBufferSize;

	BOOL bNeedData = TRUE;
	BOOL bResult = TRUE;
	BOOL bEndLoop = FALSE;
	BOOL bWriteOutData = FALSE;

	do {
		if (bNeedData) {
			dwBytesToRead = (dwBytesLeft < c_dwBufferSize) ? dwBytesLeft : c_dwBufferSize;
			dwRead = m_Dat.Read(inBuffer, dwBytesToRead);

			if (dwRead != dwBytesToRead) {
				bResult = FALSE;
				bWriteOutData = FALSE;
				break;
			}
			else {
				dwBytesLeft -= dwRead;
			}

			stream.next_in = inBuffer;
			stream.avail_in = dwRead;
			bNeedData = FALSE;
		}

		rc = ::inflate(&stream, Z_SYNC_FLUSH);

		switch(rc) {
			case Z_OK: {
				if (stream.avail_in == 0) {
					if (dwBytesLeft != 0) {
						// Need more input
						bNeedData = TRUE;
					}
					else {
						// All data inflated, but Z_STREAM_END not returned
						bEndLoop = TRUE;
						bWriteOutData = TRUE;
						break;
					}
				}

				if (stream.avail_out == 0) {	// Need more output
					bWriteOutData = TRUE;
				}

				break;
			}

			case Z_STREAM_END: {				// Inflated
				bEndLoop = TRUE;
				bWriteOutData = TRUE;
				break;
			}

			default: {							// Error
				bEndLoop = TRUE;
				bResult = FALSE;
			}
		}

		if (bWriteOutData) {
			ar.Write(outBuffer, c_dwBufferSize - stream.avail_out);
			bWriteOutData = FALSE;
			stream.next_out  = outBuffer;
			stream.avail_out = c_dwBufferSize;
		}
	} while(!bEndLoop);

	rc = ::inflateEnd(&stream);
	return bResult;
}

BOOL CDatFile2::Deflate(CArchive& ar, DWORD dwLength, LPCTSTR lpszName, int level)
{
	// Cut tail of file with files list
	if (m_Dat.GetLength() != 0) {
		if (!m_bNeedFlush) {
			DWORD dwDatLength;
			DWORD dwFileListLength;

			m_Dat.Seek(-8, CFile::end);
			m_Dat.Read(&dwFileListLength, sizeof(dwFileListLength));
			m_Dat.Read(&dwDatLength, sizeof(dwDatLength));
			m_Dat.SetLength(dwDatLength - dwFileListLength - 8);
		}
	}

	ULONGLONG ullDatLength = m_Dat.SeekToEnd();

	// File descriptor
	CFileDescriptor2 descriptor;
	descriptor.m_dwFileLength = dwLength;
	descriptor.m_strFileName = lpszName;
	descriptor.m_dwOffset = DWORD(ullDatLength);

	// Buffers
	const DWORD c_dwBufferSize = 65536;
	BYTE inBuffer[c_dwBufferSize];
	BYTE outBuffer[c_dwBufferSize];

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
		if (level < 0 || level > 9) {
			level = 9;
		}

		int rc;
		z_stream stream;
		int flush;

		stream.zalloc   = Z_NULL;
		stream.zfree    = Z_NULL;
		stream.opaque   = Z_NULL;
		stream.next_in  = Z_NULL;
		stream.avail_in = 0;

		rc = ::deflateInit(&stream, level);

		if (rc != Z_OK) {
			return FALSE;
		}

		stream.next_out  = outBuffer;
		stream.avail_out = c_dwBufferSize;

		BOOL bNeedData = TRUE;
		BOOL bEndLoop = FALSE;
		BOOL bWriteOutData = FALSE;

		do {
			if (bNeedData) {
				dwBytesToRead = (dwBytesLeft < c_dwBufferSize) ? dwBytesLeft : c_dwBufferSize;
				dwRead = ar.Read(inBuffer, dwBytesToRead);

				if (dwRead != dwBytesToRead) {
					bResult = FALSE;
					bWriteOutData = FALSE;
					break;
				}
				else {
					dwBytesLeft -= dwRead;
					flush = (dwBytesLeft == 0) ?  Z_FINISH : Z_SYNC_FLUSH;
				}

				stream.next_in = inBuffer;
				stream.avail_in = dwRead;

				bNeedData = FALSE;
			}

			rc = ::deflate(&stream, flush);

			switch(rc) {
				case Z_OK: {
					if (stream.avail_in == 0) {		// Need more input
						bNeedData = TRUE;
					}

					if (stream.avail_out == 0) {	// Need more output
						bWriteOutData = TRUE;
					}

					break;
				}

				case Z_STREAM_END: {				// Deflated
					bEndLoop = TRUE;
					bWriteOutData = TRUE;
					break;
				}

				default: {							// Error
					bEndLoop = TRUE;
					bResult = FALSE;
				}
			}

			if (bWriteOutData) {
				fileTemp.Write(outBuffer, c_dwBufferSize - stream.avail_out);
				bWriteOutData = FALSE;
				stream.next_out  = outBuffer;
				stream.avail_out = c_dwBufferSize;
			}
		} while(!bEndLoop);

		rc = ::deflateEnd(&stream);
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
		do {
			dwBytesToRead = (dwBytesLeft < c_dwBufferSize) ? dwBytesLeft : c_dwBufferSize;
			dwRead = pArSource->Read(inBuffer, dwBytesToRead);

			if (dwRead != dwBytesToRead) {
				bResult = FALSE;
				break;
			}

			m_Dat.Write(inBuffer, dwBytesToRead);
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

BOOL CDatFile2::Delete()
{
	if (m_nCurrent == -1) {
		return FALSE;
	}

	// Cut tail of file with files list
	if (m_Dat.GetLength() != 0) {
		if (!m_bNeedFlush) {
			DWORD dwDatLength;
			DWORD dwFileListLength;

			m_Dat.Seek(-8, CFile::end);
			m_Dat.Read(&dwFileListLength, sizeof(dwFileListLength));
			m_Dat.Read(&dwDatLength, sizeof(dwDatLength));
			m_Dat.SetLength(dwDatLength - dwFileListLength - 8);
		}
	}

	m_FileList.RemoveAt(m_nCurrent);
	m_nCurrent = -1;
	m_bNeedFlush = TRUE;

	return TRUE;
}


CDatFile2* g_pDatFile2ForSort;

int CompareDescriptorIndex2(const void* pIndex1, const void* pIndex2)
{
	INT_PTR nIndex1 = *((INT_PTR*)pIndex1);
	INT_PTR nIndex2 = *((INT_PTR*)pIndex2);

	CFileDescriptorBase descriptor1;
	CFileDescriptorBase descriptor2;

	descriptor1 = (*g_pDatFile2ForSort)[nIndex1];
	descriptor2 = (*g_pDatFile2ForSort)[nIndex2];

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

BOOL CDatFile2::Shrink(void (*pCallback)(INT_PTR nIndex))
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

	g_pDatFile2ForSort = this;
	qsort(pIndexArray, nFileCount, sizeof(INT_PTR), CompareDescriptorIndex2);

	// Move files
	const DWORD c_dwBufferSize = 65536;
	BYTE buffer[c_dwBufferSize];

	DWORD dwWritePos = 0;
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
			if (pCallback) {
				pCallback(pIndexArray[i]);
			}

			if (!m_bNeedFlush) {
				DWORD dwDatLength;
				DWORD dwFileListLength;

				m_Dat.Seek(-8, CFile::end);
				m_Dat.Read(&dwFileListLength, sizeof(dwFileListLength));
				m_Dat.Read(&dwDatLength, sizeof(dwDatLength));
				m_Dat.SetLength(dwDatLength - dwFileListLength - 8);
				m_bNeedFlush = TRUE;
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

	m_bNeedFlush = TRUE;

	Flush();

	return TRUE;
}
