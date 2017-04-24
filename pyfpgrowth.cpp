#include "Python.h"
#include <algorithm>
using namespace std;
#pragma warning(disable:4996) 
#define BUFFER 20000
#define MAXITEMNUM 20000//the maxinum number of items, items whose indexes higher than it will be ignored
double Items[MAXITEMNUM];//the array recording the support of every item
long itemsize;


class TransNode
{
public:
	long value;
	TransNode *right;
	long len;
public:
	TransNode()
	{
		value = 0;
		right = NULL;
		len = 0;
	}
	~TransNode()
	{
		if (right != NULL)
		{
			delete right;
		}
	}
};

class Data
{
public:
	TransNode *trs;
	Data *next;
public:
	Data()
	{
		trs = NULL;
		next = NULL;
	}
	~Data()
	{
		if (trs != NULL)
		{
			delete trs;
		}
		if (next != NULL)
		{
			delete next;
		}
	}
};

class FreqPat
{
public:
	long value;
	FreqPat *right;
	long len;
	long support;
public:
	FreqPat()
	{
		value = 0;
		right = NULL;
		len = 0;
	}
	~FreqPat()
	{
		delete right;
	}
};

class FrePatRsult
{
public:
	FreqPat *pat;
	FrePatRsult *next;
	long PatNum;
public:
	FrePatRsult()
	{
		pat = NULL;
		next = NULL;
		PatNum = 0;
	}
	~FrePatRsult()
	{
		delete pat;
		delete next;
	}
};

FrePatRsult* theResult;
Data *Dataset;

class Node//the node in FPtree
{
public:
	long value;//the value
	long num;//the number
	Node *parent;//parent node
	Node* Son;
	Node* nextSamePar;
	Node* nextRight;
public:
	Node()
	{
		value = 0;
		num = 0;
		parent = NULL;
		Son = NULL;
		nextSamePar = NULL;
		nextRight = NULL;
	}
	~Node()
	{
		if (nextRight != NULL)
		{
			delete nextRight;
		}
	}

};

class Header//the data struct of head table
{
public:
	long value;
	double num;
	Header *parent;
	Header *son;
	Node *nextRight;//the nodes linked with the head table
	long rNum;//the number of nodes linked with the head table
public:
	Header()
	{
		value = 0;
		num = 0;
		rNum = 0;
		son = NULL;
		parent = NULL;
		nextRight = NULL;
	}
	~Header()
	{
		if (value < 0)
		{
			parent->son = NULL;
		}
		if (son != NULL)
		{
			delete son;
		}
		if (nextRight != NULL)
		{
			delete nextRight;
		}
	}
};

double transNum;

Header *FreItem(double minsup)//Scanning the whole dataset once and output the headTable
{
	memset(Items, 0, BUFFER*sizeof(double));
	transNum = 0;
	itemsize = 0;
	long value;
	long i;
	Data *strans;
	for (strans = Dataset; strans->next != NULL; strans = strans->next)//Counting the support of every item
	{
		TransNode *sNode;
		for (sNode = strans->trs; sNode->right != NULL; sNode = sNode->right)
		{
			value = sNode->value;
			if (value < MAXITEMNUM)
			{
				Items[value]++;
				if (value > itemsize)
				{
					itemsize = value;
				}
			}
		}
		transNum++;
	}

	Header *Head;
	Head = new Header;
	Head->num = -1;
	Head->parent = Head;
	Head->value = -1;
	Head->son = Head;
	Head->rNum = 0;

	for (i = 0; i < itemsize + 1; i++)//building the head table
	{
		Items[i] = Items[i] + 0.0000001*double(i);
		if (Items[i] >= minsup*transNum && Items[i] >= 1)
		{
			Header *Hsearch;
			Hsearch = Head;
			Header *Item;
			Item = new Header;
			Item->num = Items[i];
			Item->value = i;
			Item->rNum = 0;
			while (!(Item->num >= Hsearch->num && (Item->num <= Hsearch->parent->num || Hsearch->parent->num == -1)))
			{
				Hsearch = Hsearch->parent;
			}
			Item->son = Hsearch;
			Item->parent = Hsearch->parent;
			Hsearch->parent->son = Item;
			Hsearch->parent = Item;
		}
		else
		{
			Items[i] = -1;
		}
	}
	return Head;//Output the headTable
}

