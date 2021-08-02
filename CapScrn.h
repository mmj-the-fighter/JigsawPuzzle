#define HDIB HANDLE 

HBITMAP CopyWindowToBitmap();
HPALETTE GetSystemPalette(HWND hwnd) ;
HDIB ChangeBitmapFormat(HBITMAP hBitmap, 
								   WORD wBitCount, 
								   DWORD dwCompression, 
                                   HPALETTE hPal,
								   HWND hwnd); 

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B') 

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4) 

 enum {       
 ERR_MIN = 0,                     // All error #s >= this value       
 ERR_NOT_DIB = 0,                 // Tried to load a file, NOT a DIB!       
 ERR_MEMORY,                      // Not enough memory!       
 ERR_READ,                        // Error reading file!       
 ERR_LOCK,                        // Error on a GlobalLock()!       
 ERR_OPEN,                        // Error opening a file!       
 ERR_CREATEPAL,                   // Error creating palette.       
 ERR_GETDC,                       // Couldn't get a DC.       
 ERR_CREATEDDB,                   // Error create a DDB.       
 ERR_STRETCHBLT,                  // StretchBlt() returned failure.       
 ERR_STRETCHDIBITS,               // StretchDIBits() returned failure.       
 ERR_SETDIBITSTODEVICE,           // SetDIBitsToDevice() failed.       
 ERR_STARTDOC,                    // Error calling StartDoc().       
 ERR_NOGDIMODULE,                 // Couldn't find GDI module in memory.       
 ERR_SETABORTPROC,                // Error calling SetAbortProc().       
 ERR_STARTPAGE,                   // Error calling StartPage().       
 ERR_NEWFRAME,                    // Error calling NEWFRAME escape.       
 ERR_ENDPAGE,                     // Error calling EndPage().       
 ERR_ENDDOC,                      // Error calling EndDoc().       
 ERR_SETDIBITS,                   // Error calling SetDIBits().       
 ERR_FILENOTFOUND,                // Error opening file in GetDib()       
 ERR_INVALIDHANDLE,               // Invalid Handle       
 ERR_DIBFUNCTION,                 // Error on call to DIB function       
 ERR_MAX                          // All error #s < this value      
 
 };   

WORD DibNumColors (VOID FAR * pv)
{     
	int bits;     
	LPBITMAPINFOHEADER lpbi;     
	LPBITMAPCOREHEADER lpbc;      
	lpbi = ((LPBITMAPINFOHEADER)pv);     
	lpbc = ((LPBITMAPCOREHEADER)pv);      
	
	/*  With the BITMAPINFO format headers, the size of the palette      
	*  is in biClrUsed, whereas in the BITMAPCORE - style headers, it      
	*  is dependent on the bits per pixel ( = 2 raised to the power of      
	*  bits/pixel).      
	*/     
	
	if (lpbi->biSize != sizeof(BITMAPCOREHEADER))     
	{         
		if (lpbi->biClrUsed != 0)             
			return (WORD)lpbi->biClrUsed;         
	
		bits = lpbi->biBitCount;     
	}     
	else         
		bits = lpbc->bcBitCount;      
	
	switch (bits)     
	{     
		case 1:         
			return 2;     
		case 4:         
			return 16;     
		case 8:         
			return 256;     
		default:         
			/* A 24 bitcount DIB has no color table */         
			return 0;     

	} 
	
} 

WORD PaletteSize (VOID FAR * pv) 
{     
	LPBITMAPINFOHEADER lpbi;     
	WORD NumColors;      
	lpbi      = (LPBITMAPINFOHEADER)pv;     
	NumColors = DibNumColors(lpbi);      
	if (lpbi->biSize == sizeof(BITMAPCOREHEADER))         
		return NumColors * sizeof(RGBTRIPLE);     
	else         
		return NumColors * sizeof(RGBQUAD); 

} 


