' ########################################################################################
' Print Preview Dialog
' File: PrintPreviewDlg.inc
' Contents: Generic popup Print Preview dialog that uses the CPrintPreview canvas
' Compiler: FreeBasic 32 & 64 bit
' Copyright (c) 2023 Paul Squires. Use at your own risk.
' THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
' EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
' MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
' ########################################################################################

#define UNICODE

#include once "Afx/CWindow.inc"
#include once "Afx/CPrint.inc"
#include once "../src/CPrintPreview.inc"

const PREVIEW_TOOLBAR_HEIGHT = 30
const IDC_CANVAS  = 100
const IDC_DOPRINT = 101
const IDC_GOFIRST = 102
const IDC_GOPREV  = 103
const IDC_GONEXT  = 104
const IDC_GOLAST  = 105
const IDC_PAGENUM = 106
const IDC_DOZOOM  = 107
const IDC_DOCLOSE = 108


' ========================================================================================
' UpdatePageCaption
' Update our caption to show the correct page number (Page xx of xx)
' ========================================================================================
function UpdatePageCaption( byval hWndCaption as HWND, byval pCanvas as CPrintPreview ptr ) as long
   dim as CWSTR wszText
   if pCanvas then wszText = "Page " & pCanvas->CurrentPage + 1 & " of " & pCanvas->PageCount
   AfxSetWindowText( hWndCaption, wszText )
   function = 0         
end function         


' ========================================================================================
' PrintPreviewDlg Window procedure
' ========================================================================================
Function PrintPreviewDlg_WndProc( _
               ByVal hWin   As HWnd, _
               ByVal uMsg   As UINT, _
               ByVal wParam As WPARAM, _
               ByVal lParam As LPARAM _
               ) As LRESULT

   Select Case uMsg

    	case WM_DESTROY
         PostQuitMessage(0)
         exit function

      case WM_SIZE
         dim as long cxClient = loword( lParam )
         dim as long cyClient = hiword( lParam )

         ' Position the canvas window immediately under the navigation controls
         dim as long nCtrlHeight = AfxScaleY( PREVIEW_TOOLBAR_HEIGHT )
         SetWindowPos( GetDlgItem( hWin, IDC_CANVAS ), 0, _
                                   0, nCtrlHeight, _
                                   cxClient, cyClient - nCtrlHeight, _
                                   SWP_NOZORDER or SWP_SHOWWINDOW )
         exit function

      CASE WM_COMMAND
         ' Rather than use a global/shared pointer to our CPrintPreview class, the 
         ' pointer already exists in the UserData area of the CWindow class associated
         ' with our canvas. So, we merely need to get that pointer from UserData(0).
         dim pWindow as CWindow ptr = AfxCWindowPtr( GetDlgItem(hWin, IDC_CANVAS) )
         dim pCanvas as CPrintPreview ptr 
         if pWindow then pCanvas = cast( CPrintPreview ptr, pWindow->UserData(0) )

         SELECT CASE GET_WM_COMMAND_ID(wParam, lParam)
            CASE IDC_DOPRINT
               IF GET_WM_COMMAND_CMD(wParam, lParam) = BN_CLICKED THEN
                  if pCanvas then 
                     if pCanvas->ChoosePrinter(hWin) then
                        pCanvas->PrintDoc()
                        AfxRedrawWindow( GetDlgItem( hWin, IDC_CANVAS ) )
                     end if   
                  end if
                  EXIT FUNCTION
               END IF
         
            CASE IDC_GOFIRST
               IF GET_WM_COMMAND_CMD(wParam, lParam) = BN_CLICKED THEN
                  if pCanvas then
                     pCanvas->CurrentPage = 0
                     UpdatePageCaption( GetDlgItem(hWin, IDC_PAGENUM), pCanvas)
                     AfxRedrawWindow( GetDlgItem( hWin, IDC_CANVAS ) )
                  end if
               END IF

            CASE IDC_GOPREV
               IF GET_WM_COMMAND_CMD(wParam, lParam) = BN_CLICKED THEN
                  if pCanvas then
                     if pCanvas->CurrentPage - 1 >= 0 then
                        pCanvas->CurrentPage = pCanvas->CurrentPage - 1
                        UpdatePageCaption( GetDlgItem(hWin, IDC_PAGENUM), pCanvas)
                        AfxRedrawWindow( GetDlgItem( hWin, IDC_CANVAS ) )
                     end if   
                  end if
               END IF

            CASE IDC_GONEXT
               IF GET_WM_COMMAND_CMD(wParam, lParam) = BN_CLICKED THEN
                  if pCanvas then
                     if pCanvas->CurrentPage + 1 <= pCanvas->PageCount - 1 then
                        pCanvas->CurrentPage = pCanvas->CurrentPage + 1
                        UpdatePageCaption( GetDlgItem(hWin, IDC_PAGENUM), pCanvas)
                        AfxRedrawWindow( GetDlgItem( hWin, IDC_CANVAS ) )
                     end if   
                  end if
               END IF

            CASE IDC_GOLAST
               IF GET_WM_COMMAND_CMD(wParam, lParam) = BN_CLICKED THEN
                  if pCanvas then
                     if pCanvas->CurrentPage <> pCanvas->PageCount - 1 then
                        pCanvas->CurrentPage = pCanvas->PageCount - 1
                        UpdatePageCaption( GetDlgItem(hWin, IDC_PAGENUM), pCanvas)
                        AfxRedrawWindow( GetDlgItem( hWin, IDC_CANVAS ) )
                     end if
                  end if
               END IF

            CASE IDC_DOZOOM
               IF GET_WM_COMMAND_CMD(wParam, lParam) = CBN_SELCHANGE THEN
                  dim as HWND hCombo = cast(HWND, lParam)
                  dim as long nCurSel = SendMessage( hCombo, CB_GETCURSEL, 0, 0 )
                  if pCanvas then 
                     pCanvas->Zoom = cast( ZoomLevel, nCurSel )
                     pCanvas->VScrollPos = 1
                     pCanvas->HScrollPos = 1
                     AfxRedrawWindow( GetDlgItem( hWin, IDC_CANVAS ) )
                  end if   
               END IF

            CASE IDC_DOCLOSE
               IF GET_WM_COMMAND_CMD(wParam, lParam) = BN_CLICKED THEN
                  SendMessage( hWin, WM_CLOSE, 0, 0 )
                  exit function
               END IF

         END SELECT
         
   End Select

    
   Function = DefWindowProc(hWin, uMsg, wParam, lParam)

