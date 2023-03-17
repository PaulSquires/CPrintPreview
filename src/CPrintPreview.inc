' ########################################################################################
' Print Preview Class
' File: CPrintPreview.inc
' Contents: Print Preview canvas window that can be placed on a print preview dialog.
' Compiler: FreeBasic 32 & 64 bit
' Copyright (c) 2023 Paul Squires. Use at your own risk.
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
' EXPRESSED or IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
' MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
' ########################################################################################

#define UNICODE

#include once "CPrintPreview.bi"


' ========================================================================================
' Default constructor
' Initializes a new instance of the CPrintPreview class.
' ========================================================================================
constructor CPrintPreview

   dim as long cxMms, cyMms, cxPix, cyPix
   
   dim as HDC hdc
   hdc = GetDC( 0 )
   cxMms = GetDeviceCaps( hdc, HORZSIZE )
   cyMms = GetDeviceCaps( hdc, VERTSIZE )
   cxPix = GetDeviceCaps( hdc, HORZRES)
   cyPix = GetDeviceCaps( hdc, VERTRES)
   ReleaseDC( 0, hdc )
   
   this.xDPi = (cxPix * 254 / cxMms / 10) 
   this.yDPi = (cyPix * 254 / cyMms / 10) 
   
   ' Attach the default printer (if one exists)
   dim as CWSTR DefaultPrinter = this.DefaultPrinterName
   if DefaultPrinter > "" then
      this.PrinterName = DefaultPrinter
      ' Set the default values for the printer
      this.copies = 1
      this.Orientation = DMORIENT_PORTRAIT
      this.PaperWidth = 8.5
      this.PaperHeight = 11
   end if
   
end constructor


' ========================================================================================
' Destructor
' ========================================================================================
destructor CPrintPreview
   ' Delete the metafiles held in memory
   for i as long = lbound(this.pages) to ubound(this.pages)
      if this.pages(i).hEMF then 
         DeleteEnhMetaFile( this.pages(i).hEMF )
      end if
   next

   ' Delete any fonts and pens that were added to the Canvas
   this.DisposeFonts
   this.DisposePens
   
   if this.pCWindow then delete(this.pCWindow)
end destructor


' ========================================================================================
' Delete any fonts that were added to the Canvas
' ========================================================================================
function CPrintPreview.DisposeFonts() as long
   for i as long = lbound(this.hFonts) to ubound(this.hFonts)
      if this.hFonts(i) then deleteobject(this.hFonts(i))
      this.hFonts(i) = 0
   next
   function = 0
end function


' ========================================================================================
' Delete any pens that were added to the Canvas
' ========================================================================================
function CPrintPreview.DisposePens() as long
   for i as long = lbound(this.hPens) to ubound(this.hPens)
      if this.hPens(i) then deleteobject(this.hPens(i))
      this.hPens(i) = 0
   next
   function = 0
end function


' ========================================================================================
' Set the active page to create and/or modify
' ========================================================================================
function CPrintPreview.ModifyPage( byval nPageNumber as long ) as long
   ' Obviously prevent any negative page numbers
   if nPageNumber < 0 then exit function

   ' Check if the dynamic array is big enough
   if nPageNumber > ubound(this.pages) then
      redim preserve this.pages(nPageNumber)
   end if

   ' Determine if the Page type has been initialized for the incoming page number. 
   if this.pages(nPageNumber).hdcEMF = 0 then
      this.pages(nPageNumber).hdcEMF = CreateEnhMetaFile( null, null, null, null )
   end if
   
   m_CurrentPage = nPageNumber

   function = 0
end function


' ========================================================================================
' Attach the print preview canvas to a parent window
' ========================================================================================
function CPrintPreview.AttachToParentWindow( _
               byval hWndParent as HWND, _
               byval ControlId as long _
               ) as long

   if hWndParent then 
      ' Close all EMF hdc thereby creating the EMF handles
      this.CloseAllEMF()

      if this.pCWindow then Delete(this.pCWindow)
      this.pCWindow = new CWindow
      
      ' Save our CPrintPreview pointer into the CWindow so that the user can
      ' retrieve it easily from the calling program's code.
      this.pCWindow->UserData(0) = cast( long_PTR, @this )
      
      this.pCWindow->Create( HWND_DESKTOP, "", _
           @CPrintPreview.WndProcCanvas, 0, 0, 0, 0, _
           WS_CLIPSIBLINGS or WS_CLIPCHILDREN or WS_HSCROLL or WS_VSCROLL, _
           WS_EX_CONTROLPARENT or WS_EX_LEFT or WS_EX_LTRREADING or WS_EX_RIGHTSCROLLBAR)

      AfxAddWindowStyle( this.pCWindow->hWindow, WS_CHILD )
      AfxRemoveWindowStyle( this.pCWindow->hWindow, WS_CAPTION )
      
      SetWindowLongPtr( this.pCWindow->hWindow, GWLP_ID, ControlId )
      SetParent( this.pCWindow->hWindow, hWndParent )

      ' Send a WM_SIZE message to the parent window in order to allow
      ' the user to be able to position and size the canvas control.
      dim as RECT rc
      GetClientRect( hWndParent, @rc )
      SendMessage( hWndParent, WM_SIZE, SIZE_RESTORED, MAKELONG( rc.right, rc.bottom ) )
   end if
   
   function = 0   
