#ifndef _CLOCK_H_
#define _CLOCK_H_

/*
*针对多线程不能同时访问的对象,需要判断lock返回值
*/
class CLock
{
private:
	typedef void* HANDLE;
	
	volatile unsigned int _status; 
	const HANDLE _event;

	CLock(const CLock& rhs);
	CLock &operator = (const CLock& rhs);
public:
	CLock();
	~CLock();

	int lock();
	int unlock();
};

/*
*发现其它线程正在访问对象时，跳过不执行，使用如下：
if(CTry.have_a_try())
{
	//do something

	CTry.finish();
}
*/
class CTry
{
private:
	volatile unsigned int _status;
public:
	CTry():_status(1){}
	
	int have_a_try();
	void finish();
};


#endif