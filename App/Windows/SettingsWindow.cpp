#include "SettingsWindow.h"
#include "ProgressWindow.h"
#include "../App.h"

#include <wx/statline.h>
#include <thread>

#include <Tera/FPackage.h>

const wxString RootDir = wxS("S1Game");
const wxString UpdatesUrl = wxS("https://github.com/VenoMKO/RealEditor/releases");

enum ControlElementId {
	Path = wxID_HIGHEST + 1,
	Browse,
	DirCache,
	Mappers,
	Warnings,
	Register,
	Unregister,
	Update
};

bool IsValidDir(const wxString& path)
{
	return path.EndsWith(wxFILE_SEP_PATH + RootDir) || path.EndsWith(wxFILE_SEP_PATH + RootDir + wxFILE_SEP_PATH);
}

SettingsWindow::SettingsWindow(const FAppConfig& currentConfig, FAppConfig& output, bool allowRebuild, const wxPoint& pos)
	: wxDialog(nullptr, wxID_ANY, wxS("Settings"), pos, wxSize(668, 375), wxCAPTION | wxCLOSE_BOX | wxSYSTEM_MENU | wxTAB_TRAVERSAL)
	, CurrentConfig(currentConfig)
	, NewConfig(output)
	, AllowRebuild(allowRebuild)
{
	NewConfig = currentConfig;
	SetIcon(wxICON(#114));
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText5;
	m_staticText5 = new wxStaticText(this, wxID_ANY, wxT("S1Game"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText5->Wrap(-1);
	m_staticText5->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer1->Add(m_staticText5, 0, wxALL, 5);

	wxPanel* m_panel2;
	m_panel2 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText6;
	m_staticText6 = new wxStaticText(m_panel2, wxID_ANY, wxT("Select the S1Game folder of your Tera client. This path will be used to find and load packages and resources. The path may look like this: \"D:\\Games\\Gameforge\\Tera\\Client\\S1Game\\\"."), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText6->Wrap(600);
	bSizer4->Add(m_staticText6, 0, wxALL, 5);

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText7;
	m_staticText7 = new wxStaticText(m_panel2, wxID_ANY, wxT("Path:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText7->Wrap(-1);
	bSizer5->Add(m_staticText7, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	PathField = new wxTextCtrl(m_panel2, ControlElementId::Path, CurrentConfig.RootDir.WString(), wxDefaultPosition, wxSize(500, -1), wxTE_PROCESS_ENTER);
	bSizer5->Add(PathField, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	BrowseButton = new wxButton(m_panel2, ControlElementId::Browse, wxT("Browse..."), wxDefaultPosition, wxDefaultSize, 0);
	bSizer5->Add(BrowseButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);


	bSizer4->Add(bSizer5, 1, wxEXPAND, 5);


	m_panel2->SetSizer(bSizer4);
	m_panel2->Layout();
	bSizer4->Fit(m_panel2);
	bSizer1->Add(m_panel2, 0, wxALL | wxEXPAND, 5);

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer(wxHORIZONTAL);

	wxPanel* m_panel7;
	m_panel7 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText10;
	m_staticText10 = new wxStaticText(m_panel7, wxID_ANY, wxT("Utils"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText10->Wrap(-1);
	m_staticText10->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer12->Add(m_staticText10, 0, wxALL, 5);

	wxPanel* m_panel8;
	m_panel8 = new wxPanel(m_panel7, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer(wxVERTICAL);

	RebuildCacheButton = new wxButton(m_panel8, ControlElementId::DirCache, wxT("Rebuild Cache"), wxDefaultPosition, wxDefaultSize, 0);
	RebuildCacheButton->SetToolTip(wxT("Update a cached list of packages in your S1Game folder. Usefull when ") + wxTheApp->GetAppDisplayName() + wxT(" can't find an object."));
	bSizer13->Add(RebuildCacheButton, 0, wxALL | wxEXPAND, 5);

	UpdateMappingButton = new wxButton(m_panel8, ControlElementId::Mappers, wxT("Update Mapping"), wxDefaultPosition, wxDefaultSize, 0);
	UpdateMappingButton->SetToolTip(wxT("Decrypt and update mappers. May be usefull after Tera update."));
	bSizer13->Add(UpdateMappingButton, 0, wxALL | wxEXPAND, 5);

	ResetWarningsButton = new wxButton(m_panel8, ControlElementId::Warnings, wxT("Reset Warnings"), wxDefaultPosition, wxDefaultSize, 0);
	ResetWarningsButton->SetToolTip(wxT("Reset all hidden UI warnings."));
	bSizer13->Add(ResetWarningsButton, 0, wxALL | wxEXPAND, 5);


	m_panel8->SetSizer(bSizer13);
	m_panel8->Layout();
	bSizer13->Fit(m_panel8);
	bSizer12->Add(m_panel8, 0, wxEXPAND | wxALL, 5);


	m_panel7->SetSizer(bSizer12);
	m_panel7->Layout();
	bSizer12->Fit(m_panel7);
	bSizer10->Add(m_panel7, 1, wxALL, 5);

	wxPanel* m_panel6;
	m_panel6 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer(wxVERTICAL);

	wxStaticText* m_staticText8;
	m_staticText8 = new wxStaticText(m_panel6, wxID_ANY, wxT("Associate filetypes"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText8->Wrap(-1);
	m_staticText8->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString));

	bSizer11->Add(m_staticText8, 0, wxALL, 5);

	wxPanel* m_panel3;
	m_panel3 = new wxPanel(m_panel6, wxID_ANY, wxDefaultPosition, wxSize(-1, -1), wxBORDER_THEME | wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer(wxVERTICAL);

	wxPanel* m_panel5;
	m_panel5 = new wxPanel(m_panel3, wxID_ANY, wxDefaultPosition, wxSize(-1, -1), wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* m_staticText9;
	m_staticText9 = new wxStaticText(m_panel5, wxID_ANY, wxT("This allows you to open *.gpk, *.gmp, *.upk and *.u packages by double clicking in the Explorer window. Press Associate to enable the feature or Dissociate to disable."), wxDefaultPosition, wxSize(-1, -1), 0);
	m_staticText9->Wrap(300);
	bSizer7->Add(m_staticText9, 0, wxALL, 5);

	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer(wxVERTICAL);

	WasRegistered = ((App*)wxTheApp)->CheckMimeTypes();

	RegisterButton = new wxButton(m_panel5, ControlElementId::Register, wxT("Associate"), wxDefaultPosition, wxDefaultSize, 0);
	RegisterButton->SetToolTip(wxT("Allow to open packages in the Explorer by double clicking them."));
	RegisterButton->Enable(!WasRegistered);
	bSizer9->Add(RegisterButton, 0, wxALL, 5);

	UnregisterButton = new wxButton(m_panel5, ControlElementId::Unregister, wxT("Dissociate"), wxDefaultPosition, wxDefaultSize, 0);
	UnregisterButton->SetToolTip(wxT("Don't open packages in the Explorer by double clicking them."));
	UnregisterButton->Enable(WasRegistered);
	bSizer9->Add(UnregisterButton, 0, wxALL, 5);


	bSizer7->Add(bSizer9, 1, wxEXPAND, 5);


	m_panel5->SetSizer(bSizer7);
	m_panel5->Layout();
	bSizer7->Fit(m_panel5);
	bSizer6->Add(m_panel5, 0, wxALL, 5);


	m_panel3->SetSizer(bSizer6);
	m_panel3->Layout();
	bSizer6->Fit(m_panel3);
	bSizer11->Add(m_panel3, 0, wxALL, 5);


	m_panel6->SetSizer(bSizer11);
	m_panel6->Layout();
	bSizer11->Fit(m_panel6);
	bSizer10->Add(m_panel6, 0, wxALL, 5);


	bSizer1->Add(bSizer10, 1, wxEXPAND, 5);

	wxStaticLine* m_staticline1;
	m_staticline1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	bSizer1->Add(m_staticline1, 0, wxEXPAND | wxALL, 0);

	wxPanel* m_panel9;
	m_panel9 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer(wxHORIZONTAL);

	UpdatesButton = new wxButton(m_panel9, ControlElementId::Update, wxT("Updates"), wxDefaultPosition, wxDefaultSize, 0);
	UpdatesButton->SetHelpText(wxT("Open GitHub release page to check if a new version is available"));
	bSizer14->Add(UpdatesButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	VersionLabel = new wxStaticText(m_panel9, wxID_ANY, GetAppVersion(), wxDefaultPosition, wxDefaultSize, 0);
	VersionLabel->Wrap(-1);
	bSizer14->Add(VersionLabel, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	wxPanel* m_panel10;
	m_panel10 = new wxPanel(m_panel9, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	bSizer14->Add(m_panel10, 1, wxEXPAND | wxALL, 5);

	CancelButton = new wxButton(m_panel9, wxID_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer14->Add(CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

	ApplyButton = new wxButton(m_panel9, wxID_OK, wxT("Apply"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer14->Add(ApplyButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);


	m_panel9->SetSizer(bSizer14);
	m_panel9->Layout();
	bSizer14->Fit(m_panel9);
	bSizer1->Add(m_panel9, 1, wxALL | wxEXPAND, 5);


	this->SetSizer(bSizer1);
	this->Layout();

	this->Centre(wxBOTH);

	if (!IsValidDir(PathField->GetValue()))
	{
		ApplyButton->Enable(false);
		ApplyButton->SetToolTip(wxT("You must specify the \"" + RootDir + "\". Press \"Browse\" and select the folder."));
	}
	if (!AllowRebuild || !IsValidDir(CurrentConfig.RootDir.WString()))
	{
		RebuildCacheButton->Enable(false);
		UpdateMappingButton->Enable(false);
	}
}

void SettingsWindow::OnBrowseClicked(wxCommandEvent&)
{
	wxDirDialog* openPanel = new wxDirDialog(this, wxS("Select \"") + RootDir + wxS("\" folder"), PathField->GetLabelText());
	openPanel->Center();
	if (openPanel->ShowModal() == wxID_OK)
	{
		const wxString path = openPanel->GetPath();
		if (IsValidDir(path))
		{
			PathField->SetLabelText(path);
		}
		else
		{
			wxMessageBox(wxS("Folder must be called \"") + RootDir + wxS("\""), wxS("Error: Invalid path!"), wxICON_ERROR);
		}
	}
	openPanel->Destroy();
}

void SettingsWindow::OnPathChanged(wxCommandEvent&)
{
	if (!IsValidDir(PathField->GetValue()))
	{
		ApplyButton->Enable(false);
		ApplyButton->SetToolTip(wxT("You must specify the \"" + RootDir + "\". Press \"Browse\" and select the folder."));
	}
	else
	{
		ApplyButton->Enable(true);
		NewConfig.RootDir = PathField->GetValue().ToStdWstring();
	}
}

void SettingsWindow::OnUpdateDirCacheClicked(wxCommandEvent&)
{
	ProgressWindow progress(this);
	progress.SetCurrentProgress(-1);
	progress.SetCanCancel(false);
	bool ok = true;
	std::thread([&progress, &ok] {
		SendEvent(&progress, UPDATE_PROGRESS_DESC, wxT("Updating folder cache..."));
		Sleep(200);
		try
		{
			FPackage::UpdateDirCache();
		}
		catch (const std::exception& e)
		{
			wxMessageBox(e.what(), wxS("Error!"), wxICON_ERROR);
			ok = false;
		}
		SendEvent(&progress, UPDATE_PROGRESS_FINISH);
	}).detach();
	progress.ShowModal();
	if (ok)
	{
		wxMessageBox(wxS("Folder cache has been updated!"), wxS("Done"), wxICON_INFORMATION);
	}
}

void SettingsWindow::OnUpdateMappersClicked(wxCommandEvent&)
{
	ProgressWindow progress(this);
	progress.SetCurrentProgress(-1);
	bool ok = true;
	std::thread([&progress, &ok] {
		SendEvent(&progress, UPDATE_PROGRESS_DESC, wxT("Updating package mapper..."));
		Sleep(200);
		try
		{
			FPackage::LoadPkgMapper(true);
		}
		catch (const std::exception& e)
		{
			wxMessageBox(e.what(), wxS("Error!"), wxICON_ERROR);
			ok = false;
			SendEvent(&progress, UPDATE_PROGRESS_FINISH);
			return;
		}
		if (progress.IsCanceled())
		{
			SendEvent(&progress, UPDATE_PROGRESS_FINISH);
			return;
		}
		SendEvent(&progress, UPDATE_PROGRESS_DESC, wxT("Updating composite mapper..."));

		try
		{
			FPackage::LoadCompositePackageMapper(true);
		}
		catch (const std::exception& e)
		{
			wxMessageBox(e.what(), wxS("Error!"), wxICON_ERROR);
			ok = false;
			SendEvent(&progress, UPDATE_PROGRESS_FINISH);
			return;
		}
		if (progress.IsCanceled())
		{
			SendEvent(&progress, UPDATE_PROGRESS_FINISH);
			return;
		}
		SendEvent(&progress, UPDATE_PROGRESS_DESC, wxT("Updating object redirector mapper..."));
		try
		{
			FPackage::LoadObjectRedirectorMapper(true);
		}
		catch (const std::exception& e)
		{
			wxMessageBox(e.what(), wxS("Error!"), wxICON_ERROR);
			ok = false;
			SendEvent(&progress, UPDATE_PROGRESS_FINISH);
			return;
		}
		SendEvent(&progress, UPDATE_PROGRESS_FINISH);
	}).detach();
	progress.ShowModal();
	if (ok)
	{
		wxMessageBox(wxS("Mappers have been updated!"), wxS("Done"), wxICON_INFORMATION);
	}
}

void SettingsWindow::OnResetWarningClicked(wxCommandEvent&)
{
	// TODO: reset hidden message boxes here
	NewConfig.CompositeDumpPath = FString();
	wxMessageBox(wxS("UI Warnings were cleared!\nPress Apply to save changes."), wxS("Done"), wxICON_INFORMATION);
}

void SettingsWindow::OnRegisterClicked(wxCommandEvent&)
{
	App* app = (App*)wxTheApp;
	wxCommandEvent tmp;
	app->OnRegisterMime(tmp);
	bool registered = app->CheckMimeTypes();
	RegisterButton->Enable(!registered);
	UnregisterButton->Enable(registered);
	if (registered)
	{
		wxMessageBox(wxS("The filetype has been associated successfully!"), wxS("Done"), wxICON_INFORMATION);
	}
	else
	{
		wxMessageBox(wxS("Failed to associate!"), wxS("Error!"), wxICON_INFORMATION);
	}
}

void SettingsWindow::OnUnregisterClicked(wxCommandEvent&)
{
	App* app = (App*)wxTheApp;
	wxCommandEvent tmp;
	app->OnUnregisterMime(tmp);
	bool registered = app->CheckMimeTypes();
	RegisterButton->Enable(!registered);
	UnregisterButton->Enable(registered);
	if (!registered)
	{
		wxMessageBox(wxS("The filetype has been dissociated successfully!"), wxS("Done"), wxICON_INFORMATION);
	}
	else
	{
		wxMessageBox(wxS("Failed to dissociate!"), wxS("Error!"), wxICON_INFORMATION);
	}
}

void SettingsWindow::OnUpdateClicked(wxCommandEvent&)
{
	wxLaunchDefaultBrowser(UpdatesUrl);
}

void SettingsWindow::OnCancelClicked(wxCommandEvent&)
{
	EndModal(wxID_CANCEL);
}

void SettingsWindow::OnOkClicked(wxCommandEvent&)
{
	if (IsValidDir(PathField->GetValue()))
	{
		if (!WasRegistered && RegisterButton->IsEnabled())
		{
			wxMessageDialog dialog(nullptr, wxT("You haven't associated *.gpk filetype with the ") + wxTheApp->GetAppDisplayName() + wxT(". This allows you to open packages without starting the application. Would you like to associate now?"), wxT("Associate *.gpk files?"), wxYES_NO | wxICON_INFORMATION);
			if (dialog.ShowModal() == wxID_YES)
			{
				wxCommandEvent tmp;
				OnRegisterClicked(tmp);
			}
		}
		EndModal(wxID_OK);
	}
}

wxBEGIN_EVENT_TABLE(SettingsWindow, wxDialog)
EVT_BUTTON(ControlElementId::Browse, SettingsWindow::OnBrowseClicked)
EVT_BUTTON(wxID_OK, SettingsWindow::OnOkClicked)
EVT_BUTTON(wxID_CANCEL, SettingsWindow::OnCancelClicked)
EVT_BUTTON(ControlElementId::Register, SettingsWindow::OnRegisterClicked)
EVT_BUTTON(ControlElementId::Unregister, SettingsWindow::OnUnregisterClicked)
EVT_BUTTON(ControlElementId::Update, SettingsWindow::OnUpdateClicked)
EVT_BUTTON(ControlElementId::DirCache, SettingsWindow::OnUpdateDirCacheClicked)
EVT_BUTTON(ControlElementId::Mappers, SettingsWindow::OnUpdateMappersClicked)
EVT_BUTTON(ControlElementId::Warnings, SettingsWindow::OnResetWarningClicked)
EVT_TEXT(ControlElementId::Path, SettingsWindow::OnPathChanged)
EVT_TEXT_ENTER(ControlElementId::Path, SettingsWindow::OnOkClicked)
wxEND_EVENT_TABLE()