end function               


' ========================================================================================
' Displays the printer dialog to select a printer.
' If the user clicks the OK button, the return value is true.
' If the user canceled or closed the Print dialog box or an error occurred, the return value is false.
' ========================================================================================
function CPrintPreview.ChoosePrinter( byval hWin as HWND = null ) as boolean
   if this.Printer.ChoosePrinter(hWin) then
      if this.WindowHandle then AfxRedrawWindow(this.WindowHandle)
      return true
   end if
   return false
end function


' ========================================================================================
' Set the scrollbar positions (vertical and horizntal)
' ========================================================================================
property CPrintPreview.VScrollPos() as long
   property = m_VScrollPos
end property

property CPrintPreview.VScrollPos( byval nValue as long )
   m_VScrollPos = nValue
end property

property CPrintPreview.HScrollPos() as long
   property = m_HScrollPos
end property

property CPrintPreview.HScrollPos( byval nValue as long )
   m_HScrollPos = nValue
end property


' ========================================================================================
' Return the Window handle for the canvas.
' ========================================================================================
property CPrintPreview.WindowHandle() as HWND
   if this.pCWindow then
      property = this.pCWindow->hWindow
   end if      
end property


' ========================================================================================
' Return the version number of the CPrintPreview class.
' ========================================================================================
property CPrintPreview.VersionNumber() as CWSTR
   property = m_VersionNumber
end property


' ========================================================================================
' Set or Return the current printer name.
' ========================================================================================
property CPrintPreview.PrinterName( byref wszPrinterName as wstring ) 
   this.Printer.AttachPrinter(wszPrinterName)
end property

property CPrintPreview.PrinterName() as CWSTR
   property = this.Printer.GetPrinterName
end property

property CPrintPreview.DefaultPrinterName() as CWSTR
   property = this.Printer.GetDefaultPrinter
end property


' ========================================================================================
' Set or Return the current print Job Name.
' ========================================================================================
property CPrintPreview.JobName( byref wszJobName as wstring ) 
   m_JobName = wszJobName
end property

property CPrintPreview.JobName() as CWSTR
   property = m_JobName
end property


' ========================================================================================
' Set or Return the Orientation of the paper.
' DMORIENT_PORTRAIT (1) = Portrait
' DMORIENT_LANDSCAPE (2) = Landscape
' ========================================================================================
property CPrintPreview.Orientation( byval nOrientation as long )
   if nOrientation < DMORIENT_PORTRAIT then nOrientation = DMORIENT_PORTRAIT
   if nOrientation > DMORIENT_LANDSCAPE then nOrientation = DMORIENT_LANDSCAPE
   this.Printer.Orientation = nOrientation
   if this.WindowHandle then AfxRedrawWindow(this.WindowHandle)
end property

property CPrintPreview.Orientation() as long
   property = this.Printer.Orientation
end property


' ========================================================================================
' Set or Return the Paper Width.
' ========================================================================================
property CPrintPreview.PaperWidth( byval nPaperWidth as single )
   ' If necessary, adjust inches to Centimeters.
   if this.MeasurementUnits = 0 then nPaperWidth = nPaperWidth * 2.54
   ' Convert to 1/10 mm
   this.Printer.PaperWidth = nPaperWidth * 100
   if this.WindowHandle then AfxRedrawWindow(this.WindowHandle)
end property

property CPrintPreview.PaperWidth() as single
   ' Convert to Centimeters (value returned is 1/10 mm)
   dim as single ret = this.Printer.PaperWidth / 100
   ' If necessary, convert to inches.
   if this.MeasurementUnits = 0 then ret = ret / 2.54
   property = ret
end property


' ========================================================================================
' Set or Return the Paper Height.
' ========================================================================================
property CPrintPreview.PaperHeight( byval nPaperHeight as single )
   ' If necessary, adjust inches to Centimeters.
   if this.MeasurementUnits = 0 then nPaperHeight = nPaperHeight * 2.54
   ' Convert to 1/10 mm
   this.Printer.PaperLength = nPaperHeight * 100
   if this.WindowHandle then AfxRedrawWindow(this.WindowHandle)
end property

property CPrintPreview.PaperHeight() as single
   ' Convert to Centimeters (value returned is 1/10 mm)
   dim as single ret = this.Printer.PaperLength / 100
   ' If necessary, convert to inches.
   if this.MeasurementUnits = 0 then ret = ret / 2.54
   property = ret
