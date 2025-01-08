// �ű�������Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "�ű�������.h"
#include "�ű�������Dlg.h"
#include "ICTScript2.h"
#include "CTScript2.h"
#include "ctScriptTestCase2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CScriptDlg �Ի���




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


// CScriptDlg ��Ϣ�������

ns_my_script2::CCTScript script;
ns_my_script2::ICTScript _ctscript;
ns_my_script2::ICTScript * ctscript=NULL;
string stlstr;
vector<pair<string, ns_my_script2::Variable > > envs;

BOOL CScriptDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	InitActiveApp("ctScript", 1024 * 1024 * 10, 0, 0);
	ctscript = &_ctscript;
	if(NULL==ctscript)
	{
		MessageBox("�ڴ治��","����",MB_ICONSTOP);
		OnCancel();
	}
	ctscript->AttachScript(&script);
	pair<string, ns_my_script2::Variable > tmppair;
	ns_my_script2::Variable tmpvar;
	tmpvar.type= ns_my_script2::Variable::LONG;
	tmppair.first="dis_total";
	tmppair.second=tmpvar;
	envs.push_back(tmppair);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CScriptDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CScriptDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CScriptDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString str;

	m_Edit_Source.GetWindowText(str);
	if(ctscript->GetSource()!=str.GetString() || !ctscript->IsCompiled())
	{
		if(!ctscript->Compile(str,&envs))
		{
			MessageBox(ctscript->GetMessage().c_str(),"����",MB_OK|MB_ICONSTOP);
		}
		m_Edit_Report.SetWindowText(ctscript->Report(stlstr).c_str());
	}
	//OnOK();
}

void CScriptDlg::OnBnClickedOk2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
			if(0==i%1000)MessageBox(var.GetString().c_str(),"���н��");
		}
		envs[0].second=10L;
		if(!ctscript->Execute(var,&envs))
		{
			MessageBox(ctscript->GetMessage().c_str(),"����",MB_OK|MB_ICONSTOP);
		}
		else
		{
			MessageBox(var.GetString().c_str(),"���н��");
		}
		m_Edit_Report.SetWindowText(ctscript->Report(stlstr).c_str());
	}
}

void CScriptDlg::OnBnClickedOk3()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_Edit_Report.SetWindowText(ns_my_script2::CPluginMap::PluginHelp(stlstr).c_str());
}

void CScriptDlg::OnBnClickedOk4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_Edit_Report.SetWindowText(ns_my_script2::CHelp::LanguageHelp(stlstr).c_str());
}

void CScriptDlg::OnBnClickedOk5()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_Edit_Report.SetWindowText(ns_my_script2::CHelp::Example(stlstr).c_str());
}


void CScriptDlg::OnBnClickedOk6()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ns_my_script2::CTestCase tc;
	string msg;
	if (tc.doTestCase())
	{
		MessageBox("���Գɹ�");
	}
	else
	{
		MessageBox("����ʧ��");
	}
	m_Edit_Report.SetWindowText(msg.c_str());
	m_Edit_Report.LineScroll(m_Edit_Report.GetLineCount());
}
