//
// MyToolWindow.h
//
// This file contains the implementation of a tool window that hosts a .NET user control
//

#pragma once

#include <initguid.h>
#include <AtlWin.h>
#include <VSLWindows.h>

#include "..\Test_VSPackage2UI\Resource.h"
#include "Library.h"
#include "git2.h"

// {624ed9c3-bdfd-41fa-96c3-7c824ea32e3d}
DEFINE_GUID(EnvironmentColorsCategory, 
0x624ed9c3, 0xbdfd, 0x41fa, 0x96, 0xc3, 0x7c, 0x82, 0x4e, 0xa3, 0x2e, 0x3d);

#define VS_RGBA_TO_COLORREF(rgba) (rgba & 0x00FFFFFF)

//extern ControllerFactory g_ControllerFactory;

class Test_VSPackage2WindowPane :
	public CComObjectRootEx<CComSingleThreadModel>,
	public VsWindowPaneFromResource<Test_VSPackage2WindowPane, IDD_Test_VSPackage2_DLG>,
	public VsWindowFrameEventSink<Test_VSPackage2WindowPane>,
   public IVsSolutionEvents,
	public VSL::ISupportErrorInfoImpl<
		InterfaceSupportsErrorInfoList<IVsWindowPane,
		InterfaceSupportsErrorInfoList<IVsWindowFrameNotify,
		InterfaceSupportsErrorInfoList<IVsWindowFrameNotify3,
      InterfaceSupportsErrorInfoList<IVsSolutionEvents> > > > >,
		public IVsBroadcastMessageEvents
{
	VSL_DECLARE_NOT_COPYABLE(Test_VSPackage2WindowPane)

protected:

	// Protected constructor called by CComObject<Test_VSPackage2WindowPane>::CreateInstance.
	Test_VSPackage2WindowPane() :
		VsWindowPaneFromResource(),
		m_hBackground(nullptr),
		m_BroadcastCookie(VSCOOKIE_NIL)
   {
      s_pThiz = this;
   }

	~Test_VSPackage2WindowPane() {}
	
public:

BEGIN_COM_MAP(Test_VSPackage2WindowPane)
	COM_INTERFACE_ENTRY(IVsWindowPane)
	COM_INTERFACE_ENTRY(IVsWindowFrameNotify)
	COM_INTERFACE_ENTRY(IVsWindowFrameNotify3)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
   COM_INTERFACE_ENTRY( IVsSolutionEvents )
	COM_INTERFACE_ENTRY(IVsBroadcastMessageEvents)
END_COM_MAP()

BEGIN_MSG_MAP(Test_VSPackage2WindowPane)
	COMMAND_HANDLER(IDC_CLICKME_BTN, BN_CLICKED, OnButtonClick)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
END_MSG_MAP()

	// Function called by VsWindowPaneFromResource at the end of SetSite; at this point the
	// window pane is constructed and sited and can be used, so this is where we can initialize
	// the event sink by siting it.
	void PostSited(IVsPackageEnums::SetSiteResult /*result*/)
	{
		VsWindowFrameEventSink<Test_VSPackage2WindowPane>::SetSite(GetVsSiteCache());
		CComPtr<IVsShell> spShell = GetVsSiteCache().GetCachedService<IVsShell, SID_SVsShell>();
		spShell->AdviseBroadcastMessages(this, &m_BroadcastCookie);

		InitVSColors();

      CComPtr<IVsSolution> spIVsSolution;
      GetVsSiteCache().QueryService( IID_IVsSolution, &spIVsSolution );
      HRESULT hr = spIVsSolution->AdviseSolutionEvents( ( IVsSolutionEvents* )this, &m_SolutionCookie );

      m_Timer = ::SetTimer( m_hWnd, 1234, 30'000, OnTimer );
	}

	// Function called by VsWindowPaneFromResource at the end of ClosePane.
	// Perform necessary cleanup.
	void PostClosed()
	{
		if (nullptr != m_hBackground)
		{
			::DeleteBrush(m_hBackground);
			m_hBackground = nullptr;
		}

		CComPtr<IVsShell> spShell = GetVsSiteCache().GetCachedService<IVsShell, SID_SVsShell>();
		if (nullptr != spShell && VSCOOKIE_NIL != m_BroadcastCookie)
		{
			spShell->UnadviseBroadcastMessages(m_BroadcastCookie);
			m_BroadcastCookie = VSCOOKIE_NIL;
		}

      ::KillTimer( m_hWnd, m_Timer );
	}

	// Callback function called by ToolWindowBase when the size of the window changes.
	void OnFrameSize(int x, int y, int w, int h)
	{
		// Center button.
		CWindow button(this->GetDlgItem(IDC_CLICKME_BTN));
		RECT buttonRectangle;
		button.GetWindowRect(&buttonRectangle);

		OffsetRect(&buttonRectangle, -buttonRectangle.left, -buttonRectangle.top);

		int iLeft = (w - buttonRectangle.right) / 2; 
		if (iLeft <= 0)
		{
			iLeft = 5;
		}

		int iTop = (h - buttonRectangle.bottom) / 2; 
		if (iTop <= 0)
		{
			iTop = 5;
		}

		button.SetWindowPos(NULL, iLeft, iTop, 0, 0, SWP_NOSIZE);
	}

	LRESULT OnButtonClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
	{
      QueueContinous();

		bHandled = TRUE;
		return 0;
	}

	// Handled to set the color that should be used to draw the background of the Window Pane.
	LRESULT OnCtlColorDlg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (nullptr != m_hBackground)
		{
			bHandled = TRUE;
		}
		else
		{
			bHandled = FALSE;
		}

		return (LRESULT)m_hBackground;
	}

   static void __stdcall OnTimer( HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime )
   {
      s_pThiz->TimerHit( hWnd, nMsg, nIDEvent, dwTime );
   }

   void TimerHit( HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime )
   {
      if ( nIDEvent == m_Timer )
      {
         SetDlgItemText( IDC_STATIC_BRANCH, GetCurrentBranch() );
      }
   }

	STDMETHOD(OnBroadcastMessage)(UINT uMsg, WPARAM /*wParam*/, LPARAM /*lParam*/)
	{
		switch (uMsg)
		{
		case WM_SYSCOLORCHANGE:
		case WM_PALETTECHANGED:
			// Re-initialize VS colors when the theme changes.
			InitVSColors();
			break;
		}

		return S_OK;
	}

   virtual HRESULT STDMETHODCALLTYPE OnAfterOpenProject(
      /* [in] */ __RPC__in_opt IVsHierarchy *pHierarchy,
      /* [in] */ BOOL fAdded )
   {
      return S_OK;
   }

   virtual HRESULT STDMETHODCALLTYPE OnQueryCloseProject(
      /* [in] */ __RPC__in_opt IVsHierarchy *pHierarchy,
      /* [in] */ BOOL fRemoving,
      /* [out][in] */ __RPC__inout BOOL *pfCancel )
   {
      return S_OK;
   }

   virtual HRESULT STDMETHODCALLTYPE OnBeforeCloseProject(
      /* [in] */ __RPC__in_opt IVsHierarchy *pHierarchy,
      /* [in] */ BOOL fRemoved )
   {
      return S_OK;
   }

   virtual HRESULT STDMETHODCALLTYPE OnAfterLoadProject(
      /* [in] */ __RPC__in_opt IVsHierarchy *pStubHierarchy,
      /* [in] */ __RPC__in_opt IVsHierarchy *pRealHierarchy )
   {
      return S_OK;
   }

   virtual HRESULT STDMETHODCALLTYPE OnQueryUnloadProject(
      /* [in] */ __RPC__in_opt IVsHierarchy *pRealHierarchy,
      /* [out][in] */ __RPC__inout BOOL *pfCancel )
   {
      return S_OK;
   }

   virtual HRESULT STDMETHODCALLTYPE OnBeforeUnloadProject(
      /* [in] */ __RPC__in_opt IVsHierarchy *pRealHierarchy,
      /* [in] */ __RPC__in_opt IVsHierarchy *pStubHierarchy )
   {
      return S_OK;
   }

   virtual HRESULT STDMETHODCALLTYPE OnAfterOpenSolution(
      /* [in] */ __RPC__in_opt IUnknown *pUnkReserved,
      /* [in] */ BOOL fNewSolution )
   {
      CComPtr<IVsSolution2> spIVsSolution;
      m_VsSiteCache.QueryService( SID_SVsSolution, &spIVsSolution );
      BSTR bstrSolutionDirectory;
      BSTR bstrSolutionFile;
      BSTR bstrUserOptsFile;
      spIVsSolution->GetSolutionInfo( &bstrSolutionDirectory, &bstrSolutionFile, &bstrUserOptsFile );

      m_strSolutionDirectory = bstrSolutionDirectory;
      m_strSolutionName = bstrSolutionFile;
      m_strSolutionName = m_strSolutionName.Mid( m_strSolutionName.ReverseFind( '\\' ) + 1 );

      SetDlgItemText( IDC_STATIC_SOLUTION, m_strSolutionName );
      SetDlgItemText( IDC_STATIC_SOLUTIONDIR, m_strSolutionDirectory );
      SetDlgItemText( IDC_STATIC_BRANCH, GetCurrentBranch() );

      return S_OK;
   }

   virtual HRESULT STDMETHODCALLTYPE OnQueryCloseSolution(
      /* [in] */ __RPC__in_opt IUnknown *pUnkReserved,
      /* [out][in] */ __RPC__inout BOOL *pfCancel )
   {
      return S_OK;
   }

   virtual HRESULT STDMETHODCALLTYPE OnBeforeCloseSolution(
      /* [in] */ __RPC__in_opt IUnknown *pUnkReserved )
   {
      return S_OK;
   }

   virtual HRESULT STDMETHODCALLTYPE OnAfterCloseSolution(
      /* [in] */ __RPC__in_opt IUnknown *pUnkReserved )
   {
      m_strSolutionDirectory.Empty();
      m_strSolutionName.Empty();

      SetDlgItemText( IDC_STATIC_SOLUTION, CString() );
      SetDlgItemText( IDC_STATIC_SOLUTIONDIR, CString() );
      SetDlgItemText( IDC_STATIC_BRANCH, CString() );

      return S_OK;
   }

   bool QueueContinous()
   {
      typedef void*	TeamCityAPI;

      typedef int( *TeamCityCreateFunc )( TeamCityAPI* api, int nVerbose );
      typedef int( *TeamCityFreeFunc )( TeamCityAPI* api );
      typedef int( *TeamCityQueueBuildFunc )( TeamCityAPI api, const char* pstrBranchName, const char* pstrUsername, const char* pstrPassword );

      wchar_t filename[MAX_PATH];
      DWORD length = GetModuleFileName( reinterpret_cast<HMODULE>( &__ImageBase ),
                                        filename,
                                        _countof( filename ) );
      CString strPath( filename );
      strPath = strPath.Left( strPath.ReverseFind( '\\' ) + 1 );
      strPath += "TeamCityAPI.dll";

      RLibrary library( strPath );
      if ( !library.Load() )
      {
         DWORD dwLastError = GetLastError();
         return false;
      }

      TeamCityCreateFunc CreateAPI = (TeamCityCreateFunc)library.Resolve( "TeamCityCreate" );
      if ( !CreateAPI )
         return false;

      TeamCityAPI pTeamCity = NULL;
      CreateAPI( &pTeamCity, 1 );

      TeamCityQueueBuildFunc QueueBuild = (TeamCityQueueBuildFunc)library.Resolve( "TeamCityQueueBuild" );
      if ( !QueueBuild )
         return false;

      CString strCurrentBranch = GetCurrentBranch();
      CT2CA pszConvertedAnsiString( strCurrentBranch );
      std::string strStd( pszConvertedAnsiString );
      QueueBuild( pTeamCity, strStd.c_str(), "a.orians", "1!Smajjmd" );

      TeamCityFreeFunc FreeAPI = (TeamCityFreeFunc)library.Resolve( "TeamCityFree" );
      if ( !FreeAPI )
         return false;

      FreeAPI( &pTeamCity );
      return true;
   }

   CString GetCurrentBranch()
   {
      CString strCurrentBranch;
      wchar_t filename[MAX_PATH];
      DWORD length = GetModuleFileName( reinterpret_cast<HMODULE>( &__ImageBase ),
                                        filename,
                                        _countof( filename ) );
      CString strPath( filename );
      strPath = strPath.Left( strPath.ReverseFind( '\\' ) + 1 );
      strPath += "git2.dll";

      RLibrary library( strPath );
      if ( !library.Load() )
      {
         DWORD dwLastError = GetLastError();
         return strCurrentBranch;
      }

      typedef int( *GitInitFunc )( void );

      GitInitFunc init = (GitInitFunc)library.Resolve( "git_libgit2_init" );
      if ( init )
      {
         init();
      }

      git_repository *repo = NULL;

      typedef int( *GitOpenRepoFunc )( git_repository **out,
                                       const char *path,
                                       unsigned int flags,
                                       const char *ceiling_dirs );

      GitOpenRepoFunc openRepo = (GitOpenRepoFunc)library.Resolve( "git_repository_open_ext" );
      if ( !openRepo )
      {
         return strCurrentBranch;
      }

      CString strCurrentDirectory = GetCurrentSolutionDirectory();
      if ( strCurrentDirectory.IsEmpty() )
         return strCurrentBranch;
      CT2CA pszConvertedAnsiString( strCurrentDirectory );
      std::string strStd( pszConvertedAnsiString );
      openRepo( &repo, strStd.c_str(), 0, NULL );
      if ( repo == NULL )
      {
         typedef int( *GitShutdownFunc )( void );
         GitShutdownFunc Shutdown = (GitShutdownFunc)library.Resolve( "git_libgit2_shutdown" );
         if ( !Shutdown )
         {
            return strCurrentBranch;
         }
         Shutdown();
         return strCurrentBranch;
      }

      int error = 0;
      const char *branch = NULL;
      git_reference *head = NULL;

      typedef int( *GitRepoHeadFunc )( git_reference **out, git_repository *repo );
      GitRepoHeadFunc headRepo = (GitRepoHeadFunc)library.Resolve( "git_repository_head" );
      if ( !headRepo )
      {
         return strCurrentBranch;
      }

      error = headRepo( &head, repo );

      if ( error == GIT_EUNBORNBRANCH || error == GIT_ENOTFOUND )
         branch = NULL;
      else if ( !error )
      {
         typedef const char*( *GitRepoShortHandFunc )( const git_reference *ref );
         GitRepoShortHandFunc shortHandRepo = (GitRepoShortHandFunc)library.Resolve( "git_reference_shorthand" );
         if ( !shortHandRepo )
         {
            return strCurrentBranch;
         }
         branch = shortHandRepo( head );
      }
      else
      {
         //check_lg2( error, "failed to get current branch", NULL );
      }

      printf( "## %s\n", branch ? branch : "HEAD (no branch)" );
      strCurrentBranch = CString( branch );

      typedef void( *GitReferenceFreeFunc )( git_reference *ref );
      GitReferenceFreeFunc FreeReference = (GitReferenceFreeFunc)library.Resolve( "git_reference_free" );
      if ( !FreeReference )
      {
         return strCurrentBranch;
      }

      FreeReference( head );

      typedef void( *GitRepoFreeFunc )( git_repository *repo );
      GitRepoFreeFunc FreeRepo = (GitRepoFreeFunc)library.Resolve( "git_repository_free" );
      if ( !FreeRepo )
      {
         return strCurrentBranch;
      }
      FreeRepo( repo );

      typedef int( *GitShutdownFunc )( void );
      GitShutdownFunc Shutdown = (GitShutdownFunc)library.Resolve( "git_libgit2_shutdown" );
      if ( !Shutdown )
      {
         return strCurrentBranch;
      }
      Shutdown();

      return strCurrentBranch;
   }

   CString GetCurrentSolutionDirectory()
   {
      return m_strSolutionDirectory;
   }

private:
	// Initialize colors that are used to render the Window Pane.
	void InitVSColors()
	{
		// Obtain IVsUIShell5 from IVsUIShell
		CComQIPtr<IVsUIShell5> spIVsUIShell5(GetVsSiteCache().GetCachedService<IVsUIShell, SID_SVsUIShell>());
		VS_RGBA vsColor;

		if (nullptr != m_hBackground)
		{
			::DeleteBrush(m_hBackground);
			m_hBackground = nullptr;
		}

		if (nullptr != spIVsUIShell5 && SUCCEEDED(spIVsUIShell5->GetThemedColor(EnvironmentColorsCategory, L"Window", TCT_Background, &vsColor)))
		{
			COLORREF crBackground = VS_RGBA_TO_COLORREF(vsColor);
			m_hBackground = ::CreateSolidBrush(crBackground);
		}
		
		if (::IsWindow(this->m_hWnd))
		{
			::InvalidateRect(this->m_hWnd, nullptr /* lpRect */ , TRUE /* bErase */);
		}

      
	}

	HBRUSH m_hBackground;
	VSCOOKIE m_BroadcastCookie;
   VSCOOKIE m_SolutionCookie;

   UINT_PTR m_Timer;
   static Test_VSPackage2WindowPane* s_pThiz;

   CString m_strSolutionName;
   CString m_strSolutionDirectory;
};

