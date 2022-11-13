// ifipr.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include "spi_api.h"

const char *pluginfo[] =
{
	/* 0    */	"00IN",
	/* 1    */	"RGB+IPR Plug-in for AFX Ver 1.00",
	/* 2n+1 */	"*.IPR",
	/* 2n+2 */	"RGB+IPR",
};
CHAR szPath[2048];
CHAR buf[1000];
INT nWidth, nHeight, nDot;
#define WIDTHBYTES(i) (((i)+31)/32*4)

int __stdcall GetPluginInfo(int nIndex, LPSTR szRetBuffer, int nBufLength)
{
	OutputDebugStringA("01.GetPluginInfo\n");
	INT nSize;
	if (nIndex < 0 || nIndex >= (sizeof(pluginfo) / sizeof(char *)))
	{
		return 0;
	}
	const char *s = pluginfo[nIndex];
	nSize = lstrlenA(s) + 1;
	nSize = (nSize < nBufLength) ? nSize : nBufLength;
	CopyMemory(szRetBuffer, s, nSize);
	return nSize;
}

int __stdcall IsSupported(LPSTR szFileName, DWORD dwFlag)
{
	OutputDebugStringA("02.IsSupported\n");
	BYTE header[2000] = { 0 };
	/*
	dwFlag:
		上位ワードが0		ファイルハンドル
		上位ワードが非0	ファイル先頭部(2Kbyte以上)を読み込んだバッファへのポインタ。
		ファイルサイズが2Kbyte以下の場合もバッファは2Kbyte確保し、余分は0で埋めること。
	*/
	if (HIWORD(dwFlag) == 0)
	{
		OutputDebugStringA("02.IsSupported File\n");	// ファイルハンドル
		DWORD dwRead = 0;
		ReadFile((HANDLE)dwFlag, header, sizeof(header), &dwRead, NULL);
	}
	else
	{
		OutputDebugStringA("02.IsSupported Memory\n");	// メモリバッファ
		CopyMemory(header, (LPVOID)dwFlag, sizeof(header));
	}
	{
		PBYTE p = (PBYTE)header;
		wsprintfA(buf, "02.IsSupported szFileName=%s, data=%x,%x,%x,%x\n",
			(char*)szFileName, p[0], p[1], p[2], p[3]);
		OutputDebugStringA(buf);
	}
	BOOL isSupport = FALSE;
	PCHAR p = (PCHAR)header;
#ifdef _MSC_VER
	sscanf_s(p, "%d %d", &nWidth, &nHeight);
#else
	sscanf(p, "%d %d", &nWidth, &nHeight);
#endif
	{
		char buf[100];
		wsprintfA(buf, "02.IsSupported nWidth=%d, nHeight=%d\n", nWidth, nHeight);
		OutputDebugStringA(buf);
	}
	if (nWidth > 0 && nHeight > 0)
	{
		lstrcpyA(szPath, szFileName);
		nDot = 0;
		for (int i = 0; szPath[i] != '\0'; i++)
		{
			if (szPath[i] == '.')
			{
				nDot = i;
				{
					char buf[100];
					wsprintfA(buf, "02.IsSupported nDot=%d, p=%s\n", nDot, &szPath[i]);
					OutputDebugStringA(buf);
				}
			}
		}
		if (nDot > 0)
		{
			isSupport = TRUE;
			OutputDebugStringA("02.IsSupported isSupport = TRUE\n");
		}
	}
	return isSupport;
}

int __stdcall GetPictureInfo(LPSTR lpFileData, long lLength, unsigned int uFlag, struct PictureInfo* lpInfo)
{
	// 渡されたデータの情報
	switch (uFlag & 0x07)
	{
	default:
		OutputDebugStringA("03.GetPictureInfo UNKNOWN\n");
		return SPI_NO_FUNCTION;
	case 0: // file
		// lpFileData	: LPCSTR ファイル名
		// lLength		: 読み込み開始オフセット
		{
			wsprintfA(buf, "03.GetPictureInfo file lpFileData=%s, lLength=%d\n", (char*)lpFileData, lLength);
			OutputDebugStringA(buf);
		}
		break;
	case 1: // memory
		{
			PBYTE p = (PBYTE)lpFileData;
			wsprintfA(buf, "03.GetPictureInfo memory lLength=%d, data=%x,%x,%x,%x\n",
				lLength, p[0], p[1], p[2], p[3]);
			OutputDebugStringA(buf);
		}
		return SPI_NO_FUNCTION;
	}
	return SPI_ALL_RIGHT;
}

