//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormContactCsvImport.h"
#include "json/json.h"
#include <iostream>
#include <fstream>
#include <string>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmContactCsvImport *frmContactCsvImport;


//---------------------------------------------------------------------------
__fastcall TfrmContactCsvImport::TfrmContactCsvImport(TComponent* Owner)
	: TForm(Owner),
	bUseStyledWriter(false)
{
    edFieldSeparator->Text = ",";
	for (int i=0; i<pcWizard->PageCount; i++)
	{
		pcWizard->Pages[i]->TabVisible = false;
	}
	pageIdx = 0;
	pcWizard->ActivePage = pcWizard->Pages[pageIdx];
	UpdateButtons();
}
//---------------------------------------------------------------------------

void TfrmContactCsvImport::UpdateButtons(void)
{
	if (pcWizard->ActivePage == pcWizard->Pages[0])
	{
		btnPrev->Visible = false;
		btnNext->Visible = true;
		btnNext->Caption = "Next >";
	}
	else if (pcWizard->ActivePage == pcWizard->Pages[pcWizard->PageCount - 1])
	{
		btnPrev->Visible = true;
		btnNext->Visible = true;
		btnNext->Caption = "Finish";
	}
	else
	{
		btnPrev->Visible = true;
		btnNext->Visible = true;
		btnNext->Caption = "Next >";
	}
}

void TfrmContactCsvImport::Load(AnsiString fileName, bool bUseStyledWriter)
{
	this->fileName = fileName;
	this->bUseStyledWriter = bUseStyledWriter;

	pageIdx = 0;
	pcWizard->ActivePage = pcWizard->Pages[pageIdx];
	UpdateButtons();	

	ShowModal();
}

AnsiString TfrmContactCsvImport::GetJson(void)
{
	return asJson;
}