End Function
' ========================================================================================

' ========================================================================================
' Preview (modal) Window procedure
' ========================================================================================
function ShowPrintPreviewDlg( byval hWndParent as HWND, pCanvas as CPrintPreview ptr ) as LRESULT

   ' The Print Preview dialog expects that the calling program has already created
   ' the print preview Canvas and has passed it as a pointer to this procedure.
   ' This procedure will delete the pCanvas pointer when it exits.
   
   ' Test that a valid pCanvas exists otherwise there will be nothing to display.
   if pCanvas = null then exit function
   
   ' Create the popup window that displays the print preview.
   Dim pWindow as CWindow
    
   dim as CWSTR wszText = "Print Preview"
   
   Dim As HWnd hForm = _
   pWindow.Create( hWndParent, wszText, _
        @PrintPreviewDlg_WndProc, 0, 0, 600, 400, _
        WS_OVERLAPPEDWINDOW or WS_CLIPSIBLINGS Or WS_CLIPCHILDREN or WS_MAXIMIZE, _
        WS_EX_CONTROLPARENT Or WS_EX_LEFT Or WS_EX_LTRREADING Or WS_EX_RIGHTSCROLLBAR)

    
   ' Add the buttons
   dim as HWND hCtrl
   
   hCtrl = pWindow.AddControl("Button",   , IDC_DOPRINT, "Print",   0, 6, 55, 20)         
   AfxAddTooltip( hCtrl, "Print the document" )
   
   hCtrl = pWindow.AddControl("Button",   , IDC_GOFIRST, "<<",     75, 6, 28, 20)
   AfxAddTooltip( hCtrl, "First page" )
   
   hCtrl = pWindow.AddControl("Button",   , IDC_GOPREV,  "<",     105, 6, 28, 20)
   AfxAddTooltip( hCtrl, "Previous page" )
   
   hCtrl = pWindow.AddControl("Button",   , IDC_GONEXT,  ">",     135, 6, 28, 20)
   AfxAddTooltip( hCtrl, "Next page" )
   
   hCtrl = pWindow.AddControl("Button",   , IDC_GOLAST,  ">>",    165, 6, 28, 20)
   AfxAddTooltip( hCtrl, "Last page" )

   hCtrl = pWindow.AddControl("Label",    , IDC_PAGENUM,  "", 213, 6, 100, 20, _
                               WS_CHILD or WS_VISIBLE OR SS_CENTER or SS_CENTERIMAGE )
   
   ' Load the zoom options into the combobox
   hCtrl = pWindow.AddControl("ComboBox", , IDC_DOZOOM,  "", 333, 6, 120, 20, _
                               WS_CHILD OR WS_VISIBLE OR WS_VSCROLL OR WS_BORDER OR _
                               WS_TABSTOP OR CBS_DROPDOWNLIST OR CBS_HASSTRINGS )

   dim as long nIndex
   AfxAddTooltip( hCtrl, "Zoom level" )
   nIndex = ComboBox_AddString( hCtrl, @wstr("Fit to Window") )
   ComboBox_AddString( hCtrl, @wstr("Fit to Width") )
   ComboBox_AddString( hCtrl, @wstr("125%") )
   ComboBox_AddString( hCtrl, @wstr("100%") )
   ComboBox_AddString( hCtrl, @wstr("75%") )
   ComboBox_SetCurSel( hCtrl, nIndex )
   
   hCtrl = pWindow.AddControl("Button",   , IDC_DOCLOSE, "Close", 473, 6, 55, 20)
   AfxAddTooltip( hCtrl, "Close the Window" )
   
   pCanvas->CurrentPage = 0
   pCanvas->AttachToParentWindow( hForm, IDC_CANVAS )
   UpdatePageCaption( GetDlgItem(hForm, IDC_PAGENUM), pCanvas)

   ' Process Windows modal messages
   pWindow.DoEvents( SW_SHOW )
   
   ' Delete the pCanvas pointer
   if pCanvas then 
      ' Do not delete the Canvas until it is safe to do so. If we delete the Canvas
      ' while a job is processing to the print queue then we could get a GPF.
      do until pCanvas->IsPrinting = false
      loop
      Delete(pCanvas)
   end if
    
   function  = 0
end function
' ========================================================================================

