#ifndef TranslateH
#define TranslateH

#include <System.hpp>
#include <vector>

std::vector<AnsiString> EnumerateTranslations(void);

int LoadTranslations(AnsiString name, bool logMissingKeys);

typedef void (*TranslationCb)(void* obj);

void RegisterTranslationCb(void* obj, TranslationCb cb);
void UnregisterTranslationCb(void* obj);

void Translate(const char* key, AnsiString& text);

AnsiString Translate2(const char* key, AnsiString defaultText);

/// Translate temporary object (e.g. Caption)
#define TRANSLATE_TMP(key, tmp) tmp = Translate2(key, tmp)

#endif