void __fastcall TfrmContactCsvImport::btnNextClick(TObject *Sender)
{
	if (pcWizard->ActivePage == tsConfigureCsv)
	{
		if (edFieldSeparator->Text.Length() != 1)
		{
			ShowMessage("Enter separator first!");
			return;
		}
		if (edFieldSeparator->Text == "\"")
		{
			ShowMessage("Double-quote is not allowed as a separator.");
			return;
		}
		AnsiString asMsg;
		if (ParseCsv(fileName, edFieldSeparator->Text[1], asMsg))
		{
			AnsiString asParseMsg;
			asParseMsg.sprintf("Failed to parse %s.\n%s", fileName.c_str(), asMsg.c_str());
			Application->MessageBox(asParseMsg.c_str(), Caption.c_str(), MB_ICONSTOP);
			return;
		}
		edRows->Text = csvRows.size();
		if (csvRows.size() > 0)
		{
			edColumns->Text = csvRows[0].size();
		}
		else
		{
			edColumns->Text = "-";
		}
	}

	if (pageIdx < pcWizard->PageCount - 1)
	{
		pageIdx++;
		pcWizard->ActivePage = pcWizard->Pages[pageIdx];
		UpdateButtons();
	}
	else
	{
		AnsiString asMsg;
		int status = GenerateJson(cbJsonStyle->ItemIndex,
			chbExtractNumbers->Checked, cbDecimalSeparator->Text[1], asJson, asMsg);
		if (status == 0)
		{
			ModalResult = mrOk;
		}
		else
		{
			AnsiString asGenMsg;
			asGenMsg.sprintf("Failed to generate JSON.\n%s", asMsg.c_str());
			Application->MessageBox(asGenMsg.c_str(), Caption.c_str(), MB_ICONSTOP);
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmContactCsvImport::btnPrevClick(TObject *Sender)
{
	if (pageIdx > 0)
	{
		pageIdx--;
		pcWizard->ActivePage = pcWizard->Pages[pageIdx];
		UpdateButtons();
	}
}
//---------------------------------------------------------------------------

int ExtractCsvVal(const char*& buf, char separator, AnsiString& asField, AnsiString& asMsg)
{
	asField = "";
	asMsg = "";
	bool quoted = false;
	bool quotedClosed = false;
	if (*buf == '"')
	{
		quoted = true;
		buf++;
	}
	for (;;)
	{
		if (*buf == '"')
		{
			if (quoted == false)
			{
				asMsg = "Forbidden: double-quote in non-quoted field.";
				return 1;
			}
			else if (quotedClosed == true)
			{
				asMsg = "Forbidden: double-quote in field with already closed quotation.";
				return 1;
			}
			else
			{
				if (*(buf+1) == '"')
				{
                    // double double-quote => double-quote
					asField += "\"";
					buf++;
				}
				else
				{
					quotedClosed = true;
				}
			}
		}
		else if (*buf == separator || *buf == '\r' || *buf == '\n')
		{
			if (!quoted)
			{
				return 0;
			}
			else if (quoted && !quotedClosed)
			{
				asField += *buf;
			}
			else if (quoted && quotedClosed)
			{
				return 0;
			}
		}
		else if (*buf == '\0')
		{
			if (!quoted || (quoted && quotedClosed))
			{
				return 0;
			}
			else
			{
                asMsg = "Field quotation not closed.";
				return 1;
			}
		}
		else
		{
			if (quotedClosed == true)
			{
				asMsg = "Forbidden: text in field after closed quotation.";
				return 1;
			}
			asField += *buf;
		}
		
		buf++;
	}
}

int TfrmContactCsvImport::ParseCsv(AnsiString asFileName,
	char separator, AnsiString& asMsg)
{
	csvRows.clear();

	std::ifstream file (asFileName.c_str(), ios::in | ios::binary);
	if (file.is_open() == false)
	{
		asMsg = "Could not open file.";
		return 1;
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	std::string buffer(size, ' ');
	file.seekg(0);
	file.read(&buffer[0], size);
	file.close();

	const char* buf = buffer.c_str();
	AnsiString asField;
	Fields emptyRow;
	csvRows.push_back(emptyRow);
	for (;;)
	{
        asField = "";
		int status = ExtractCsvVal(buf, separator, asField, asMsg);
		if (status)
		{
			return 2;
		}
		Fields& fields = csvRows[csvRows.size()-1];
		fields.push_back(asField);
		if (*buf == separator)
		{

		}
		else if (*buf == '\n')
		{
			if (*(buf+1) != '\0')
			{
				csvRows.push_back(emptyRow);
			}
		}
		else if (*buf == '\r')
		{
			if (*(buf+1) == '\n')
				buf++;
			if (*(buf+1) != '\0')
			{
				csvRows.push_back(emptyRow);
			}
		}
		else if (*buf == '\0')
		{
			break;
		}
		buf++;	// skip separator/newline
		if (*buf == '\0')
		{
			break;
		}
	}

	unsigned int fieldCnt = csvRows[0].size();
	for (unsigned int i=1; i<csvRows.size(); i++)
	{
		if (csvRows[i].size() != fieldCnt)
		{
			// allow last row to be empty
			if ((i == csvRows.size() - 1) && csvRows[i].size() == 0)
			{
            	csvRows[i].pop_back();
			}
			else
			{
				asMsg.sprintf("Field numbers in rows are inconsistent (row 0: %d, row %d: %d).",
					fieldCnt, i, csvRows[i].size());
				return 3;
			}
		}
	}

	return 0;
}

void __fastcall TfrmContactCsvImport::chbExtractNumbersClick(TObject *Sender)
{
	cbDecimalSeparator->Enabled = chbExtractNumbers->Checked;
}
//---------------------------------------------------------------------------


void __fastcall TfrmContactCsvImport::chbExtractNumbersKeyPress(TObject *Sender,
      char &Key)
{
	cbDecimalSeparator->Enabled = chbExtractNumbers->Checked;
}
//---------------------------------------------------------------------------

void SetValue(Json::Value& value, AnsiString str, bool extractNumbers, TFormatSettings& fs)
{
	if (extractNumbers == false)
	{
		value = str.c_str();
	}
	else
	{
		try
		{
			int val = StrToInt(str);
			value = val;
			return;
		}
		catch (EConvertError &e)
		{
		}

		try
		{
			double val = (double)StrToFloat(str, fs);
			value = val;
			return;
		}
		catch (EConvertError &e)
		{
		}

		value = str.c_str();
	}
}

int TfrmContactCsvImport::GenerateJson(int style,
	bool extractNumbers, char decimalSeparator,
	AnsiString& asJson, AnsiString& asMsg)
{
	if (csvRows.size() == 0)
	{
		asJson = "{}";
		return 0;
	}

	Json::Value root;
	std::auto_ptr<Json::Writer> writer(NULL);
	if (bUseStyledWriter)
		writer.reset(new Json::StyledWriter);
	else
		writer.reset(new Json::FastWriter);

	TFormatSettings fs;
	GetLocaleFormatSettings(0, fs);
	fs.DecimalSeparator = decimalSeparator;

	if (style == 0)
	{
		// "Properties"
		if (csvRows.size() < 2)
		{
			asMsg = "Selected CSV->JSON conversion style requires at least two input rows\n"
				"(header + at least 1 row).";
			return 1;
		}
		unsigned int cols = csvRows[0].size();
		std::vector<AnsiString>& names = csvRows[0];
		for (unsigned int i=1; i<csvRows.size(); i++)
		{
			Json::Value &value = root[i-1];
			for (unsigned int col = 0; col < cols; col++)
			{
				SetValue(value[names[col].c_str()], csvRows[i][col], extractNumbers, fs);
			}
		}
	}
	else if (style == 1)
	{
		// "Column arrays"
		if (csvRows.size() < 2)
		{
			asMsg = "Selected CSV->JSON conversion style requires at least two input rows\n"
				"(header + at least 1 row).";
			return 1;
		}		
		unsigned int cols = csvRows[0].size();
		std::vector<AnsiString>& names = csvRows[0];		
		for (unsigned int col = 0; col < cols; col++)
		{
			Json::Value &value = root[names[col].c_str()];
			for (unsigned int i=1; i<csvRows.size(); i++)
			{
				SetValue(value[i-1], csvRows[i][col], extractNumbers, fs);
			}
		}
	}
	else
	{
		// "Row arrays"
		unsigned int cols = csvRows[0].size();
		for (unsigned int i=0; i<csvRows.size(); i++)
		{
			Json::Value &value = root[i];
			for (unsigned int col = 0; col < cols; col++)
			{
				SetValue(value[col], csvRows[i][col], extractNumbers, fs);
			}
		}
	}

    asJson = writer->write(root).c_str();

	return 0;
}

void __fastcall TfrmContactCsvImport::lblInfo2Click(TObject *Sender)
{
	ShellExecute(NULL, "open", lblInfo2->Caption.c_str(), NULL, NULL, SW_SHOWNORMAL);	
}
//---------------------------------------------------------------------------