int __stdcall GetPicture(LPSTR lpFileData, long lLength, unsigned int uFlag,
	HANDLE* phHeader, HANDLE* phImage, SPI_PROGRESS lpCallback, long lParam)
{
	// 渡されたデータの情報
	switch (uFlag & 0x07)
	{
	default:
		OutputDebugStringA("04.GetPicture UNKNOWN\n");
		return SPI_NO_FUNCTION;
	case 0: // file
		// lpFileData	: LPCSTR ファイル名
		// lLength		: 読み込み開始オフセット
		{
			wsprintfA(buf, "04.GetPicture file lpFileData=%s, lLength=%d\n", (char*)lpFileData, lLength);
			OutputDebugStringA(buf);
		}
		break;
	case 1: // memory
		{
			PBYTE p = (PBYTE)lpFileData;
			wsprintfA(buf, "04.GetPicture memory lLength=%d, data=%x,%x,%x,%x\n",
				lLength, p[0], p[1], p[2], p[3]);
			OutputDebugStringA(buf);
			szPath[nDot + 1] = 'R';
			szPath[nDot + 2] = 'G';
			szPath[nDot + 3] = 'B';
			szPath[nDot + 4] = '\0';
			HANDLE hFile = CreateFileA(szPath,
				GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				LARGE_INTEGER size;
				GetFileSizeEx(hFile, &size);
				PBYTE rgb = (PBYTE)VirtualAlloc(
					NULL, (SIZE_T)size.QuadPart, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
				if (rgb != NULL)
				{
					DWORD dwRead = 0;
					ReadFile(hFile, rgb, (DWORD)size.QuadPart, &dwRead, NULL);
					HLOCAL hHeader = LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
					if (hHeader != NULL)
					{
						*phHeader = hHeader;
						INT nWidthBytes = WIDTHBYTES(nWidth * 24);
						HLOCAL hImage = LocalAlloc(LPTR, nWidthBytes * nHeight);
						if (hImage != NULL)
						{
							*phImage = hImage;
							BITMAPINFOHEADER *lpbi = reinterpret_cast<BITMAPINFOHEADER*>(*phHeader);
							LPBYTE lpDIBBits = reinterpret_cast<LPBYTE>(*phImage);
							lpbi->biSize = sizeof(BITMAPINFOHEADER);
							lpbi->biWidth = nWidth;
							lpbi->biHeight = nHeight;
							lpbi->biPlanes = 1;
							lpbi->biBitCount = 24;
							lpbi->biCompression = BI_RGB;
							PBYTE q = rgb;
							for (INT y = 0; y < nHeight; y++)
							{
								PBYTE p = (PBYTE)lpDIBBits + ((nHeight - y - 1) * nWidthBytes);
								for (INT x = 0; x < nWidth; x++)
								{
									p[0] = q[2];
									p[1] = q[1];
									p[2] = q[0];
									p += 3;
									q += 3;
								}
							}
						}
					}
					VirtualFree(rgb, 0, MEM_RELEASE);
				}
				CloseHandle(hFile);
			}
		}
		return SPI_ALL_RIGHT;
	}
	return SPI_ALL_RIGHT;
}

int __stdcall GetPreview(LPSTR lpFileData, long lLength, unsigned int uFlag,
	HANDLE* phHeader, HANDLE *phImage, SPI_PROGRESS lpCallback, long lParam)
{
	OutputDebugStringA("GetPreview\n");
	return SPI_NO_FUNCTION;
}
