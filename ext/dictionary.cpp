#include "dictionary.h"
#include <fstream>
#include <string>
#include <iostream>
#include <string.h>
#include <boost/algorithm/string.hpp>

using namespace boost;
using namespace boost::algorithm;
using namespace std;

CDictionary::CDictionary()
{
}

CDictionary::~CDictionary()
{
	unload_directionary();
}

bool CDictionary::load_directionary(const char * dic_file, UINT& load_count)
{
	load_count = 0;
	ifstream dic_file_stream(dic_file);

	if (!dic_file_stream)
	{
		return false;
	}

	string line, chinese, pinyin;

	while (getline(dic_file_stream, line))
	{
		if (line.empty() || line[0] == '#')
		{
			continue;
		}

		if (!extract_chinese_pinyin(line, chinese, pinyin))
		{
			continue;
		}
		vector<string> vec_chinese, vec_pinyin;
		split(vec_chinese, chinese, is_any_of(" \t"));
		split(vec_pinyin, pinyin, is_any_of(" \t"));
		for (vector<string>::iterator ite = vec_chinese.begin(); ite != vec_chinese.end(); ++ite)
		{
			add_entry((const UCHAR*)ite->c_str(), ite->length());
			add_pinyin(*ite, vec_pinyin);
		}
		load_count++;
	}

	dic_file_stream.close();
	
	return true;
}

UINT CDictionary::get_utf8_charactor(const UCHAR* utf8,
										size_t length,
										UINT& out_put_char)
{
	out_put_char = 0;

	if(!length || !utf8)
	{
		return 0;
	}
	

	size_t byte_count = 0;

	UCHAR head = utf8[0];

	//如果最高位为0,则占用一个字节
	if((head & 0x80) == 0)
	{
		byte_count = 1;
	}
	//如果最高位为110，则占用2字节
	else if((head & 0xE0) == 0xC0)
	{
		byte_count = 2;
	}
	//如果最高位为1110，则占用3字节
	else if((head & 0xF0) == 0xE0)
	{
		byte_count = 3;
	}
	//出错了,取一字节
	else
	{
		byte_count = 1;
	}

	size_t copy_len = byte_count > length ?
		length : byte_count;

	memcpy(&out_put_char, utf8, copy_len);

	return copy_len;
}

UINT CDictionary::get_utf8_charactor_length(const UCHAR* utf8, size_t length)
{
	if(!length || !utf8)
	{
		return 0;
	}
	

	size_t byte_count = 0;

	UCHAR head = utf8[0];

	//如果最高位为0,则占用一个字节
	if((head & 0x80) == 0)
	{
		byte_count = 1;
	}
	//如果最高位为110，则占用2字节
	else if((head & 0xE0) == 0xC0)
	{
		byte_count = 2;
	}
	//如果最高位为1110，则占用3字节
	else if((head & 0xF0) == 0xE0)
	{
		byte_count = 3;
	}
	//出错了,取一字节
	else
	{
		byte_count = 1;
	}

	size_t copy_len = byte_count > length ?
		length : byte_count;

	return copy_len;
}


bool CDictionary::add_entry(const UCHAR * entry, size_t length)
{
	if(!entry || 0 == length)
	{
		return false;
	}

	size_t start_pos = 0;

	UINT out_put_char = 0;

	DicItem *pItem = &m_dicRootItem;

	DicItem *pTmpItem = NULL;

	while(start_pos < length)
	{
		//取得第一个字符
		size_t _offset = get_utf8_charactor(entry + start_pos, 
						length - start_pos, out_put_char);
		
		DicItemMap::iterator ite = pItem->children_map.find(out_put_char);
		
		//如果没有找到,则构造一个新的节点插入到根map
		if(ite == pItem->children_map.end())
		{
			pTmpItem = new(nothrow) DicItem;
			if(!pTmpItem)
			{
				return false;
			}
			pItem->children_map.insert(pair<int, DicItem*>(out_put_char, pTmpItem));
			pItem->hasChildren = true;
		}
		else
		{
			pTmpItem = pItem->children_map[out_put_char];
		}

		//移位到下一个字符
		start_pos += _offset;

		//如果结束，则说明是一个完整的词
		if(start_pos == length)
		{
			pTmpItem->isWord = true;
		}

		pItem = pTmpItem;
	}

	return true;
}

