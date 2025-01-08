// 脚本编译器Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "脚本编译器.h"
#include "脚本编译器Dlg.h"
#include "ICTScript2.h"
#include "CTScript2.h"
#include "ctScriptTestCase2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CScriptDlg 对话框




CScriptDlg::CScriptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScriptDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScriptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SOURCE, m_Edit_Source);
	DDX_Control(pDX, IDC_EDIT_REPORT, m_Edit_Report);
}

BEGIN_MESSAGE_MAP(CScriptDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CScriptDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDOK2, &CScriptDlg::OnBnClickedOk2)
	ON_BN_CLICKED(IDOK3, &CScriptDlg::OnBnClickedOk3)
	ON_BN_CLICKED(IDOK4, &CScriptDlg::OnBnClickedOk4)
	ON_BN_CLICKED(IDOK5, &CScriptDlg::OnBnClickedOk5)
	ON_BN_CLICKED(IDOK6, &CScriptDlg::OnBnClickedOk6)
END_MESSAGE_MAP()


// CScriptDlg 消息处理程序

ns_my_script2::CCTScript script;
ns_my_script2::ICTScript _ctscript;
ns_my_script2::ICTScript * ctscript=NULL;
string stlstr;
vector<pair<string, ns_my_script2::Variable > > envs;

BOOL CScriptDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitActiveApp("ctScript", 1024 * 1024 * 10, 0, 0);
	ctscript = &_ctscript;
	if(NULL==ctscript)
	{
		MessageBox("内存不足","错误",MB_ICONSTOP);
		OnCancel();
	}
	ctscript->AttachScript(&script);
	pair<string, ns_my_script2::Variable > tmppair;
	ns_my_script2::Variable tmpvar;
	tmpvar.type= ns_my_script2::Variable::LONG;
	tmppair.first="dis_total";
	tmppair.second=tmpvar;
	envs.push_back(tmppair);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CScriptDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CScriptDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CScriptDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CScriptDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;

	m_Edit_Source.GetWindowText(str);
	if(ctscript->GetSource()!=str.GetString() || !ctscript->IsCompiled())
	{
		if(!ctscript->Compile(str,&envs))
		{
			MessageBox(ctscript->GetMessage().c_str(),"出错",MB_OK|MB_ICONSTOP);
		}
		m_Edit_Report.SetWindowText(ctscript->Report(stlstr).c_str());
	}
	//OnOK();
}

void CScriptDlg::OnBnClickedOk2()
{
	// TODO: 在此添加控件通知处理程序代码
	OnBnClickedOk();
	if(ctscript->IsCompiled())
	{
		ns_my_script2::Variable var;
		for(long i=0;i<10000;++i)
		{
			envs[0].second=i;
			if(!ctscript->Execute(var,&envs))
			{
				MessageBox("error");
			}
			if(0==i%1000)MessageBox(var.GetString().c_str(),"运行结果");
		}
		envs[0].second=10L;
		if(!ctscript->Execute(var,&envs))
		{
			MessageBox(ctscript->GetMessage().c_str(),"出错",MB_OK|MB_ICONSTOP);
		}
		else
		{
			MessageBox(var.GetString().c_str(),"运行结果");
		}
		m_Edit_Report.SetWindowText(ctscript->Report(stlstr).c_str());
	}
}

void CScriptDlg::OnBnClickedOk3()
{
	// TODO: 在此添加控件通知处理程序代码
	m_Edit_Report.SetWindowText(ns_my_script2::CPluginMap::PluginHelp(stlstr).c_str());
}

void CScriptDlg::OnBnClickedOk4()
{
	// TODO: 在此添加控件通知处理程序代码
	m_Edit_Report.SetWindowText(ns_my_script2::CHelp::LanguageHelp(stlstr).c_str());
}

void CScriptDlg::OnBnClickedOk5()
{
	// TODO: 在此添加控件通知处理程序代码
	m_Edit_Report.SetWindowText(ns_my_script2::CHelp::Example(stlstr).c_str());
}


void CScriptDlg::OnBnClickedOk6()
{
	// TODO: 在此添加控件通知处理程序代码
	ns_my_script2::CTestCase tc;
	string msg;
	if (tc.doTestCase())
	{
		MessageBox("测试成功");
	}
	else
	{
		MessageBox("测试失败");
	}
	m_Edit_Report.SetWindowText(msg.c_str());
	m_Edit_Report.LineScroll(m_Edit_Report.GetLineCount());
}
