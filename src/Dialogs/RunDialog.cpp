
#include "Main.h"
#include "WxStuff.h"
#include "RunDialog.h"
#include "Icons.h"
#include "SFileDialog.h"
#include "Executables.h"
#include "ResourceArchiveChooser.h"
#include "Archive.h"
#include "ArchiveManager.h"
#include <wx/gbsizer.h>
#include <wx/statline.h>
#include <wx/filename.h>


CVAR(String, run_last_exe, "", CVAR_SAVE)
CVAR(Int, run_last_config, 0, CVAR_SAVE)
CVAR(String, run_last_extra, "", CVAR_SAVE)


class RunConfigDialog : public wxDialog
{
private:
	wxTextCtrl*	text_name;
	wxTextCtrl*	text_params;
	
public:
	RunConfigDialog(wxWindow* parent, string title, string name, string params) : wxDialog(parent, -1, title)
	{
		// Setup sizer
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		SetSizer(sizer);

		wxGridBagSizer* gb_sizer = new wxGridBagSizer(8, 4);
		sizer->Add(gb_sizer, 1, wxEXPAND|wxALL, 10);

		// Config name
		gb_sizer->Add(new wxStaticText(this, -1, "Config Name:"), wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
		text_name = new wxTextCtrl(this, -1, name);
		gb_sizer->Add(text_name, wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND);

		// Config params
		gb_sizer->Add(new wxStaticText(this, -1, "Parameters:"), wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
		text_params = new wxTextCtrl(this, -1, params);
		gb_sizer->Add(text_params, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);

		wxStaticText* label_help = new wxStaticText(this, -1, "");
		gb_sizer->Add(label_help, wxGBPosition(2, 0), wxGBSpan(1, 2), wxEXPAND);

		gb_sizer->Add(CreateStdDialogButtonSizer(wxOK|wxCANCEL), wxGBPosition(3, 0), wxGBSpan(1, 2), wxALIGN_RIGHT);
		gb_sizer->AddGrowableCol(1);
		gb_sizer->AddGrowableRow(2);

		label_help->SetLabel("%i - Base resource archive\n%r - Resource archive(s)\n%a - Current archive\n%mn - Map name\n%mw - Map number (eg. E1M1 = 1 1, MAP02 = 02)");
		label_help->Wrap(300);
	}
	~RunConfigDialog() {}

	string getName()
	{
		return text_name->GetValue();
	}

	string getParams()
	{
		return text_params->GetValue();
	}
};


RunDialog::RunDialog(wxWindow* parent, Archive* archive)
: wxDialog(parent, -1, "Run", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	wxGridBagSizer* gb_sizer = new wxGridBagSizer(4, 4);
	sizer->Add(gb_sizer, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 10);

	// Game Executable
	gb_sizer->Add(new wxStaticText(this, -1, "Game Executable:"), wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	choice_game_exes = new wxChoice(this, -1);
	gb_sizer->Add(choice_game_exes, wxGBPosition(0, 1), wxGBSpan(1, 2), wxEXPAND);
	btn_add_game = new wxBitmapButton(this, -1, getIcon("t_plus"));
	gb_sizer->Add(btn_add_game, wxGBPosition(0, 3));

	// Executable path
	gb_sizer->Add(new wxStaticText(this, -1, "Path:"), wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	text_exe_path = new wxTextCtrl(this, -1, "");
	text_exe_path->Enable(false);
	gb_sizer->Add(text_exe_path, wxGBPosition(1, 1), wxGBSpan(1, 2), wxEXPAND);
	btn_browse_exe = new wxBitmapButton(this, -1, getIcon("t_open"));
	btn_browse_exe->SetToolTip("Browse...");
	gb_sizer->Add(btn_browse_exe, wxGBPosition(1, 3));

	// Configuration
	gb_sizer->Add(new wxStaticText(this, -1, "Run Configuration:"), wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	choice_config = new wxChoice(this, -1);
	gb_sizer->Add(choice_config, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND);
	btn_edit_config = new wxBitmapButton(this, -1, getIcon("t_settings"));
	btn_edit_config->SetToolTip("Edit command line");
	gb_sizer->Add(btn_edit_config, wxGBPosition(2, 2));
	btn_add_config = new wxBitmapButton(this, -1, getIcon("t_plus"));
	gb_sizer->Add(btn_add_config, wxGBPosition(2, 3));

	// Extra parameters
	gb_sizer->Add(new wxStaticText(this, -1, "Extra Parameters:"), wxGBPosition(3, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
	text_extra_params = new wxTextCtrl(this, -1, run_last_extra);
	gb_sizer->Add(text_extra_params, wxGBPosition(3, 1), wxGBSpan(1, 3), wxEXPAND);

	// Resources
	wxStaticBox* frame = new wxStaticBox(this, -1, "Resources");
	wxStaticBoxSizer* framesizer = new wxStaticBoxSizer(frame, wxVERTICAL);
	sizer->AddSpacer(10);
	sizer->Add(framesizer, 1, wxEXPAND|wxLEFT|wxRIGHT, 10);
	rac_resources = new ResourceArchiveChooser(this, archive);
	framesizer->Add(rac_resources, 1, wxEXPAND|wxALL, 4);

	// Dialog buttons
	wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
	sizer->AddSpacer(8);
	sizer->Add(hbox, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);

	btn_run = new wxButton(this, -1, "Run");
	hbox->AddStretchSpacer();
	hbox->Add(btn_run, 0, wxEXPAND|wxRIGHT, 4);

	btn_cancel = new wxButton(this, -1, "Cancel");
	hbox->Add(btn_cancel, 0, wxEXPAND);

	// Populate game executables dropdown
	int last_index = -1;
	for (unsigned a = 0; a < Executables::nGameExes(); a++)
	{
		Executables::game_exe_t* exe = Executables::getGameExe(a);
		choice_game_exes->AppendString(exe->name);

		if (exe->id == run_last_exe)
			last_index = choice_game_exes->GetCount() - 1;
	}
	if ((int)choice_game_exes->GetCount() > last_index)
	{
		choice_game_exes->Select(last_index);
		openGameExe(last_index);
		choice_config->Select(run_last_config);
	}

	// Bind Events
	btn_add_game->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &RunDialog::onBtnAddGame, this);
	btn_browse_exe->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &RunDialog::onBtnBrowseExe, this);
	btn_edit_config->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &RunDialog::onBtnEditConfig, this);
	btn_add_config->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &RunDialog::onBtnAddConfig, this);
	btn_run->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &RunDialog::onBtnRun, this);
	btn_cancel->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &RunDialog::onBtnCancel, this);
	choice_game_exes->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &RunDialog::onChoiceGameExe, this);
	choice_config->Bind(wxEVT_COMMAND_CHOICE_SELECTED, &RunDialog::onChoiceConfig, this);

