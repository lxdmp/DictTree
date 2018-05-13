#include <assert.h>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#include <stdio.h>

/*
 * 字典树实现(Dict & DictNode)
 */
class DictNode
{
public:
	DictNode():_end_of_word(false){}
	DictNode(const char ch) : _ch(ch), _end_of_word(false){}
	DictNode(const DictNode &other)
	{
		_ch = other._ch;
		_end_of_word = other._end_of_word;
	}
	
	const char ch() const{return _ch;}
	void setCh(const char ch){this->_ch = ch;}

	bool endOfWord() const{return _end_of_word;}
	void setEndOfWord(bool is_end){_end_of_word = is_end;}

	const std::set<DictNode>& subNodes() const{return _sub_nodes;}

	bool operator<(const DictNode &other) const{return ch()<other.ch();}
	void output(std::ostringstream &s, int depth = 0) const
	{
		for(int i=0; i<depth; ++i)
			s<<"  ";
		s<<_ch;
		if(_end_of_word)
			s<<" (End)";

		for(std::set<DictNode>::const_iterator iter=_sub_nodes.begin(); 
			iter!=_sub_nodes.end(); ++iter)
		{
			s<<std::endl;
			iter->output(s, depth+1);
		}
	}

	const DictNode* subNode(const char ch) const
	{
		DictNode query_node(ch);
		return subNode(query_node);
	}

	const DictNode* subNode(const DictNode &node) const
	{
		std::set<DictNode>::const_iterator iter = _sub_nodes.find(node);
		if(iter==_sub_nodes.end())
			return NULL;
		return &(*iter);
	}

private:
	char _ch;
	bool _end_of_word;
	std::set<DictNode> _sub_nodes;
};

class Dict
{
public:
	Dict(){}

	const std::set<DictNode>& content() const
	{
		return _entries;
	}

	const DictNode* subNode(const char ch) const
	{
		DictNode query_node(ch);
		return subNode(query_node);
	}

	const DictNode* subNode(const DictNode &node) const
	{
		std::set<DictNode>::const_iterator iter = _entries.find(node);
		if(iter==_entries.end())
			return NULL;
		return &(*iter);
	}

	void addWord(const std::string &word)
	{
		addWord(word.c_str(), word.length());
	}

	void addWord(const char *word, size_t len)
	{
		std::set<DictNode> *nodes = &_entries;
		for(size_t i=0; i<len; ++i)
		{
			DictNode query_node(word[i]), *node = NULL;
			std::set<DictNode>::iterator iter = nodes->find(query_node);
			if(iter!=nodes->end()){
				node = const_cast<DictNode*>(&(*iter));
			}else{
				std::pair<std::set<DictNode>::iterator, bool> insert_result = nodes->insert(query_node);
				assert(insert_result.second==true);
				node = const_cast<DictNode*>(&(*insert_result.first));
			}
			assert(node);
			if(i+1==len)
				node->setEndOfWord(true);
			nodes = const_cast<std::set<DictNode>*>(&(node->subNodes()));
		}
	}

	void output(std::ostringstream &s) const
	{
		int depth = 0;
		for(std::set<DictNode>::const_iterator iter=_entries.begin();
			iter!=_entries.end(); ++iter)
		{
			if(iter!=_entries.begin())
				s<<std::endl;
			iter->output(s, depth);
		}
	}

private:
	std::set<DictNode> _entries;
};

/*
 * 利用字典树实现子串的最大/最小匹配
 */
struct SubStrWithDict
{
	std::vector<std::pair<int, std::string> > _sub_strs;
	const Dict &_dict;
	bool _max_matched;

	SubStrWithDict(const Dict &dict) : 
		_dict(dict), 
		_max_matched(true)
	{
	}

	void setMaxMatched(){_max_matched = true;}
	void setMinMatched(){_max_matched = false;}

	void operator()(const std::string str)
	{
		this->operator()(str.c_str(), str.length());
	}
	void operator()(const char *str, size_t len)
	{
		_sub_strs.clear();
		this->do_work(str, len);
	}

	std::string try_match_once(const DictNode *root, const char *str, size_t len) const
	{
		std::string ret;
		size_t idx = 0;
		std::string buf;
		const DictNode *node = root;
		buf.push_back(node->ch());

		while(idx<len)
		{
			node = node->subNode(str[idx++]);
			if(!node)
				break;
			buf.push_back(node->ch());
			if(node->endOfWord())
			{
				ret = buf;
				if(!_max_matched)
					break;
			}
		}
		return ret;
	}

	void do_work(const char *str, size_t len, size_t start=0)
	{
		size_t idx = start;
		const DictNode *node = NULL;
		while(true)
		{
			while( idx<len && 
				(node=_dict.subNode(str[idx++]))==NULL);
			if(idx>=len || !node)
				return;
			std::string match_result = try_match_once(node, &str[idx], len-idx);
			if(!match_result.empty()){
				_sub_strs.push_back(std::make_pair(idx-1, match_result));
				idx = idx-1+match_result.size();	
			}
		};
	}
};

int main()
{
	Dict dict;
	const char* dict_words[] = {
		/*
		"abc", 
		"abcd", 
		"1ab", 
		"abd"
		*/
		"西湖", 
		"西湖博物馆"
	};
	for(size_t word_idx=0; word_idx<sizeof(dict_words)/sizeof(dict_words[0]); ++word_idx)
	{
		std::string word(dict_words[word_idx]);
		dict.addWord(word);
	}
	
	std::ostringstream s;
	dict.output(s);
	std::cout<<s.str()<<std::endl;

	const char* inputs[] = {
		//"21abbabcd", 
		"我去西湖博物馆看西湖", 
		"西湖在西湖博物馆旁边"
	};
	SubStrWithDict query(dict);
	//query.setMinMatched();
	for(size_t i=0; i<sizeof(inputs)/sizeof(inputs[0]); ++i)
	{
		std::string input = inputs[i];
		query(input);
		const std::vector<std::pair<int, std::string> > &result = query._sub_strs;
		std::cout<<"input : "<<input<<std::endl;
		for(size_t i=0; i<result.size(); ++i)
			std::cout<<result[i].first<<" : "<<result[i].second<<std::endl;
	}
	return 0;
}

