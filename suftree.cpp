#include <memory>
#include <string>
#include <map>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

struct TData;
class TSuffixNode {
public:
	std::map<char, TSuffixNode*> Road;
	TSuffixNode* SufLinks;
	std::pair<std::string::iterator, std::string::iterator> Coordinates;
	long long int SufIndex;

	TSuffixNode(std::string::iterator it1, std::string::iterator it2);
	~TSuffixNode() = default;
};

class TSuffixTree {
	public:
		TSuffixNode* Root;
		TSuffixTree(std::string& input);
		~TSuffixTree();
		int GoSuffixTree(std::string& str, int offset, TData& tdata);
		void NodePrint(TSuffixNode* node, int depth);
		void TreePrint();
		void Jump(TData& tdata);

	private:
		std::string Text;
		long long int Remainder, ActiveLength;
		TSuffixNode *NodeForSufLinks, *ActiveNode;
		std::string::iterator ActiveEdge;
		long long EdgeLength(TSuffixNode* node);
		void AddSufLinks(TSuffixNode* node);
		void Destroy(TSuffixNode* node);
		void TreeExtend(std::string::iterator add);
		bool WalkDown(TSuffixNode* node);
};

TSuffixNode::TSuffixNode(std::string::iterator it1, std::string::iterator it2) {
	Coordinates.first = it1;
	Coordinates.second = it2;
	this->SufLinks = nullptr;
	this->SufIndex = -1;
}

void TSuffixTree::Destroy(TSuffixNode* node) {
	for (std::map<char, TSuffixNode*>::iterator it = node->Road.begin(); it != node->Road.end(); ++it) {
		Destroy(it->second);
	}
	delete node;
}

TSuffixTree::~TSuffixTree() {
	Destroy(Root);
}

TSuffixTree::TSuffixTree(std::string& input) {
	Text = input;
	Remainder = 0;
	ActiveLength = 0;
	Root = new TSuffixNode(Text.end(), Text.end());
	ActiveNode = NodeForSufLinks = Root->SufLinks = Root;
	for (std::string::iterator it = Text.begin(); it != Text.end(); ++it) {
		TreeExtend(it);
	}
}

long long TSuffixTree::EdgeLength(TSuffixNode* node) {
	return node->Coordinates.second - node->Coordinates.first;
}

bool TSuffixTree::WalkDown(TSuffixNode* node) {
	if (ActiveLength >= EdgeLength(node)) {
		ActiveEdge += EdgeLength(node);
		ActiveNode = node;
		ActiveLength -= EdgeLength(node);
		return true;
	}
	else {
		return false;
	}
}

void TSuffixTree::AddSufLinks(TSuffixNode* node) {
	if (NodeForSufLinks != Root) {
		NodeForSufLinks->SufLinks = node;
		NodeForSufLinks = node;
	}
	else {
		NodeForSufLinks = node;
	}
}

void TSuffixTree::TreeExtend(std::string::iterator newSymbol) {
	++Remainder;
	NodeForSufLinks = Root;
	while (Remainder > 0) {
		if (ActiveLength == 0) {
			ActiveEdge = newSymbol;
		}
		std::map<char, TSuffixNode*>::iterator iterator = ActiveNode->Road.find(*ActiveEdge);
		TSuffixNode* nextNode = (iterator != ActiveNode->Road.end()) ? iterator->second : nullptr;
		if (nextNode == nullptr) {
			TSuffixNode* newLeaf = new TSuffixNode(newSymbol, Text.end());
			ActiveNode->Road[*ActiveEdge] = newLeaf;
			AddSufLinks(ActiveNode);
		}
		else {
			if (WalkDown(nextNode)) {
				continue;
			}
			if (*(nextNode->Coordinates.first + 	ActiveLength) == *newSymbol) {
				AddSufLinks(ActiveNode);
				++ActiveLength;
				break;
			}
			TSuffixNode* newInnerNode = new TSuffixNode(nextNode->Coordinates.first, nextNode->Coordinates.first + ActiveLength);
			TSuffixNode* newLeaf = new TSuffixNode(newSymbol, Text.end());
			ActiveNode->Road[*ActiveEdge] = newInnerNode;
			newInnerNode->Road[*newSymbol] = newLeaf;
			nextNode->Coordinates.first += ActiveLength;
			newInnerNode->Road[*nextNode->Coordinates.first] = nextNode;
			AddSufLinks(newInnerNode);
		}
		--Remainder;
		if (ActiveNode == Root && ActiveLength > 0) {
			ActiveEdge = newSymbol - Remainder + 1;
			--ActiveLength;
		}
		else {
			if (ActiveNode->SufLinks != nullptr) {
				ActiveNode = ActiveNode->SufLinks;
			}
			else {
				ActiveNode = Root;
			}
		}
	}
}
struct TData {
	std::pair<std::string::iterator, std::string::iterator> Coordinates;
	TSuffixNode* node; //узел, который сейчас обрабатывается
	long long int k = 0; //тут хранится статистика совпадений
	long long int length;
	long long int offset; //отступ в тексте
	bool leaf; //остановка произошла в листе или нет
};