	gb_sizer->AddGrowableCol(1, 1);
	SetInitialSize(wxSize(500, 400));
	Layout();
	CenterOnParent();
}

RunDialog::~RunDialog()
{
}

void RunDialog::openGameExe(unsigned index)
{
	// Clear
	choice_config->Clear();
	text_exe_path->SetValue("");

	// Populate configs
	Executables::game_exe_t* exe = Executables::getGameExe(index);
	if (exe)
	{
		for (unsigned a = 0; a < exe->configs.size(); a++)
			choice_config->AppendString(exe->configs[a].key);

		choice_config->SetSelection(0);
		text_exe_path->SetValue(exe->path);
	}
}

string RunDialog::getSelectedCommandLine(Archive* archive, string map_name, string map_file)
{
	Executables::game_exe_t* exe = Executables::getGameExe(choice_game_exes->GetSelection());
	if (exe)
	{
		// Get exe path
		string path = exe->path;
		if (path.IsEmpty())
			return "";

		unsigned cfg = choice_config->GetSelection();
		if (cfg < exe->configs.size())
		{
			path += " ";
			path += exe->configs[cfg].value;
		}

		// IWAD
		path.Replace("%i", S_FMT("\"%s\"", CHR(theArchiveManager->baseResourceArchive()->getFilename())));

		// Resources
		path.Replace("%r", getSelectedResourceList());

		// Archive (+ temp map if specified)
		if (map_file.IsEmpty())
			path.Replace("%a", S_FMT("\"%s\"", CHR(archive->getFilename())));
		else
			path.Replace("%a", S_FMT("\"%s\" \"%s\"", CHR(archive->getFilename()), CHR(map_file)));

		// Map name
		path.Replace("%mn", map_name);

		// Map warp
		if (path.Contains("%mw"))
		{
			string mn = map_name.Lower();

			// MAPxx
			string mapnum;
			if (mn.StartsWith("map", &mapnum))
				path.Replace("%mw", mapnum);

			// ExMx
			else if (mn[0] == 'e' && mn[2] == 'm')
				path.Replace("%mw", S_FMT("%c %c", mn[1], mn[3]));
		}

		// Extra parameters
		if (!text_extra_params->GetValue().IsEmpty())
		{
			path += " ";
			path += text_extra_params->GetValue();
		}

		LOG_MESSAGE(2, "Run command: %s", CHR(path));
		return path;
	}

	return "";
}

