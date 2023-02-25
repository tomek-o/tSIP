//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "FormSettings.h"
#include "FormAccount.h"
#include "FormHotkeys.h"
#include "FormPhones.h"
#include "FormUaConfOpus.h"
#include "FormDialpadConf.h"
#include "FormVideoConf.h"
#include "AudioDevicesList.h"
#include "AudioModules.h"
#include "ProgrammableButtons.h"
#include "FormLuaScript.h"
#include "UaMain.h"
#include "NetInterfaces.h"
#include "Paths.h"
#include "Registry.hpp"
#include "Branding.h"
#include "Translate.h"
#include "baresip_base_config.h"
#include <FileCtrl.hpp>
#include <assert.h>
#include <stdio.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmSettings *frmSettings;

namespace
{
	int audioCodecsEnabledDraggedIndex = -1;
	int videoCodecsEnabledDraggedIndex = -1;

    inline void strncpyz(char* dst, const char* src, int dstsize)
	{
		strncpy(dst, src, dstsize);
		dst[dstsize-1] = '\0';
	}

	void StrToIntDef2(const AnsiString &str, int& value)
	{
		value = StrToIntDef(str, value);
	}

	std::vector<NetInterface> networkInterfaces;

	const char* EMPTY_TRANSLATION_NAME = "- NONE (default, English) -";
}	// namespace



//---------------------------------------------------------------------------
__fastcall TfrmSettings::TfrmSettings(TComponent* Owner)
	: TForm(Owner), lastTab(NULL)
{
	frmHotkeys = new TfrmHotkeys(tsHotkeys);
	frmHotkeys->Parent = tsHotkeys;
	frmHotkeys->Visible = true;

	frmPhones = new TfrmPhones(tsPhones);
	frmPhones->Parent = tsPhones;
	frmPhones->Visible = true;

	frmUaConfOpus = new TfrmUaConfOpus(tsUaConfOpus);
	frmUaConfOpus->Parent = tsUaConfOpus;
	frmUaConfOpus->Visible = true;

	frmDialpadConf = new TfrmDialpadConf(tsDialpad);
	frmDialpadConf->Parent = tsDialpad;
	frmDialpadConf->Visible = true;

	frmVideoConf = new TfrmVideoConf(tsVideo);
	frmVideoConf->Parent = tsVideo;
	frmVideoConf->Visible = true;

	AudioModules::FillInputSelectorCb(cbSoundInputMod);
	AudioModules::FillOutputSelectorCb(cbSoundOutputMod);
	AudioModules::FillOutputSelectorCb(cbSoundAlertOutputMod);
	AudioModules::FillOutputSelectorCb(cbSoundRingOutputMod);
	AudioModules::FillOutputSelectorCb(cbSoundOutputIntercomMod);

	for (int i=0; i<pcGeneral->PageCount; i++)
	{
		pcGeneral->Pages[i]->TabVisible = false;
		pcGeneral->Pages[i]->Visible = false;
	}

	CreatePages();
	chbHistoryShowRecordFileInHint->Visible = Branding::recording;
}
//---------------------------------------------------------------------------

TTreeNode* TfrmSettings::CreatePagesNode(TTreeNode *parent, TTabSheet *tab)
{
	AnsiString chbName;
	chbName.sprintf("%s [%s]", tab->Caption.c_str(), tab->Name.c_str());
	chbBoxLockingSettingsPages->Items->Add(chbName);

	for (unsigned int j=0; j<tmpSettings.locking.hiddenSettingsPages.size(); j++)
	{
		if (tmpSettings.locking.hiddenSettingsPages[j] == tab->Name)
		{
			tab->Visible = false;
			chbBoxLockingSettingsPages->Checked[chbBoxLockingSettingsPages->Items->Count - 1] = true;
			return NULL;
		}
	}
	chbBoxLockingSettingsPages->Checked[chbBoxLockingSettingsPages->Items->Count - 1] = false;

	TTreeNode *node;
	if (parent == NULL)
		node = tvSelector->Items->Add(parent, tab->Caption);
	else
		node = tvSelector->Items->AddChild(parent, tab->Caption);
	node->Data = tab;

	return node;
}

void TfrmSettings::CreatePages(void)
{
	tvSelector->Items->Clear();
	chbBoxLockingSettingsPages->Items->Clear();

	CreatePagesNode(NULL, tsGeneral);
	CreatePagesNode(NULL, tsNetwork);
	TTreeNode *nodeAccount = CreatePagesNode(NULL, tsAccount);
	CreatePagesNode(NULL, tsTls);
	TTreeNode *nodeMainWindow = CreatePagesNode(NULL, tsMainWindow);
	CreatePagesNode(nodeMainWindow, tsDialpad);
	CreatePagesNode(NULL, tsSpeedDial);
	CreatePagesNode(NULL, tsCalls);
	CreatePagesNode(NULL, tsMessages);
	CreatePagesNode(NULL, tsDisplay);
	TTreeNode *nodeLocking = CreatePagesNode(NULL, tsLocking);
	CreatePagesNode(nodeLocking, tsLockingSettingsPages);
	CreatePagesNode(NULL, tsBranding);
	CreatePagesNode(NULL, tsRing);
	CreatePagesNode(NULL, tsAudioIO);
	CreatePagesNode(NULL, tsAudioProcessing);
	if (Branding::recording)
		CreatePagesNode(NULL, tsRecording);
	TTreeNode *nodeCodecs = CreatePagesNode(NULL, tsCodecs);
	CreatePagesNode(nodeCodecs, tsUaConfOpus);
#ifdef USE_VIDEO
	CreatePagesNode(NULL, tsVideo);
	TTreeNode *nodeCodecsVideo = CreatePagesNode(NULL, tsVideoCodecs);
#endif
	CreatePagesNode(NULL, tsIntegration);
	CreatePagesNode(NULL, tsHotkeys);
	CreatePagesNode(NULL, tsContacts);
	CreatePagesNode(NULL, tsHistory);
	CreatePagesNode(NULL, tsPhones);
	CreatePagesNode(NULL, tsTrayNotifier);
	CreatePagesNode(NULL, tsScripts);
	CreatePagesNode(NULL, tsLogging);

	if (nodeAccount)
	{
		// make "Accounts" selected and visible
		nodeAccount->Selected = true;
	}
	else
	{
		if (tvSelector->Items->Count > 0)
		{
        	tvSelector->Items->Item[0]->Selected = true;
		}
	}
}