bool compare(long a, long b)//the operator to sort items in the transaction
{
	if (Items[a] == Items[b])
	{
		return a > b;
	}
	else
	{
		return Items[a]>Items[b];
	}
}

Node* FPtree(Header *Head)//Build FP-tree
{
	//rewind(fIn);//reset the fIn to the begin of file
	Node *Tree;//The head of fp-tree
	Tree = new Node;
	Tree->num = -1;
	Tree->value = -1;
	Tree->parent = NULL;
	long value;
	long j, shnum;
	Data *strans;
	for (strans = Dataset; strans->next != NULL; strans = strans->next)
	{
		long trans[BUFFER] = { 0 };
		j = 0;
		TransNode *sNode;
		for (sNode = strans->trs; sNode->right != NULL; sNode = sNode->right)
		{
			value = sNode->value;
			if (value < itemsize + 1)
			{
				if (Items[value] > 0)
				{
					trans[j] = value;
					j++;
				}
			}
		}
		sort(trans, trans + j, compare);//sort the items in the transactions according to the frequents
		Header *Hsearch;
		Hsearch = Head->son;
		Node *Tsearch;
		Tsearch = Tree;
		for (shnum = 0; shnum < j && Items[trans[0]]>0 && Hsearch->value>0;)//Looking for items belonging to trans and Head
		{
			if (Hsearch->value == trans[shnum])//The item belonging to trans and Head
			{
				shnum++;
				long find = 0;
				Node *sSon;
				for (sSon = Tsearch->Son; sSon != NULL; sSon = sSon->nextSamePar)//search the son of current node of tree
				{
					if (sSon->value == Hsearch->value)//if there is a node whose value is the item
					{
						sSon->num++;//its number puls one
						find = 1;
						Tsearch = sSon;//the current node is changed
						break;
					}
					if (sSon->nextSamePar == NULL)
					{
						break;
					}
				}
				if (find == 0)//if there is no son of the current node whose value is the item
				{
					if (Tsearch->Son == NULL)
					{
						Tsearch->Son = new Node;
						sSon = Tsearch->Son;
					}
					else
					{
						sSon->nextSamePar = new Node;
						sSon = sSon->nextSamePar;
					}
					sSon->value = Hsearch->value;
					sSon->num = 1;
					//Tsearch->Son[m]->sonNum = 0;
					sSon->parent = Tsearch;

					Tsearch = sSon;//the current node is changed
					if (Hsearch->rNum == 0)
					{
						Hsearch->nextRight = Tsearch;
					}
					else
					{
						Tsearch->nextRight = Hsearch->nextRight;
						Hsearch->nextRight = Tsearch;
					}
					Hsearch->rNum++;
				}
			}
			Hsearch = Hsearch->son;
		}
	}
	return Tree;
}

double sItems[MAXITEMNUM] = { 0 };

Header *condFreItem(Header *sH, double minsup)
{
	long i, Num;
	memset(sItems, 0, sizeof(double)*MAXITEMNUM);
	Node *sNode;
	for (sNode = sH->nextRight; sNode != NULL; sNode = sNode->nextRight)//sH->Right[0],sH->Right[1]....are the conditional basis
	{
		Node *sC;
		sC = sNode->parent;
		Num = sNode->num;
		while (sC->value != -1)
		{
			sItems[sC->value] = sItems[sC->value] + Num;
			sC = sC->parent;
		}
	}

	Header *sHead = new Header;
	sHead->num = -1;
	sHead->parent = sHead;
	sHead->value = -1;
	sHead->son = sHead;
	sHead->rNum = 0;

	for (i = 0; i < itemsize + 1; i++)//building the conditional head table
	{
		sItems[i] = sItems[i] + 0.0000001*double(i);
		if (sItems[i] >= minsup*transNum && sItems[i] >= 1)
		{
			Header *Hsearch;
			Hsearch = sHead;
			Header *Item;
			Item = new Header;
			Item->num = sItems[i];
			Item->value = i;
			Item->rNum = 0;
			while (!(Item->num >= Hsearch->num && (Item->num <= Hsearch->parent->num || Hsearch->parent->num == -1)))
			{
				Hsearch = Hsearch->parent;
			}
			Item->son = Hsearch;
			Item->parent = Hsearch->parent;
			Hsearch->parent->son = Item;
			Hsearch->parent = Item;
		}
		else
		{
			sItems[i] = -1;
		}
	}
	return sHead;
}

