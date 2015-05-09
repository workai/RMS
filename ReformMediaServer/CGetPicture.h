#ifndef _CGETPICTURE_H_
#define _CGETPICTURE_H_

class CGetIcon
{
public:
	long icon_port;
	virtual bool init()=0;
	virtual int getIcon(char**buf, const char* file) const =0;
};

class CGetIconHK: public CGetIcon
{
private:
	static CGetIconHK* _instance;

	CGetIconHK();

	bool _init;
	bool init();

	//static const int buf_size = 1024*200;

public:
	static const CGetIconHK* Instance()
	{
		if(!_instance)
		{
			_instance = new CGetIconHK();
		}
		if(_instance->init())
			return _instance;
		return 0;
	}

	int getIcon(char**buf, const char* file) const;
};

class CGetIconDH: public CGetIcon
{
private:
	static CGetIconDH* _instance;

	CGetIconDH();

	bool _init;
	bool init();

	//static const int buf_size = 1024*200;

public:
	static const CGetIconDH* Instance()
	{
		if(!_instance)
		{
			_instance = new CGetIconDH();
		}
		if(_instance->init())
			return _instance;
		return 0;
	}

	int getIcon(char**buf, const char* file) const;
};

class CGetIconFormFile
{
private:
	static CGetIconFormFile* _instance;

	CGetIconFormFile();

public:
	static const CGetIconFormFile* Instance()
	{
		if(!_instance)
			_instance = new CGetIconFormFile();
		return _instance;
	}

	int getIcon(char** buf, const char* file, const char* type) const;
};

#endif