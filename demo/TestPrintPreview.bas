' ########################################################################################
' Microsoft Windows
' File: TestPrintPreview.bas
' Contents: Simple test dialog that tests PrintPreviewDlg and CPrintPreview canvas.
' Compiler: FreeBasic 32 & 64 bit
' Copyright (c) 2016 José Roca. Freeware. Use at your own risk.
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
' EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
' MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
' ########################################################################################

#define UNICODE

#include once "Afx/CWindow.inc"
#include once "../src/CPrintPreview.inc"
#include once "PrintPreviewDlg.inc"


USING Afx

const IDC_PREVIEW = 101

DECLARE FUNCTION WinMain (BYVAL hInstance AS HINSTANCE, _
                          BYVAL hPrevInstance AS HINSTANCE, _
                          BYVAL szCmdLine AS ZSTRING PTR, _
                          BYVAL nCmdShow AS LONG) AS LONG

   END WinMain(GetModuleHandleW(NULL), NULL, COMMAND(), SW_NORMAL)

' // Forward declaration
DECLARE FUNCTION WndProc (BYVAL hwnd AS HWND, BYVAL uMsg AS UINT, BYVAL wParam AS WPARAM, BYVAL lParam AS LPARAM) AS LRESULT



function CreatePrintPreviewCanvas() as CPrintPreview ptr
   
   ' Create the new Invoice document 
   dim pCanvas as CPrintPreview ptr = new CPrintPreview 

   ' Add different fonts to the Canvas that we can use. These fonts are
   ' automatically released when the Canvas is destroyed (to avoid GDI leaks).
   dim as HFONT hFont(5)
   hFont(0) = pCanvas->AddFont( "Arial", 9, true )       ' Invoice/Date headings
   hFont(1) = pCanvas->AddFont( "Arial", 10 )            ' Regular line items
   hFont(2) = pCanvas->AddFont( "Arial", 11, , true )    ' Thank you for your business!
   hFont(3) = pCanvas->AddFont( "Arial", 12, true )      ' Total amount
   hFont(4) = pCanvas->AddFont( "Arial", 16 )            ' [COMPANY NAME]
   hFont(5) = pCanvas->AddFont( "Arial", 28, true )      ' INVOICE (gray color) 
   
   ' Create a solid pen of width 2. Black color is used if no color is specified.
   dim as HPEN hPen = pCanvas->AddSolidPen( 2 )  
   ' Create an invisible pen used for writing text to boxes with no border.
   dim as HPEN hPenInvisible = pCanvas->AddInvisiblePen()  
   
   ' Define gray color for solid boxes
   dim as COLORREF clrLtGray = BGR(219,219,219)
   dim as COLORREF clrDkGray = BGR(148,148,148)
   
   ' Need to set the measurement units before creating objects if
   ' you decide to use centimeters because inches is the default.
   pCanvas->MeasurementUnits = 1   ' 0:Inches, 1:Centimeters
   
   pCanvas->Orientation = DMORIENT_PORTRAIT
   pCanvas->PaperWidth = 21.59     ' cm  (8.5 inches)
   pCanvas->PaperHeight = 27.94    ' cm  (11 inches)

   ' Page #0
   pCanvas->ModifyPage(0)
   
   pCanvas->UseFont( hFont(4) )
   ' black text color and white background is used if not specified
   pCanvas->WriteText( 1.5, 1.5, "[Company Name]" )    
   
   pCanvas->UseFont( hFont(1) )
   pCanvas->WriteText( 1.5, 2.5, "[Street Address]" )
   pCanvas->WriteText( 1.5, 3.0, "[City, ST ZIP]" )
   pCanvas->WriteText( 1.5, 3.5, "Phone: (000) 000-0000" )

   pCanvas->UseFont( hFont(5) )
   pCanvas->WriteTextBox( 16.0, 1.5, 20.4, 2.5, "INVOICE", hPenInvisible, , DT_TOP or DT_RIGHT, clrDkGray )

   pCanvas->UseFont( hFont(0) )
   pCanvas->WriteTextBox( 12.5, 3.5, 16.6, 4.2, "INVOICE #", hPen, clrLtGray )
   pCanvas->WriteTextBox( 16.6, 3.5, 20.4, 4.2, "DATE", hPen, clrLtGray )
   pCanvas->UseFont( hFont(1) )
   pCanvas->WriteTextBox( 12.5, 4.2, 16.6, 4.9, "[123456]", hPen )
   pCanvas->WriteTextBox( 16.6, 4.2, 20.4, 4.9, "2023-03-14", hPen )

   pCanvas->UseFont( hFont(0) )
   pCanvas->DrawSolidRect( 1.5, 6.0, 9.5, 6.7, hPen, clrLtGray )
   pCanvas->WriteText( 1.8, 6.2, "BILL TO", , clrLtGray )
   pCanvas->UseFont( hFont(1) )
   pCanvas->WriteText( 1.5, 7.0, "[Name]" )
   pCanvas->WriteText( 1.5, 7.5, "[Company Name]")
   pCanvas->WriteText( 1.5, 8.0, "[Street Address]" )
   pCanvas->WriteText( 1.5, 8.5, "[City, ST ZIP]" )
   pCanvas->WriteText( 1.5, 9.0, "[Phone]" )
   pCanvas->WriteText( 1.5, 9.5, "[Email Address]" )

   pCanvas->UseFont( hFont(0) )
   pCanvas->DrawSolidRect( 1.5, 11.0, 16.6, 11.7, hPen, clrLtGray )
   pCanvas->WriteText( 1.8, 11.2, "DESCRIPTION", , clrLtGray )
   pCanvas->WriteTextBox( 16.6, 11.0, 20.4, 11.7, "AMOUNT", hPen, clrLtGray )
   pCanvas->DrawRect( 1.5, 11.7, 16.6, 21.0, hPen )
   pCanvas->DrawRect( 16.6, 11.7, 20.4, 21.0, hPen )

   pCanvas->UseFont( hFont(1) )
   pCanvas->WriteTextBox( 1.8, 12.0, 16.0, 12.5, "Service Fee", hPenInvisible, , DT_LEFT or DT_VCENTER )
   pCanvas->WriteTextBox( 1.8, 12.5, 16.0, 13.0, "Labor: 5 hours at $75/hr", hPenInvisible, , DT_LEFT or DT_VCENTER )
   pCanvas->WriteTextBox( 1.8, 13.0, 16.0, 13.5, "New client discount", hPenInvisible, , DT_LEFT or DT_VCENTER )
   pCanvas->WriteTextBox( 1.8, 13.5, 16.0, 14.0, "Tax (4.25% after discount)", hPenInvisible, , DT_LEFT or DT_VCENTER )

   pCanvas->WriteTextBox( 18.2, 12.0, 20.2, 12.5, "200.00", hPenInvisible, , DT_RIGHT or DT_VCENTER )
   pCanvas->WriteTextBox( 18.2, 12.5, 20.2, 13.0, "375.00", hPenInvisible, , DT_RIGHT or DT_VCENTER )
   pCanvas->WriteTextBox( 18.2, 13.0, 20.2, 13.5, "(50.00)", hPenInvisible, , DT_RIGHT or DT_VCENTER )
   pCanvas->WriteTextBox( 18.2, 13.5, 20.2, 14.0, "26.56", hPenInvisible, , DT_RIGHT or DT_VCENTER )

   pCanvas->UseFont( hFont(2) )
   pCanvas->WriteTextBox( 1.5, 21.0, 12.5, 22.0, "Thank you for your business!", hPen )

   pCanvas->UseFont( hFont(3) )
   pCanvas->DrawRect( 12.5, 21.0, 20.4, 22.0, hPen )
   pCanvas->WriteTextBox( 13.0, 21.2, 16.6, 21.8, "TOTAL", hPenInvisible, , DT_LEFT or DT_VCENTER )
   pCanvas->WriteTextBox( 16.8, 21.2, 18, 21.8, "$", hPenInvisible, , DT_LEFT or DT_VCENTER  )
   pCanvas->WriteTextBox( 18.2, 21.2, 20.2, 21.8, "551.56", hPenInvisible, , DT_RIGHT or DT_VCENTER )

   dim as CWSTR wszText
   pCanvas->UseFont( hFont(1) )
   wszText = "If you have any questions about this invoice, please contact"
   pCanvas->WriteTextBox( 1.5, 25.0, 20.4, 25.5, wszText, hPenInvisible )
   wszText = "[Name, Phone, email@address.com]"
   pCanvas->WriteTextBox( 1.5, 25.5, 20.4, 26.0, wszText, hPenInvisible )

   function = pCanvas