end property


' ========================================================================================
' Set or Return the number of copies of the document to print.
' ========================================================================================
property CPrintPreview.Copies( byval nCopies as long )
   if nCopies < 1 then nCopies = 1
   this.Printer.Copies = nCopies
end property

property CPrintPreview.Copies() as long
   property = this.Printer.Copies
end property


' ========================================================================================
' Create Font based on all set attributes of the font (bold, underline, italic, etc)
' ========================================================================================
function CPrintPreview.AddFont( _
               byref wszFontName    as wstring, _
               byval nFontSize      as long, _
               byval bFontBold      as boolean = false, _ 
               byval bFontItalic    as boolean = false, _ 
               byval bFontUnderline as boolean = false, _
               byval bFontStrikeout as boolean = false _
               ) as HFONT
               
   dim pWindow as CWindow
   
   dim as HFONT hNewFont
   hNewFont =  pWindow.CreateFont( _
                              wszFontName, _
                              nFontSize, _
                              iif(bFontBold, FW_BOLD, FW_NORMAL), _
                              bFontItalic, _
                              bFontUnderline, _
                              bFontStrikeout _
                              )
                              
   ' Add this new font into our internal array. These fonts are automatically
   ' deleted when the class is destroyed.
   dim as long ub = ubound(this.hFonts) + 1
   redim preserve this.hFonts(ub)
   this.hFonts(ub) = hNewFont
   
   function = hNewFont                           
end function


' ========================================================================================
' Selects the incoming font into the hdcEMF of the CurrentPage
' ========================================================================================
function CPrintPreview.UseFont( byval hFontHandle as HFONT ) as long                          
   if (m_CurrentPage >= lbound(this.pages)) andalso (m_CurrentPage <= ubound(this.pages)) then
      SelectObject( this.pages(m_CurrentPage).hdcEMF, hFontHandle )
   end if
   function = 0
end function


' ========================================================================================
' Create Pen based on all set attributes of the pen (style, width, color)
' STYLE
'   PS_SOLID        The pen is solid.
'   PS_DASH         The pen is dashed. This style is valid only when the pen width is one or less in device units.
'   PS_DOT          The pen is dotted. This style is valid only when the pen width is one or less in device units.
'   PS_DASHDOT      The pen has alternating dashes and dots. This style is valid only when the pen width is one or less in device units.
'   PS_DASHDOTDOT   The pen has alternating dashes and double dots. This style is valid only when the pen width is one or less in device units.
'   PS_NULL         The pen is invisible.
'   PS_INSIDEFRAME  The pen is solid. When this pen is used in any GDI drawing function that takes a bounding rectangle, the dimensions 
'                   of the figure are shrunk so that it fits entirely in the bounding rectangle, taking into account the width of the 
'                   pen. This applies only to geometric pens. 
' ========================================================================================
function CPrintPreview.AddPen( _
               byval nStyle as long, _
               byval nWidth as long, _
               byval clr as COLORREF = 0 _
               ) as HPEN
               
   dim as HPEN hNewPen = CreatePen( nStyle, nWidth, clr )
   
   ' Add this new pen into our internal array. These pens are automatically
   ' deleted when the class is destroyed.
   dim as long ub = ubound(this.hPens) + 1
   redim preserve this.hPens(ub)
   this.hPens(ub) = hNewPen
   
   function = hNewPen
end function


' ========================================================================================
' Convenience function to add a solid pen.
' ========================================================================================
function CPrintPreview.AddSolidPen( _
               byval nWidth as long, _
               byval clr as COLORREF = 0 _
               ) as HPEN
   function = this.AddPen( PS_SOLID, nWidth, clr )                  
end function

               
' ========================================================================================
' Convenience function to add a dashed pen.
' ========================================================================================
function CPrintPreview.AddDashPen( _
               byval clr as COLORREF = 0 _
               ) as HPEN
   function = this.AddPen( PS_DASH, 1, clr )                  
end function


' ========================================================================================
' Convenience function to add a dotted pen.
' ========================================================================================
function CPrintPreview.AddDotPen( _
               byval clr as COLORREF = 0 _
               ) as HPEN
   function = this.AddPen( PS_DOT, 1, clr )                  
end function


' ========================================================================================
' Convenience function to add an invisible pen.
' ========================================================================================
function CPrintPreview.AddInvisiblePen() as HPEN
   function = this.AddPen( PS_NULL, 1, 0 )                  
end function


' ========================================================================================
' Set or Return the Zoom level.
'    0 = Fit to Window
'    1 = Fit to Width
'    2 = 125
'    3 = 100
'    4 = 75
' ========================================================================================
property CPrintPreview.Zoom( byval nZoom as ZoomLevel )
   if nZoom < 0 then nZoom = 0
   if nZoom > 4 then nZoom = 4
   m_Zoom = nZoom
