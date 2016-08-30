// IMDisplay.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "IMDisplay.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "IMDisplayDoc.h"
#include "IMDisplayView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIMDisplayApp

BEGIN_MESSAGE_MAP(CIMDisplayApp, CWinApp)
	//{{AFX_MSG_MAP(CIMDisplayApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIMDisplayApp construction

CIMDisplayApp::CIMDisplayApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CIMDisplayApp object

CIMDisplayApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CIMDisplayApp initialization

BOOL CIMDisplayApp::InitInstance()
{
	if (!AfxOleInit())
		return FALSE;

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("ImageMagick"));

	LoadStdProfileSettings(5);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;

        //TH all image types, default
        pDocTemplate = new CMultiDocTemplate(
	        IDR_IMIMAGETYPE,
	        RUNTIME_CLASS(CIMDisplayDoc),
	        RUNTIME_CLASS(CChildFrame), // custom MDI child frame
	        RUNTIME_CLASS(CIMDisplayView));
	AddDocTemplate(pDocTemplate);

        // BMP
	pDocTemplate = new CMultiDocTemplate(
		IDR_IMBMPTYPE,
		RUNTIME_CLASS(CIMDisplayDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CIMDisplayView));
	AddDocTemplate(pDocTemplate);

        // EPS
        pDocTemplate = new CMultiDocTemplate(
	        IDR_IMEPSTYPE,
	        RUNTIME_CLASS(CIMDisplayDoc),
	        RUNTIME_CLASS(CChildFrame), // custom MDI child frame
	        RUNTIME_CLASS(CIMDisplayView));
	AddDocTemplate(pDocTemplate);

        // GIF
        pDocTemplate = new CMultiDocTemplate(
	        IDR_IMGIFTYPE,
	        RUNTIME_CLASS(CIMDisplayDoc),
	        RUNTIME_CLASS(CChildFrame), // custom MDI child frame
	        RUNTIME_CLASS(CIMDisplayView));
	AddDocTemplate(pDocTemplate);

        // JPEG
	pDocTemplate = new CMultiDocTemplate(
		IDR_IMJPEGTYPE,
		RUNTIME_CLASS(CIMDisplayDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CIMDisplayView));
	AddDocTemplate(pDocTemplate);

        // MIFF
        pDocTemplate = new CMultiDocTemplate(
	        IDR_IMMIFFTYPE,
	        RUNTIME_CLASS(CIMDisplayDoc),
	        RUNTIME_CLASS(CChildFrame), // custom MDI child frame
	        RUNTIME_CLASS(CIMDisplayView));
	AddDocTemplate(pDocTemplate);

        // PNG
	pDocTemplate = new CMultiDocTemplate(
		IDR_IMPNGTYPE,
		RUNTIME_CLASS(CIMDisplayDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CIMDisplayView));
	AddDocTemplate(pDocTemplate);

        // TIFF
	pDocTemplate = new CMultiDocTemplate(
		IDR_IMTIFFTYPE,
		RUNTIME_CLASS(CIMDisplayDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CIMDisplayView));
	AddDocTemplate(pDocTemplate);

        // SVG
	pDocTemplate = new CMultiDocTemplate(
		IDR_IMSVGTYPE,
		RUNTIME_CLASS(CIMDisplayDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CIMDisplayView));
	AddDocTemplate(pDocTemplate);

        // WMF
        pDocTemplate = new CMultiDocTemplate(
	        IDR_IMWMFTYPE,
	        RUNTIME_CLASS(CIMDisplayDoc),
	        RUNTIME_CLASS(CChildFrame), // custom MDI child frame
	        RUNTIME_CLASS(CIMDisplayView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// BOGUS: find a way to generalize this!
	//char	exePath[2048];
	//DWORD	pathSize = GetModuleFileName( NULL, exePath, 2048 );
	//exePath[ pathSize ] = 0;
	//char*	ps = (char*)(exePath + pathSize - 1);
	//while ( *ps != '\\' )	{ *ps = 0; ps--; }	// shrink it!
        //MagickCore::InitializeMagick( exePath );

        MagickCore::MagickCoreGenesis( NULL, MagickFalse );

	// we do this to init the coder list, but will use it
	// more seriously in the future when we actually build up
	// the list of support modules dynamically!
	try {
	    std::list<CoderInfo> coderList;
		coderInfoList( &coderList, CoderInfo::TrueMatch, CoderInfo::AnyMatch, CoderInfo::AnyMatch);
	}
	catch(Exception e) {
	    e.what();
	};

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// Enable drag/drop open
	pMainFrame->DragAcceptFiles();

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetDlgItemText (IDC_STATIC_VERSION, CString("Version: ") + MagickVersion); //TH

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CIMDisplayApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CIMDisplayApp message handlers