string RunDialog::getSelectedResourceList()
{
	return rac_resources->getSelectedResourceList();
}

string RunDialog::getSelectedExeDir()
{
	Executables::game_exe_t* exe = Executables::getGameExe(choice_game_exes->GetSelection());
	if (exe)
	{
		wxFileName fn(exe->path);
		return fn.GetPath(wxPATH_GET_VOLUME);
	}

	return "";
}

string RunDialog::getSelectedExeId()
{
	Executables::game_exe_t* exe = Executables::getGameExe(choice_game_exes->GetSelection());
	if (exe)
		return exe->id;
	else
		return "";
}

void RunDialog::onBtnAddGame(wxCommandEvent& e)
{
	string name = wxGetTextFromUser("Enter a name for the game executable");
	Executables::addGameExe(name);
	choice_game_exes->AppendString(name);
	choice_game_exes->Select(choice_game_exes->GetCount() - 1);
	openGameExe(Executables::nGameExes() - 1);
}
 
void RunDialog::onBtnBrowseExe(wxCommandEvent& e)
{
	Executables::game_exe_t* exe = Executables::getGameExe(choice_game_exes->GetSelection());

	if (exe)
	{
		SFileDialog::fd_info_t info;
		if (SFileDialog::openFile(info, "Browse for game executable", "Executable files (*.exe)|*.exe", this, exe->exe_name))
		{
			text_exe_path->SetValue(info.filenames[0]);
			exe->path = info.filenames[0];
		}
	}
}

void RunDialog::onBtnAddConfig(wxCommandEvent& e)
{
	if (choice_game_exes->GetSelection() < 0)
		return;

	Executables::game_exe_t* exe = Executables::getGameExe(choice_game_exes->GetSelection());

	RunConfigDialog dlg(this, S_FMT("Add Run Config for %s", CHR(exe->name)), "", "");
	if (dlg.ShowModal() == wxID_OK)
	{
		exe->configs.push_back(key_value_t(dlg.getName(), dlg.getParams()));
		choice_config->AppendString(dlg.getName());
		choice_config->Select(choice_config->GetCount() - 1);
	}
}

void RunDialog::onBtnEditConfig(wxCommandEvent& e)
{
	if (choice_game_exes->GetSelection() < 0 || choice_config->GetSelection() < 0)
		return;

	Executables::game_exe_t* exe = Executables::getGameExe(choice_game_exes->GetSelection());
	int index = choice_config->GetSelection();
	string name = exe->configs[index].key;
	string params = exe->configs[index].value;

	RunConfigDialog dlg(this, "Edit Run Config", name, params);
	if (dlg.ShowModal() == wxID_OK)
	{
		exe->configs[index].key = dlg.getName();
		exe->configs[index].value = dlg.getParams();
		choice_config->SetString(index, dlg.getName());
	}
}

void RunDialog::onBtnRun(wxCommandEvent& e)
{
	if (text_exe_path->GetValue() == "" || !wxFileExists(text_exe_path->GetValue()))
	{
		wxMessageBox("Invalid executable path", "Error", wxICON_ERROR);
		return;
	}

	run_last_extra = text_extra_params->GetValue();
	EndModal(wxID_OK);
}

void RunDialog::onBtnCancel(wxCommandEvent& e)
{
	run_last_extra = text_extra_params->GetValue();
	EndModal(wxID_CANCEL);
}

void RunDialog::onChoiceGameExe(wxCommandEvent& e)
{
	openGameExe(e.GetSelection());
	run_last_exe = getSelectedExeId();
}

void RunDialog::onChoiceConfig(wxCommandEvent& e)
{
	run_last_config = choice_config->GetSelection();
}
