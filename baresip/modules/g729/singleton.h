/** \file	singleton.h
	\brief	Singleton object creational pattern template
*/

/** \note
	Usage:

	class CSth : public CSingleton<CSth>
	{
		friend CSingleton<CSth>;
		private:
			CSth();
			~CSth();
		...
	}

*/

#ifndef SingletonH
#define SingletonH

/** \brief Singleton pattern template class
*/
template <typename T> class CSingleton
{
public:
    /** \brief Get pointer to object
    */
	static T* Instance()
    {
		if (instance == NULL) instance = new T;
		return instance;
	};
	/** \brief Destroy object. This may seem unnecessary, but it makes runtime error checking happy.
	*/
	static void Destroy()
	{
		delete instance;
		instance = NULL;
	};
protected:
	CSingleton() {};
	virtual ~CSingleton() {};
private:
	CSingleton(const CSingleton& source) {
	};
	CSingleton& operator=(const CSingleton&);
	static T* instance;
};

// static initialization
template <typename T> T* CSingleton<T>::instance = NULL;


#endif
