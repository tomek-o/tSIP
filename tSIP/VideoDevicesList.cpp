//---------------------------------------------------------------------------
#pragma hdrstop

#include "VideoDevicesList.h"
#include "VideoModules.h"
#include "Log.h"

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>

#include <comutil.h>
#include <commctrl.h>
#include <dshow.h>
#include <qedit.h>

//---------------------------------------------------------------------------

#pragma package(smart_init)

VideoDevicesList::VideoDevicesList(void)
{

}

void VideoDevicesList::Refresh(void)
{
	dshowDevsIn.clear();

	ICreateDevEnum *dev_enum;
	IEnumMoniker *enum_mon;
	IMoniker *mon;
	ULONG fetched;
	HRESULT res;
	bool found = false;


	res = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
			       CLSCTX_INPROC_SERVER,
			       IID_ICreateDevEnum, (void**)&dev_enum);
	if (res != NOERROR)
	{
		LOG("Failed to create system devices enumerator!\n");
		return;
	}

	res = dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
					      &enum_mon, 0);
	if (res != NOERROR)
	{
		LOG("Failed to create video input class enumerator!\n");
		return;
	}

	enum_mon->Reset();
	while (enum_mon->Next(1, &mon, &fetched) == S_OK && !found) {

		IPropertyBag *bag;
		VARIANT var;
		char dev_name[256];
		int len = 0;

		res = mon->BindToStorage(0, 0, IID_IPropertyBag,
					 (void **)&bag);
		if (!SUCCEEDED(res))
			continue;

		::VariantInit(&var);			
		var.vt = VT_BSTR;

		LPVOID fns = CoTaskMemAlloc(26);
		memcpy(fns, L"FriendlyName", 26);
		res = bag->Read((wchar_t*)fns, &var, NULL);
		CoTaskMemFree(fns);
		if (NOERROR != res)
			continue;

		len = WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1,
					  dev_name, sizeof(dev_name),
					  NULL, NULL);

		SysFreeString(var.bstrVal);
		bag->Release();

		if (len > 0) {
			dshowDevsIn.push_back(dev_name);
		}

		mon->Release();
	}
}

void VideoDevicesList::FillComboBox(Stdctrls::TComboBox *target, AnsiString module, bool out, AnsiString selected)
{
    target->Tag = 0;
	target->Items->Clear();
	std::vector<AnsiString> *v = NULL;
	if (module == VideoModules::dshow)
	{
		if (out == false)
			v = &VideoDevicesList::Instance().dshowDevsIn;
		else
			return;
	}
	else
	{
		return;
    }
	assert(v);
	for (int i=0; i<v->size(); i++)
	{
		AnsiString dev = v->at(i);
		target->Items->Add(dev);
		if (dev == selected)
		{
			target->ItemIndex = i;
		}
	}
	// convention: if selected device is not found - add it at last item with [NOT FOUND] text and set non-zero Tag
	if (target->ItemIndex < 0 && target->Items->Count > 0 && selected != "")
	{
		AnsiString text;
		#undef sprintf
		text.sprintf("[NOT FOUND] %s", selected.c_str());
		target->Items->Add(text);
		target->ItemIndex = target->Items->Count - 1;
		target->Tag = 1;
	}
}