end property

property CPrintPreview.Zoom() as ZoomLevel
   property = m_Zoom
end property


' ========================================================================================
' Sets / returns the radius of rounded corners. A value different from zero causes 
' Boxes (Rectangles) to be drawn with rounded corners that have the given radius.
' ========================================================================================
property CPrintPreview.CornerRadius( byval nRadius as long )
   if nRadius < 0 then nRadius = 0
   m_CornerRadius = nRadius
end property

property CPrintPreview.CornerRadius() as long
   property = m_CornerRadius
end property


' ========================================================================================
' Set or Return the Measurement Units used to specifiy coordinates and paper sizes.
' 0 = Inches
' 1 = Centimeters
' ========================================================================================
property CPrintPreview.MeasurementUnits( byval nUnits as long )
   if nUnits < 0 then nUnits = 0
   if nUnits > 1 then nUnits = 1
   m_MeasurementUnits = nUnits
end property

property CPrintPreview.MeasurementUnits() as long
   property = m_MeasurementUnits
end property


' ========================================================================================
' Set or Return the CurrentPage (the page currently being displayed or modified).
' All text and graphic actions are performed on the CurrentPage. 
' ========================================================================================
property CPrintPreview.CurrentPage( byval nPageNum as long )
   if nPageNum < 0 then nPageNum = 0
   if nPageNum > ubound(this.pages) then nPageNum = ubound(this.pages)
   m_CurrentPage = nPageNum
end property

property CPrintPreview.CurrentPage() as long
   property = m_CurrentPage
end property


' ========================================================================================
' Return the number of pages in the document (array is zero based).
' ========================================================================================
property CPrintPreview.PageCount() as long
   property = ubound(this.pages) + 1
end property


' ========================================================================================
' Outputs text formatted with the current alignment settings within a rectangle at 
' position Left, Top with the right border at Right and the bottom border at Bottom.
' if incoming nRight or nBottom are negative values then we treat them as nWidth and 
' nHeight instead of absolute values.
' ========================================================================================
function CPrintPreview.WriteText( _
               byval nLeft as single, _
               byval nTop as single, _
               byref wszText as wstring, _
               byval clrText as COLORREF = 0, _
               byval clrBack as COLORREF = -1 _
               ) as long

   dim as HDC hdcEMF = this.pages(m_CurrentPage).hdcEMF
   
   ' If necessary, convert Centimeters to Inches.
   dim as single nConvert = 1
   if this.MeasurementUnits = 1 then nConvert = 2.54
   
   if clrBack = -1 then clrBack = BGR(255,255,255)
   
   SetTextColor( hdcEMF, clrText )
   SetBkColor( hdcEMF, clrBack )
   
   function = TextOut( hdcEMF, _
            (nLeft / nConvert) * this.xDpi, _
            (nTop / nConvert)  * this.yDpi, _
            wszText, len(wszText) )
            
end function


' ========================================================================================
' Outputs text formatted with the alignment, text color, box color and width.
' clrFill:
'   If no fill is specified then the rectangle uses a null brush to paint the area.
' Alignment (default is DT_CENTER or VCENTER):
'   DT_LEFT, DT_TOP, DT_RIGHT, DT_BOTTOM, DT_CENTER, DT_VCENTER
' clrText:
'   If no text color is specified then black is used.
' clrBack:
'   If no text back color is specified then clrFill is used. If clrFill was not
'   specified then white is used.
' ========================================================================================
function CPrintPreview.WriteTextBox( _
               byval nLeft as single, _
               byval nTop as single, _
               byval nRight as single, _
               byval nBottom as single, _
               byref wszText as wstring, _
               byval hPen as HPEN, _
               byval clrFill as COLORREF = -1, _
               byval nAlignment as long = DT_CENTER or DT_VCENTER, _
               byval clrText as COLORREF = 0, _
               byval clrBack as COLORREF = -1 _
               ) as long

   dim as HDC hdcEMF = this.pages(m_CurrentPage).hdcEMF

   ' draw the enclosing box/rectangle
   this.DrawRectInternal( nLeft, nTop, nRight, nBottom, hPen, clrFill )
   
   ' If necessary, convert Centimeters to Inches.
   dim as single nConvert = 1
   if this.MeasurementUnits = 1 then nConvert = 2.54
   
   ' Draw the text inside the box/rectangle
   SetTextColor( hdcEMF, clrText )
   if clrBack = -1 then clrBack = iif(clrFill = -1, BGR(255,255,255), clrFill)
   SetBkColor( hdcEMF, clrBack )
   
   dim as RECT rc
   SetRect( @rc, _
            (nLeft / nConvert) * this.xDpi, _
            (nTop / nConvert)  * this.yDpi, _
            (nRight / nConvert)  * this.xDpi, _
            (nBottom / nConvert) * this.yDpi)

   function = DrawText( _
               hdcEMF, _
               wszText, len(wszText), _
               @rc, _
               nAlignment or DT_SINGLELINE )

   ' Need to draw hollow rectangle over top again in case DT_LEFT text was used because
   ' the drawn text would have overwritten the left edge of the initial rectangle.
   this.DrawRectInternal( nLeft, nTop, nRight, nBottom, hPen, -1 )

