// rescuekj2View.cpp : implementation of the CRescuekj2View class
//

#include "stdafx.h"
#include "rescuekj2.h"

#include "rescuekj2Doc.h"
#include "rescuekj2View.h"
#include <iostream>
#include <cstring>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRescuekj2View

IMPLEMENT_DYNCREATE(CRescuekj2View, CFormView)

BEGIN_MESSAGE_MAP(CRescuekj2View, CFormView)
	//{{AFX_MSG_MAP(CRescuekj2View)
	ON_COMMAND(IDC_ADD, OnAdd)
	ON_COMMAND(Zoom_In, OnZoomIn)
	ON_COMMAND(Zoom_Out, OnZoomOut)
	ON_COMMAND(Move_Pan, OnMovePan)
	ON_COMMAND(Cen_ter, OnCenter)
	ON_COMMAND(IDC_SET, OnTuCengSet)
	ON_WM_TIMER()
	ON_WM_SIZE()
	//ON_COMMAND(IDC_CESHI, OnCeshi)
	ON_COMMAND(IDC_JIANTOU, OnJiantou)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CFormView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRescuekj2View construction/destruction

CRescuekj2View::CRescuekj2View()
	: CFormView(CRescuekj2View::IDD)
{
	//{{AFX_DATA_INIT(CRescuekj2View)
	m_strRXData = _T("");
	//}}AFX_DATA_INIT
	// TODO: add construction code here

}

CRescuekj2View::~CRescuekj2View()
{

}

void CRescuekj2View::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_MAP1,m_ctrlMapX);
	CRescuekj2App *spp = (CRescuekj2App *)AfxGetApp();
	//{{AFX_DATA_MAP(CRescuekj2View)
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Control(pDX, IDC_MSCOMM1, spp->m_ctrlComm);
	DDX_Text(pDX, IDC_EDIT_HISTORYMEG, m_strRXData);
	//}}AFX_DATA_MAP
}

BOOL CRescuekj2View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CRescuekj2View::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
	m_bFlash=FALSE;
	int cx=0,cy=0;

	/******　　　MapX的初始化      **********/

	CRect rect;
	GetClientRect(&rect);
	cx=rect.Width();
	cy=rect.Height();
	m_ctrlMapX.MoveWindow(20,20,cx-400,cy-30,TRUE);//设置地图的大小
	m_ctrlMapX.SetGeoSet("jiangsu/china2012.gst");
	m_ctrlMapX.SetTitleText("");
	m_ctrlMapX.SetCenterX(118.85);//设置中心点经度纬度
	m_ctrlMapX.SetCenterY(32.06);
	m_ctrlMapX.SetZoom(500);//设置初始缩放大小
	CWnd *pWnd;  
	pWnd = GetDlgItem(IDC_EDIT_HISTORYMEG);    //根据控件ID号获取控件指针，此处为历史显示框
	pWnd->MoveWindow(cx-350,cy-320,320,308,TRUE);    //前两个为起始点，后两个为宽度和高度 
	pWnd = GetDlgItem(IDC_STATIC_HISTORY);    //此处为历史消息
	pWnd->MoveWindow(cx-350,cy-345,70,30,TRUE);    

	m_ctrlMapX.GetLayers().CreateLayer("tmplayer",NULL,1);//建立目标点层
	lyr = m_ctrlMapX.GetLayers().Item("tmplayer");
	m_ctrlMapX.GetLayers().SetAnimationLayer(lyr);
	
	m_traceLayer = m_ctrlMapX.GetLayers().CreateLayer("guiji",NULL, 2);//建立目标轨迹层
	m_ctrlMapX.GetLayers().SetAnimationLayer(m_traceLayer);
	m_traceLayer.SetSelectable(FALSE);
	m_traceLayer.SetDrawLabelsAfter(TRUE);//影响标注刷新的关键

		m_tab.InsertItem(0, _T("串口设置"));
		m_tab.InsertItem(1, _T("功率检测"));
		m_tab.InsertItem(2, _T("身体状况"));
		//设定在Tab内显示的范围
		CRect rc;
		m_tab.GetClientRect(rc);////获得TAB控件的坐标
		//定位选项卡页的位置，这里可以根据情况自己调节偏移量
		rc.top += 30;
		rc.bottom -= 0;
		rc.left += 0;
		rc.right -= 0;
		//创建两个对话框
		m_page1.Create(IDD_PORT, &m_tab);//&m_tab也可以改成Tab的ID
		m_page2.Create(IDD_GONGLV, &m_tab);
		m_page3.Create(IDD_INFO, &m_tab);
		//将子页面移动到指定的位置
		m_page1.MoveWindow(&rc);
		m_page2.MoveWindow(&rc);
		m_page3.MoveWindow(&rc);
		//显示子页面
		//显示初始页面
		m_page1.ShowWindow(SW_SHOW);
		m_page2.ShowWindow(SW_HIDE);
		m_page3.ShowWindow(SW_HIDE);
		//保存当前选择
		//m_CurSelTab = 0;
		m_tab.SetCurSel(0);

		for(int i=0;i<50;i++)
			book[i]=0;
			

	/*****    获得所有控件    ********/
	CRect rectWnd;
    GetWindowRect(&rectWnd);//得到当前对话框的坐标
    listRect.AddTail(&rectWnd);//将坐标添加到链表listRect的末尾
    CWnd *pWndChild=GetWindow(GW_CHILD);
    while (pWndChild)//依次得到对话框上控件的坐标，并将所有的控件坐标存储在链表中
   {
        pWndChild->GetWindowRect(&rectWnd);
        listRect.AddTail(&rectWnd);//由于依次将控件坐标添加到链表末尾，所以开头的坐标是对话框的坐标
        pWndChild=pWndChild->GetNextWindow();
	}
	/******   控件获取到此结束    *******/

	/*****      下面用于选择初始值和打开串口        ******/
	m_page1.Oninitial();	//必须放到次对话框创建后
	m_page2.Initial_gonglv();
	m_page3.Initial();
	/********     串口设置到此结束      **********/

	myAccess.OnInitADOConn();			//连接数据库
	myAccess.Openace();

	SetTimer(ID_TIMER_FLASH,1000,NULL);//打开定时器

}

