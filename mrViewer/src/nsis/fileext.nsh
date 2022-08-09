!include nsDialogs.nsh 
!include WinMessages.nsh 
!ifndef LVM_GETITEMTEXT 
!define /math LVM_GETITEMTEXTA ${LVM_FIRST} + 45 
!define /math LVM_GETITEMTEXTW ${LVM_FIRST} + 115 
${_NSIS_DEFAW} LVM_GETITEMTEXT 
!endif 
Var hListCtl
Var bCheckAll
Var bUnCheckAll
Page Custom LVPageCreate LVPageLeave 

Function AddCheckedListViewItemWith1SubItem  
System::Store S  
Pop $4 
Pop $3  
Pop $2  
Pop $1  
System::Call '*(i ${LVIF_TEXT},i 0x7fffffff,i 0,i,&i${NSIS_PTR_SIZE},tr2,i,i,p)p.r9'  
SendMessage $1 ${LVM_INSERTITEM} "" $9 $0  
System::Call '*$9(i${LVIF_STATE},i,i,i0x2000,&i${NSIS_PTR_SIZE} ${LVIS_STATEIMAGEMASK},p,i,i,p)'  
IntCmpU $4 0 +2 
SendMessage $1 ${LVM_SETITEMSTATE} $0 $9 $8
System::Call '*$9(i,i 0x7fffffff,i 1,i,i,tr3,i,i,p)'  
SendMessage $1 ${LVM_SETITEMTEXT} $0 $9  
System::Free $9  
System::Store L  
FunctionEnd

!macro LVUnCheckAll hLV tempvar
System::Call '*(i ${LVIF_STATE},i,i 0,i0x1000,&i${NSIS_PTR_SIZE} ${LVIS_STATEIMAGEMASK},p0,i0,i,p)p.s'
Pop ${tempvar}
SendMessage ${hLV} ${LVM_SETITEMSTATE} -1 ${tempvar}
System::Free ${tempvar}
!macroend

Function UnCheckAll
  !insertmacro LVUnCheckAll $hListCtl $0
FunctionEnd

!macro LVCheckAll hLV tempvar
System::Call '*(i ${LVIF_STATE},i,i 0,i0x2000,&i${NSIS_PTR_SIZE} ${LVIS_STATEIMAGEMASK},p0,i0,i,p)p.s'
Pop ${tempvar}
SendMessage ${hLV} ${LVM_SETITEMSTATE} -1 ${tempvar}
System::Free ${tempvar}
!macroend

Function CheckAll
  !insertmacro LVCheckAll $hListCtl $0
FunctionEnd

!macro AddCheckedListViewItemWith1SubItem hLV txt sub1 checked 
Push ${hLV}  
Push "${txt}"  
Push "${sub1}"  
Push "${checked}" 
Call AddCheckedListViewItemWith1SubItem  
!macroend  

Function LVPageCreate 
nsDialogs::Create 1018 
Pop $0 

${NSD_CreateLabel} 0 0 100% 12u "File associations for mrViewer"