end function


' ========================================================================================
' Internal function to draw all boxes (rectangles).
' ========================================================================================
function CPrintPreview.DrawRectInternal( _
                   byval nLeft as single, _
                   byval nTop as single, _
                   byval nRight as single, _
                   byval nBottom as single, _
                   byval hPen as HPEN, _
                   byval clrFill as COLORREF _
                   ) as long

   dim as HDC hdcEMF = this.pages(m_CurrentPage).hdcEMF
   
   ' If necessary, convert Centimeters to Inches.
   dim as single nConvert = 1
   if this.MeasurementUnits = 1 then nConvert = 2.54
   
   SaveDC( hdcEMF )

   dim as HBRUSH hBrush
   if clrFill = -1 then 
      hBrush = GetStockObject(HOLLOW_BRUSH)
   else
      hBrush = CreateSolidBrush(clrFill)
   end if
   
   SelectObject( hdcEMF, hBrush )
   SelectObject( hdcEMF, hPen )
   
   function = RoundRect( hdcEMF, _
            (nLeft / nConvert) * this.xDpi, _
            (nTop / nConvert)  * this.yDpi, _
            (nRight / nConvert)  * this.xDpi, _
            (nBottom / nConvert) * this.yDpi, _
            m_CornerRadius, m_CornerRadius )
   
   RestoreDC( hdcEMF, -1 )
   
   if hBrush then DeleteObject( hBrush )

end function


' ========================================================================================
' Draw a filled box (rectangle) using the passed Pen and fill color.
' ========================================================================================
function CPrintPreview.DrawSolidRect( _
                   byval nLeft as single, _
                   byval nTop as single, _
                   byval nRight as single, _
                   byval nBottom as single, _
                   byval hPen as HPEN, _
                   byval clrFill as COLORREF _
                   ) as long
   return DrawRectInternal( nLeft, nTop, nRight, nBottom, hPen, clrFill )
end function


' ========================================================================================
' Draw a box (rectangle) using the passed Pen.
' ========================================================================================
function CPrintPreview.DrawRect( _
                   byval nLeft as single, _
                   byval nTop as single, _
                   byval nRight as single, _
                   byval nBottom as single, _
                   byval hPen as HPEN _
                   ) as long
   return DrawRectInternal( nLeft, nTop, nRight, nBottom, hPen, -1 )
end function
                   

' ========================================================================================
' Draw a line based using the passed pen.
' ========================================================================================
function CPrintPreview.DrawLine( _
                   byval nLeft as single, _
                   byval nTop as single, _
                   byval nRight as single, _
                   byval nBottom as single, _
                   byval hPen as HPEN _
                   ) as long

   dim as HDC hdcEMF = this.pages(m_CurrentPage).hdcEMF
   
   ' If necessary, convert Centimeters to Inches.
   dim as single nConvert = 1
   if this.MeasurementUnits = 1 then nConvert = 2.54
   
   SaveDC( hdcEMF )

   SelectObject( hdcEMF, hPen )
   
   MoveToEx( hdcEMF, _
            (nLeft / nConvert) * this.xDpi, _
            (nTop / nConvert)  * this.yDpi, _
            null )

   function = LineTo( hdcEMF, _
               (nRight / nConvert)  * this.xDpi, _
               (nBottom / nConvert) * this.yDpi )

   RestoreDC( hdcEMF, -1 )
   
end function


' ========================================================================================
' Close all EMF hdc thereby creating the EMF handle
' ========================================================================================
function CPrintPreview.CloseAllEMF() as long
   for i as long = lbound(this.pages) to ubound(this.pages)
      if this.pages(i).hdcEMF then
         this.pages(i).hEMF = CloseEnhMetaFile( this.pages(i).hdcEMF )
         this.pages(i).hdcEMF = null
      end if
   next
   function = 0
end function


