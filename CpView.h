///////////////////////////////////////////////////////////
//  CpView.h
//  Implementation of the Class CpView
//  Created on:      04-Jul-2015 08:47:00
//  Original author: Phung Kim Phuong
///////////////////////////////////////////////////////////

#if !defined(EA_148303EF_CC07_41df_8407_CE6D423D78AE__INCLUDED_)
#define EA_148303EF_CC07_41df_8407_CE6D423D78AE__INCLUDED_

/**
 * Draw contexts manager
 */
class CpView
{

public:
	CpView();
	virtual ~CpView();
	void DrawStatic();
	void DrawDynamic();

private:
	/**
	 * Scale to convert Lat Long to screen coordinates
	 */
	float m_Scale;
	/**
	 * Draw context vor dynamic object, this context could be regularly changed
	 */
	QImage * m_dynamic_object;
	/**
	 * Draw context for static objects (maps, stations, cities...)
	 */
	QPixmap * m_static_context;

};
#endif // !defined(EA_148303EF_CC07_41df_8407_CE6D423D78AE__INCLUDED_)
