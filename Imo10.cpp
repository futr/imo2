//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("Main.cpp", SatViewMainForm);
USEFORM("Status.cpp", StatusForm);
USEFORM("PixValue.cpp", PixForm);
USEFORM("About.cpp", AboutForm);
USEFORM("PresetNDVI.cpp", PresetFormNDVI);
USEFORM("PresetHTM.cpp", PresetFormTHM);
USEFORM("PresetLS_THM.cpp", PresetFormLS_THM);
USEFORM("HelpFormCode.cpp", HelpForm);
USEFORM("SaveFormUnit.cpp", SaveForm);
USEFORM("ColorBar.cpp", ColorBarForm);
USEFORM("ColorBarConfigUnit.cpp", ColorBarConfigForm);
USEFORM("ColorMapConfigUnit.cpp", ColorMapConfigForm);
USEFORM("PresetFormAGDEMUnit.cpp", PresetFormAGDEM);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->Title = "SatelliteEye";
		Application->CreateForm(__classid(TSatViewMainForm), &SatViewMainForm);
		Application->CreateForm(__classid(TStatusForm), &StatusForm);
		Application->CreateForm(__classid(TPixForm), &PixForm);
		Application->CreateForm(__classid(TAboutForm), &AboutForm);
		Application->CreateForm(__classid(TPresetFormNDVI), &PresetFormNDVI);
		Application->CreateForm(__classid(TPresetFormTHM), &PresetFormTHM);
		Application->CreateForm(__classid(TPresetFormLS_THM), &PresetFormLS_THM);
		Application->CreateForm(__classid(THelpForm), &HelpForm);
		Application->CreateForm(__classid(TSaveForm), &SaveForm);
		Application->CreateForm(__classid(TColorBarForm), &ColorBarForm);
		Application->CreateForm(__classid(TColorBarConfigForm), &ColorBarConfigForm);
		Application->CreateForm(__classid(TColorMapConfigForm), &ColorMapConfigForm);
		Application->CreateForm(__classid(TPresetFormAGDEM), &PresetFormAGDEM);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