bool comparecond(long a, long b)//the operator to sort items in the transaction
{
	if (sItems[a] == sItems[b])
	{
		return a > b;
	}
	else
	{
		return sItems[a]>sItems[b];
	}
}

Node* condFPtree(Header *sH, Header *sHead)	//building the conditional fp tree
{
	Node *sTree;
	sTree = new Node;
	sTree->num = -1;
	sTree->value = -1;
	sTree->parent = NULL;
	//sTree->sonNum = 0;
	long j, shnum;
	Node *sNode;
	for (sNode = sH->nextRight; sNode != NULL; sNode = sNode->nextRight)
	{
		Node *sC;
		sC = sNode->parent;
		long trans[BUFFER] = { 0 };
		j = 0;
		while (sC->value != -1)	//store the transaction of conditional basis
		{
			trans[j] = sC->value;
			sC = sC->parent;
			j++;
		}
		sort(trans, trans + j, comparecond);//sort the items in the trans of the conditional basis according to their support in the conditional basis
		Header *Hsearch;
		Hsearch = sHead->son;
		Node *Tsearch;
		Tsearch = sTree;
		for (shnum = 0; shnum < j && sItems[trans[0]]>0 && Hsearch->value>0;)//Looking for items belonging to trans and Head
		{
			if (Hsearch->value == trans[shnum])//The item belonging to trans and Head
			{
				shnum++;
				long find = 0;
				Node *sSon;
				for (sSon = Tsearch->Son; sSon != NULL; sSon = sSon->nextSamePar)//search the son of current node of tree
				{
					if (sSon->value == Hsearch->value)//if there is a node whose value is the item
					{
						sSon->num = sSon->num + sNode->num;//its number puls one
						find = 1;
						Tsearch = sSon;//the current node is changed
						break;
					}
					if (sSon->nextSamePar == NULL)
					{
						break;
					}
				}
				if (find == 0)//if there is no son of the current node whose value is the item
				{
					if (Tsearch->Son == NULL)
					{
						Tsearch->Son = new Node;
						sSon = Tsearch->Son;
					}
					else
					{
						sSon->nextSamePar = new Node;
						sSon = sSon->nextSamePar;
					}
					sSon->value = Hsearch->value;
					sSon->num = sNode->num;
					//Tsearch->Son[m]->sonNum = 0;
					sSon->parent = Tsearch;

					Tsearch = sSon;//the current node is changed

					if (Hsearch->rNum == 0)
					{
						Hsearch->nextRight = Tsearch;
					}
					else
					{
						Tsearch->nextRight = Hsearch->nextRight;
						Hsearch->nextRight = Tsearch;
					}
					Hsearch->rNum++;
				}
			}
			Hsearch = Hsearch->son;
		}
	}
	return sTree;
}

void FP_growth(Node *Tree, Header *Head, double minsup, long itemset[BUFFER], long len)	//mining the frequent itemsets
{
	Header *sH;
	sH = Head->parent;
	long i;
	while (sH->value != -1)
	{
		long beta[BUFFER] = { 0 };//show the frequent itemsets
		long length = len;
		for (i = 0; i < length; i++)
		{
			beta[i] = itemset[i];
		}
		beta[length] = sH->value;
		length++;

		//Output the frequent patterns
		theResult->PatNum++;
		FrePatRsult *sResult;
		for (sResult = theResult; sResult->next != NULL; sResult = sResult->next)
		{

		}//find the end of all the existed frequent patterns
		FreqPat *sPat;
		sResult->pat = new FreqPat;//create a new pattern
		sPat = sResult->pat;
		sPat->support = long(sH->num);//the support of this pattern
		sPat->len = length;//the length of this pattern
		for (i = 0; i < length; i++)
		{
			sPat->value = beta[i];
			sPat->right = new FreqPat;
			sPat = sPat->right;
		}
		sResult->next = new FrePatRsult;
		sResult = sResult->next;
		//Complete the output
		Header *sHead;
		sHead = condFreItem(sH, minsup);//building the headtable of the conditional basis
		Node *sTree;
		sTree = condFPtree(sH, sHead);//buildng the conditional fp tree
		FP_growth(sTree, sHead, minsup, beta, length);//frquent itemsets are growthing
		sH = sH->parent;

		delete sTree;
		delete sHead;
	}
}

