' ########################################################################################
' Print Preview Class
' File: CPrintPreview.bi
' Contents: Print Preview canvas window that can be placed on a print preview dialog.
' Compiler: FreeBasic 32 & 64 bit
' Copyright (c) 2023 Paul Squires. Use at your own risk.
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
' EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
' MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
' ########################################################################################

#define UNICODE

#include once "Afx/CWindow.inc"
#include once "Afx/CPrint.inc"

enum ZoomLevel
   FitToWindow = 0
   FitToWidth
   Percent125
   Percent100
   Percent75
end enum

using Afx

' ########################################################################################
' Page TYPE
' ########################################################################################
type PageTYPE
   hdcEMF       as HDC                ' hdc of the metafile
   hEMF         as HENHMETAFILE       ' handle of the metafile
end type
      
' ########################################################################################
' CPrintPreview class.
' ########################################################################################
type CPrintPreview 
   private:
      m_JobName          as CWSTR    = "Untitled"
      m_CurrentPage      as long     = 0        
      m_Zoom             as long     = ZoomLevel.FitToWindow
      m_VScrollPos       as long     = 1
      m_HScrollPos       as long     = 1
      m_MeasurementUnits as long     = 0    ' inches
      m_CornerRadius     as long     = 0
      m_VersionNumber    as CWSTR    = "1.0.0"
      
   public:
      Printer         as CPrint
      pCWindow        as CWindow ptr        ' Canvas CWindow
      
      dim pages(any)  as PageTYPE           ' variable length dynamic array
      dim hFonts(any) as HFONT
      dim hPens(any)  as HPEN
      
      xDPi            as long = 0           ' horiz screen dots per inch
      yDPi            as long = 0           ' vert screen dots per inch
      rcPage          as RECT               ' canvas rect calculated in PaintRoutine
      IsPrinting      as boolean = false

      declare constructor
      declare destructor

      declare static Function WndProcCanvas( byval HWnd as HWnd, byval uMsg as UINT, byval wParam as WPARAM, _
                                             byval lParam as LPARAM ) as LRESULT
      declare function SetScrollBar( byval hwnd as HWND, byval vhPage as long, byval vhMax as long, _
                                     byval vhPos as long, byval vhBar as long ) as long
      declare function PaintRoutine( byval hwnd as HWND, byval hdc as HDC, byval nPageNum as long ) as long
      declare function CloseAllEMF() as long

      declare function AttachToParentWindow( byval hWndParent as HWND, byval ControlId as long ) as long
      declare function ModifyPage( byval nPageNumber as long ) as long
      declare function ChoosePrinter( byval hWin as HWND = null ) as boolean
      declare function PrintDoc() as boolean
      declare function AddFont( byref wszFontName as wstring, byval nFontSize as long, byval bFontBold as boolean = false, _
                                byval bFontItalic as boolean = false, byval bFontUnderline as boolean = false, _
                                byval bFontStrikeout as boolean = false ) as HFONT
      declare function UseFont( byval hFontHandle as HFONT ) as long                          
      declare function AddPen( byval nStyle as long, byval nWidth as long, byval clr as COLORREF = 0 ) as HPEN
      declare function AddSolidPen( byval nWidth as long, byval clr as COLORREF = 0 ) as HPEN
      declare function AddDashPen( byval clr as COLORREF = 0 ) as HPEN
      declare function AddDotPen( byval clr as COLORREF = 0 ) as HPEN
      declare function AddInvisiblePen() as HPEN
      declare function DisposeFonts() as long
      declare function DisposePens() as long
      declare function WriteText( byval nLeft as single, byval nTop as single, _
                                  byref wszText as wstring, byval clrText as COLORREF = 0, _
                                  byval clrBack as COLORREF = -1 ) as long
      declare function WriteTextBox( byval nLeft as single, byval nTop as single, byval nRight as single, byval nBottom as single, _
                                     byref wszText as wstring, byval hPen as HPEN, byval clrFill as COLORREF = -1, _
                                     byval nAlignment as long = DT_CENTER or DT_VCENTER, _
                                     byval clrText as COLORREF = 0, _
                                     byval clrBack as COLORREF = -1 ) as long
      declare function DrawRectInternal( byval nLeft as single, byval nTop as single, byval nRight as single, _
                                         byval nBottom as single, byval hPen as HPEN, byval clrFill as COLORREF ) as long
      declare function DrawRect( byval nLeft as single, byval nTop as single, byval nRight as single, _
                                 byval nBottom as single, byval hPen as HPEN ) as long
      declare function DrawSolidRect( byval nLeft as single, byval nTop as single, byval nRight as single, _
                                      byval nBottom as single, byval hPen as HPEN, byval clrFill as COLORREF ) as long
      declare function DrawLine( byval nLeft as single, byval nTop as single, byval nRight as single, _
                                 byval nBottom as single, byval hPen as HPEN ) as long
      declare property CornerRadius( byval nRadius as long )
      declare property CornerRadius() as long 
      declare property MeasurementUnits( byval nUnits as long )
      declare property MeasurementUnits() as long 
      declare property CurrentPage( byval nPageNum as long )
      declare property CurrentPage() as long 
      declare property Zoom( byval nZoom as ZoomLevel )
      declare property Zoom() as ZoomLevel
      declare property PageCount() as long 
      declare property VScrollPos() as long
      declare property VScrollPos( byval nValue as long )
      declare property HScrollPos() as long
      declare property HScrollPos( byval nValue as long )
      declare property WindowHandle() as HWND
      declare property JobName( byref wszJobName as wstring ) 
      declare property JobName() as CWSTR
      declare property PrinterName( byref wszPrinterName as wstring ) 
      declare property PrinterName() as CWSTR
      declare property DefaultPrinterName() as CWSTR
      declare property VersionNumber() as CWSTR
      
      declare property Orientation( byval nOrientation as long )
      declare property Orientation() as long
      declare property PaperWidth( byval nPaperWidth as single )
      declare property PaperWidth() as single
      declare property PaperHeight( byval nPaperHeight as single )
      declare property PaperHeight() as single
      declare property Copies() as long
      declare property Copies( byval nCopies as long )
      
end type