' ========================================================================================
' Generic paint routine that can output to either display or printer devices.
' ========================================================================================
function CPrintPreview.PaintRoutine( _
               byval hwnd as HWND, _
               byval hdcDestination as HDC, _
               byval nPageNum as long _
               ) as long
   
   if (nPageNum < 0) orelse (nPageNum > this.PageCount) then
      exit function
   end if
   
   dim as ENHMETAHEADER emh
   dim as long cxMms, cyMms, cxPix, cyPix
   dim as long xImage, yImage, cxImage, cyImage
   dim as RECT rc
    
   cxMms = GetDeviceCaps( hdcDestination, HORZSIZE )
   cyMms = GetDeviceCaps( hdcDestination, VERTSIZE )
   cxPix = GetDeviceCaps( hdcDestination, HORZRES)
   cyPix = GetDeviceCaps( hdcDestination, VERTRES)

   GetEnhMetaFileHeader( this.pages(nPageNum).hEMF, sizeof(emh), @emh )
    
   ' Get the size of the metafile image in units of 0.01 millimeters
   xImage  = emh.rclFrame.left
   yImage  = emh.rclFrame.top
   cxImage = emh.rclFrame.right - emh.rclFrame.left
   cyImage = emh.rclFrame.bottom - emh.rclFrame.top
    
   ' Convert the millimeters to pixels by multiplying by the pixel
   ' dimension of the output device, dividing by the millimeter 
   ' dimension, and then dividing by 100 to account for the metrical
   ' dimension in 0.01 millimeters.
   xImage  = xImage  * cxPix / cxMms / 100
   yImage  = yImage  * cyPix / cyMms / 100
   cxImage = cxImage * cxPix / cxMms / 100
   cyImage = cyImage * cyPix / cyMms / 100
    
   ' The size of the image is now in pixels but not specific to the
   ' video display. This is a pixel dimension appropriate for the
   ' destination device.
    
   ' Output the image on the screen/printer
   rc.left   = xImage
   rc.right  = xImage + cxImage
   rc.top    = yImage
   rc.bottom = yImage + cyImage
    
   ' if this page is being output to the printer then simply do that
   ' now at full size. No need to bother with all of the subsequent
   ' that deals with displaying on the screen.
   if hdcDestination = this.Printer.GetDC then
      PlayEnhMetaFile( hdcDestination, this.pages(nPageNum).hEMF, @rc )
      exit function
   end if
   
   ' Create a bitmap to copy the metafile into. We then can then BitBlt
   ' or StretchBlt that bitmap into the destination hdc based on zoom factor.
   dim as HDC hdcDesktop, memDC
   dim as HBITMAP hbit, hbitold
   dim as RECT rcPage, rcClient
   dim as long nLeft, nTop, cxPage, cyPage 
   dim as single nInchPageWidth, nInchPageHeight
   
   ' If necessary, convert Centimeters to Inches.
   dim as single nConvert = 1
   if this.MeasurementUnits = 1 then nConvert = 2.54

   ' Create a page rect based on the size of the selected paper and orientation
   nInchPageWidth  = this.PaperWidth / nConvert
   nInchPageHeight = this.PaperHeight / nConvert
   if this.Orientation = DMORIENT_LANDSCAPE then
      swap nInchPageWidth, nInchPageHeight
   end if   
   
   ' Create the rect for the display version (rcPage) of the document
   rcPage.left   = 0 
   rcPage.top    = 0
   rcPage.right  = nInchPageWidth * xDPi
   rcPage.bottom = nInchPageHeight * yDPi
   
   ' Save the rcPage to the instance so that external functions such as VSCROLL
   ' and HSCROLL can access the data.
   this.rcPage = rcPage
   
   hdcDesktop = GetDC( 0 )
   memDC      = CreateCompatibleDC( 0 )
   hbit       = CreateCompatibleBitmap( hdcDesktop, rcPage.right, rcPage.bottom )
   hbitold    = SelectObject( memDC, hbit )

   SelectObject( memDC, GetStockObject(WHITE_BRUSH) )
   PatBlt( memDC, 0, 0, rcPage.right, rcPage.bottom, PATCOPY )
   
   ' Show the page elements
   PlayEnhMetaFile( memDC, this.pages(nPageNum).hEMF, @rc )
   
   ' Center the image
   GetClientRect( hwnd, @rcClient )

   dim as single zoomfactor
   dim as single tempscalefactorx 
   dim as single tempscalefactory 

   dim as long y = rcPage.bottom - ( rcClient.Bottom - rcClient.top )
   dim as long x = rcPage.right - rcClient.right
   dim as long nTopGrayMargin = AfxScaleY(5)
   dim as long nBottomGrayMargin = nTopGrayMargin
         
   select case this.Zoom
      case ZoomLevel.FitToWindow
         tempscalefactorx = (rcClient.right - rcClient.left) / rcPage.right
         tempscalefactory = (rcClient.bottom - rcClient.top) / rcPage.bottom
         if tempscalefactorx >= tempscalefactory then
            zoomfactor = tempscalefactory 
         else
            zoomfactor = tempscalefactorx 
         end if
         EnableScrollBar( hwnd, SB_BOTH, ESB_DISABLE_BOTH )
         x = 0: y = 0
         
      case ZoomLevel.FitToWidth   
         zoomfactor = (rcClient.right - rcClient.left) / rcPage.right
         ' The horizontal scrollbar should always be disabled because we are
         ' always stretching the canvas to the width of the window.
         EnableScrollBar( hwnd, SB_HORZ, ESB_DISABLE_BOTH )
         x = 0
         nTopGrayMargin = 0
         nBottomGrayMargin = 0

      case ZoomLevel.Percent125
         zoomfactor = 1.25
         
      case ZoomLevel.Percent100
         zoomfactor = 1

      case ZoomLevel.Percent75
         zoomfactor = 0.75
   end select

   if y > 0 then
      EnableScrollBar( hwnd, SB_VERT, ESB_ENABLE_BOTH )
      this.SetScrollBar( hwnd, rcPage.bottom / 10, rcPage.bottom, this.VScrollPos, SB_VERT )
   else
      EnableScrollBar( hwnd, SB_VERT, ESB_DISABLE_BOTH )   
   end if         

   if x > 0 then
      EnableScrollBar( hwnd, SB_HORZ, ESB_ENABLE_BOTH )
      this.SetScrollBar( hwnd, rcPage.right / 10, rcPage.right, this.HScrollPos, SB_HORZ )
   else
      EnableScrollBar( hwnd, SB_HORZ, ESB_DISABLE_BOTH )   
   end if         

   ' Center the image
   cxPage = ( rcPage.right - rcPage.left ) * zoomfactor
   nLeft = (rcClient.right - rcClient.left - cxPage) / 2
   
   dim as RECT rcCanvas
   rcCanvas.left   = nLeft
   rcCanvas.top    = rcClient.top + nTopGrayMargin
   rcCanvas.right  = (rcPage.right - this.HScrollPos) * zoomfactor
   rcCanvas.bottom = (rcPage.bottom - this.VScrollPos) * zoomfactor - nBottomGrayMargin
   
   ' Copy the zoom'd canvas area 
   StretchBlt( hdcDestination, _
               rcCanvas.left, rcCanvas.top, _
               rcCanvas.right, rcCanvas.bottom, _
               memDC, _
               0, this.VScrollPos, _
               rcPage.right - this.HScrollPos, rcPage.bottom - this.VScrollPos, _
               SRCCOPY )

   ' Paint the light gray background that surrounds the Canvas.
   ' Make sure that the possible zoom'd page has the original page width so that the
   ' gray area will draw around the full page rather than a zoom'd StretchBlt.
   rcCanvas.right = rcCanvas.left + cxPage
   dim as HRGN hRgnClient = CreateRectRgnIndirect( @rcClient )
   dim as HRGN hRgnCanvas = CreateRectRgnIndirect( @rcCanvas )
   dim as HRGN hRgnDiff = CreateRectRgn( 0, 0, 0, 0 )

   CombineRgn( hRgnDiff, hRgnClient, hRgnCanvas, RGN_XOR )
   FillRgn( hdcDestination, hRgnDiff, GetStockObject(LTGRAY_BRUSH) )

   DeleteObject hRgnClient
   DeleteObject hRgnCanvas
   DeleteObject hRgnDiff
   DeleteObject( SelectObject( memDC, hbitold ) )
   DeleteDC( memDC )
   ReleaseDC( 0, hdcDesktop )

   function = 0