WORD SaveDIB(HDIB hDib, LPSTR lpFileName)
{
	BITMAPFILEHEADER    bmfHdr;     // Header for Bitmap file     
	LPBITMAPINFOHEADER  lpBI;       // Pointer to DIB info structure     
	HANDLE              fh;         // file handle for opened file     
	DWORD               dwDIBSize;     
	DWORD               dwWritten;      
	if (!hDib)         
		return ERR_INVALIDHANDLE;  
	
//   lpFileName = "C:\\Test.bmp";

   fh = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);      
   
   if (fh == INVALID_HANDLE_VALUE)         
		return ERR_OPEN;      
   // Get a pointer to the DIB memory, the first of which contains     
   // a BITMAPINFO structure      
   lpBI = (LPBITMAPINFOHEADER)GlobalLock(hDib);     
   if (!lpBI)     
   {         
		CloseHandle(fh);         
		return ERR_LOCK;     
	}     
	// Check to see if we're dealing with an OS/2 DIB.  If so, don't     
	// save it because our functions aren't written to deal with these     
	// DIBs.  
   if (lpBI->biSize != sizeof(BITMAPINFOHEADER))     
   {         
		GlobalUnlock(hDib);         
		CloseHandle(fh);         
		return ERR_NOT_DIB;     
	}      
	// Fill in the fields of the file header      
	// Fill in file type (first 2 bytes must be "BM" for a bitmap)      
	bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM" 

 dwDIBSize = *(LPDWORD)lpBI + PaletteSize((LPSTR)lpBI);        
 // Now calculate the size of the image      
 // It's an RLE bitmap, we can't calculate size, so trust the biSizeImage     
 // field      
 
 if ((lpBI->biCompression == BI_RLE8) || (lpBI->biCompression == BI_RLE4))         
  dwDIBSize += lpBI->biSizeImage;     
 else     
 {         
  DWORD dwBmBitsSize;  
  // Size of Bitmap Bits only          
  // It's not RLE, so size is Width (DWORD aligned) * Height          
  dwBmBitsSize = WIDTHBYTES((lpBI->biWidth)*((DWORD)lpBI->biBitCount)) * lpBI->biHeight;          
  dwDIBSize += dwBmBitsSize;          
  // Now, since we have calculated the correct size, why don't we         
  // fill in the biSizeImage field (this will fix any .BMP files which          
  // have this field incorrect).          
  lpBI->biSizeImage = dwBmBitsSize;     
 }       
 
  // Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)                         
  bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);     
  bmfHdr.bfReserved1 = 0;     
  bmfHdr.bfReserved2 = 0;      
 
  // Now, calculate the offset the actual bitmap bits will be in     
  // the file -- It's the Bitmap file header plus the DIB header,     
  // plus the size of the color table.          
 
 bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + lpBI->biSize +             PaletteSize((LPSTR)lpBI);      
 
 // Write the file header      
 
 WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);      
 
 // Write the DIB header and the bits -- use local version of     
  // MyWrite, so we can write more than 32767 bytes of data          
 
 WriteFile(fh, (LPSTR)lpBI, dwDIBSize, &dwWritten, NULL);      
 GlobalUnlock(hDib);     
 CloseHandle(fh);      
 
 if (dwWritten == 0)         
  return ERR_OPEN; 
 
 // oops, something happened in the write     
  else         
  return 0; // Success code 
 
}

VOID SaveScreenShot( HWND hWnd)
{	    
	    static int Filecount;		
		char  Filename[512];

		sprintf(Filename,"GAME%d.bmp",Filecount);

		HBITMAP HBitmap = CopyWindowToBitmap();
		HPALETTE hPal = NULL;
		if( !(hPal = GetSystemPalette(NULL)) )
		{
			MessageBox(NULL,"Error getting System Palette","SCREEN CAPTURE",MB_OK); 
			_exit(-1);
		}
		HANDLE hDIB = ChangeBitmapFormat(HBitmap, 
			                                   24,                  
											   BI_RGB,
											   hPal , 
											   NULL);
		SaveDIB(hDIB, Filename);
		hPal = NULL;
		Filecount++;
}

HBITMAP CopyWindowToBitmap()
{
	HDC         hScrDC, hMemDC;         // screen DC and memory DC     
	HGDIOBJ     hOldBitmap , hBitmap;
	
   
	// create a DC for the screen and create     
	// a memory DC compatible to screen DC          

   hScrDC = CreateDC("DISPLAY", NULL, NULL, NULL);     
   hMemDC = CreateCompatibleDC(hScrDC);      // get points of rectangle to grab  
   
   
   // create a bitmap compatible with the screen DC     
   
   hBitmap = CreateCompatibleBitmap(hScrDC, 800, 600);      
   
   // select new bitmap into memory DC     
   
   hOldBitmap =   SelectObject (hMemDC, hBitmap);      
   
   // bitblt screen DC to memory DC     
   
   BitBlt(hMemDC, 0, 0, 800, 600, hScrDC, 0, 0, SRCCOPY);     
   
   // select old bitmap back into memory DC and get handle to     
   // bitmap of the screen          
   
   hBitmap = SelectObject(hMemDC, hOldBitmap);      
   
   // clean up      
   
   DeleteDC(hScrDC);     
   DeleteDC(hMemDC);      
   
   // return handle to the bitmap      
   
   return (HBITMAP)hBitmap; 

}

