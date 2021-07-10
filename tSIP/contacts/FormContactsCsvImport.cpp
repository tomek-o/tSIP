//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormContactsCsvImport.h"
#include "Contacts.h"
#include <iostream>
#include <fstream>
#include <string>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmContactsCsvImport *frmContactsCsvImport;


//---------------------------------------------------------------------------
__fastcall TfrmContactsCsvImport::TfrmContactsCsvImport(TComponent* Owner)
	: TForm(Owner),
	contacts(NULL)
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

void TfrmContactsCsvImport::UpdateButtons(void)
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

void TfrmContactsCsvImport::Load(AnsiString fileName, Contacts *contacts)
{
	this->fileName = fileName;
    this->contacts = contacts;

	pageIdx = 0;
	pcWizard->ActivePage = pcWizard->Pages[pageIdx];
	UpdateButtons();	

	ShowModal();
}

void __fastcall TfrmContactsCsvImport::btnNextClick(TObject *Sender)
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

	if (pcWizard->ActivePage == tsReadCsv)
	{
		if (csvRows[0].empty())
		{
			ShowMessage("No columns found in CSV file.");
			return;
		}

		cbDescriptionColumn->Clear();
		cbNumberColumn1->Clear();
		cbNumberColumn1->Items->Add("- none -");
		cbNumberColumn2->Clear();
		cbNumberColumn2->Items->Add("- none -");
		cbNumberColumn3->Clear();
		cbNumberColumn3->Items->Add("- none -");
		cbCompanyColumn->Clear();
		cbCompanyColumn->Items->Add("- none -");
		cbNoteColumn->Clear();
		cbNoteColumn->Items->Add("- none -");
		for (unsigned int i=0; i<csvRows[0].size(); i++)
		{
			AnsiString text;
			text.sprintf("%d: %s", i+1, csvRows[0][i].c_str());
			cbDescriptionColumn->Items->Add(text);
			cbNumberColumn1->Items->Add(text);
			cbNumberColumn2->Items->Add(text);
			cbNumberColumn3->Items->Add(text);
			cbCompanyColumn->Items->Add(text);
			cbNoteColumn->Items->Add(text);
		}
		cbDescriptionColumn->ItemIndex = 0;
		cbNumberColumn1->ItemIndex = 0;
		cbNumberColumn2->ItemIndex = 0;
		cbNumberColumn3->ItemIndex = 0;
		cbCompanyColumn->ItemIndex = 0;
		cbNoteColumn->ItemIndex = 0;
	}

	if (pageIdx < pcWizard->PageCount - 1)
	{
		pageIdx++;
		pcWizard->ActivePage = pcWizard->Pages[pageIdx];
		UpdateButtons();
	}
	else
	{
		if (cbNumberColumn1->ItemIndex == 0 &&
			cbNumberColumn2->ItemIndex == 0 &&
			cbNumberColumn3->ItemIndex == 0)
		{
			ShowMessage("Select at least one column for numbers");
			return;
        }

		AnsiString asMsg;
		int status = UpdateContacts();
		if (status == 0)
		{
			ModalResult = mrOk;
		}
		else
		{
			AnsiString asGenMsg;
			asGenMsg.sprintf("Failed to update contacts.");
			Application->MessageBox(asGenMsg.c_str(), Caption.c_str(), MB_ICONSTOP);
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmContactsCsvImport::btnPrevClick(TObject *Sender)
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

int TfrmContactsCsvImport::ParseCsv(AnsiString asFileName,
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

int TfrmContactsCsvImport::UpdateContacts(void)
{
	if (csvRows.size() == 0)
	{
		return 0;
	}

	unsigned int id = 0;
	if (chbSkipFirstRow->Checked) {
		if (csvRows.size() < 2)
		{
			//asMsg = "Selected CSV->JSON conversion style requires at least two input rows\n"
			//	"(header + at least 1 row).";
			return 1;
		}
		id = 1;
	}

	int descriptionId =	cbDescriptionColumn->ItemIndex;
	int numberId1 = cbNumberColumn1->ItemIndex - 1;	// "- none -" offset
	int numberId2 = cbNumberColumn2->ItemIndex - 1;
	int numberId3 = cbNumberColumn3->ItemIndex - 1;
	int companyId = cbCompanyColumn->ItemIndex - 1;
	int noteId = cbNoteColumn->ItemIndex - 1;

	int addedCnt = 0;
	int skippedCnt = 0;
	for (id; id < csvRows.size(); ++id)
	{
		Contacts::Entry entry;
		Fields &fields = csvRows[id];
		int fieldsSize = fields.size();
		if (fieldsSize > descriptionId &&
			fieldsSize > numberId1 &&
			fieldsSize > numberId2 &&
			fieldsSize > numberId3 &&
			fieldsSize > companyId
			)
		{
			entry.description = fields[descriptionId].c_str();
			if (numberId1 >= 0)
			{
				entry.uri1 = fields[numberId1].c_str();
			}
			if (numberId2 >= 0)
			{
				entry.uri2 = fields[numberId2].c_str();
			}
			if (numberId3 >= 0)
			{
				entry.uri3 = fields[numberId3].c_str();
			}
			if (companyId >= 0)
			{
				entry.company = fields[companyId].c_str();
			}
			if (noteId >= 0 && fieldsSize > noteId)
			{
            	entry.note = fields[noteId].c_str();
			}
			if (chbSkipDuplicatedNumbers->Checked)
			{
				if (contacts->GetEntry(entry.uri1))
				{
                    skippedCnt++;
					continue;
				}
			}
			contacts->GetEntries().push_back(entry);
			addedCnt++;
		}
	}

	AnsiString asMsg;
	asMsg.sprintf("Added %d, skipped %d contact(s).", addedCnt, skippedCnt);
	Application->MessageBox(asMsg.c_str(), Caption.c_str(), MB_ICONINFORMATION);

	return 0;
}

void __fastcall TfrmContactsCsvImport::lblInfo2Click(TObject *Sender)
{
	ShellExecute(NULL, "open", lblInfo2->Caption.c_str(), NULL, NULL, SW_SHOWNORMAL);	
}
//---------------------------------------------------------------------------