end function


' ========================================================================================
' Print the current document
' ========================================================================================
function CPrintPreview.PrintDoc() as boolean
   ' Close all EMF hdc thereby creating the EMF handles
   this.CloseAllEMF()

   ' We can't do anything if we have nothing to print to.
   if this.PrinterName =  "" then 
      if this.Printer.ChoosePrinter( this.WindowHandle ) = false then
         return true   ' error
      end if   
   end if

   dim as HDC hdcPrn = this.Printer.GetDC
   if hdcPrn = 0 then exit function
   
   dim as DOCINFO di
   di.cbSize      = sizeof(DOCINFO)
   di.lpszDocName = this.JobName
   
   this.IsPrinting = true
   
   StartDoc( hdcPrn, @di ) 
   
   for iPage as long = 0 to this.PageCount - 1
      StartPage( hdcPrn)
      this.PaintRoutine( 0, hdcPrn, iPage )
      EndPage( hdcPrn)
   next
   
   EndDoc( hdcPrn ) 
   DeleteDC( hdcPrn )

   this.IsPrinting = false
   
   function = false
end function


' ========================================================================================
' CPrintPreview SetScrollBar position
' ========================================================================================
function CPrintPreview.SetScrollBar( _
               byval hwnd as HWND, _
               byval vhPage as long, _
               byval vhMax as long, _
               byval vhPos as long, _
               byval vhBar as long _
               ) as long

  dim si as SCROLLINFO

  si.cbSize = sizeof(si)
  si.fMask  = SIF_ALL or SIF_DISABLENOSCROLL
  si.nMin   = 0
  si.nMax   = MAX(0, vhMax)
  si.nPage  = vhPage
  si.nPos   = vhPos
  SetScrollInfo( hwnd, vhBar, @si, -1 )
  
  function = 0
end function