/////////////////////////////////////////////////////////////////////////////
// CRescuekj2View printing

BOOL CRescuekj2View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CRescuekj2View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CRescuekj2View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CRescuekj2View::OnPrint(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	// TODO: add customized printing code here
}

/////////////////////////////////////////////////////////////////////////////
// CRescuekj2View diagnostics

#ifdef _DEBUG
void CRescuekj2View::AssertValid() const
{
	CFormView::AssertValid();
}

void CRescuekj2View::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CRescuekj2Doc* CRescuekj2View::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CRescuekj2Doc)));
	return (CRescuekj2Doc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CRescuekj2View message handlers

void CRescuekj2View::OnAdd() 
{
	// TODO: Add your command handler code here
	add addinfor;
	addinfor.DoModal();
}


void CRescuekj2View::OnZoomIn() //放大工具
{
	this->m_ctrlMapX.SetCurrentTool(miZoomInTool);	
}

void CRescuekj2View::OnZoomOut() //缩小工具
{
	// TODO: Add your command handler code here
	this->m_ctrlMapX.SetCurrentTool(miZoomOutTool);	
}

void CRescuekj2View::OnMovePan()	//抓手工具
{
	// TODO: Add your command handler code here
	this->m_ctrlMapX.SetCurrentTool(miPanTool);	
}

void CRescuekj2View::OnCenter() //居中工具
{
	// TODO: Add your command handler code here
	this->m_ctrlMapX.SetCurrentTool(miCenterTool);	
}

void CRescuekj2View::OnJiantou() //箭头工具
{
	// TODO: Add your command handler code here
	this->m_ctrlMapX.SetCurrentTool(miArrowTool);
}

void CRescuekj2View::OnTuCengSet() //图层设置
{
	// TODO: Add your command handler code here
	this->m_ctrlMapX.GetLayers().LayersDlg();
}

void CRescuekj2View::OnTimer(UINT nIDEvent) //定时器函数
{
	// TODO: Add your message handler code here and/or call default

	if(ID_TIMER_FLASH==nIDEvent)
	{
		m_bFlash=!m_bFlash;
		if(m_bFlash)
		{
			CMapXFeatures fts=m_ctrlMapX.GetLayers().Item(1).AllFeatures();//突出显示选中的图元
			m_ctrlMapX.GetLayers().Item(1).GetSelection().Add(fts);
		}
		else
		{
			m_ctrlMapX.GetLayers().Item(1).GetSelection().ClearSelection();
		}
	}
	CFormView::OnTimer(nIDEvent);
}

void CRescuekj2View::guiji(int id,double y,double x,double yg,double xg)
{
	CMapXFeature ft;
	ft=lyr.AllFeatures().Item(id);
	ft.Offset(y-yg,x-xg);
	
	xt.GetParts().Item(id).AddXY(y, x);//画轨迹

	ft.Update();
	xt.Update();
}
void CRescuekj2View::guiji_initial(int id,double y0,double x0,double y1,double x1) 
{
	// TODO: Add your command handler code here
	
	//CMapXFeatures ftr;
	//CString lyrName;
	//CMapXFeatureFactory ffy;
	/*x=32.32;
	y=119.52;
	yg=y;
	xg=x;
	m_ctrlMapX.GetLayers().CreateLayer("tmplayer",NULL,1);//建立目标点层
	lyr = m_ctrlMapX.GetLayers().Item("tmplayer");
	m_ctrlMapX.GetLayers().SetAnimationLayer(lyr);
	
	m_traceLayer = m_ctrlMapX.GetLayers().CreateLayer("guiji",NULL, 2);//建立目标轨迹层
	m_ctrlMapX.GetLayers().SetAnimationLayer(m_traceLayer);
	m_traceLayer.SetSelectable(FALSE);
	m_traceLayer.SetDrawLabelsAfter(TRUE);//影响标注刷新的关键



	CMapXFeature ft,st;
try
{
	if (ft.CreateDispatch(ft.GetClsid()))
	{
		ft.Attach(m_ctrlMapX.GetDispatch());
		ft = m_ctrlMapX.GetFeatureFactory().CreateSymbol();
		CMapXStyle s = ft.GetStyle();
		s.SetSymbolType(miSymbolTypeBitmap);
		s.SetSymbolBitmapSize(8);  //飞机大小
		s.SetSymbolBitmapTransparent(true);
		s.SetSymbolBitmapName("STOP1-32.bmp");
		ft.SetStyle(s);
		ft.GetPoint().Set(y,x);
		st=lyr.AddFeature(ft);
		st.Update();
	}
	else
	{
	AfxThrowOleException(CO_E_CLASS_CREATE_FAILED);
	}
	*/
	
		//guiji(117,30);

	/**********       画轨迹       *******/
try
{
		CMapXFeature gt,ft;
		gt.CreateDispatch(gt.GetClsid());
	    gt.Attach(m_ctrlMapX.GetDispatch(FALSE));
	    gt.SetType(miFeatureTypeLine);
	    gt.GetStyle().SetLineColor(miColorRed);
	    gt.GetStyle().SetLineStyle(3);//77-铁轨(参考帮助)
	    gt.GetStyle().SetLineWidth(2);
	    CMapXPoints pts;
	    pts.CreateDispatch(pts.GetClsid());
	    //加入坐标数据
	    pts.AddXY(y0, x0);
	    pts.AddXY(y1, x1);
	    gt.GetParts().Add(pts);
	    //加入到目标图层
	    xt=m_traceLayer.AddFeature((LPDISPATCH)gt); 

		ft=lyr.AllFeatures().Item(id);
		ft.Offset(y1-y0,x1-x0);
		ft.Update();

		xt.Update();
   /*************      到此     **************/

}

catch (COleDispatchException *e)
	{
	e->ReportError();
	e->Delete();
	}
catch (COleException *e)
	{
	e->ReportError();
	e->Delete();
	}	
}

void CRescuekj2View::biaodian(double y, double x)
{
	/*
	CMapXLayer lyr;
	lyr = m_ctrlMapX.GetLayers().Item("tmplayer");
	CMapXFeature ft,fr,fs;
	ft=lyr.AllFeatures().Item(1);
	fr=ft.Clone();
	fr.GetPoint().Set(y,x);
	fs=lyr.AddFeature(fr);
	fs.Update();
	*/
	CMapXFeature ft,st;
try
{
	if (ft.CreateDispatch(ft.GetClsid()))
	{
		ft.Attach(m_ctrlMapX.GetDispatch());
		ft = m_ctrlMapX.GetFeatureFactory().CreateSymbol();
		CMapXStyle s = ft.GetStyle();
		s.SetSymbolType(miSymbolTypeBitmap);
		s.SetSymbolBitmapSize(8);  //飞机大小
		s.SetSymbolBitmapTransparent(true);
		s.SetSymbolBitmapName("STOP1-32.bmp");
		ft.SetStyle(s);
		ft.GetPoint().Set(y,x);
		st=lyr.AddFeature(ft);
		st.Update();
	}
}
catch (COleDispatchException *e)
	{
	e->ReportError();
	e->Delete();
	}
catch (COleException *e)
	{
	e->ReportError();
	e->Delete();
	}	
}
void CRescuekj2View::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	if(listRect.GetCount()>0)//看链表是否为空
        {
            CRect rectDlgNow;
            GetWindowRect(&rectDlgNow);//得到当前对话框的坐标
            POSITION mp=listRect.GetHeadPosition();//取得存储在链表中的头元素，其实就是前边的对话框坐标
            CRect rectDlgSaved;
            rectDlgSaved=listRect.GetNext(mp);
            ScreenToClient(rectDlgNow);
            float fRateScaleX=(float)(rectDlgNow.right-rectDlgNow.left)/(rectDlgSaved.right-rectDlgSaved.left);//拖拉后的窗口大小与原来窗口大小的比例
            float fRateScaleY=(float)(rectDlgNow.bottom-rectDlgNow.top)/(rectDlgSaved.bottom-rectDlgSaved.top);
            ClientToScreen(rectDlgNow);
            CRect rectChildSaved;
            CWnd *pWndChild=GetWindow(GW_CHILD);

            int n=1;  //当前处理缩放控件的TAB值
            while (pWndChild)
            {
                rectChildSaved=listRect.GetNext(mp);
                rectChildSaved.left=rectDlgNow.left+(int)((rectChildSaved.left-rectDlgSaved.left)*fRateScaleX);
                rectChildSaved.top=rectDlgNow.top+(int)((rectChildSaved.top-rectDlgSaved.top)*fRateScaleY);
                rectChildSaved.right=rectDlgNow.right+(int)((rectChildSaved.right-rectDlgSaved.right)*fRateScaleX);
				rectChildSaved.bottom=rectDlgNow.bottom+(int)((rectChildSaved.bottom-rectDlgSaved.bottom)*fRateScaleY);
                ScreenToClient(rectChildSaved);
                pWndChild->MoveWindow(rectChildSaved);
                pWndChild = pWndChild->GetNextWindow();
            }
        }
        Invalidate(); //强制重绘窗口
	// TODO: Add your message handler code here
	//m_ControlPos.MoveControls();
}