void __fastcall TfrmSettings::FormShow(TObject *Sender)
{
	AudioDevicesList::Instance().Refresh();

	tmpSettings = appSettings;

	cbSoundInputMod->ItemIndex = AudioModules::GetInputModuleCbIndex(tmpSettings.uaConf.audioCfgSrc.mod);
	cbSoundInputModChange(NULL);
	edSoundInputWave->Text = tmpSettings.uaConf.audioCfgSrc.wavefile.c_str();

	cbSoundOutputMod->ItemIndex = AudioModules::GetOutputModuleCbIndex(tmpSettings.uaConf.audioCfgPlay.mod);
	cbSoundOutputModChange(NULL);

	cbSoundAlertOutputMod->ItemIndex = AudioModules::GetOutputModuleCbIndex(tmpSettings.uaConf.audioCfgAlert.mod);
	cbSoundAlertOutputModChange(NULL);

	cbSoundRingOutputMod->ItemIndex = AudioModules::GetOutputModuleCbIndex(tmpSettings.uaConf.audioCfgRing.mod);
	cbSoundRingOutputModChange(NULL);

	cbSoundOutputIntercomMod->ItemIndex = AudioModules::GetOutputModuleCbIndex(tmpSettings.uaConf.audioCfgPlayIntercom.mod);
	cbSoundOutputIntercomModChange(NULL);

	cbAec->ItemIndex = tmpSettings.uaConf.aec;


	edLocalAddress->Text = tmpSettings.uaConf.local.c_str();

	edIfName->Text = tmpSettings.uaConf.ifname.c_str();
	{
		cbNetworkInterfaces->Items->Clear();
		if (GetNetInterfaces(networkInterfaces) == 0)
		{
			for (unsigned int i=0; i<networkInterfaces.size(); i++)
			{
				const NetInterface &ni = networkInterfaces[i];
				AnsiString text;
				AnsiString driverName = GetNetAdapterDriverName(ni.name);
				text.sprintf("%s  -  %s  -  %s", ni.ip.c_str(), driverName.c_str(), ni.name.c_str());
				cbNetworkInterfaces->Items->Add(text);
			}
		}
		cbNetworkInterfaces->Items->Add("Custom (Windows: full GUID with braces) / none (empty string)");
		cbNetworkInterfaces->ItemIndex = cbNetworkInterfaces->Items->Count - 1;
		for (unsigned int i=0; i<networkInterfaces.size(); i++)
		{
			const NetInterface &ni = networkInterfaces[i];
			if (ni.name == tmpSettings.uaConf.ifname.c_str())
			{
				cbNetworkInterfaces->ItemIndex = i;
			}
		}
		UpdateNetworkInterface();
	}

	edRtpPortMin->Text = tmpSettings.uaConf.avt.portMin;
	edRtpPortMax->Text = tmpSettings.uaConf.avt.portMax;
	edJbufDelayMin->Text = tmpSettings.uaConf.avt.jbufDelayMin;
	edJbufDelayMax->Text = tmpSettings.uaConf.avt.jbufDelayMax;
	edRtpTimeout->Text = tmpSettings.uaConf.avt.rtpTimeout;

	edMessagesReplyCode->Text = tmpSettings.uaConf.messages.replyCode;
	edMessagesReplyReason->Text = tmpSettings.uaConf.messages.replyReason.c_str();
	chbMessagesDoNotReply->Checked = tmpSettings.uaConf.messages.doNotReply;

	for (int i=0; i<tmpSettings.uaConf.accounts.size(); i++)
	{
		frmAccount = new TfrmAccount(NULL, i, tmpSettings.uaConf.accounts[i]);
		frmAccount->Parent = frmSettings->scrbAccounts;
		frmAccount->Visible = true;
		// force realign
		frmAccount->Align = alBottom;
		frmAccount->Align = alTop;
		frmAccountVector.push_back(frmAccount);
	}

	edGuiScaling->Text = tmpSettings.gui.scalingPct;

	edCollapsedWidth->Text = tmpSettings.frmMain.collapsedWidth;
	edCollapsedHeight->Text = tmpSettings.frmMain.collapsedHeight;
	edExpandedWidth->Text = tmpSettings.frmMain.expandedWidth;
	edExpandedHeight->Text = tmpSettings.frmMain.expandedHeight;
	chbFrmMainUseClientAreaSizes->Checked = tmpSettings.frmMain.bUseClientAreaSizes;
	edExpandingPosLeftOffset->Text = tmpSettings.frmMain.expandingPosLeftOffset;
	edExpandingPosTopOffset->Text = tmpSettings.frmMain.expandingPosTopOffset;

	edCallPanelCollapsedLeft->Text = tmpSettings.frmMain.collapsedCallPanelLeft;
	edCallPanelCollapsedTop->Text = tmpSettings.frmMain.collapsedCallPanelTop;
	edCallPanelExpandedLeft->Text = tmpSettings.frmMain.expandedCallPanelLeft;
	edCallPanelExpandedTop->Text = tmpSettings.frmMain.expandedCallPanelTop;

	edMainPanelCollapsedLeft->Text = tmpSettings.frmMain.collapsedMainPanelLeft;
	edMainPanelCollapsedTop->Text = tmpSettings.frmMain.collapsedMainPanelTop;
	edMainPanelExpandedLeft->Text = tmpSettings.frmMain.expandedMainPanelLeft;
	edMainPanelExpandedTop->Text = tmpSettings.frmMain.expandedMainPanelTop;

	std::vector<AnsiString> translations = EnumerateTranslations();
	cbTranslation->Clear();
	cbTranslation->Items->Add(EMPTY_TRANSLATION_NAME);
	cbTranslation->ItemIndex = 0;
	for (unsigned int i=0; i<translations.size(); i++)
	{
		cbTranslation->Items->Add(translations[i]);
		if (translations[i] == tmpSettings.Translation.language)
			cbTranslation->ItemIndex = i + 1;
	}
	chbTranslationLogMissingKeys->Checked = tmpSettings.Translation.logMissingKeys;	

	chbAlwaysOnTop->Checked = tmpSettings.frmMain.bAlwaysOnTop;
	chbStartMinimizedToTray->Checked = tmpSettings.frmMain.bStartMinimizedToTray;

	chbTrayNotifierShowOnIncoming->Checked = tmpSettings.frmTrayNotifier.showOnIncoming;
	chbSkipTrayNotifierIfMainWindowVisible->Checked = tmpSettings.frmTrayNotifier.skipIfMainWindowVisible;
	chbTrayNotifierShowOnOutgoing->Checked = tmpSettings.frmTrayNotifier.showOnOutgoing;
	edTrayNotifierBackgroundImage->Text = tmpSettings.frmTrayNotifier.backgroundImage;
	edTrayNotifierGuiScaling->Text = tmpSettings.frmTrayNotifier.scalingPct;
	chbTrayNotifierHideWhenAnsweringCall->Checked = tmpSettings.frmTrayNotifier.hideWhenAnsweringCall;
	chbTrayNotifierHideWhenAnsweringCallAutomatically->Checked = tmpSettings.frmTrayNotifier.hideWhenAnsweringCallAutomatically;
	chbTrayNotifierDoNotChangePosition->Checked = tmpSettings.frmTrayNotifier.doNotChangePosition;

	chbFrmMainUseCustomApplicationTitle->Checked = tmpSettings.frmMain.bUseCustomApplicationTitle;
	edFrmMainCustomApplicationTitle->Text = tmpSettings.frmMain.customApplicationTitle;
	chbFrmMainUseCustomCaption->Checked = tmpSettings.frmMain.bUseCustomCaption;
	edFrmMainCustomCaption->Text = tmpSettings.frmMain.customCaption;

	chbFrmMainHideCallPanel->Checked = tmpSettings.frmMain.bHideCallPanel;
	chbFrmMainHideMainPanel->Checked = tmpSettings.frmMain.bHideMainPanel;

	chbSpeedDialPopupMenu->Checked = tmpSettings.frmMain.bSpeedDialPopupMenu;
	chbSpeedDialIgnorePresenceNote->Checked = tmpSettings.frmMain.bSpeedDialIgnorePresenceNote;
	chbSpeedDialIgnoreDialogInfoRemoteIdentity->Checked = tmpSettings.frmMain.bSpeedDialIgnoreDialogInfoRemoteIdentity;
	chbSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing->Checked = tmpSettings.frmMain.bSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing;
	chbSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated->Checked = tmpSettings.frmMain.bSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated;
	cbDialogInfoPreferredState->ItemIndex = tmpSettings.frmMain.dialogInfoPreferredState;
	chbSpeedDialUseGrid->Checked = tmpSettings.frmSpeedDial.useGrid;
	edSpeedDialGridSize->Text = tmpSettings.frmSpeedDial.gridSize;
	chbSpeedDialSaveAllSettings->Checked = tmpSettings.frmSpeedDial.saveAllSettings;
	chbSpeedDialDragApplicationWithButtonContainer->Checked = tmpSettings.frmSpeedDial.dragApplicationWithButtonContainer;

	chbKioskMode->Checked = tmpSettings.frmMain.bKioskMode;
	chbHideSpeedDialToggleButton->Checked = tmpSettings.frmMain.bHideSpeedDialToggleButton;
	chbHideMouseCursor->Checked = tmpSettings.frmMain.bHideMouseCursor;

	chbHideStatusBar->Checked = tmpSettings.frmMain.bHideStatusBar;
	chbHideMainMenu->Checked = tmpSettings.frmMain.bHideMainMenu;
	chbFrmMainBorderless->Checked = tmpSettings.frmMain.bBorderless;
	chbHideDialpad->Checked = tmpSettings.frmMain.bHideDialpad;

	chbFrmMainShowWhenAnsweringCall->Checked = tmpSettings.frmMain.bShowWhenAnsweringCall;
	chbFrmMainShowWhenMakingCall->Checked = tmpSettings.frmMain.bShowWhenMakingCall;

	chbXBtnMinimize->Checked = tmpSettings.frmMain.bXBtnMinimize;
	chbRestoreMainWindowOnIncomingCall->Checked = tmpSettings.frmMain.bRestoreOnIncomingCall;
    chbSingleInstance->Checked = tmpSettings.frmMain.bSingleInstance;

	chbLogToFile->Checked = tmpSettings.Logging.bLogToFile;
	chbLogFlush->Checked = tmpSettings.Logging.bFlush;
	chbLogMessages->Checked = tmpSettings.uaConf.logMessages;
	chbLogMessagesOnlyFirstLines->Checked = tmpSettings.uaConf.logMessagesOnlyFirstLine;
	chbLogAubuf->Checked = tmpSettings.uaConf.logAubuf;
	cmbMaxUiLogLines->ItemIndex = -1;
	for (int i=0; i<cmbMaxUiLogLines->Items->Count; i++)
	{
		if ((unsigned int)StrToInt(cmbMaxUiLogLines->Items->Strings[i]) >= tmpSettings.Logging.iMaxUiLogLines)
		{
			cmbMaxUiLogLines->ItemIndex = i;
			break;
		}
	}
	if (cmbMaxUiLogLines->ItemIndex == -1)
	{
		cmbMaxUiLogLines->ItemHeight = cmbMaxUiLogLines->Items->Count - 1;
	}
	cbLogMaxFileSize->Text = tmpSettings.Logging.iMaxFileSize;
	cbLogRotate->ItemIndex = tmpSettings.Logging.iLogRotate;
	ShowFonts();

	chbUserOnlyClip->Checked = tmpSettings.Display.bUserOnlyClip;
	chbDecodeUtfDisplayToAnsi->Checked = tmpSettings.Display.bDecodeUtfDisplayToAnsi;
	chbUsePAssertedIdentity->Checked = tmpSettings.Display.bUsePAssertedIdentity;
	edMainIconFile->Text = tmpSettings.frmMain.mainIcon;
	edTrayNotificationImage->Text = tmpSettings.frmMain.trayNotificationImage;
	edBmpBtnBackspace->Text = tmpSettings.frmMain.bitmaps.bmpBtnBackspace;
	edBmpBtnConsoleShow->Text = tmpSettings.frmMain.bitmaps.bmpConsoleShow;
	edBmpBtnConsoleHide->Text = tmpSettings.frmMain.bitmaps.bmpConsoleHide;
	edBmpBtnResetMicVolume->Text = tmpSettings.frmMain.bitmaps.bmpBtnResetMicVolume;
	edBmpBtnResetSpeakerVolume->Text = tmpSettings.frmMain.bitmaps.bmpBtnResetSpeakerVolume;
	edBrandingAppUrl->Text = tmpSettings.branding.appUrl;

	chbNoBeepOnEnterKey->Checked = tmpSettings.frmMain.bNoBeepOnEnterKey;
	chbHideSettings->Checked = tmpSettings.frmMain.bHideSettings;
	chbHideViewMenu->Checked = tmpSettings.frmMain.bHideView;
	chbHideToolsMenu->Checked = tmpSettings.frmMain.bHideTools;
	chbHideHelpMenu->Checked = tmpSettings.frmMain.bHideHelp;

	chbCustomUserAgent->Checked = tmpSettings.uaConf.customUserAgent;
	edUserAgent->Text = tmpSettings.uaConf.userAgent.c_str();

	chAddFilterWMCopyData->Checked = tmpSettings.Integration.bAddFilterWMCopyData;
	edProtocolName->Text = tmpSettings.Integration.asProtocol;
	chbDoNotUseSipPrefixForDirectIpCalls->Checked = tmpSettings.Integration.bDoNotUseSipPrefixForDirectIpCalls;
	chbDoNotPassParametersToPreviousInstance->Checked = tmpSettings.Integration.bDoNotPassParametersToPreviousInstance;
	chbAcceptCommandLineScript->Checked = tmpSettings.Integration.bAcceptCommandLineScript;

	cbSipAccessUrlMode->ItemIndex = tmpSettings.SipAccessUrl.accessMode;

	{
		lboxAudioCodecsAvailable->Clear();
		std::vector<AnsiString> codecs;
		if (Ua::Instance().GetAudioCodecList(codecs) == 0)
		{
			for (unsigned int i=0; i<codecs.size(); i++)
			{
				lboxAudioCodecsAvailable->Items->Add(codecs[i]);
			}
		}
		lboxAudioCodecsEnabled->Clear();
	}

	{
		lboxVideoCodecsAvailable->Clear();
		std::vector<AnsiString> codecs;
		if (Ua::Instance().GetVideoCodecList(codecs) == 0)
		{
			for (unsigned int i=0; i<codecs.size(); i++)
			{
				lboxVideoCodecsAvailable->Items->Add(codecs[i]);
			}
		}
		lboxVideoCodecsEnabled->Clear();
	}

	if (tmpSettings.uaConf.accounts.size() > 0)
	{
        /** \todo multiple accounts */
		for (unsigned int i=0; i<tmpSettings.uaConf.accounts[0].audio_codecs.size(); i++)
		{
			AnsiString name = tmpSettings.uaConf.accounts[0].audio_codecs[i].c_str();
			for (int codec = 0; codec < lboxAudioCodecsAvailable->Items->Count; codec++)
			{
				if (lboxAudioCodecsAvailable->Items->Strings[codec] == name)
				{
					lboxAudioCodecsAvailable->Items->Delete(codec);
					lboxAudioCodecsEnabled->Items->Add(name);
					break;
				}
			}
		}
		for (unsigned int i=0; i<tmpSettings.uaConf.accounts[0].video_codecs.size(); i++)
		{
			AnsiString name = tmpSettings.uaConf.accounts[0].video_codecs[i].c_str();
			for (int codec = 0; codec < lboxVideoCodecsAvailable->Items->Count; codec++)
			{
				if (lboxVideoCodecsAvailable->Items->Strings[codec] == name)
				{
					lboxVideoCodecsAvailable->Items->Delete(codec);
					lboxVideoCodecsEnabled->Items->Add(name);
					break;
				}
			}
		}
	}

	chbRecordingEnabled->Checked = tmpSettings.uaConf.recording.enabled;
	cbRecDirType->ItemIndex = tmpSettings.uaConf.recording.recDir;
	cbRecDirTypeChange(NULL);
	this->edCustomRecDir->Text = tmpSettings.uaConf.recording.customRecDir.c_str();
	cbRecordingChannels->ItemIndex = tmpSettings.uaConf.recording.channels - 1;
	cbRecordingChannelsChange(NULL);
	if (tmpSettings.uaConf.recording.side < cbRecordedSide->Items->Count)
	{
		cbRecordedSide->ItemIndex = tmpSettings.uaConf.recording.side;
	}
	if (tmpSettings.uaConf.recording.fileFormat < cbRecordingFileFormat->Items->Count)
	{
    	cbRecordingFileFormat->ItemIndex = tmpSettings.uaConf.recording.fileFormat;
	}
	edRecordingBitrate->Text = tmpSettings.uaConf.recording.bitrate;
	cbRecordingStart->ItemIndex = tmpSettings.uaConf.recording.recStart;
	chbRecordingNoNumberB64Encoding->Checked = tmpSettings.uaConf.recording.noNumberB64Encoding;

	chbAutoAnswer->Checked = tmpSettings.uaConf.autoAnswer;
	edAutoAnswerCode->Text = tmpSettings.uaConf.autoAnswerCode;
	edAutoAnswerReason->Text = tmpSettings.uaConf.autoAnswerReason.c_str();
	edAutoAnswerDelayMin->Text = tmpSettings.uaConf.autoAnswerDelayMin;
	edAutoAnswerDelayMax->Text = tmpSettings.uaConf.autoAnswerDelayMax;

	chbAutoAnswerCallInfo->Checked = tmpSettings.uaConf.autoAnswerCallInfo;
	edAutoAnswerCallInfoDelayMin->Text = tmpSettings.uaConf.autoAnswerCallInfoDelayMin;

	chbAnswerOnEventTalk->Checked = tmpSettings.uaConf.answerOnEventTalk;

	chbHandleOodRefer->Checked = tmpSettings.uaConf.handleOodRefer;

	memoCallsExtraHeaderLines->Text = tmpSettings.Calls.extraHeaderLines;
	chbDisconnectCallOnAudioError->Checked = tmpSettings.Calls.bDisconnectCallOnAudioError;

	edWebRtcAecMsInSndCardBuf->Text = tmpSettings.uaConf.webrtcAec.msInSndCardBuf;
	edWebRtcAecSkew->Text = tmpSettings.uaConf.webrtcAec.skew;

	chbAudioPreprocessingTxEnabled->Checked = tmpSettings.uaConf.audioPreprocTx.enabled;
	chbAudioPreprocessingTxDenoiseEnabled->Checked = tmpSettings.uaConf.audioPreprocTx.denoiseEnabled;
	chbAudioPreprocessingTxAgcEnabled->Checked = tmpSettings.uaConf.audioPreprocTx.agcEnabled;
	chbAudioPreprocessingTxVadEnabled->Checked = tmpSettings.uaConf.audioPreprocTx.vadEnabled;
	chbAudioPreprocessingTxDereverbEnabled->Checked = tmpSettings.uaConf.audioPreprocTx.dereverbEnabled;
	edAudioPreprocessingTxAgcLevel->Text = tmpSettings.uaConf.audioPreprocTx.agcLevel;

	{
		const UaConf::AudioGate &gate = tmpSettings.uaConf.audioGateTx;
		chbAudioTxGateEnabled->Checked = gate.enabled;
		edAudioTxGateCloseThreshold->Text = gate.closeThreshold;
		edAudioTxGateHoldMs->Text = gate.holdMs;
		edAudioTxGateAttackMs->Text = gate.attackMs;
		edAudioTxGateReleaseMs->Text = gate.releaseMs;
	}

	chbAudioRxAgcEnabled->Checked = tmpSettings.uaConf.audioAgcRx.enabled;
	edAudioRxAgcTarget->Text = tmpSettings.uaConf.audioAgcRx.target;
	edAudioRxAgcMaxGain->Text = tmpSettings.uaConf.audioAgcRx.maxGain;
	edAudioRxAgcAttackRate->Text = tmpSettings.uaConf.audioAgcRx.attackRate;
	edAudioRxAgcReleaseRate->Text = tmpSettings.uaConf.audioAgcRx.releaseRate;

	edPortaudioInSuggestedLatency->Text = tmpSettings.uaConf.audioPortaudio.inSuggestedLatency;
	edPortaudioOutSuggestedLatency->Text = tmpSettings.uaConf.audioPortaudio.outSuggestedLatency;
	chbStartAudioSourceAtCallStart->Checked = tmpSettings.uaConf.startAudioSourceAtCallStart;

	AudioPreprocessingUpdate();

	edRingDefault->Text = tmpSettings.Ring.defaultRing;
	edRingBellcoreDr1->Text = tmpSettings.Ring.bellcore[0];
	edRingBellcoreDr2->Text = tmpSettings.Ring.bellcore[1];
	edRingBellcoreDr3->Text = tmpSettings.Ring.bellcore[2];
	edRingBellcoreDr4->Text = tmpSettings.Ring.bellcore[3];
	edRingBellcoreDr5->Text = tmpSettings.Ring.bellcore[4];
	edRingBellcoreDr6->Text = tmpSettings.Ring.bellcore[5];
	edRingBellcoreDr7->Text = tmpSettings.Ring.bellcore[6];
	edRingBellcoreDr8->Text = tmpSettings.Ring.bellcore[7];

	chbLoopRingWithoutSilence->Checked = tmpSettings.uaConf.loopRingWithoutSilence;

	edMessagesRing->Text = tmpSettings.Messages.ring;

	edTlsCertificate->Text = tmpSettings.uaConf.tls.certificate.c_str();
	edTlsCaFile->Text = tmpSettings.uaConf.tls.caFile.c_str();
	chbTlsUseWindowsRootCaStore->Checked = tmpSettings.uaConf.tls.useWindowsRootCaStore;
	chbTlsVerifyServerCertificate->Checked = tmpSettings.uaConf.tls.verifyServer;

	chbContactPopupShowOnIncoming->Checked = tmpSettings.frmContactPopup.showOnIncoming;
	chbContactPopupShowOnOutgoing->Checked = tmpSettings.frmContactPopup.showOnOutgoing;
	chbContactOpenFileOnIncoming->Checked = tmpSettings.Contacts.openFileOnIncoming;
	chbContactOpenFileOnOutgoing->Checked = tmpSettings.Contacts.openFileOnOutgoing;
	chbContactStoreNoteInSeparateFile->Checked = tmpSettings.Contacts.storeNoteInSeparateFile;

	edContactHttpQuery->Text = tmpSettings.HttpQuery.url;
	cbHttpQueryOpenMode->ItemIndex = tmpSettings.HttpQuery.openMode;

	edContactsFile->Text = tmpSettings.Contacts.file;
	chbContactsCheckIfFileUpdated->Checked = tmpSettings.Contacts.checkIfFileUpdated;
	edContactsCheckIfFileUpdated->Text = tmpSettings.Contacts.checkIfFileUpdatedPeriod;
	chbContactFilterUsingNote->Checked = tmpSettings.Contacts.filterUsingNote;

	chbHistoryNoStoreToFile->Checked = tmpSettings.history.noStoreToFile;
	chbHistoryUsePaiForDisplayIfAvailable->Checked = tmpSettings.history.usePaiForDisplayIfAvailable;
	chbHistoryUsePaiForDialIfAvailable->Checked = tmpSettings.history.usePaiForDialIfAvailable;
	chbHistoryShowHint->Checked = tmpSettings.history.showHint;
	chbHistoryFormatCallDurationAsHourMinSec->Checked = tmpSettings.history.formatCallDurationAsHourMinSec;
	chbHistoryShowCodecNameInHint->Checked = tmpSettings.history.showCodecNameInHint;
	chbHistoryShowLastReplyCodeInHint->Checked = tmpSettings.history.showLastCodeInHint;
	chbHistoryShowLastReplyLineInHint->Checked = tmpSettings.history.showLastReplyLineInHint;
	chbHistoryShowRecordFileInHint->Checked = tmpSettings.history.showRecordFileInHint;

    edScriptOnMakeCallFile->Text = tmpSettings.Scripts.onMakeCall;
	edScriptOnCallStateChangeFile->Text = tmpSettings.Scripts.onCallState;
	edScriptOnRecorderStateFile->Text = tmpSettings.Scripts.onRecorderState;
	edScriptOnEncryptionStateFile->Text = tmpSettings.Scripts.onEncryptionState;
	edScriptOnStreamingStateChangeFile->Text = tmpSettings.Scripts.onStreamingState;
	edScriptOnRegistrationStateChangeFile->Text = tmpSettings.Scripts.onRegistrationState;
	edScriptOnTimerFile->Text = tmpSettings.Scripts.onTimer;
	edScriptTimer->Text = tmpSettings.Scripts.timer;
	edScriptOnTimer2File->Text = tmpSettings.Scripts.onTimer2;
	edScriptTimer2->Text = tmpSettings.Scripts.timer2;
	edScriptOnStartupFile->Text = tmpSettings.Scripts.onStartup;
	edScriptOnDialogInfoFile->Text = tmpSettings.Scripts.onDialogInfo;
	edScriptOnDialFile->Text = tmpSettings.Scripts.onDial;
	edScriptOnProgrammableButtonFile->Text = tmpSettings.Scripts.onProgrammableButton;
	edScriptOnProgrammableButtonMouseUpDownFile->Text = tmpSettings.Scripts.onProgrammableButtonMouseUpDown;
	edScriptOnAudioErrorFile->Text = tmpSettings.Scripts.onAudioDeviceError;
	edScriptOnCustomRequestReplyFile->Text = tmpSettings.Scripts.onCustomRequestReply;
	edScriptOnContactNoteOpenFile->Text = tmpSettings.Scripts.onContactNoteOpen;

	chbShowSettingsIfAnyAccountSettingsIsHidden->Checked = tmpSettings.frmMain.bShowSettingsIfAccountSettingIsHidden;

	chbNoTaskbarButtonRestore->Checked = tmpSettings.frmMain.bNoTaskbarButtonRestore;
	chbNoTrayIcon->Checked = tmpSettings.frmMain.bNoTrayIcon;

	if (tmpSettings.frmMain.layout >= 0 && tmpSettings.frmMain.layout < cbFrmMainLayout->Items->Count)
	{
		cbFrmMainLayout->ItemIndex = tmpSettings.frmMain.layout;
	}
	if (tmpSettings.frmMain.dialComboboxOrder >= 0 && tmpSettings.frmMain.dialComboboxOrder < cbDialComboboxOrder->Items->Count)
	{
		cbDialComboboxOrder->ItemIndex = tmpSettings.frmMain.dialComboboxOrder;
	}


	frmHotkeys->SetCfg(&tmpSettings.hotKeyConf);

	frmPhones->SetCfg(&tmpSettings.phoneConf);

	frmUaConfOpus->SetCfg(&tmpSettings.uaConf.opus);

	frmDialpadConf->SetCfg(&tmpSettings.dialpad);

	frmVideoConf->SetCfg(&tmpSettings.video, &tmpSettings.uaConf);

	if (tmpSettings.locking.hiddenSettingsPages != previousHiddenSettingsPages)
	{
		CreatePages();
		previousHiddenSettingsPages = tmpSettings.locking.hiddenSettingsPages;
	}
}