nsDialogs::CreateControl "SysListView32" ${DEFAULT_STYLES}|${WS_TABSTOP}|${WS_VSCROLL}|${LVS_REPORT} ${WS_EX_WINDOWEDGE}|${WS_EX_CLIENTEDGE} 0 30% 100% 70% "" 
Pop $hListCtl 
IntOp $0 ${LVS_EX_FULLROWSELECT} | ${LVS_EX_CHECKBOXES} 
SendMessage $hListCtl ${LVM_SETEXTENDEDLISTVIEWSTYLE} 0 $0 
System::Call '*(i${LVCF_TEXT}|${LVCF_SUBITEM},i,i,t "Extension",i,i 0)p.r9' 
SendMessage $hListCtl ${LVM_INSERTCOLUMN} 0x7fffffff $9 
System::Call '*$9(i,i,i,t "Description",i,i 1)' 
SendMessage $hListCtl ${LVM_INSERTCOLUMN} 0x7fffffff $9 
System::Free $9 
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".au" "Solaris Audio file" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".bit" "mental ray Bitmap" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".bmp" "Windows/OS2 Bitmap" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".braw" "Blackmagic RAW Movie" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".cin" "Kodak Cineon" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".ct" "mental ray contour" 1 
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".dds" "DirectDraw Surface" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".dib" "Windows Device Independent Bitmap" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".dng" "Adobe Digital Negative Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".dpx" "Digital Picture Exchange" 1 
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".exr" "ILM OpenEXR" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".gif" "Graphics Interchange Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".hdr" "Radiance High Dynamic Range" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".hdri" "Radiance High Dynamic Range Imaging" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".iff" "Interchange File Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".jpeg" "JPEG File Interchange Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".jpg" "JPEG File Interchange Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".jfif" "JPEG File Interchange Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".nt" "mental ray normal map" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mt" "mental ray motion map" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".map" "mental ray rip map (non-tiled)" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".pbm" "Portable Bit Map" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".pgm" "Portable Gray Map" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".pic" "Softimage picture format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".png" "Portable Network Graphics" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".pnm" "Portable Anymap Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".ppm" "Portable PixMap" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".psd" "Photoshop" 0
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".ras" "Sun Raster Format" 1 
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".reel" "mrViewer's EDL Reel Format" 1 
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".rgb" "SGI RGB Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".rgba" "SGI RGBA Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".rla" "Run Length Encoded A SGI Format" 1 
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".rpf" "Autodesk's Rich Pixel Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".sgi" "SGI Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".shmap" "mental ray Shadow Map" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".st" "mental ray Scalar Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".sxr" "ILM Stereo OpenEXR Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".tga" "TrueVision TARGA Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".tiff" "Tagged Image File Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".tif" "Tagged Image File Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".xcf" "GIMP single layer" 0
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".zt" "mental ray Z depth files" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".z" "Pixar Renderman Z files" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".avi" "Audio Video Interleave" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".divx" "DIVX Media Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".dv" "Digital Video Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".flv" "Flash Video Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".m4v" "Apple M4V Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mkv" "Matroska Video Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mov" "Apple's Quicktime Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mp4" "MPEG4 Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mpg" "MPEG Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mpeg" "MPEG Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mpeg2" "MPEG2 Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mpeg3" "MPEG3 Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mpeg4" "MPEG4 Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mxf" "Material eXchange Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".qt" "Apple's Quicktime" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".rm" "RealNetwork's Real Media" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".vob" "DVD Video Object Format" 0
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".vp9" "WebM Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".webm" "WebM Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".wmv" "Windows Media Video" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".y4m" "YUV4MPEG2 Format" 1
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".mp3" "MPEG1/2 Audio Layer III" 0
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".ogg" "Ogg Audio Format" 0
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".vorbis" "Ogg Vorbis Audio Format" 0
!insertmacro AddCheckedListViewItemWith1SubItem $hListCtl ".wav" "Waveform Audio Format" 0

SendMessage $hListCtl ${LVM_SETCOLUMNWIDTH} 0 -1 
SendMessage $hListCtl ${LVM_SETCOLUMNWIDTH} 1 -1

${NSD_CreateButton} 0 15u 30% 10% "Check All"
Pop $bCheckAll
${NSD_OnClick} $bCheckAll checkAll

${NSD_CreateButton} 70% 15u 30% 10% "Uncheck All"
Pop $bUnCheckAll
${NSD_OnClick} $bUnCheckAll unCheckAll

System::Call 'USER32::PostMessage(p $hwndparent, i ${WM_NEXTDLGCTL}, p $hListCtl, i1)'
nsDialogs::Show 
FunctionEnd 

Function LVPageLeave 
System::Call '*(&t${NSIS_MAX_STRLEN},i)p.r8' 
System::Call '*(i ${LVIF_TEXT},i,i 0,i,&i${NSIS_PTR_SIZE},pr8,i${NSIS_MAX_STRLEN},i,p)p.r9' 
SendMessage $hListCtl ${LVM_GETITEMCOUNT} "" "" $1 
StrCpy $0 0 
${DoWhile} $0 < $1 
    SendMessage $hListCtl ${LVM_GETITEMSTATE} $0 ${LVIS_STATEIMAGEMASK} $2 
    IntOp $2 $2 & 0x2000 
    ${If} $2 <> 0 
        SendMessage $hListCtl ${LVM_GETITEMTEXT} $0 $9 $2 
        System::Call '*$8(&t${NSIS_MAX_STRLEN}.r7)' 
	WriteRegStr HKCR '$7' '' 'mrViewer'
	WriteINIStr '$INSTDIR\\fileext.ini' ext '$7' 1
    ${EndIf}
    IntOp $0 $0 + 1 
${Loop} 
System::Free $8 
System::Free $9 
FunctionEnd