Test_VSPackage2WindowPane* Test_VSPackage2WindowPane::s_pThiz = nullptr;


class Test_VSPackage2ToolWindow :
	public VSL::ToolWindowBase<Test_VSPackage2ToolWindow>
{
public:
	// Constructor of the tool window object.
	// The goal of this constructor is to initialize the base class with the site cache
	// of the owner package.
	Test_VSPackage2ToolWindow(const PackageVsSiteCache& rPackageVsSiteCache):
		ToolWindowBase(rPackageVsSiteCache)
	{
	}

	// Caption of the tool window.
	const wchar_t* const GetCaption() const
	{
		static CStringW strCaption;
		// Avoid to load the string from the resources more that once.
		if (0 == strCaption.GetLength())
		{
			VSL_CHECKBOOL_GLE(
				strCaption.LoadStringW(_AtlBaseModule.GetResourceInstance(), IDS_WINDOW_TITLE));
		}
		return strCaption;
	}

	// Creation flags for this tool window.
	VSCREATETOOLWIN GetCreationFlags() const
	{
		return CTW_fInitNew|CTW_fForceCreate;
	}

	// Return the GUID of the persistence slot for this tool window.
	const GUID& GetToolWindowGuid() const
	{
		return CLSID_guidPersistanceSlot;
	}

	IUnknown* GetViewObject()
	{
		// Should only be called once per-instance
		VSL_CHECKBOOLEAN_EX(m_spView == NULL, E_UNEXPECTED, IDS_E_GETVIEWOBJECT_CALLED_AGAIN);

		// Create the object that implements the window pane for this tool window.
		CComObject<Test_VSPackage2WindowPane>* pViewObject;
		VSL_CHECKHRESULT(CComObject<Test_VSPackage2WindowPane>::CreateInstance(&pViewObject));

		// Get the pointer to IUnknown for the window pane.
		HRESULT hr = pViewObject->QueryInterface(IID_IUnknown, (void**)&m_spView);
		if (FAILED(hr))
		{
			// If QueryInterface failed, then there is something wrong with the object.
			// Delete it and throw an exception for the error.
			delete pViewObject;
			VSL_CHECKHRESULT(hr);
		}

		return m_spView;
	}

	// This method is called by the base class after the tool window is created.
	// We use it to set the icon for this window.
	void PostCreate()
	{
		CComVariant srpvt;
		srpvt.vt = VT_I4;
		srpvt.intVal = IDB_IMAGES;
		// We don't want to make the window creation fail only becuase we can not set
		// the icon, so we will not throw if SetProperty fails.
		if (SUCCEEDED(GetIVsWindowFrame()->SetProperty(VSFPROPID_BitmapResource, srpvt)))
		{
			srpvt.intVal = 1;
			GetIVsWindowFrame()->SetProperty(VSFPROPID_BitmapIndex, srpvt);
		}
	}

private:
	CComPtr<IUnknown> m_spView;
};
