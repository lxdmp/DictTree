/*
 * 字典树实现(Dict & DictNode)
 */
#ifndef _DICT_H_
#define _DICT_H_

#include <sstream>
#include <list>
#include <vector>
#include <set>

class SubStrWithDict;

class DictNode
{
public:
	DictNode();
	DictNode(const char ch);
	DictNode(const DictNode &other);
	
	const char ch() const;
	void setCh(const char ch);

	bool endOfWord() const;
	void setEndOfWord(bool is_end);

	const std::set<DictNode>& subNodes() const;

	bool operator<(const DictNode &other) const;

	const DictNode* subNode(const char ch) const;
	const DictNode* subNode(const DictNode &node) const;
	const DictNode* addSubNode(const char ch);

	void output(std::ostringstream &s, int depth = 0) const;

private:
	char _ch;
	bool _end_of_word;
	std::set<DictNode> _sub_nodes;
};

class Dict
{
public:
	Dict();

	const DictNode* addWord(const std::string &word);
	const DictNode* addWord(const char *word, size_t len);

	bool hasWord(const std::string &word) const;
	bool hasWord(const char *word, size_t len) const;

	std::vector<std::string> allWords() const;

	void output(std::ostringstream &s) const;

protected:
	const DictNode* subNode(const char ch) const;
	const DictNode* subNode(const DictNode &node) const;

private:
	static void searchAllWords(
		std::list<const DictNode*> &buf, 
		std::vector<std::string> &result, 
		const DictNode *node
	);

private:
	DictNode _root_node;
	friend class SubStrWithDict;
};

/*
 * 利用字典树(Dict)实现子串的最大/最小匹配
 */
class SubStrWithDict
{
public:
	typedef std::vector<std::pair<int, std::string> > ResultType;

public:
	SubStrWithDict(const Dict &dict);

	void setMaxMatched();
	void setMinMatched();

	void operator()(const std::string str);
	void operator()(const char *str, size_t len);

	const ResultType& result() const;

private:
	std::string try_match_once(const DictNode *root, const char *str, size_t len) const;
	void do_work(const char *str, size_t len, size_t start=0);

private:
	ResultType _sub_strs;
	const Dict &_dict;
	bool _max_matched;
};

#endif