//проход по суффиксному дереву для вычисления статистики совпадений
int TSuffixTree::GoSuffixTree(std::string& str, int offset, TData& tdata) {

	bool flag = true;
	auto it_str = str.begin() + offset + tdata.offset;
	auto it_tree = tdata.node->Coordinates.first + tdata.length;

	while (flag) {
		if (it_str == str.end()) {
			flag = false;
			break;
		}
		if (it_tree == tdata.node->Coordinates.second) {

			if (tdata.node->Road.count(*it_str)) {
				tdata.node = tdata.node->Road[*it_str];
				it_tree = tdata.node->Coordinates.first;
			}
			else {
				flag = false;
				if (tdata.node->Coordinates.second == Text.end()) {
					tdata.leaf = true;
				}
				else {
					tdata.leaf = false;
				}
				break;
			}
		}
		if (flag && *it_str == *it_tree) {
			tdata.k++;
			it_tree++;
			it_str++;
			tdata.leaf = false;
		}
		else {
			flag = false;
		}
	}
	tdata.Coordinates.first = tdata.node->Coordinates.first;
	tdata.Coordinates.second = it_tree;
	tdata.length = tdata.Coordinates.second - tdata.Coordinates.first;
	return tdata.k;
}
//скачки по счетчику
void TSuffixTree::Jump(TData& tdata) {
	if (tdata.k > 1) {
//если есть суф.ссылка, переход по ней
		if (tdata.node->SufLinks != 0 && tdata.node->SufLinks != Root) {
			tdata.node = tdata.node->SufLinks;
			long long int length = tdata.node->Coordinates.second - tdata.node->Coordinates.first;
			while (length < tdata.length) {
				if (length < tdata.length) {

					tdata.node = tdata.node->Road[*(tdata.Coordinates.first + length)];
					tdata.length -= length;
					tdata.Coordinates.first = tdata.Coordinates.first + length;
				}
				length = tdata.node->Coordinates.second - tdata.node->Coordinates.first;
			}
			tdata.offset = tdata.k - 1;
			tdata.k--;
		}
		else {
//иначе идем из корня к нужной вершине
			tdata.Coordinates.first = tdata.Coordinates.second - tdata.k;
			tdata.length = tdata.k;
			tdata.Coordinates.first++;
			tdata.length--;
			tdata.node = Root;
			tdata.node = tdata.node->Road[*(tdata.Coordinates.first)];
			long long int length = tdata.node->Coordinates.second - tdata.node->Coordinates.first;
			while (length < tdata.length) {
				if (length < tdata.length) {
					tdata.node = tdata.node->Road[*(tdata.Coordinates.first + length)];
					tdata.length -= length;
					tdata.Coordinates.first = tdata.Coordinates.first + length;
				}
				length = tdata.node->Coordinates.second - tdata.node->Coordinates.first;
			}
			tdata.offset = tdata.k - 1;
			tdata.k--;
		}
	}
	else {
		tdata.k = 0;
		tdata.offset = 0;
		tdata.length = 0;
		tdata.node = Root;
		tdata.Coordinates.first = Text.begin();
		tdata.Coordinates.second = Text.begin();
	}
}

int main() {
	std::ofstream fs;
	std::ifstream input;
	fs.sync_with_stdio(false);
	fs.tie(nullptr);
	input.sync_with_stdio(false);
	input.tie(nullptr);
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);
	std::string text;
	std::getline(std::cin, text);
	std::string str;
	std::getline(std::cin, str);
	TData tdata;
	if (text.compare("") != 0 && str.compare("") != 0 && str.size() >= text.size()) {
		std::string text1 = text + "$";
		TSuffixTree tree(text1);
		tdata.node = tree.Root;
		tdata.Coordinates.first = text.begin();
		tdata.Coordinates.second = text.begin();
		tdata.k = 0;
		tdata.length = false;
		tdata.offset = 0;
		tdata.length = 0;
		for (int i = 0; i < str.size() - text.size() + 1; ++i) {
			tree.GoSuffixTree(str, i, tdata);
			if (tdata.k == text.size()) {
				std::cout << i + 1 << std::endl;
			}
			tree.Jump(tdata);
		}
	}
	return 0;
}