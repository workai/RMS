/*************************************
	作者:mycro
	邮箱:mycrolee@163.com
	整理时间:2007-03-27
*************************************/
#pragma once


#pragma warning(disable:4786)

#define MAX_CONTENT_LENGTH 1024

class DOMElement;
typedef int(*LPENUM_CALLBACK_FUNC)(DOMElement* element,void* pRef);

#include <vector>
#include <map>
#include <string>
#include <utility>
#include <utility>

using namespace std;

typedef vector<DOMElement*> ElementList;
typedef vector<DOMElement*>::iterator ElementIterator;

typedef map<string, string> AttributeMap;
typedef map<string, string>::const_iterator AttributeIterator;

typedef pair<string, string> StringPair;

#define MLPCSTR const char*

class DOMElement
{
protected:
	string m_tag;

	char* m_content;
	
//	char* pcontent;
	
public:
	int m_level;
	DOMElement* m_parent;
	DOMElement* m_next;
	DOMElement* m_preced;
	
	ElementList m_children;
	AttributeMap _attribute;

public:
	DOMElement* GetNextElement();
	string GetAttrStr();
	DOMElement();
	DOMElement(DOMElement* parent);
	virtual ~DOMElement(); 
	void FreeChilds();

	void SetAttribute(MLPCSTR attribute, MLPCSTR value);
	void GetAttribute(MLPCSTR attribute, string &value);

	char* getTextContent(){return m_content;}
	void setTextContent(MLPCSTR content)
	{ 
		if(m_content)
		{
			delete[] m_content;
		}
		m_content = new char[strlen(content) + 1];  
		memcpy(m_content, content, strlen(content) + 1);
	}

	string get_tag(){return m_tag;}
	void set_tag(MLPCSTR tag){m_tag = tag;}
	
	DOMElement* get_parent(){return m_parent;}
	void set_parent(DOMElement* parent){m_parent = parent;}

	int get_level(){return m_level;}
	void set_level(int level){m_level = level;}

	int Enumerate(LPENUM_CALLBACK_FUNC pFunc,void* pRef);
	int BuildElement(string& xml);
	
	DOMElement* AddChild(MLPCSTR tag, MLPCSTR content);

	int GetChildrenByTag(MLPCSTR tag, ElementList &list);
	DOMElement* GetFirstChildByTag(MLPCSTR tag,int IsGrand = FALSE);
};
