// resourcehelper.h
/////////////////////////////////////////////////////////

#ifndef __RESOURCE_HELPER__H__
#define __RESOURCE_HELPER__H__

class CTemporaryResourceOverride
{
public:
  CTemporaryResourceOverride(HINSTANCE hInstNew);
  CTemporaryResourceOverride(); // default construction

  virtual ~CTemporaryResourceOverride();

  static void SetDefaultResource(HINSTANCE hInstNew);

public:
  void   CommonConstruction(HINSTANCE);
  static HINSTANCE m_hInstanceDefault;

  HINSTANCE m_hInstanceOld;
};

#endif // __RESOURCE_HELPER__H__
