// SkipListVS.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

class SkipList {
public:
	SkipList();
	~SkipList();

	int elementCount;

	// non-modifying member functions

	/*
	It prints the key, value, level
	of each node of the skip list.

	Prints two nodes per line.
	*/
	void print() const;

	string* at(int) const;

	/*
	It searches the skip list and
	returns the element corresponding
	to the searchKey; otherwise it returns
	failure, in the form of null pointer.
	*/
	std::string* find(int searchKey) const;

	// modifying member functions

	/*
	It searches the skip list for elements
	with seachKey, if there is an element
	with that key its value is reassigned to the
	newValue, otherwise it creates and splices
	a new node, of random level.
	*/
	void insert(int searchKey, const std::string& newValue);

	/*
	It deletes the element containing
	searchKey, if it exists.
	*/
	void erase(int searchKey);

private:

	struct Node {
		int key;
		std::string value;

		int count;

		// pointers to successor nodes
		std::vector<Node*> forward;

		Node(int k, const std::string& v, int level) :
			key(k), value(v), forward(level, nullptr), count(0)
		{}
	};

	// Generates node levels in the range [1, maxLevel).    
	int randomLevel() const;

	//Returns number of incoming and outgoing pointers
	static int nodeLevel(const Node* v);

	//creates a node on the heap and returns a pointer to it.   
	static Node* makeNode(int key, std::string val, int level);

	// Returns the first node for which node->key < searchKey is false  
	Node* lower_bound(int searchKey) const;

	/*
	* Returns a collection of Pointers to Nodes
	* result[i] hold the last node of level i+1 for which result[i]->key < searchKey is true
	*/
	std::vector<Node*> predecessors(int searchKey) const;
	std::vector<Node*> predecessorsIncrementCount(int searchKey) const;

	// data members 
	const float probability;
	const int maxLevel;
	Node* head; // pointer to first node
	Node* NIL;  // last node
};


//==============================================================================
// Class SkipList member implementations

SkipList::SkipList() :
	probability(0.5),
	maxLevel(16),
	elementCount(0)
{
	int headKey = std::numeric_limits<int>::min();
	head = new Node(headKey, "head", maxLevel);

	int nilKey = std::numeric_limits<int>::max();
	NIL = new Node(nilKey, "NIL", maxLevel);

	std::fill(head->forward.begin(), head->forward.end(), NIL);
}

SkipList::~SkipList() {
	auto node = head;
	while (node->forward[0]) {
		auto tmp = node;
		node = node->forward[0];
		delete tmp;
	}
	delete node;
}

std::string* SkipList::find(int searchKey) const {
	std::string* res{};
	if (auto x = lower_bound(searchKey)) {
		if (x->key == searchKey && x != NIL) {
			res = &(x->value);
		}
	}
	return res;
}

void SkipList::print() const {
	Node* list = head->forward[0];
	int lineLength = 0;

	std::cout << "{";

	while (list != NIL) {
		std::cout << "value: " << list->value
			<< ", key: " << list->key
			<< ", level: " << nodeLevel(list);

		list = list->forward[0];

		if (list != NIL) std::cout << " : ";

		if (++lineLength % 2 == 0) std::cout << "\n";
	}
	std::cout << "}\n";
}

string* SkipList::at(int index) const {
	int targetCount = elementCount - index;

	std::vector<Node*> result(nodeLevel(head), nullptr);
	Node* x = head;

	bool fullBreak = false;
	unsigned int i;
	for (i = nodeLevel(head); !fullBreak && i-- > 0;) {
		while (!fullBreak && x->forward[i]->count >= targetCount && x->forward[i]->value != "NIL") {
			if (x->forward[i]->count == targetCount) {
				fullBreak = true;
				break;
			}

			x = x->forward[i];
		}
		//result[i] = x;
	}

	if (x->forward[i]->count == targetCount)
		return &(x->forward[i]->value);
	else
		return nullptr;
}

