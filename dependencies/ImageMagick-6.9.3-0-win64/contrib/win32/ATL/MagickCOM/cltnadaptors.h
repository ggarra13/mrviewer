#ifndef _CLTNADAPTORS_H
#define _CLTNADAPTORS_H

template <typename T>
struct _CopyVariantFromAdaptItf {
	static HRESULT copy(VARIANT* p1, CAdapt< CComPtr<T> >* p2) {
    HRESULT hr = p2->m_T->QueryInterface(IID_IDispatch, (void**)&p1->pdispVal);
    if( SUCCEEDED(hr) ) {
      p1->vt = VT_DISPATCH;
    }
    else {
      hr = p2->m_T->QueryInterface(IID_IUnknown, (void**)&p1->punkVal);
      if( SUCCEEDED(hr) ) {
        p1->vt = VT_UNKNOWN;
      }
    }

    return hr;
  }
	
  static void init(VARIANT* p)    { VariantInit(p); }
  static void destroy(VARIANT* p) { VariantClear(p); }
};

template <typename T>
struct _CopyItfFromAdaptItf {
	static HRESULT copy(T** p1, CAdapt< CComPtr<T> >* p2) {
    if( *p1 = p2->m_T ) return (*p1)->AddRef(), S_OK;
    return E_POINTER;
  }
	
  static void init(T** p)    {}
  static void destroy(T** p) { if( *p ) (*p)->Release(); }
};

#endif