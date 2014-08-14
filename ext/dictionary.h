#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include "types.h"
#include <vector>
#include <string>
#include <unordered_map>
using namespace std;

struct DicItem;

typedef unordered_map<UINT, DicItem*> DicItemMap;

struct DicItem
{
	bool isWord;
	bool hasChildren;
	DicItemMap children_map;
	DicItem():isWord(false), hasChildren(false){}
};

class CDictionary
{
public:
	CDictionary();
	virtual ~CDictionary();
	bool load_directionary(const char* dic_file, UINT& load_count);
	void unload_directionary();
	void chinese2pinyin(const string& chinese, string& pinyin, const string split);
protected:
	UINT get_utf8_charactor(const UCHAR* utf8, size_t length, UINT& out_put_char);
	UINT get_utf8_charactor_length(const UCHAR* utf8, size_t length);
	bool add_entry(const UCHAR* entry, size_t length);
	void add_pinyin(const string& chinese, const vector<string>& pinyin);
	void free_dic_item(DicItem* pItem);
	bool is_english_charactor(const UINT utf8, size_t length);
	bool extract_chinese_pinyin(const string& line, string& chinese, string& pinyin);
	void split_words(const UCHAR* utf8, size_t length, vector<string> &out_words);
private:
	DicItem m_dicRootItem;
	unordered_map<string, vector<string> > m_pinyin;
};

#endif//_DICTIONARY_H_