' ========================================================================================
' CPrintPreview Canvas Window procedure (STATIC)
' ========================================================================================
function CPrintPreview.WndProcCanvas( _
               byval hwnd   as HWND, _
               byval uMsg   as UINT, _
               byval wParam as WPARAM, _
               byval lParam as LPARAM _
               ) as LRESULT

   ' The WndProcCanvas procedure is STATIC so we need to get the instance
   ' of the Canvas that is callign this function.
   dim pWindow as CWindow ptr = AfxCWindowPtr(hwnd)
   dim pCanvas as CPrintPreview ptr 
   if pWindow then pCanvas = cast( CPrintPreview ptr, pWindow->UserData(0) )
   static as long accumDelta

   select case uMsg
    
      case WM_DESTROY
         if pCanvas then
            if pCanvas->pCWindow then 
               delete(pCanvas->pCWindow)
               pCanvas->pCWindow = null
            end if
         end if
         
         
      case WM_ERASEBKGND
         ' Return non-zero because we are handling the complete painting 
         ' of the window via our paint routine so we do not need any extra
         ' flicker from a client area erase.
         return CTRUE
          

      case WM_PAINT
         dim as HDC hdc
         dim as PAINTSTRUCT ps
          
         hdc = BeginPaint( hwnd, @ps )
        
         if pCanvas then pCanvas->PaintRoutine( hwnd, hdc, pCanvas->CurrentPage )
          
         EndPaint( hwnd, @ps )
          
         exit function


      Case WM_MOUSEWHEEL
         if pCanvas = 0 then exit function
         ' accumulate delta until scroll one line (up +120, down -120). 
         ' 120 is the Microsoft default delta
         dim as long zDelta = GET_WHEEL_DELTA_WPARAM( wParam )
         accumDelta = accumDelta + zDelta
         if accumDelta >= 120 then       ' scroll up 
            SendMessage( hwnd, WM_VSCROLL, makelong(SB_PAGEUP, 0), 0 ) 
            accumDelta = 0
         elseif accumDelta <= -120 then  ' scroll down
            SendMessage( hwnd, WM_VSCROLL, makelong(SB_PAGEDOWN, 0), 0 ) 
            accumDelta = 0
         end if
         exit function


      case WM_VSCROLL
         if pCanvas = 0 then exit function
         
         dim iVscrollInc as long
         select case loword( wParam )
           case SB_LINEUP     : iVScrollInc = -pCanvas->rcPage.bottom / 100
           case SB_LINEDOWN   : iVScrollInc =  pCanvas->rcPage.bottom / 100
           case SB_PAGEUP     : iVScrollInc = -pCanvas->rcPage.bottom / 10
           case SB_PAGEDOWN   : iVScrollInc =  pCanvas->rcPage.bottom / 10
           case SB_THUMBTRACK : iVScrollInc = hiword(wParam) - pCanvas->VScrollPos
         end select
 
         if iVScrollInc then
            pCanvas->VScrollPos = pCanvas->VScrollPos + iVScrollInc
            if pCanvas->VScrollPos < 0 then pCanvas->VScrollPos = 0
            if pCanvas->VScrollPos > pCanvas->rcPage.bottom then pCanvas->VScrollPos = pCanvas->rcPage.bottom
            pCanvas->SetScrollBar( hwnd, pCanvas->rcPage.bottom / 10, pCanvas->rcPage.bottom, pCanvas->VScrollPos, SB_VERT )
            AfxRedrawWindow( hwnd )
         end if
         exit function


      case WM_HSCROLL
         if pCanvas = 0 then exit function
  
         dim iHScrollInc as long
         select case loword( wParam )
            case SB_LINELEFT   : iHScrollInc = -pCanvas->rcPage.right / 20
            case SB_LINERIGHT  : iHScrollInc =  pCanvas->rcPage.right / 20
            case SB_PAGELEFT   : iHScrollInc = -pCanvas->rcPage.right / 10
            case SB_PAGERIGHT  : iHScrollInc =  pCanvas->rcPage.right / 10
            case SB_THUMBTRACK : iHScrollInc = hiword(wParam) - pCanvas->HScrollPos
         end select
           
         if iHScrollInc then
            pCanvas->HScrollPos = pCanvas->HScrollPos + iHScrollInc
            if pCanvas->HScrollPos < 0 then pCanvas->HScrollPos = 0
            if pCanvas->HScrollPos > pCanvas->rcPage.right then pCanvas->HScrollPos = pCanvas->rcPage.right
            pCanvas->SetScrollBar( hwnd, pCanvas->rcPage.right / 10, pCanvas->rcPage.right, pCanvas->HScrollPos, SB_HORZ )
            AfxRedrawWindow( hwnd )
         end if     
         exit function

   end select

   function = DefWindowProc(hwnd, uMsg, wParam, lParam)

end function
' ========================================================================================