long fpgrowth(double minsup)
{
	Header *fphead;
	Node *fpTree;
	fphead = FreItem(minsup);//scanning the dataset and find those items with supports higher than minsup
	fpTree = FPtree(fphead);//building fp-tree
	long itemset[BUFFER] = { 0 };
	long len = 0;
	FP_growth(fpTree, fphead, minsup, itemset, len);//mining the freqeunt itemsets
	delete fpTree;
	delete fphead;
	return 0;
}

static PyObject *pyfpgrowthError;

static PyObject *
pyfpgrowth(PyObject *self, PyObject *args)
{
	theResult = new FrePatRsult;//the final result mined by fpgrowth
	Dataset = new Data;
	PyObject* pyDataset;//define the dataset
	double minsup;

	if (!PyArg_ParseTuple(args, "Od", &pyDataset, &minsup))//read the input to the dataset, and read the minimum support
		return NULL;

	long len = PyList_Size(pyDataset);
	long i, j;
	Data* sData;
	TransNode* sNode;
	sData = Dataset;
	for (i = 0; i < len; i++)
	{
		sData->trs = new TransNode;
		PyObject* trans;
		trans = PyList_GetItem(pyDataset, i);
		long len2 = PyList_Size(trans);
		sNode = sData->trs;
		sNode->len = len2;
		for (j = 0; j < len2; j++)
		{	
			PyObject *V = PyList_GetItem(trans, j);
			sNode->value = PyInt_AS_LONG(V);
			sNode->right = new TransNode;
			sNode = sNode->right;
		}
		sData->next = new Data;
		sData = sData->next;
	}

	fpgrowth(minsup);//using fpgrowth to mine the results.
	
	PyObject* pyOutlist = PyList_New(theResult->PatNum);

	FrePatRsult *sResult;
	i = 0;
	for (sResult = theResult; sResult->next != NULL; sResult = sResult->next)//transform the result to the Python List
	{
		PyObject *Pat = PyList_New(sResult->pat->len + 1);
		j = 0;
		FreqPat *sPat;
		for (sPat = sResult->pat; sPat->right != NULL; sPat = sPat->right)
		{
			Py_INCREF(Pat);
			PyList_SetItem(Pat, j, Py_BuildValue("i", sPat->value));
			j++;
		}
		Py_INCREF(Pat);
		PyList_SetItem(Pat, j, Py_BuildValue("i", sResult->pat->support));

		Py_INCREF(pyOutlist);
		PyList_SetItem(pyOutlist, i, Py_BuildValue("O", Pat));
		i++;
	}

	delete Dataset;
	delete theResult;
	return pyOutlist;
}

static PyMethodDef pyfpgrowthMethods[] = {
	{ "fpgrowth", pyfpgrowth, METH_VARARGS,
	"the fpgrowth algorithm\n\n\
	for example\n\
	fi=fpgrowth([[1,2,3],[1,4],...,[1,7,8]],0.2)\n\n\
	input <dataset> <input>\n\
		<dataset>:	a list of transactions, where every transaction is also\n\
					a list containing integers higher than 0\n\
		<input>: a double value\n\n\
	output <freq-itemsets>\n\
		<freq-itemsets>:a list of frequent itemsets, where every itemset is also\n\
						a list, whose last value is its absolute support\
	"},
	{ NULL, NULL, 0, NULL }        /* Sentinel */
};

PyMODINIT_FUNC
initpyfpgrowth(void)
{
	PyObject *m;

	m = Py_InitModule("pyfpgrowth", pyfpgrowthMethods);
	if (m == NULL)
		return;

	pyfpgrowthError = PyErr_NewException("pyfpgrowth.error", NULL, NULL);
	Py_INCREF(pyfpgrowthError);
	PyModule_AddObject(m, "error", pyfpgrowthError);
}