void TfrmSettings::UpdateNetworkInterface(void)
{
	if (cbNetworkInterfaces->ItemIndex == cbNetworkInterfaces->Items->Count - 1)
	{
		lblNetworkInterfaceInfo->Visible = true;
		edIfName->Visible = true;
	}
	else
	{
		lblNetworkInterfaceInfo->Visible = false;
		edIfName->Visible = false;
	}
}

//---------------------------------------------------------------------------
void __fastcall TfrmSettings::btnCancelClick(TObject *Sender)
{
	this->Close();
}
//---------------------------------------------------------------------------
void __fastcall TfrmSettings::btnApplyClick(TObject *Sender)
{
	tmpSettings.gui.scalingPct = StrToIntDef(edGuiScaling->Text, 100);
	if (tmpSettings.gui.scalingPct < tmpSettings.gui.SCALING_MIN ||
		tmpSettings.gui.scalingPct > tmpSettings.gui.SCALING_MAX)
	{
		tmpSettings.gui.scalingPct = 100;
	}

	{
		int tmp;
		tmp = StrToIntDef(edCollapsedWidth->Text, tmpSettings.frmMain.collapsedWidth);
		if (tmp >= Settings::_frmMain::MIN_WIDTH)
			tmpSettings.frmMain.collapsedWidth = tmp;
		tmp = StrToIntDef(edCollapsedHeight->Text, tmpSettings.frmMain.collapsedHeight);
		if (tmp >= Settings::_frmMain::MIN_HEIGHT)
			tmpSettings.frmMain.collapsedHeight = tmp;

		tmp = StrToIntDef(edExpandedWidth->Text, tmpSettings.frmMain.expandedWidth);
		if (tmp >= Settings::_frmMain::MIN_WIDTH)
			tmpSettings.frmMain.expandedWidth = tmp;
		tmp = StrToIntDef(edExpandedHeight->Text, tmpSettings.frmMain.expandedHeight);
		if (tmp >= Settings::_frmMain::MIN_HEIGHT)
			tmpSettings.frmMain.expandedHeight = tmp;

		tmp = StrToIntDef(edExpandingPosLeftOffset->Text, tmpSettings.frmMain.expandingPosLeftOffset);
		tmpSettings.frmMain.expandingPosLeftOffset = tmp;
		tmp = StrToIntDef(edExpandingPosTopOffset->Text, tmpSettings.frmMain.expandingPosTopOffset);
        tmpSettings.frmMain.expandingPosTopOffset = tmp;
	}
	tmpSettings.frmMain.bUseClientAreaSizes = chbFrmMainUseClientAreaSizes->Checked;

	{
		StrToIntDef2(edCallPanelCollapsedLeft->Text, tmpSettings.frmMain.collapsedCallPanelLeft);
		StrToIntDef2(edCallPanelCollapsedTop->Text, tmpSettings.frmMain.collapsedCallPanelTop);
		StrToIntDef2(edCallPanelExpandedLeft->Text, tmpSettings.frmMain.expandedCallPanelLeft);
		StrToIntDef2(edCallPanelExpandedTop->Text, tmpSettings.frmMain.expandedCallPanelTop);

		StrToIntDef2(edMainPanelCollapsedLeft->Text, tmpSettings.frmMain.collapsedMainPanelLeft);
		StrToIntDef2(edMainPanelCollapsedTop->Text, tmpSettings.frmMain.collapsedMainPanelTop);
		StrToIntDef2(edMainPanelExpandedLeft->Text, tmpSettings.frmMain.expandedMainPanelLeft);
		StrToIntDef2(edMainPanelExpandedTop->Text, tmpSettings.frmMain.expandedMainPanelTop);
	}

	if (cbTranslation->ItemIndex == 0)
	{
		tmpSettings.Translation.language = "";
	}
	else
	{
		tmpSettings.Translation.language = cbTranslation->Text;
	}
	tmpSettings.Translation.logMissingKeys = chbTranslationLogMissingKeys->Checked;	

	tmpSettings.frmMain.bAlwaysOnTop = chbAlwaysOnTop->Checked;
	tmpSettings.frmMain.bStartMinimizedToTray = chbStartMinimizedToTray->Checked;
	tmpSettings.frmTrayNotifier.showOnIncoming = chbTrayNotifierShowOnIncoming->Checked;
	tmpSettings.frmTrayNotifier.skipIfMainWindowVisible = chbSkipTrayNotifierIfMainWindowVisible->Checked;
	tmpSettings.frmTrayNotifier.showOnOutgoing = this->chbTrayNotifierShowOnOutgoing->Checked;
	tmpSettings.frmTrayNotifier.backgroundImage = edTrayNotifierBackgroundImage->Text;
	tmpSettings.frmTrayNotifier.scalingPct = StrToIntDef(edTrayNotifierGuiScaling->Text, tmpSettings.frmTrayNotifier.scalingPct);
	if (tmpSettings.frmTrayNotifier.scalingPct < Settings::_frmTrayNotifier::SCALING_MIN ||
		tmpSettings.frmTrayNotifier.scalingPct > Settings::_frmTrayNotifier::SCALING_MAX)
	{
		tmpSettings.frmTrayNotifier.scalingPct = Settings::_frmTrayNotifier::SCALING_DEF;
	}
	tmpSettings.frmTrayNotifier.hideWhenAnsweringCall = chbTrayNotifierHideWhenAnsweringCall->Checked;
	tmpSettings.frmTrayNotifier.hideWhenAnsweringCallAutomatically = chbTrayNotifierHideWhenAnsweringCallAutomatically->Checked;
	tmpSettings.frmTrayNotifier.doNotChangePosition = chbTrayNotifierDoNotChangePosition->Checked;

	tmpSettings.frmMain.bUseCustomApplicationTitle = chbFrmMainUseCustomApplicationTitle->Checked;
	tmpSettings.frmMain.customApplicationTitle = edFrmMainCustomApplicationTitle->Text;
	tmpSettings.frmMain.bUseCustomCaption = chbFrmMainUseCustomCaption->Checked;
	tmpSettings.frmMain.customCaption = edFrmMainCustomCaption->Text;

	tmpSettings.frmMain.bHideCallPanel = chbFrmMainHideCallPanel->Checked;
	tmpSettings.frmMain.bHideMainPanel = chbFrmMainHideMainPanel->Checked;

	tmpSettings.frmMain.bSpeedDialPopupMenu = chbSpeedDialPopupMenu->Checked;
	tmpSettings.frmMain.bSpeedDialIgnorePresenceNote = chbSpeedDialIgnorePresenceNote->Checked;
	tmpSettings.frmMain.bSpeedDialIgnoreDialogInfoRemoteIdentity = chbSpeedDialIgnoreDialogInfoRemoteIdentity->Checked;
	tmpSettings.frmMain.bSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing = chbSpeedDialKeepPreviousDialogInfoRemoteIdentityIfMissing->Checked;
	tmpSettings.frmMain.bSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated = chbSpeedDialIgnoreOrClearDialogInfoRemoteIdentityIfTerminated->Checked;
	tmpSettings.frmMain.dialogInfoPreferredState = static_cast<Settings::_frmMain::DialogInfoPreferredState>(cbDialogInfoPreferredState->ItemIndex);

	tmpSettings.frmSpeedDial.useGrid = chbSpeedDialUseGrid->Checked;
	{
		int tmp = StrToIntDef(edSpeedDialGridSize->Text, tmpSettings.frmSpeedDial.gridSize);
		if (tmp >= Settings::_frmSpeedDial::MIN_GRID_SIZE && tmp <= Settings::_frmSpeedDial::MAX_GRID_SIZE)
			tmpSettings.frmSpeedDial.gridSize = tmp;
	}
	tmpSettings.frmSpeedDial.saveAllSettings = chbSpeedDialSaveAllSettings->Checked;
	tmpSettings.frmSpeedDial.dragApplicationWithButtonContainer = chbSpeedDialDragApplicationWithButtonContainer->Checked;

	tmpSettings.frmMain.bKioskMode = chbKioskMode->Checked;
	tmpSettings.frmMain.bHideSpeedDialToggleButton = chbHideSpeedDialToggleButton->Checked;
	tmpSettings.frmMain.bHideMouseCursor = chbHideMouseCursor->Checked;

	tmpSettings.frmMain.bHideStatusBar = chbHideStatusBar->Checked;
	tmpSettings.frmMain.bHideMainMenu = chbHideMainMenu->Checked;
	tmpSettings.frmMain.bBorderless = chbFrmMainBorderless->Checked;
	tmpSettings.frmMain.bHideDialpad = chbHideDialpad->Checked;

	tmpSettings.frmMain.bShowWhenAnsweringCall = chbFrmMainShowWhenAnsweringCall->Checked;
	tmpSettings.frmMain.bShowWhenMakingCall = chbFrmMainShowWhenMakingCall->Checked;

	tmpSettings.frmMain.bXBtnMinimize = chbXBtnMinimize->Checked;
	tmpSettings.frmMain.bRestoreOnIncomingCall = chbRestoreMainWindowOnIncomingCall->Checked;
	tmpSettings.frmMain.bSingleInstance = chbSingleInstance->Checked;

	tmpSettings.Logging.bLogToFile = chbLogToFile->Checked;
	tmpSettings.Logging.bFlush = chbLogFlush->Checked;
	tmpSettings.Logging.iMaxFileSize = StrToIntDef(cbLogMaxFileSize->Text, tmpSettings.Logging.iMaxFileSize);
	if (tmpSettings.Logging.iMaxFileSize < Settings::_Logging::MIN_MAX_FILE_SIZE || tmpSettings.Logging.iMaxFileSize > Settings::_Logging::MAX_MAX_FILE_SIZE)
	{
		tmpSettings.Logging.iMaxFileSize = Settings::_Logging::DEF_MAX_FILE_SIZE;
	}
	tmpSettings.Logging.iLogRotate = cbLogRotate->ItemIndex;
	tmpSettings.Logging.iMaxUiLogLines = StrToInt(cmbMaxUiLogLines->Text);	
	tmpSettings.uaConf.logMessages = chbLogMessages->Checked;
	tmpSettings.uaConf.logMessagesOnlyFirstLine = chbLogMessagesOnlyFirstLines->Checked;
	tmpSettings.uaConf.logAubuf = chbLogAubuf->Checked;

	tmpSettings.Display.bUserOnlyClip = chbUserOnlyClip->Checked;
	tmpSettings.Display.bDecodeUtfDisplayToAnsi = chbDecodeUtfDisplayToAnsi->Checked;
	tmpSettings.Display.bUsePAssertedIdentity = chbUsePAssertedIdentity->Checked;

	tmpSettings.frmMain.mainIcon = edMainIconFile->Text;
	tmpSettings.frmMain.trayNotificationImage = edTrayNotificationImage->Text;
	tmpSettings.frmMain.bitmaps.bmpBtnBackspace = edBmpBtnBackspace->Text;
	tmpSettings.frmMain.bitmaps.bmpConsoleShow = edBmpBtnConsoleShow->Text;
	tmpSettings.frmMain.bitmaps.bmpConsoleHide = edBmpBtnConsoleHide->Text;
	tmpSettings.frmMain.bitmaps.bmpBtnResetMicVolume = edBmpBtnResetMicVolume->Text;
	tmpSettings.frmMain.bitmaps.bmpBtnResetSpeakerVolume = edBmpBtnResetSpeakerVolume->Text;
	tmpSettings.branding.appUrl = edBrandingAppUrl->Text;

	tmpSettings.frmMain.bNoBeepOnEnterKey = chbNoBeepOnEnterKey->Checked;
	tmpSettings.frmMain.bHideSettings = chbHideSettings->Checked;
	tmpSettings.frmMain.bHideView = chbHideViewMenu->Checked;
	tmpSettings.frmMain.bHideTools = chbHideToolsMenu->Checked;
	tmpSettings.frmMain.bHideHelp = chbHideHelpMenu->Checked;

	tmpSettings.uaConf.customUserAgent = chbCustomUserAgent->Checked;
	tmpSettings.uaConf.userAgent = edUserAgent->Text.c_str();

	tmpSettings.Integration.bAddFilterWMCopyData = chAddFilterWMCopyData->Checked;
	tmpSettings.Integration.asProtocol = edProtocolName->Text;
	tmpSettings.Integration.bDoNotUseSipPrefixForDirectIpCalls = chbDoNotUseSipPrefixForDirectIpCalls->Checked;
	tmpSettings.Integration.bDoNotPassParametersToPreviousInstance = chbDoNotPassParametersToPreviousInstance->Checked;
	tmpSettings.Integration.bAcceptCommandLineScript = chbAcceptCommandLineScript->Checked;

	tmpSettings.SipAccessUrl.accessMode = static_cast<Settings::_SipAccessUrl::AccessMode>(cbSipAccessUrlMode->ItemIndex);

	tmpSettings.uaConf.audioCfgSrc.mod = AudioModules::GetInputModuleFromCbIndex(cbSoundInputMod->ItemIndex);
	if (cbSoundInputDev->Tag == 0 || cbSoundInputDev->ItemIndex != cbSoundInputDev->Items->Count - 1)
	{
		tmpSettings.uaConf.audioCfgSrc.dev = cbSoundInputDev->Text.c_str();
	}
	tmpSettings.uaConf.audioCfgSrc.wavefile = edSoundInputWave->Text.c_str();

	tmpSettings.uaConf.audioCfgPlay.mod = AudioModules::GetOutputModuleFromCbIndex(cbSoundOutputMod->ItemIndex);
	if (cbSoundOutputDev->Tag == 0 || cbSoundOutputDev->ItemIndex != cbSoundOutputDev->Items->Count - 1)
	{
		tmpSettings.uaConf.audioCfgPlay.dev = cbSoundOutputDev->Text.c_str();
	}

	tmpSettings.uaConf.audioCfgAlert.mod = AudioModules::GetOutputModuleFromCbIndex(cbSoundAlertOutputMod->ItemIndex);
	if (cbSoundAlertOutputDev->Tag == 0 || cbSoundAlertOutputDev->ItemIndex != cbSoundAlertOutputDev->Items->Count - 1)
	{
		tmpSettings.uaConf.audioCfgAlert.dev = cbSoundAlertOutputDev->Text.c_str();
	}

	tmpSettings.uaConf.audioCfgRing.mod = AudioModules::GetOutputModuleFromCbIndex(cbSoundRingOutputMod->ItemIndex);
	if (cbSoundRingOutputDev->Tag == 0 || cbSoundRingOutputDev->ItemIndex != cbSoundRingOutputDev->Items->Count - 1)
	{
		tmpSettings.uaConf.audioCfgRing.dev = cbSoundRingOutputDev->Text.c_str();
	}

	tmpSettings.uaConf.audioCfgPlayIntercom.mod = AudioModules::GetOutputModuleFromCbIndex(cbSoundOutputIntercomMod->ItemIndex);
	if (cbSoundOutputIntercomDev->Tag == 0 || cbSoundOutputIntercomDev->ItemIndex != cbSoundOutputIntercomDev->Items->Count - 1)
	{
		tmpSettings.uaConf.audioCfgPlayIntercom.dev = cbSoundOutputIntercomDev->Text.c_str();
	}


	tmpSettings.uaConf.aec = (UaConf::Aec)cbAec->ItemIndex;

	tmpSettings.uaConf.local = edLocalAddress->Text.c_str();
	if (cbNetworkInterfaces->ItemIndex == cbNetworkInterfaces->Items->Count - 1)
	{
		tmpSettings.uaConf.ifname = edIfName->Text.c_str();
	}
	else
	{
		tmpSettings.uaConf.ifname = networkInterfaces[cbNetworkInterfaces->ItemIndex].name.c_str();
	}
	tmpSettings.uaConf.avt.portMin = StrToIntDef(edRtpPortMin->Text, 0);
	tmpSettings.uaConf.avt.portMax = StrToIntDef(edRtpPortMax->Text, 0);
	if (tmpSettings.uaConf.avt.ValidatePorts())
	{
		ShowMessage("Invalid RTP ports configuration!");
		return;
	}
	tmpSettings.uaConf.avt.jbufDelayMin = StrToIntDef(edJbufDelayMin->Text, 0);
	tmpSettings.uaConf.avt.jbufDelayMax = StrToIntDef(edJbufDelayMax->Text, 0);
	if (tmpSettings.uaConf.avt.ValidateJbufDelay())
	{
		ShowMessage("Invalid jitter buffer delay configuration!");
		return;
	}
	tmpSettings.uaConf.avt.rtpTimeout = StrToIntDef(edRtpTimeout->Text, -1);
	if (tmpSettings.uaConf.avt.ValidateRtpTimeout())
	{
		ShowMessage("Invalid RTP timeout configuration!");
		return;
	}

	tmpSettings.uaConf.messages.replyCode = StrToIntDef(edMessagesReplyCode->Text, tmpSettings.uaConf.messages.replyCode);
	tmpSettings.uaConf.messages.replyReason = edMessagesReplyReason->Text.c_str();
	tmpSettings.uaConf.messages.doNotReply = chbMessagesDoNotReply->Checked;

	if (lboxAudioCodecsAvailable->Items->Count > 0 || lboxAudioCodecsEnabled->Items->Count > 0)	// both lists may be empty if UA failed to initialize
	{
		tmpSettings.uaConf.accounts[0].audio_codecs.clear();
		for (int i=0; i<lboxAudioCodecsEnabled->Items->Count; i++)
		{
			tmpSettings.uaConf.accounts[0].audio_codecs.push_back(lboxAudioCodecsEnabled->Items->Strings[i].c_str());
		}
	}

	if (lboxVideoCodecsAvailable->Items->Count > 0 || lboxVideoCodecsEnabled->Items->Count > 0)	// both lists may be empty if UA failed to initialize
	{
		tmpSettings.uaConf.accounts[0].video_codecs.clear();
		for (int i=0; i<lboxVideoCodecsEnabled->Items->Count; i++)
		{
			tmpSettings.uaConf.accounts[0].video_codecs.push_back(lboxVideoCodecsEnabled->Items->Strings[i].c_str());
		}
	}

	tmpSettings.uaConf.recording.enabled = chbRecordingEnabled->Checked;
	tmpSettings.uaConf.recording.recDir = static_cast<UaConf::RecordingCfg::RecDir>(cbRecDirType->ItemIndex);
	tmpSettings.uaConf.recording.customRecDir = edCustomRecDir->Text.c_str();
	tmpSettings.uaConf.recording.channels = cbRecordingChannels->ItemIndex + 1;
	tmpSettings.uaConf.recording.side = cbRecordedSide->ItemIndex;
	tmpSettings.uaConf.recording.fileFormat = cbRecordingFileFormat->ItemIndex;
	{
		int bitrate = StrToIntDef(edRecordingBitrate->Text, -1);
		if (bitrate >= 0)
		{
			tmpSettings.uaConf.recording.bitrate = bitrate;
		}
	}
	tmpSettings.uaConf.recording.recStart = static_cast<UaConf::RecordingCfg::RecStart>(cbRecordingStart->ItemIndex);
	tmpSettings.uaConf.recording.noNumberB64Encoding = chbRecordingNoNumberB64Encoding->Checked;

   	tmpSettings.uaConf.autoAnswer = chbAutoAnswer->Checked;
	tmpSettings.uaConf.autoAnswerCode = StrToIntDef(edAutoAnswerCode->Text, 200);
	tmpSettings.uaConf.autoAnswerReason = edAutoAnswerReason->Text.c_str();
	tmpSettings.uaConf.autoAnswerDelayMin = StrToIntDef(edAutoAnswerDelayMin->Text, 0);
	tmpSettings.uaConf.autoAnswerDelayMax = StrToIntDef(edAutoAnswerDelayMax->Text, 0);
	if (tmpSettings.uaConf.autoAnswerDelayMin > tmpSettings.uaConf.autoAnswerDelayMax)
	{
		tmpSettings.uaConf.autoAnswerDelayMin = 0;
		tmpSettings.uaConf.autoAnswerDelayMax = 0;		
	}

	tmpSettings.uaConf.autoAnswerCallInfo = chbAutoAnswerCallInfo->Checked;
	int val = StrToIntDef(edAutoAnswerCallInfoDelayMin->Text, 0);
	if (val < 0)
		val = 0;
	tmpSettings.uaConf.autoAnswerCallInfoDelayMin = val;

	tmpSettings.uaConf.answerOnEventTalk = chbAnswerOnEventTalk->Checked;

	tmpSettings.uaConf.handleOodRefer = chbHandleOodRefer->Checked;

	tmpSettings.Calls.extraHeaderLines = memoCallsExtraHeaderLines->Text.Trim();
	tmpSettings.Calls.bDisconnectCallOnAudioError = chbDisconnectCallOnAudioError->Checked;

	tmpSettings.uaConf.webrtcAec.msInSndCardBuf = StrToIntDef(edWebRtcAecMsInSndCardBuf->Text, 120);
	if (tmpSettings.uaConf.webrtcAec.msInSndCardBuf < 0) {
    	tmpSettings.uaConf.webrtcAec.msInSndCardBuf = 120;
	}
	tmpSettings.uaConf.webrtcAec.skew = StrToIntDef(edWebRtcAecSkew->Text, 0);

	tmpSettings.uaConf.audioPreprocTx.enabled = chbAudioPreprocessingTxEnabled->Checked;
	tmpSettings.uaConf.audioPreprocTx.denoiseEnabled = chbAudioPreprocessingTxDenoiseEnabled->Checked;
	tmpSettings.uaConf.audioPreprocTx.agcEnabled = chbAudioPreprocessingTxAgcEnabled->Checked;
	tmpSettings.uaConf.audioPreprocTx.vadEnabled = chbAudioPreprocessingTxVadEnabled->Checked;
	tmpSettings.uaConf.audioPreprocTx.dereverbEnabled = chbAudioPreprocessingTxDereverbEnabled->Checked;
	tmpSettings.uaConf.audioPreprocTx.agcLevel = StrToIntDef(edAudioPreprocessingTxAgcLevel->Text, UaConf::AudioPreproc::AGC_LEVEL_DEFAULT);

	{
		UaConf::AudioGate &gate = tmpSettings.uaConf.audioGateTx;
		gate.enabled = chbAudioTxGateEnabled->Checked;
		gate.closeThreshold = StrToIntDef(edAudioTxGateCloseThreshold->Text, gate.closeThreshold);
		gate.holdMs = StrToIntDef(edAudioTxGateHoldMs->Text, gate.holdMs);
		gate.attackMs = StrToIntDef(edAudioTxGateAttackMs->Text, gate.attackMs);
		gate.releaseMs = StrToIntDef(edAudioTxGateReleaseMs->Text, gate.releaseMs);
	}

	tmpSettings.uaConf.audioAgcRx.enabled = chbAudioRxAgcEnabled->Checked;
	tmpSettings.uaConf.audioAgcRx.target = StrToIntDef(edAudioRxAgcTarget->Text, tmpSettings.uaConf.audioAgcRx.target);
	tmpSettings.uaConf.audioAgcRx.maxGain = StrToFloatDef(edAudioRxAgcMaxGain->Text, tmpSettings.uaConf.audioAgcRx.maxGain);
	tmpSettings.uaConf.audioAgcRx.attackRate = StrToFloatDef(edAudioRxAgcAttackRate->Text, tmpSettings.uaConf.audioAgcRx.attackRate);
	tmpSettings.uaConf.audioAgcRx.releaseRate = StrToFloatDef(edAudioRxAgcReleaseRate->Text, tmpSettings.uaConf.audioAgcRx.releaseRate);

	tmpSettings.uaConf.audioPortaudio.inSuggestedLatency = StrToFloatDef(edPortaudioInSuggestedLatency->Text, tmpSettings.uaConf.audioPortaudio.inSuggestedLatency);
	tmpSettings.uaConf.audioPortaudio.outSuggestedLatency = StrToFloatDef(edPortaudioOutSuggestedLatency->Text, tmpSettings.uaConf.audioPortaudio.outSuggestedLatency);
	tmpSettings.uaConf.startAudioSourceAtCallStart = chbStartAudioSourceAtCallStart->Checked;

	tmpSettings.Ring.defaultRing = edRingDefault->Text;
	tmpSettings.Ring.bellcore[0] = edRingBellcoreDr1->Text;
	tmpSettings.Ring.bellcore[1] = edRingBellcoreDr2->Text;
	tmpSettings.Ring.bellcore[2] = edRingBellcoreDr3->Text;
	tmpSettings.Ring.bellcore[3] = edRingBellcoreDr4->Text;
	tmpSettings.Ring.bellcore[4] = edRingBellcoreDr5->Text;
	tmpSettings.Ring.bellcore[5] = edRingBellcoreDr6->Text;
	tmpSettings.Ring.bellcore[6] = edRingBellcoreDr7->Text;
	tmpSettings.Ring.bellcore[7] = edRingBellcoreDr8->Text;

	tmpSettings.uaConf.loopRingWithoutSilence = chbLoopRingWithoutSilence->Checked;

	tmpSettings.Messages.ring = edMessagesRing->Text;

	tmpSettings.uaConf.tls.certificate = edTlsCertificate->Text.c_str();
	tmpSettings.uaConf.tls.caFile = edTlsCaFile->Text.c_str();
	tmpSettings.uaConf.tls.useWindowsRootCaStore = chbTlsUseWindowsRootCaStore->Checked;
	tmpSettings.uaConf.tls.verifyServer = chbTlsVerifyServerCertificate->Checked;

	tmpSettings.frmContactPopup.showOnIncoming = chbContactPopupShowOnIncoming->Checked;
	tmpSettings.frmContactPopup.showOnOutgoing = chbContactPopupShowOnOutgoing->Checked;
	tmpSettings.Contacts.openFileOnIncoming = chbContactOpenFileOnIncoming->Checked;
	tmpSettings.Contacts.openFileOnOutgoing = chbContactOpenFileOnOutgoing->Checked;
    tmpSettings.Contacts.storeNoteInSeparateFile = chbContactStoreNoteInSeparateFile->Checked;

	tmpSettings.HttpQuery.url = edContactHttpQuery->Text;
	tmpSettings.HttpQuery.openMode = static_cast<Settings::_HttpQuery::OpenMode>(cbHttpQueryOpenMode->ItemIndex);

	tmpSettings.Contacts.file = edContactsFile->Text.Trim();
	tmpSettings.Contacts.checkIfFileUpdated = chbContactsCheckIfFileUpdated->Checked;
	tmpSettings.Contacts.checkIfFileUpdatedPeriod = StrToIntDef(edContactsCheckIfFileUpdated->Text, tmpSettings.Contacts.checkIfFileUpdatedPeriod);
	tmpSettings.Contacts.filterUsingNote = chbContactFilterUsingNote->Checked;

	tmpSettings.history.noStoreToFile = chbHistoryNoStoreToFile->Checked;
	tmpSettings.history.usePaiForDisplayIfAvailable = chbHistoryUsePaiForDisplayIfAvailable->Checked;
	tmpSettings.history.usePaiForDialIfAvailable = chbHistoryUsePaiForDialIfAvailable->Checked;
	tmpSettings.history.showHint = chbHistoryShowHint->Checked;
	tmpSettings.history.formatCallDurationAsHourMinSec = chbHistoryFormatCallDurationAsHourMinSec->Checked;
	tmpSettings.history.showCodecNameInHint = chbHistoryShowCodecNameInHint->Checked;
	tmpSettings.history.showLastCodeInHint = chbHistoryShowLastReplyCodeInHint->Checked;
	tmpSettings.history.showLastReplyLineInHint = chbHistoryShowLastReplyLineInHint->Checked;
	tmpSettings.history.showRecordFileInHint = chbHistoryShowRecordFileInHint->Checked;

	tmpSettings.Scripts.onMakeCall = edScriptOnMakeCallFile->Text;
	tmpSettings.Scripts.onCallState = edScriptOnCallStateChangeFile->Text;
	tmpSettings.Scripts.onRecorderState = edScriptOnRecorderStateFile->Text;
	tmpSettings.Scripts.onEncryptionState = edScriptOnEncryptionStateFile->Text;
	tmpSettings.Scripts.onStreamingState = edScriptOnStreamingStateChangeFile->Text;
	tmpSettings.Scripts.onRegistrationState = edScriptOnRegistrationStateChangeFile->Text;
	tmpSettings.Scripts.onTimer = edScriptOnTimerFile->Text;
	tmpSettings.Scripts.timer = StrToIntDef(edScriptTimer->Text, -1);
	if (tmpSettings.Scripts.timer <= 0)
		tmpSettings.Scripts.timer = 1000;
	tmpSettings.Scripts.onTimer2 = edScriptOnTimer2File->Text;
	tmpSettings.Scripts.timer2 = StrToIntDef(edScriptTimer2->Text, -1);
	if (tmpSettings.Scripts.timer2 <= 0)
		tmpSettings.Scripts.timer2 = 1000;
	tmpSettings.Scripts.onStartup = edScriptOnStartupFile->Text;
	tmpSettings.Scripts.onDialogInfo = edScriptOnDialogInfoFile->Text;
	tmpSettings.Scripts.onDial = edScriptOnDialFile->Text;
	tmpSettings.Scripts.onProgrammableButton = edScriptOnProgrammableButtonFile->Text;
	tmpSettings.Scripts.onProgrammableButtonMouseUpDown = edScriptOnProgrammableButtonMouseUpDownFile->Text;
	tmpSettings.Scripts.onAudioDeviceError = edScriptOnAudioErrorFile->Text;
	tmpSettings.Scripts.onCustomRequestReply = edScriptOnCustomRequestReplyFile->Text;
	tmpSettings.Scripts.onContactNoteOpen = edScriptOnContactNoteOpenFile->Text;

	tmpSettings.frmMain.bShowSettingsIfAccountSettingIsHidden = chbShowSettingsIfAnyAccountSettingsIsHidden->Checked;

	tmpSettings.frmMain.bNoTaskbarButtonRestore = chbNoTaskbarButtonRestore->Checked;
	tmpSettings.frmMain.bNoTrayIcon = chbNoTrayIcon->Checked;

	tmpSettings.frmMain.layout = cbFrmMainLayout->ItemIndex;
	tmpSettings.frmMain.dialComboboxOrder = static_cast<Settings::_frmMain::DialComboboxOrder>(cbDialComboboxOrder->ItemIndex);

	frmUaConfOpus->Apply();

	frmDialpadConf->Apply();

	frmVideoConf->Apply();

	{
		tmpSettings.locking.hiddenSettingsPages.clear();
		for (int i=0; i<chbBoxLockingSettingsPages->Count; i++)
		{
			if (chbBoxLockingSettingsPages->Checked[i])
			{
				AnsiString str = chbBoxLockingSettingsPages->Items->Strings[i];
				// try to extract tab name from between last []
				char* lastSqOpen = StrRScan(str.c_str(), '[');	// find last '['
				if (lastSqOpen)
				{
					lastSqOpen++;
					char* lastSqClose = strchr(lastSqOpen, ']');
					if (lastSqClose)
					{
						*lastSqClose = '\0';
						tmpSettings.locking.hiddenSettingsPages.push_back(lastSqOpen);
					}
				}
			}
		}
	}

	appSettings = tmpSettings;
	this->Close();	
}
//---------------------------------------------------------------------------



