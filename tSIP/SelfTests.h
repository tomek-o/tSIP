//---------------------------------------------------------------------------

#ifndef SelfTestsH
#define SelfTestsH
//---------------------------------------------------------------------------

#ifdef _DEBUG

/** \brief Run all self-tests and log the result of each. Debug builds only -
 *  both this declaration and its definition (SelfTests.cpp) are compiled
 *  out entirely for release builds, so callers must guard call sites with
 *  #ifdef _DEBUG as well.
 */
void RunSelfTests(void);

#endif // _DEBUG

#endif