HPALETTE GetSystemPalette(HWND hwnd) 
{ 
    HDC hDC;                // handle to a DC 
    static HPALETTE hPal = NULL;   // handle to a palette 
    HANDLE hLogPal;         // handle to a logical palette 
    LPLOGPALETTE lpLogPal;  // pointer to a logical palette 
    int nColors;            // number of colors 
 
    // Find out how many palette entries we want. 

    hDC = CreateDC ( TEXT("DISPLAY"), NULL, NULL, NULL );
 
    if (!hDC) 
        return NULL; 
	 
    nColors = 16; //PalEntriesOnDevice(hDC);   // Number of palette entries 
 
    // Allocate room for the palette and lock it. 
 
    hLogPal = GlobalAlloc(GHND, sizeof(LOGPALETTE) + nColors * 
            sizeof(PALETTEENTRY)); 
 
    // if we didn't get a logical palette, return NULL 
 
    if (!hLogPal) 
        return NULL; 
 
    // get a pointer to the logical palette 
 
    lpLogPal = (LPLOGPALETTE)GlobalLock(hLogPal); 
 
    // set some important fields 
 
    lpLogPal->palVersion = 0x300; //PALVERSION; 
    lpLogPal->palNumEntries = nColors; 
 
    // Copy the current system palette into our logical palette 
 
    GetSystemPaletteEntries(hDC, 0, nColors, 
            (LPPALETTEENTRY)(lpLogPal->palPalEntry)); 
 
    // Go ahead and create the palette.  Once it's created, 
    // we no longer need the LOGPALETTE, so free it.     
 
    hPal = CreatePalette(lpLogPal); 
 
    // clean up 
 
    GlobalUnlock(hLogPal); 
    GlobalFree(hLogPal); 
	::ReleaseDC(hwnd, hDC);
 
    return hPal; 
} 

HANDLE AllocRoomForDIB(BITMAPINFOHEADER bi, HBITMAP hBitmap , HWND hwnd) 
{ 
    DWORD               dwLen; 
    HANDLE              hDIB; 
    HDC                 hDC; 
    LPBITMAPINFOHEADER  lpbi; 
    HANDLE              hTemp; 
 
    // Figure out the size needed to hold the BITMAPINFO structure 
    // (which includes the BITMAPINFOHEADER and the color table). 
 
    dwLen = bi.biSize + PaletteSize((LPSTR) &bi); 
    hDIB  = GlobalAlloc(GHND,dwLen); 
 
    // Check that DIB handle is valid 
 
    if (!hDIB) 
        return NULL; 
 
    // Set up the BITMAPINFOHEADER in the newly allocated global memory, 
    // then call GetDIBits() with lpBits = NULL to have it fill in the 
    // biSizeImage field for us. 
 
    lpbi  = (LPBITMAPINFOHEADER)GlobalLock(hDIB); 
    *lpbi = bi; 
 
    hDC = CreateDC ( TEXT("DISPLAY"), NULL, NULL, NULL );
 
    GetDIBits(hDC, hBitmap, 0, (UINT) bi.biHeight, NULL, (LPBITMAPINFO)lpbi, 
            DIB_RGB_COLORS); 
 	::ReleaseDC(hwnd, hDC);
 
 
    // If the driver did not fill in the biSizeImage field, 
    // fill it in -- NOTE: this is a bug in the driver! 
     
    if (lpbi->biSizeImage == 0) 
        lpbi->biSizeImage = WIDTHBYTES((DWORD)lpbi->biWidth * 
                lpbi->biBitCount) * lpbi->biHeight; 
 
    // Get the size of the memory block we need 
 
    dwLen = lpbi->biSize + PaletteSize((LPSTR) &bi) + lpbi->biSizeImage; 
 
    // Unlock the memory block 
 
    GlobalUnlock(hDIB); 
 
    // ReAlloc the buffer big enough to hold all the bits  
 
    if (hTemp = GlobalReAlloc(hDIB,dwLen,0)) 
        return hTemp; 
    else 
    { 
        // Else free memory block and return failure 
 
        GlobalFree(hDIB); 
        return NULL; 
    } 
} 