void CDictionary::split_words(const UCHAR* utf8,
								size_t length,
								vector<string> &out_words)
{
	out_words.clear();
	if(!utf8 || 0 == length)
	{
		return;
	}

	size_t start_pos = 0;
	size_t next_pos = 0;
	UINT out_put_char = 0;
	DicItem *pItem = &m_dicRootItem;

	while(next_pos < length)
	{
		//取得第一个字符
		size_t _offset = get_utf8_charactor(utf8 + next_pos, 
						length - next_pos, out_put_char);
		
		//查找该字符是否在分词库中
		DicItemMap::iterator ite; 

		//对非汉字的字符做特殊处理，直接返回"找不到"
		ite = pItem->children_map.find(out_put_char);

		//如果没有找到，则说明该词在分词库中不存在应该截取输出
		if(ite == pItem->children_map.end())
		{
			//不完整的词
			if(!pItem->isWord)
			{
				size_t _offset_tmp = get_utf8_charactor_length(utf8 + start_pos, 
										length - start_pos);

				out_words.push_back(string((CHAR*)utf8 + start_pos, _offset_tmp));
				
				start_pos += _offset_tmp;
				next_pos = start_pos;
			}
			//完整的词
			else
			{
				out_words.push_back(string((CHAR*)utf8 + start_pos, next_pos - start_pos));
				start_pos = next_pos;
			}

			pItem = &m_dicRootItem;
		}
		else
		{
			next_pos += _offset;

			pItem = ite->second;
		}
	}
	

	//如果到了结束还未分词完毕，则输出剩下的词
	if (next_pos == length)
	{
		out_words.push_back(string((CHAR*)utf8+start_pos));
	}
	else
	{
		while (start_pos < length)
		{
			UINT len = get_utf8_charactor_length(utf8 + start_pos, length - start_pos);
			out_words.push_back(string((CHAR*)utf8 + start_pos, len));
			start_pos += len;
		}
	}
}

void CDictionary::free_dic_item(DicItem* pItem)
{
	if(!pItem)
	{
		return;
	}

	DicItemMap& _map = pItem->children_map;

	//递归释放子节点
	for(DicItemMap::iterator ite = _map.begin();
		ite != _map.end(); ++ite)
	{
		free_dic_item(ite->second);
		delete ite->second;
	}

	_map.clear();
}

void CDictionary::unload_directionary()
{
	free_dic_item(&m_dicRootItem);
	m_pinyin.clear();
}

bool CDictionary::is_english_charactor(UINT utf8,
										size_t length)
{
	if(1 == length)
	{
		return  (utf8 >= 'a' && utf8 <= 'z') || (utf8 >= 'A' && utf8 <= 'Z') ?
				true : false;
	}

	return false;
}

void CDictionary::add_pinyin(const string& chinese, const vector<string>& pinyin)
{
	m_pinyin.insert(pair<string, vector<string> >(chinese, pinyin));
}

bool CDictionary::extract_chinese_pinyin(const string& line, string& chinese, string& pinyin)
{
	size_t pos, pos1;
	string _line = line.substr(0, line.find('/'));
	trim(_line);
	if (string::npos == (pos = _line.find('[')))
	{
		return false;
	}
	chinese = _line.substr(0, pos);
	if (string::npos == (pos1 = _line.find(']')))
	{
		return  false;
	}
	pinyin = _line.substr(pos+1, pos1-pos-1);
	trim(chinese);
	trim(pinyin);
	return true;
}

void CDictionary::chinese2pinyin(const string& chinese, string& pinyin, const string split)
{
	bool is_pinyin = true;
	vector<string> split_word;
	split_words((const UCHAR*)chinese.c_str(), chinese.length(), split_word);
	for (vector<string>::iterator ite = split_word.begin();
		ite != split_word.end(); ++ite)
	{
		unordered_map<string, vector<string> >::iterator ite_pinyins;
		if ((ite_pinyins = m_pinyin.find(*ite)) != m_pinyin.end())
		{
			vector<string>& pinyins = ite_pinyins->second;
			for (vector<string>::iterator ite_pys = pinyins.begin(); ite_pys != pinyins.end(); ++ite_pys)
			{
				if (!is_pinyin)
				{
					pinyin.append(split);
					is_pinyin = true;
				}
				pinyin.append(*ite_pys);
				pinyin.append(split);
			}
		}
		else
		{
			pinyin.append(*ite);
			is_pinyin = false;
		}
	}
}