void __fastcall TfrmSettings::btnAddAccountClick(TObject *Sender)
{
	UaConf::Account acc;
	tmpSettings.uaConf.accounts.push_back(acc);
	int i = tmpSettings.uaConf.accounts.size() - 1;

	//scrbAccounts->AutoScroll = false;

	frmAccount = new TfrmAccount(NULL, i, tmpSettings.uaConf.accounts[i]);
	frmAccount->Parent = scrbAccounts;
	frmAccount->Visible = true;
	// force realign
	frmAccount->Align = alBottom;
	frmAccount->Align = alTop;

	//scrbAccounts->AutoScroll = true;

	// scroll to newly added account position (to the end)
	//scrbAccounts->ScrollBy(0, scrbAccounts->Height);
	frmAccount->SetFocus();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::FormClose(TObject *Sender, TCloseAction &Action)
{
	for (int i=0; i<frmAccountVector.size(); i++)
	{
		delete frmAccountVector[i];
	}
	frmAccountVector.clear();
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::cbSoundInputModChange(TObject *Sender)
{
	AnsiString mod = AudioModules::GetInputModuleFromCbIndex(cbSoundInputMod->ItemIndex);
	if (mod == AudioModules::portaudio ||
		mod == AudioModules::winwave ||
		mod == AudioModules::winwave2)
	{
		btnSelectWaveFile->Visible = false;
		edSoundInputWave->Visible = false;
		cbSoundInputDev->Visible = true;
		lblSoundInputDevice->Visible = true;
		AudioDevicesList::FillComboBox(cbSoundInputDev, mod, false, tmpSettings.uaConf.audioCfgSrc.dev.c_str());
	}
	else if (mod == AudioModules::aufile || mod == AudioModules::aufileMm)
	{
		btnSelectWaveFile->Visible = true;
		edSoundInputWave->Visible = true;
		cbSoundInputDev->Visible = false;
		lblSoundInputDevice->Visible = true;
	}
	else if (mod == AudioModules::nullaudio)
	{
		btnSelectWaveFile->Visible = false;
		edSoundInputWave->Visible = false;
		cbSoundInputDev->Visible = false;
		lblSoundInputDevice->Visible = false;
	}
	else
	{
		assert(!"Unhandled cbSoundInputMod item index!");
	}
}
//---------------------------------------------------------------------------

void TfrmSettings::ChangeSoundOutputMod(TComboBox *target, TLabel *label, int moduleIndex, AnsiString selected)
{
	AnsiString mod = AudioModules::GetOutputModuleFromCbIndex(moduleIndex);
	if (mod == AudioModules::portaudio || mod == AudioModules::winwave || mod == AudioModules::winwave2)
	{
		target->Visible = true;
		label->Visible = true;
		AudioDevicesList::FillComboBox(target, mod, true, selected);
	}
	else if (mod == AudioModules::nullaudio)
	{
		target->Visible = false;
		label->Visible = false;
	}
	else
	{
		assert(!"Unhandled output module index!");
	}
}

void __fastcall TfrmSettings::cbSoundOutputModChange(TObject *Sender)
{
	ChangeSoundOutputMod(cbSoundOutputDev, lblSoundOutputDev, cbSoundOutputMod->ItemIndex, tmpSettings.uaConf.audioCfgPlay.dev.c_str());
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::cbSoundAlertOutputModChange(TObject *Sender)
{
	ChangeSoundOutputMod(cbSoundAlertOutputDev, lblSoundAlertOutputDev, cbSoundAlertOutputMod->ItemIndex, tmpSettings.uaConf.audioCfgAlert.dev.c_str());
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::cbSoundRingOutputModChange(TObject *Sender)
{
	ChangeSoundOutputMod(cbSoundRingOutputDev, lblSoundRingOutputDev, cbSoundRingOutputMod->ItemIndex, tmpSettings.uaConf.audioCfgRing.dev.c_str());
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::cbSoundOutputIntercomModChange(TObject *Sender)
{
	ChangeSoundOutputMod(cbSoundOutputIntercomDev, lblSoundOutputIntercomDev, cbSoundOutputIntercomMod->ItemIndex, tmpSettings.uaConf.audioCfgPlayIntercom.dev.c_str());
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::btnCreateIntegrationProtocolClick(TObject *Sender)
{
	TRegistry* registry = NULL;
	AnsiString proto = edProtocolName->Text.Trim();
	if (proto.Length() == 0)
	{
        MessageBox(this->Handle, "Protocol name can not be empty", this->Caption.c_str(), MB_ICONINFORMATION);
		return;
	}
	try
	{
		registry = new TRegistry();
		registry->RootKey =  HKEY_CLASSES_ROOT;

		if (registry->OpenKey(proto, true))
		{
			AnsiString val, key;
			val.sprintf("URL:%s Protocol", proto.c_str());
			registry->WriteString("", val);
			registry->WriteInteger("BrowserFlags", 8);
			registry->WriteInteger("EditFlags", 2);
			registry->WriteString("URL Protocol", "");
			registry->CloseKey();

			key.sprintf("%s\\DefaultIcon", proto.c_str());
			if (registry->OpenKey(key, true))
			{
				val.sprintf("%s,0", Application->ExeName.c_str());
				registry->WriteString("", val);
				registry->CloseKey();
			}

			key.sprintf("%s\\shell", proto.c_str());
			if (registry->OpenKey(key, true))
			{
				registry->WriteString("", "open");
				registry->CloseKey();
			}

			key.sprintf("%s\\shell\\open\\command", proto.c_str());
			if (registry->OpenKey(key, true))
			{
				val.sprintf("%s /%s=\"%1\"", Application->ExeName.c_str(), Branding::appProto.c_str());
				if (LowerCase(Paths::GetProfileDir()) != LowerCase(ExtractFileDir(Application->ExeName)))
				{
                	val.cat_printf(" /profiledir=\"%s\"", Paths::GetProfileDir().c_str());
				}
				registry->WriteString("", val);
				registry->CloseKey();
			}
			MessageBox(this->Handle, "Registry updated", this->Caption.c_str(), MB_ICONINFORMATION);
		}
	}
	catch (...)
	{
        MessageBox(this->Handle, "Failed to create registry entries", this->Caption.c_str(), MB_ICONINFORMATION);
	}
	if (registry)
	{
    	delete registry;
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::btnRingSelectClick(TObject *Sender)
{
	TButton *btn = dynamic_cast<TButton*>(Sender);
	assert(btn);
	TEdit *edit = NULL;
	AnsiString *str = NULL;
	if (btn == btnRingSelectDefault)
	{
		edit = edRingDefault;
		str = &tmpSettings.Ring.defaultRing;
	}
	else if (btn == btnRingSelectBellcoreDr1)
	{
		edit = edRingBellcoreDr1;
		str = &tmpSettings.Ring.bellcore[0];
	}
	else if (btn == btnRingSelectBellcoreDr2)
	{
		edit = edRingBellcoreDr2;
		str = &tmpSettings.Ring.bellcore[1];
	}
	else if (btn == btnRingSelectBellcoreDr3)
	{
		edit = edRingBellcoreDr3;
		str = &tmpSettings.Ring.bellcore[2];
	}
	else if (btn == btnRingSelectBellcoreDr4)
	{
		edit = edRingBellcoreDr4;
		str = &tmpSettings.Ring.bellcore[3];
	}
	else if (btn == btnRingSelectBellcoreDr5)
	{
		edit = edRingBellcoreDr5;
		str = &tmpSettings.Ring.bellcore[4];
	}
	else if (btn == btnRingSelectBellcoreDr6)
	{
		edit = edRingBellcoreDr6;
		str = &tmpSettings.Ring.bellcore[5];
	}
	else if (btn == btnRingSelectBellcoreDr7)
	{
		edit = edRingBellcoreDr7;
		str = &tmpSettings.Ring.bellcore[6];
	}
	else if (btn == btnRingSelectBellcoreDr8)
	{
		edit = edRingBellcoreDr8;
		str = &tmpSettings.Ring.bellcore[7];
	}
	dlgOpenRing->InitialDir = Paths::GetProfileDir();
	dlgOpenRing->FileName = Paths::GetProfileDir() + "\\" + edit->Text;
	if (dlgOpenRing->Execute())
	{
		if (UpperCase(Paths::GetProfileDir()) != UpperCase(ExtractFileDir(dlgOpenRing->FileName)))
		{
			MessageBox(this->Handle, "Ring file was not updated.\nFor portability ring WAVE files must be placed in application directory.", this->Caption.c_str(), MB_ICONEXCLAMATION);
			return;
		}
		*str = ExtractFileName(dlgOpenRing->FileName);
		edit->Text = *str;
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::FormKeyPress(TObject *Sender, char &Key)
{
	if (Key == VK_ESCAPE)
	{
		Key = 0;
		this->Close();
	}	
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::cbRecDirTypeChange(TObject *Sender)
{
	UaConf::RecordingCfg::RecDir dirType = static_cast<UaConf::RecordingCfg::RecDir>(cbRecDirType->ItemIndex);
	if (dirType == UaConf::RecordingCfg::RecDirRelative)
	{
		lblCustomRecDir->Visible = false;
		edCustomRecDir->Visible = false;
		btnSelectCustomRecDir->Visible = false;
	}
	else
	{
		lblCustomRecDir->Visible = true;
		edCustomRecDir->Visible = true;
		btnSelectCustomRecDir->Visible = true;
	}	
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::cbRecordingChannelsChange(TObject *Sender)
{
	if (cbRecordingChannels->ItemIndex == 0)
	{
		lblRecordedSide->Visible = true;
		cbRecordedSide->Visible = true;
	}
	else
	{
		lblRecordedSide->Visible = false;
		cbRecordedSide->Visible = false;
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::btnSelectCustomRecDirClick(TObject *Sender)
{
    AnsiString asDir;
	if (DirectoryExists(edCustomRecDir->Text))
		asDir = edCustomRecDir->Text;
	if (SelectDirectory("Recordings dir:", "", asDir))
		edCustomRecDir->Text = asDir;
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::btnSelectWaveFileClick(TObject *Sender)
{
	dlgOpenRing->InitialDir = Paths::GetProfileDir();
	if (edSoundInputWave->Text != "")
	{
		dlgOpenRing->FileName = Paths::GetProfileDir() + "\\" + edSoundInputWave->Text;
	}
	if (dlgOpenRing->Execute())
	{
		if (UpperCase(Paths::GetProfileDir()) != UpperCase(ExtractFileDir(dlgOpenRing->FileName)))
		{
			MessageBox(this->Handle, "File was not updated.\nFor portability source WAVE files must be placed in application directory.", this->Caption.c_str(), MB_ICONEXCLAMATION);
			return;
		}
		edSoundInputWave->Text = ExtractFileName(dlgOpenRing->FileName);
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::tvSelectorChange(TObject *Sender, TTreeNode *Node)
{
	if (Node && Node->Selected)
	{
		TTabSheet *tab = reinterpret_cast<TTabSheet*>(Node->Data);
		assert(tab);
		tab->Visible = true;
		if (lastTab)
			lastTab->Visible = false;
		lastTab = tab;
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::btnSelectImageClick(
      TObject *Sender)
{
	TEdit *edit;
	openDialog->Filter = "Bitmaps (*.bmp)|*.bmp|All files|*.*";	// default filter
	if (Sender == btnSelectMainIconFile)
	{
		edit = edMainIconFile;
		openDialog->Filter = "Icon files (*.ico)|*.ico|All files|*.*";
	}
	else if (Sender == btnSelectTrayNotificationImage)
	{
		edit = edTrayNotificationImage;
	}
	else if (Sender == btnTrayNotifierBackgroundImage)
	{
		edit = edTrayNotifierBackgroundImage;
	}
	else if (Sender == btnSelectBmpBtnBackspace)
	{
		edit = edBmpBtnBackspace;
	}
	else if (Sender == btnSelectBmpBtnConsoleShow)
	{
		edit = edBmpBtnConsoleShow;
	}
	else if (Sender == btnSelectBmpBtnConsoleHide)
	{
		edit = edBmpBtnConsoleHide;
	}
	else if (Sender == btnSelectBmpBtnResetMicVolume)
	{
		edit = edBmpBtnResetMicVolume;
	}
	else if (Sender == btnSelectBmpBtnResetSpeakerVolume)
	{
		edit = edBmpBtnResetSpeakerVolume;
	}
	else
	{
        assert(!"Unhandler sender!");
		return;
	}
	AnsiString dir = Paths::GetProfileDir() + "\\img\\";
	openDialog->InitialDir = dir;
	if (FileExists(dir + edit->Text))
		openDialog->FileName = dir + edit->Text;
	else
		openDialog->FileName = "";
	if (openDialog->Execute())
	{
    	edit->Text = ExtractFileName(openDialog->FileName);
	}
}
//---------------------------------------------------------------------------

void TfrmSettings::AudioCodecEnableSelected(void)
{
	for (int i=0; i<lboxAudioCodecsAvailable->Items->Count; i++)
	{
		if (lboxAudioCodecsAvailable->Selected[i])
		{
			lboxAudioCodecsEnabled->Items->Add(lboxAudioCodecsAvailable->Items->Strings[i]);
		}
	}
	lboxAudioCodecsAvailable->DeleteSelected();
}

void TfrmSettings::AudioCodecDisableSelected(void)
{
	for (int i=0; i<lboxAudioCodecsEnabled->Items->Count; i++)
	{
		if (lboxAudioCodecsEnabled->Selected[i])
		{
			lboxAudioCodecsAvailable->Items->Add(lboxAudioCodecsEnabled->Items->Strings[i]);
		}
	}
	lboxAudioCodecsEnabled->DeleteSelected();
}

void TfrmSettings::AudioPreprocessingUpdate(void)
{
	bool state = chbAudioPreprocessingTxEnabled->Checked;
	chbAudioPreprocessingTxDenoiseEnabled->Enabled = state;
	chbAudioPreprocessingTxAgcEnabled->Enabled = state;
	chbAudioPreprocessingTxVadEnabled->Enabled = state;
	chbAudioPreprocessingTxDereverbEnabled->Enabled = state;
	edAudioPreprocessingTxAgcLevel->Enabled = state;
	lblAudioPreprocessingTxAgcLevel->Enabled = state;
}

void __fastcall TfrmSettings::btnAudioCodecEnableClick(TObject *Sender)
{
	AudioCodecEnableSelected();
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::btnAudioCodecDisableClick(TObject *Sender)
{
	AudioCodecDisableSelected();
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::lboxAudioCodecsAvailableDblClick(TObject *Sender)
{
	AudioCodecEnableSelected();
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::lboxAudioCodecsEnabledDblClick(TObject *Sender)
{
	AudioCodecDisableSelected();
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::lboxAudioCodecsEnabledStartDrag(TObject *Sender,
      TDragObject *&DragObject)
{
	audioCodecsEnabledDraggedIndex = lboxAudioCodecsEnabled->ItemIndex;	
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::lboxAudioCodecsEnabledDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
	Accept = true;	
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::lboxAudioCodecsEnabledDragDrop(TObject *Sender,
      TObject *Source, int X, int Y)
{
	TListBox *lbox = lboxAudioCodecsEnabled;
	int dropIndex = lbox->ItemAtPos(Point(X,Y), false);
	if (dropIndex >= lbox->Items->Count)
	{
    	dropIndex--;
	}
	if (dropIndex >= 0)
	{
		lbox->Items->Move(audioCodecsEnabledDraggedIndex, dropIndex);
		lbox->ItemIndex = dropIndex;
    }
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::chbAudioPreprocessingTxEnabledClick(
      TObject *Sender)
{
	AudioPreprocessingUpdate();
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::btnSelectedScriptClick(
      TObject *Sender)
{
	TEdit *edit = NULL;
	if (Sender == btnSelectedScriptOnMakeCallChange)
	{
        edit = edScriptOnMakeCallFile;
    }
	else if (Sender == btnSelectedScriptOnCallStateChange)
	{
		edit = edScriptOnCallStateChangeFile;
	}
	else if (Sender == btnSelectedScriptOnRecorderStateChange)
	{
		edit = edScriptOnRecorderStateFile;
	}
	else if (Sender == btnSelectedScriptOnEncryptionStateChange)
	{
		edit = edScriptOnEncryptionStateFile;
	}
	else if (Sender == btnSelectedScriptOnStreamingStateChange)
	{
		edit = edScriptOnStreamingStateChangeFile;
	}
	else if (Sender == btnSelectedScriptOnRegistrationStateChange)
	{
		edit = edScriptOnRegistrationStateChangeFile;
    }
	else if (Sender == btnSelectedScriptOnTimerChange)
	{
		edit = edScriptOnTimerFile;
	}
	else if (Sender == btnSelectedScriptOnTimer2Change)
	{
    	edit = edScriptOnTimer2File;
	}
	else if (Sender == btnSelectedScriptOnStartupChange)
	{
		edit = edScriptOnStartupFile;
	}
	else if (Sender == btnSelectedScriptOnDialogInfoChange)
	{
        edit = edScriptOnDialogInfoFile;
	}
	else if (Sender == btnSelectedScriptOnDialChange)
	{
        edit = edScriptOnDialFile;
	}
	else if (Sender == btnSelectedScriptOnProgrammableButtonChange)
	{
    	edit = edScriptOnProgrammableButtonFile;
	}
	else if (Sender == btnSelectedScriptOnProgrammableButtonMouseUpDownChange)
	{
		edit = edScriptOnProgrammableButtonMouseUpDownFile;
	}
	else if (Sender == btnSelectedScriptOnAudioErrorChange)
	{
    	edit = edScriptOnAudioErrorFile;
	}
	else if (Sender == btnSelectedScriptOnCustomRequestReplyChange)
	{
		edit = edScriptOnCustomRequestReplyFile;
	}
	else if (Sender == btnSelectedScriptOnContactNoteOpenChange)
	{
    	edit = edScriptOnContactNoteOpenFile;
	}
	else
	{
		assert(0);
		return;
    }
	AnsiString dir = Paths::GetProfileDir() + "\\scripts";
	ForceDirectories(dir);
	openDialog->InitialDir = dir;
	openDialog->Filter = "Lua files (*.lua)|*.lua|All files|*.*";
	AnsiString fileName = dir + "\\" + edit->Text;
	if (FileExists(fileName))
		openDialog->FileName = fileName;
	else
		openDialog->FileName = "";
	if (openDialog->Execute())
	{
		if (UpperCase(dir) != UpperCase(ExtractFileDir(openDialog->FileName)))
		{
			MessageBox(this->Handle, "Entry was not updated.\nFor portability script files must be placed in \"script\" subdirectory.", this->Caption.c_str(), MB_ICONEXCLAMATION);
			return;
		}
		edit->Text = ExtractFileName(openDialog->FileName);
	}
}
//---------------------------------------------------------------------------


void __fastcall TfrmSettings::btnSelectedScriptEditClick(
      TObject *Sender)
{
	TEdit *edit = NULL;
	AnsiString eventName;
	if (Sender == btnSelectedScriptOnMakeCallEdit)
	{
		edit = edScriptOnMakeCallFile;
		eventName = "on_make_call";
	}
	else if (Sender == btnSelectedScriptOnCallStateEdit)
	{
		edit = edScriptOnCallStateChangeFile;
		eventName = "on_call_state";
	}
	else if (Sender == btnSelectedScriptOnRecorderStateEdit)
	{
		edit = edScriptOnRecorderStateFile;
		eventName = "on_recorder_state";
	}
	else if (Sender == btnSelectedScriptOnEncryptionStateEdit)
	{
		edit = edScriptOnEncryptionStateFile;
		eventName = "on_encryption_state";
	}
	else if (Sender == btnSelectedScriptOnStreamingStateEdit)
	{
		edit = edScriptOnStreamingStateChangeFile;
		eventName = "on_streaming_state";
	}
	else if (Sender == btnSelectedScriptOnRegistrationStateEdit)
	{
		edit = edScriptOnRegistrationStateChangeFile;
		eventName = "on_registration_state";
	}
	else if (Sender == btnSelectedScriptOnTimerEdit)
	{
		edit = edScriptOnTimerFile;
		eventName = "on_timer";
	}
	else if (Sender == btnSelectedScriptOnTimer2Edit)
	{
		edit = edScriptOnTimer2File;
		eventName = "on_timer2";
	}
	else if (Sender == btnSelectedScriptOnStartupEdit)
	{
		edit = edScriptOnStartupFile;
		eventName = "on_startup";
	}
	else if (Sender == btnSelectedScriptOnDialogInfoEdit)
	{
		edit = edScriptOnDialogInfoFile;
		eventName = "on_dialog_info";
	}
	else if (Sender == btnSelectedScriptOnDialEdit)
	{
		edit = edScriptOnDialFile;
		eventName = "on_dial";
	}
	else if (Sender == btnSelectedScriptOnProgrammableButtonEdit)
	{
		edit = edScriptOnProgrammableButtonFile;
		eventName = "on_programmable_button";
	}
	else if (Sender == btnSelectedScriptOnProgrammableButtonMouseUpDownEdit)
	{
		edit = edScriptOnProgrammableButtonMouseUpDownFile;
		eventName = "on_programmable_button_mouse_up_down";
	}
	else if (Sender == btnSelectedScriptOnAudioErrorEdit)
	{
		edit = edScriptOnAudioErrorFile;
		eventName = "on_audio_error";
	}
	else if (Sender == btnSelectedScriptOnCustomRequestReplyEdit)
	{
		edit = edScriptOnCustomRequestReplyFile;
		eventName = "on_custom_request_reply";
	}
	else if (Sender == btnSelectedScriptOnContactNoteOpenEdit)
	{
		edit = edScriptOnContactNoteOpenFile;
		eventName = "on_contact_note_open";
	}
	else
	{
		assert(0);
		return;
	}
	AnsiString dir = Paths::GetProfileDir() + "\\scripts";
	ForceDirectories(dir);
	AnsiString file = dir + "\\";
	if (edit->Text != "")
	{
		file += edit->Text;
	}
	else
	{
		AnsiString name = eventName + "_" + FormatDateTime("yyyymmdd_hhnnss_zzz", Now()) + ".lua";
		file += name;
		edit->Text = name;
	}
	if (!FileExists(file))
	{
		FILE *fp = fopen(file.c_str(), "wb");
		if (fp)
		{
			fclose(fp);
		}
		else
		{
			MessageBox(this->Handle, "Could not create file in \"script\" subdirectory.", this->Caption.c_str(), MB_ICONEXCLAMATION);
			return;
		}
	}

	TfrmLuaScript *frmLuaScript = new TfrmLuaScript(NULL);
	frmLuaScript->Show();
	frmLuaScript->OpenFile(file);
}
//---------------------------------------------------------------------------



void __fastcall TfrmSettings::btnLoggingConsoleFontSelectClick(TObject *Sender)
{
	TButton *btn = dynamic_cast<TButton*>(Sender);
	assert(btn);
	struct Font *font = &tmpSettings.Logging.consoleFont;
	fontDialog->Font->Name = font->name;
	fontDialog->Font->Size = font->size;
	fontDialog->Font->Style = font->style;
	if (fontDialog->Execute())
	{
	#if 0
		if (btn->Tag == 2 || btn->Tag == 4)	// Scintilla: own styling
		{
			fontDialog->Font->Style = TFontStyles();
			//fontDialog->Options << fdNoStyleSel;
		}
	#endif
		font->name = fontDialog->Font->Name;
		font->size = fontDialog->Font->Size;
		font->style = fontDialog->Font->Style;
		ShowFonts();
	}
}
//---------------------------------------------------------------------------

void TfrmSettings::ShowFonts(void)
{
	TEdit *ed;
	struct Font *font;

	ed = this->edLoggingConsoleFont;
	font = &tmpSettings.Logging.consoleFont;

	ed->Font->Name = font->name;
	ed->Font->Size = font->size;
	ed->Font->Style = font->style;
}

void __fastcall TfrmSettings::btnOpenRecordingFolderClick(TObject *Sender)
{
	AnsiString dir;
	if (appSettings.uaConf.recording.recDir == UaConf::RecordingCfg::RecDirRelative)
	{
		dir = Paths::GetProfileDir() + "\\recordings\\";
	}
	else
	{
		dir = appSettings.uaConf.recording.customRecDir.c_str();
		if (dir[dir.Length()] != '\\')
			dir += "\\";
	}
	ShellExecute(NULL, "explore", dir.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::cbNetworkInterfacesChange(TObject *Sender)
{
	UpdateNetworkInterface();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::btnSelectContactsFileClick(TObject *Sender)
{
	TEdit *edit = edContactsFile;
	openDialog->Filter = "JSON files (*.json)|*.json|All files|*.*";
	AnsiString fileName = edit->Text;
	if (FileExists(fileName))
		openDialog->FileName = fileName;
	else
		openDialog->FileName = "";
	if (openDialog->Execute())
	{
		edit->Text = openDialog->FileName;
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::btnMessagesSelectRingClick(TObject *Sender)
{
	TButton *btn = dynamic_cast<TButton*>(Sender);
	assert(btn);
	TEdit *edit = NULL;
	AnsiString *str = NULL;
	if (btn == btnMessagesSelectRing)
	{
		edit = edMessagesRing;
		str = &tmpSettings.Messages.ring;
	}
	else
	{
		assert(!"Unhandled Sender!");
		return;
	}
	dlgOpenRing->InitialDir = Paths::GetProfileDir();
	dlgOpenRing->FileName = Paths::GetProfileDir() + "\\" + edit->Text;
	if (dlgOpenRing->Execute())
	{
		if (UpperCase(Paths::GetProfileDir()) != UpperCase(ExtractFileDir(dlgOpenRing->FileName)))
		{
			MessageBox(this->Handle, "Audio file was not updated.\nFor portability ring WAVE files must be placed in application directory.", this->Caption.c_str(), MB_ICONEXCLAMATION);
			return;
		}
		*str = ExtractFileName(dlgOpenRing->FileName);
		edit->Text = *str;
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::btnSelectTlsClick(TObject *Sender)
{
	TEdit *edit;
	openDialog->Filter = "PEM file (*.pem)|*.pem|All files|*.*";
	if (Sender == btnSelectTlsCertificate)
	{
		edit = edTlsCertificate;
	}
	else if (Sender == btnSelectTlsCaFile)
	{
		edit = edTlsCaFile;
	}
	else
	{
		assert(!"Unhandler sender!");
		return;
	}
	AnsiString dir = Paths::GetProfileDir() + "\\certificates\\";
	if (ForceDirectories(dir) == false)
	{
		ShowMessage("Failed to create directory for certificates!");
	}
	openDialog->InitialDir = dir;
	if (FileExists(dir + edit->Text))
		openDialog->FileName = dir + edit->Text;
	else
		openDialog->FileName = "";
	if (openDialog->Execute())
	{
    	edit->Text = ExtractFileName(openDialog->FileName);
	}	
}
//---------------------------------------------------------------------------


void __fastcall TfrmSettings::lboxVideoCodecsAvailableDblClick(TObject *Sender)
{
	VideoCodecEnableSelected();
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::lboxVideoCodecsEnabledDblClick(TObject *Sender)
{
	VideoCodecDisableSelected();
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::btnVideoCodecEnableClick(TObject *Sender)
{
	VideoCodecEnableSelected();	
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::btnVideoCodecDisableClick(TObject *Sender)
{
	VideoCodecDisableSelected();
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::lboxVideoCodecsEnabledStartDrag(TObject *Sender,
      TDragObject *&DragObject)
{
	videoCodecsEnabledDraggedIndex = lboxVideoCodecsEnabled->ItemIndex;	
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::lboxVideoCodecsEnabledDragDrop(TObject *Sender,
      TObject *Source, int X, int Y)
{
	TListBox *lbox = lboxVideoCodecsEnabled;
	int dropIndex = lbox->ItemAtPos(Point(X,Y), false);
	if (dropIndex >= lbox->Items->Count)
	{
		dropIndex--;
	}
	if (dropIndex >= 0)
	{
		lbox->Items->Move(videoCodecsEnabledDraggedIndex, dropIndex);
		lbox->ItemIndex = dropIndex;
	}
}
//---------------------------------------------------------------------------

void __fastcall TfrmSettings::lboxVideoCodecsEnabledDragOver(TObject *Sender,
      TObject *Source, int X, int Y, TDragState State, bool &Accept)
{
	Accept = true;	
}
//---------------------------------------------------------------------------

void TfrmSettings::VideoCodecEnableSelected(void)
{
	for (int i=0; i<lboxVideoCodecsAvailable->Items->Count; i++)
	{
		if (lboxVideoCodecsAvailable->Selected[i])
		{
			lboxVideoCodecsEnabled->Items->Add(lboxVideoCodecsAvailable->Items->Strings[i]);
		}
	}
	lboxVideoCodecsAvailable->DeleteSelected();
}

void TfrmSettings::VideoCodecDisableSelected(void)
{
	for (int i=0; i<lboxVideoCodecsEnabled->Items->Count; i++)
	{
		if (lboxVideoCodecsEnabled->Selected[i])
		{
			lboxVideoCodecsAvailable->Items->Add(lboxVideoCodecsEnabled->Items->Strings[i]);
		}
	}
	lboxVideoCodecsEnabled->DeleteSelected();
}
