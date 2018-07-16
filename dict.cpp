#include <assert.h>
#include <iostream>
#include <sstream>
#include <set>
#include <list>
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

	const DictNode* addSubNode(const char ch)
	{
		DictNode new_sub_node(ch);
		std::pair<std::set<DictNode>::iterator, bool> insert_res = this->_sub_nodes.insert(new_sub_node);
		return &(*insert_res.first);
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

	const DictNode* subNode(const char ch) const
	{
		return this->_root_node.subNode(ch);
	}

	const DictNode* subNode(const DictNode &node) const
	{
		return this->_root_node.subNode(node);
	}

	void addWord(const std::string &word)
	{
		addWord(word.c_str(), word.length());
	}

	const DictNode* addWord(const char *word, size_t len)
	{
		const DictNode *new_added_word_end_node = NULL;
		const DictNode *parent_node = &(this->_root_node);
		for(size_t i=0; i<len; ++i)
		{
			const char ch = word[i];
			const DictNode *sub_node = parent_node->subNode(ch);
			if(!sub_node)
				sub_node = const_cast<DictNode*>(parent_node)->addSubNode(ch);
			assert(sub_node);
			if(i+1==len)
			{
				if(!sub_node->endOfWord())
					const_cast<DictNode*>(sub_node)->setEndOfWord(true);
				new_added_word_end_node = sub_node;
			}
			parent_node = sub_node;
		}
		return new_added_word_end_node;
	}

	bool hasWord(const std::string &word) const
	{
		return hasWord(word.c_str(), word.length());
	}

	bool hasWord(const char *word, size_t len) const
	{
		const DictNode *parent_node = &(this->_root_node);
		for(size_t i=0; i<len; ++i)
		{
			const char ch = word[i];
			const DictNode *sub_node = parent_node->subNode(ch);
			if(!sub_node)
				return false;
			if( i+1==len &&
				!sub_node->endOfWord() )
				return false;
			parent_node = sub_node;
		}
		return true;
	}

	std::vector<std::string> allWords() const
	{
		std::list<const DictNode*> buf;
		std::vector<std::string> result;
		const std::set<DictNode> &subNodes = this->_root_node.subNodes();
		for(std::set<DictNode>::const_iterator subNodesIter=subNodes.begin(); 
			subNodesIter!=subNodes.end(); ++subNodesIter)
		{
			const DictNode *node = &(*subNodesIter);
			Dict::searchAllWords(buf, result, node);
		}
		return result;
	}

	void output(std::ostringstream &s) const
	{
		int depth = 0;
		for(std::set<DictNode>::const_iterator iter=this->_root_node.subNodes().begin();
			iter!=this->_root_node.subNodes().end(); ++iter)
		{
			if(iter!=this->_root_node.subNodes().begin())
				s<<std::endl;
			iter->output(s, depth);
		}
	}

private:
	static void searchAllWords(
		std::list<const DictNode*> &buf, 
		std::vector<std::string> &result, 
		const DictNode *node
	)
	{
		buf.push_back(node);
		if(node->endOfWord())
		{
			std::string new_str;
			new_str.assign(buf.size(), ' ');
			size_t idx = 0;
			for(std::list<const DictNode*>::const_iterator buf_iter=buf.begin(); 
				buf_iter!=buf.end(); ++buf_iter, ++idx)
			{
				new_str[idx] = (*buf_iter)->ch();
			}
			result.push_back(new_str);
		}
		const std::set<DictNode> &subNodes = node->subNodes();
		for(std::set<DictNode>::const_iterator subNodesIter=subNodes.begin(); 
			subNodesIter!=subNodes.end(); ++subNodesIter)
		{
			const DictNode *node = &(*subNodesIter);
			Dict::searchAllWords(buf, result, node);
		}
		buf.pop_back();
	}

private:
	DictNode _root_node;
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
			if(!match_result.empty())
			{
				_sub_strs.push_back(std::make_pair(idx-1, match_result));
				idx = idx-1+match_result.size();	
			}
		};
	}
};

#ifdef DICT_UNIT_TEST
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

	std::vector<std::string> loaded_words = dict.allWords();
	for(size_t i=0; i<loaded_words.size(); ++i)
		std::cout<<loaded_words[i]<<std::endl;

	const char* whether_contain_words[] = {
		/*
		"abc", 
		"abcd", 
		"1ab", 
		"abd"
		*/
		"西湖", 
		"西湖我", 
		"西湖博物馆", 
		"西湖博物馆你", 
		"西湖他博物馆"
	};
	for(size_t word_idx=0; word_idx<sizeof(whether_contain_words)/sizeof(whether_contain_words[0]); ++word_idx)
	{
		std::string word(whether_contain_words[word_idx]);
		std::cout<<word<<" : "<<(dict.hasWord(word)?"True":"False")<<std::endl;
	}

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
#endif