HDIB ChangeBitmapFormat(HBITMAP hBitmap, 
								   WORD wBitCount, 
								   DWORD dwCompression, 
                                   HPALETTE hPal,
								   HWND hwnd) 
{ 
    HDC                hDC;          // Screen DC 
    HDIB               hNewDIB=NULL; // Handle to new DIB 
    BITMAP             Bitmap;       // BITMAP data structure 
    BITMAPINFOHEADER   bi;           // Bitmap info. header 
    LPBITMAPINFOHEADER lpbi;         // Pointer to bitmap header 
    HPALETTE           hOldPal=NULL; // Handle to palette 
    WORD               NewBPP;       // New bits per pixel 
    DWORD              NewComp;      // New compression format 
 
    // Check for a valid bitmap handle 
 
    if (!hBitmap) 
        return NULL; 
 
    // Validate wBitCount and dwCompression 
    // They must match correctly (i.e., BI_RLE4 and 4 BPP or 
    // BI_RLE8 and 8BPP, etc.) or we return failure 
     
    if (wBitCount == 0) 
    { 
        NewComp = dwCompression; 
        if (NewComp == BI_RLE4) 
            NewBPP = 4; 
        else if (NewComp == BI_RLE8) 
            NewBPP = 8; 
        else // Not enough info */ 
            return NULL; 
    } 
    else if (wBitCount == 1 && dwCompression == BI_RGB) 
    { 
        NewBPP = wBitCount; 
        NewComp = BI_RGB; 
    } 
    else if (wBitCount == 4) 
    { 
        NewBPP = wBitCount; 
        if (dwCompression == BI_RGB || dwCompression == BI_RLE4) 
            NewComp = dwCompression; 
        else 
            return NULL; 
    } 
    else if (wBitCount == 8) 
    { 
        NewBPP = wBitCount; 
        if (dwCompression == BI_RGB || dwCompression == BI_RLE8) 
            NewComp = dwCompression; 
        else 
            return NULL; 
    } 
    else if (wBitCount == 24 && dwCompression == BI_RGB) 
    { 
        NewBPP = wBitCount; 
        NewComp = BI_RGB; 
    } 
    else 
        return NULL; 
 
    // Get info about the bitmap 
 
    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap); 
 
    // Fill in the BITMAPINFOHEADER appropriately 
 
    bi.biSize               = sizeof(BITMAPINFOHEADER); 
    bi.biWidth              = Bitmap.bmWidth; 
    bi.biHeight             = Bitmap.bmHeight; 
    bi.biPlanes             = 1; 
    bi.biBitCount           = NewBPP; 
    bi.biCompression        = NewComp; 
    bi.biSizeImage          = 0; 
    bi.biXPelsPerMeter      = 0; 
    bi.biYPelsPerMeter      = 0; 
    bi.biClrUsed            = 0; 
    bi.biClrImportant       = 0; 
 
    // Go allocate room for the new DIB 
 
    hNewDIB = AllocRoomForDIB(bi, hBitmap , hwnd); 
    if (!hNewDIB) 
        return NULL; 
 
    // Get a pointer to the new DIB 
 
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hNewDIB); 
 
    // If we have a palette, get a DC and select/realize it 
 
    if (hPal) 
    { 
 		hDC = CreateDC ( TEXT("DISPLAY"), NULL, NULL, NULL );
        hOldPal = SelectPalette(hDC, hPal, FALSE); 
        RealizePalette(hDC); 
    } 
 
    // Call GetDIBits and get the new DIB bits 
 
    if (!GetDIBits(hDC, hBitmap, 0, (UINT) lpbi->biHeight, (LPSTR)lpbi + 
            (WORD)lpbi->biSize + PaletteSize((LPSTR)lpbi), (LPBITMAPINFO)lpbi, 
            DIB_RGB_COLORS)) 
    { 
        GlobalUnlock(hNewDIB); 
        GlobalFree(hNewDIB); 
        hNewDIB = NULL; 
    } 
 
    // Clean up and return 
 
    if (hOldPal) 
    { 
        SelectPalette(hDC, hOldPal, TRUE); 
        RealizePalette(hDC); 
		::ReleaseDC(hwnd, hDC);
     } 
 
    // Unlock the new DIB's memory block 
 
    if (hNewDIB) 
        GlobalUnlock(hNewDIB); 
 
    return hNewDIB; 
}