end function



' ========================================================================================
' Main
' ========================================================================================
FUNCTION WinMain (BYVAL hInstance AS HINSTANCE, _
                  BYVAL hPrevInstance AS HINSTANCE, _
                  BYVAL szCmdLine AS ZSTRING PTR, _
                  BYVAL nCmdShow AS LONG) AS LONG

   ' // Creates the main window
   DIM pWindow AS CWindow
   pWindow.Create(NULL, "Print Preview Class Demo", @WndProc)

   ' // Sizes it by setting the wanted width and height of its client area
   pWindow.SetClientSize(500, 320)

   ' // Centers the window
   pWindow.Center

   ' // Adds a button
   pWindow.AddControl("Button", , IDC_PREVIEW, "&Preview", 350, 200, 75, 23)
   pWindow.AddControl("Button", , IDCANCEL, "&Close", 350, 250, 75, 23)

   ' // Displays the window and dispatches the Windows messages
   FUNCTION = pWindow.DoEvents(nCmdShow)

END FUNCTION
' ========================================================================================

' ========================================================================================
' Main window procedure
' ========================================================================================
FUNCTION WndProc (BYVAL hwnd AS HWND, BYVAL uMsg AS UINT, BYVAL wParam AS WPARAM, BYVAL lParam AS LPARAM) AS LRESULT

   SELECT CASE uMsg

      CASE WM_COMMAND
         SELECT CASE GET_WM_COMMAND_ID(wParam, lParam)
            CASE IDCANCEL
               ' // If ESC key pressed, close the application by sending an WM_CLOSE message
               IF GET_WM_COMMAND_CMD(wParam, lParam) = BN_CLICKED THEN
                  SendMessageW hwnd, WM_CLOSE, 0, 0
                  EXIT FUNCTION
               END IF
         
            CASE IDC_PREVIEW
               ' Create an in-memory print preview Canvas
               dim pCanvas as CPrintPreview ptr = CreatePrintPreviewCanvas()
               ' Pass our Canvas to our dialog that displays the print preview. If we
               ' simply wanted to output the Canvas to the printer then we could 
               ' set the printer properties to the Canvas and call pCanvas->PrintDoc
               ShowPrintPreviewDlg( HWND, pCanvas )
         END SELECT

      CASE WM_SIZE
         ' // Optional resizing code
         IF wParam <> SIZE_MINIMIZED THEN
            ' // Retrieve a pointer to the CWindow class
            DIM pWindow AS CWindow PTR = AfxCWindowPtr(hwnd)
            ' // Move the position of the button
            IF pWindow THEN 
               pWindow->MoveWindow GetDlgItem(hwnd, IDCANCEL), pWindow->ClientWidth - 120, pWindow->ClientHeight - 50, 75, 23, CTRUE
               pWindow->MoveWindow GetDlgItem(hwnd, IDC_PREVIEW), pWindow->ClientWidth - 120, pWindow->ClientHeight - 100, 75, 23, CTRUE
            END IF   
         END IF

    	CASE WM_DESTROY
         ' // Ends the application by sending a WM_QUIT message
         PostQuitMessage(0)
         EXIT FUNCTION

   END SELECT

   ' // Default processing of Windows messages
   FUNCTION = DefWindowProcW(hwnd, uMsg, wParam, lParam)

END FUNCTION
' ========================================================================================