BEGIN_EVENTSINK_MAP(CRescuekj2View, CFormView)
    //{{AFX_EVENTSINK_MAP(CRescuekj2View)
	ON_EVENT(CRescuekj2View, IDC_MSCOMM1, 1 /* OnComm */, OnCommMscomm1, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CRescuekj2View::OnCommMscomm1() 
{

	// TODO: Add your control notification handler code here
	VARIANT variant_inp;//定义一个VARIANT类对象
	COleSafeArray safearray_inp;//定义一个COleSafeArray对象
	LONG len,k;
	BYTE rxdata[2048];//设置BYTE数组 AN 8—intterthat is not signed.
	char buf[50];
	bool cha=false;
	CString strtemp;
	//显示时间
	time_t m_time;
	tm *t;
	int hour,min,sec;
	int year, mon, day;
	CString name,strst;//str_receive,
	time(&m_time); //获取当前时钟值
	t=localtime(&m_time);  //转换为 tm 结构类型
	hour=t->tm_hour;  //得到小时
	min=t->tm_min; //得到分钟
	sec=t->tm_sec; //得到秒
	year=t->tm_year+1900;  //得到年份
	mon =t->tm_mon+1; //得到月份
	day =t->tm_mday;  //得到日期

	//st.Format( "\n %4d 年%2d 月 %2d 日  %02d:%02d:%02d  " , year,mon,day,hour, min, sec);
	strst.Format( "%4d/%2d/%2d  %02d:%02d:%02d ",year,mon,day,hour, min, sec);
	CRescuekj2App *spp = (CRescuekj2App *)AfxGetApp();
	if (spp->m_ctrlComm.GetCommEvent()==2)//事件值为2表示接收缓冲区内有数据
	{
		////以下你可以根据自己的通信协议加入处理代码
		variant_inp=spp->m_ctrlComm.GetInput();//读缓冲区
		safearray_inp=variant_inp;//VARIANT型变量转换为ColeSafeArray型变量
		len=safearray_inp.GetOneDimSize();
		for(k=0;k<len;k++)
			{
				safearray_inp.GetElement(&k,rxdata+k);//转换为BYTE型数组
			}
		for(k=0;k<len;k++)//将数组转换为Cstring型变量
			{
				BYTE bt=*(char*)(rxdata+k);
			//if(m_ctrlHexSend.GetCheck())//如果是HEX显示则转为16进制
			//	strtemp.Format("%02x ",bt);	//将16进制数送入临时变量strtemp存放
			//	else
				buf[k]=bt;
			//	strtemp.Format("%c",bt);//将字符送入临时变量strtemp存放
			//	str_receive+=strtemp;//加入接收编辑框对应字符串
			}
	}
	//消息提示框$GLJC
	char *p=strstr(buf,"$GLZK");
	char *q=strstr(buf,"$TXXX");
	if(p!=NULL)
	{
		if(buf[7]==0x03&&buf[8]==0x00&&buf[9]==0x14)//判断用户机地址是否合法	
		{
			int value[10];
			for(int i=0;i<10;i++)
				value[i]=buf[10+i];
			m_page2.Show_gonglv(value);
		}
		else
			AfxMessageBox("非法用户!");
	}
	else 
		if(q!=NULL)
	{
		if(buf[7]==0x03&&buf[8]==0x00&&buf[9]==0x14)//判断用户机地址是否合法	
		{
			int id,bloodshou,bloodzh,beat,height;
			double lgtd,latd,temper;
			id=(unsigned char)buf[18]*256+(unsigned char)buf[19];
			cha=myAccess.Chaxun(id,book[id]);
		//	AfxMessageBox("测试!");
			if(cha==true)
			{
			lgtd=((unsigned char)buf[22]/60.0+buf[21])/60+buf[20];
			latd=((unsigned char)buf[26]/60.0+buf[25])/60+buf[24];
			
			if(book[id]==0)
			{
				biaodian(lgtd,latd);//在地图上标点
				book[id]=1;
			}
			else if(book[id]==1)
			{
				guiji_initial(id,pre[id].y,pre[id].x,lgtd,latd);
				book[id]=2;
			}
			else
				guiji(id,lgtd,latd,pre[id].y,pre[id].x);

			pre[id].y=lgtd;
			pre[id].x=latd;

			height=(unsigned char)buf[28]*256+(unsigned char)buf[29];
			bloodshou=(unsigned char)buf[30];
			bloodzh=(unsigned char)buf[31];
			temper=30+0.1*buf[32];
			beat=(unsigned char)buf[33];

			name=spp->strname;
			m_page3.Show_info(id,name,bloodshou,bloodzh,beat,temper);
			myAccess.Write(id,strst,lgtd,latd,height,bloodshou,bloodzh,beat,temper);
			}

			strtemp.Format("ID: %d, 姓名: %s, 位置: %d°%d'%d \"E,  %d°%d'%d \"N",id,name,(unsigned char)buf[20],buf[21],buf[22],(unsigned char)buf[24],buf[25],buf[26]);
			m_strRXData+=strst+strtemp+"\r\n";
			UpdateData(FALSE);//更新编辑框内容(主要是接收编辑框中的)

		}	
	}
	else
	AfxMessageBox("非法用户!");
}



void CRescuekj2View::OnAppExit() 
{
	// TODO: Add your command handler code here
		myAccess.ExitConnect();				//关闭数据库
		AfxGetMainWnd()->SendMessage(WM_CLOSE);
}

void CRescuekj2View::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
    switch (m_tab.GetCurSel())
    {
        case 0:
            m_page1.ShowWindow(SW_SHOW);
            m_page2.ShowWindow(SW_HIDE);
			m_page3.ShowWindow(SW_HIDE);
            break;
        case 1:
            m_page1.ShowWindow(SW_HIDE);
            m_page2.ShowWindow(SW_SHOW);
			m_page3.ShowWindow(SW_HIDE);

            break;
        case 2:
			m_page1.ShowWindow(SW_HIDE);
			m_page2.ShowWindow(SW_HIDE);
			m_page3.ShowWindow(SW_SHOW);
            break;
    }
}