void SkipList::insert(int searchKey, const std::string& newValue) {
	auto preds = predecessors(searchKey);

	{//reassign value if node exists and return
		auto next = preds[0]->forward[0];
		if (next->key == searchKey && next != NIL) {
			next->value = newValue;
			return;
		}
	}

	preds = predecessorsIncrementCount(searchKey);

	// create new node
	const int newNodeLevel = randomLevel();
	auto newNodePtr = makeNode(searchKey, newValue, newNodeLevel);

	// connect pointers of predecessors and new node to respective successors
	for (int i = 0; i < newNodeLevel; ++i) {
		newNodePtr->forward[i] = preds[i]->forward[i];
		preds[i]->forward[i] = newNodePtr;

		newNodePtr->count = preds[i]->count;
	}

	elementCount++;
}


void SkipList::erase(int searchKey) {
	auto preds = predecessors(searchKey);

	//check if the node exists
	auto node = preds[0]->forward[0];
	if (node->key != searchKey || node == NIL) {
		return;
	}

	// update pointers and delete node 
	for (size_t i = 0; i < nodeLevel(node); ++i) {
		preds[i]->forward[i] = node->forward[i];
	}
	delete node;

	elementCount--;
}

//###### private member functions ######
int SkipList::nodeLevel(const Node* v) {
	return v->forward.size();
}

SkipList::Node* SkipList::makeNode(int key, std::string val, int level) {
	return new Node(key, val, level);
}

int SkipList::randomLevel() const {
	int v = 1;
	while (((double)std::rand() / RAND_MAX) < probability &&
		v < maxLevel) {
		v++;
	}
	return v;
}

SkipList::Node* SkipList::lower_bound(int searchKey) const {
	Node* x = head;

	for (unsigned int i = nodeLevel(head); i-- > 0;) {
		while (x->forward[i]->key < searchKey) {
			x = x->forward[i];
		}
	}
	return x->forward[0];
}

std::vector<SkipList::Node*> SkipList::predecessors(int searchKey) const {
	std::vector<Node*> result(nodeLevel(head), nullptr);
	Node* x = head;

	for (unsigned int i = nodeLevel(head); i-- > 0;) {
		while (x->forward[i]->key < searchKey) {
			x = x->forward[i];
		}
		result[i] = x;
	}

	return result;
}

std::vector<SkipList::Node*> SkipList::predecessorsIncrementCount(int searchKey) const {
	std::vector<Node*> result(nodeLevel(head), nullptr);
	Node* x = head;

	for (unsigned int i = nodeLevel(head); i-- > 0;) {
		while (x->forward[i]->key < searchKey) {
			x->count++;
			x = x->forward[i];
		}
		result[i] = x;
	}

	result[0]->count++;
	return result;
}

class Dummy
{
public:
	int param1;
	string param2;

	Dummy()
	{
		param1 = 0;
		param2 = "";
	}

	Dummy(int p1, string p2)
	{
		param1 = p1;
		param2 = p2;
	}
};


template<class T>
class SkipListGeneric
{
	SkipList _skipList;
	int(*_getKey)(T val);

	void erase(T val)
	{

	}

	T at(int index)
	{

	}

	void insert(T val)
	{
		_skipList.insert(_getKey(val), val);
	}
};

//==================================================
int main() {

	// 1.Initialize an empty SkipList object
	SkipList s;

	// 2. insert()
	for (int i = 2500000; i >= 1; --i) {
		/*std::stringstream ss;
		ss << i;*/

		if (i % 10000 == 0)
			printf("%d\n", i);

		s.insert(i, "");
	}
	s.insert(90, "90");
	s.insert(91, "91");

	// 2a. print()
	s.print();
	std::cout << std::endl;

	auto res = s.at(91);
	cout << *res << endl;

	// 3. find()        
	auto f = s.find(10);
	if (f) std::cout << "Node found!\nvalue: " << *f << '\n';
	else std::cout << "Node NOT found!\n";

	return 0;

	// 4. insert() - reassign
	s.insert(40, "TEST");

	// 4a. print()
	s.print();
	std::cout << std::endl;

	// 5. erase()
	s.erase(40);

	// 5a. print();
	s.print();
	std::cout << std::endl;

	std::cout << "\nDone!\n";
	getchar();
}