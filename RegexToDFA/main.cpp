#include <iostream>
#include <vector>
#include <string>
#include <algorithm>


using namespace std;


class Node
{
private:
	int number;
	char symbol;

	vector<int> firstPos;
	vector<int> lastPos;
	vector<int> followPos;
	bool nullable;
	bool leaf;
	
public:
	Node* parent, * left, * right;

	Node(char symbol, bool leaf = false)
	{
		this->symbol = symbol;
		parent = left = right = NULL;
		this->leaf = leaf;
	}

	void setNumber(int n)
	{
		number = n;
	}

	bool isLeaf()
	{
		return leaf;
	}

	#pragma region Functii nullable

	void computeNullable()
	{
		// Caz frunza
		if (leaf)
		{
			nullable = false;
			return;
		}

		if (symbol == '|')
		{
			// compute recursiv
			left->computeNullable();
			right->computeNullable();

			nullable = left->getNullable() || right->getNullable();
		}

		if (symbol == '.')
		{
			// compute recursiv
			left->computeNullable();
			right->computeNullable();

			nullable = left->getNullable() && right->getNullable();
		}

		if (symbol == '*')
		{
			// compute recursiv
			left->computeNullable();

			nullable = true;
		}
	}

	bool getNullable()
	{
		return nullable;
	}

	#pragma endregion
	
	#pragma region Functii firstpos

	void computeFirstPos()
	{
		firstPos.clear();

		if (leaf)
		{
			firstPos.push_back(number);
		}

		if (symbol == '|')
		{
			// compute recursiv
			left->computeFirstPos();
			right->computeFirstPos();

			vector<int> stanga = left->getFirstPos(), dreapta = right->getFirstPos();
			firstPos.insert(firstPos.end(), stanga.begin(), stanga.end());
			firstPos.insert(firstPos.end(), dreapta.begin(), dreapta.end());
		}

		if (symbol == '.')
		{
			// compute recursiv
			left->computeFirstPos();
			right->computeFirstPos();

			vector<int> stanga = left->getFirstPos(), dreapta = right->getFirstPos();
			if (left->nullable)
			{
				firstPos.insert(firstPos.end(), stanga.begin(), stanga.end());
				firstPos.insert(firstPos.end(), dreapta.begin(), dreapta.end());
			}
			else
			{
				firstPos.insert(firstPos.end(), stanga.begin(), stanga.end());
			}
		}

		if (symbol == '*')
		{
			// compute recursiv
			left->computeFirstPos();

			vector<int> stanga = left->getFirstPos();
			firstPos.insert(firstPos.end(), stanga.begin(), stanga.end());
		}
	}

	vector<int> getFirstPos()
	{
		return firstPos;
	}

	#pragma endregion

	#pragma region Functii lastpos

	void computeLastPos()
	{
		lastPos.clear();

		if (leaf)
		{
			lastPos.push_back(number);
		}

		if (symbol == '|')
		{
			// compute recursiv
			left->computeLastPos();
			right->computeLastPos();

			vector<int> stanga = left->getLastPos(), dreapta = right->getLastPos();
			lastPos.insert(lastPos.end(), stanga.begin(), stanga.end());
			lastPos.insert(lastPos.end(), dreapta.begin(), dreapta.end());
		}

		if (symbol == '.')
		{
			// compute recursiv
			left->computeLastPos();
			right->computeLastPos();

			vector<int> stanga = left->getLastPos(), dreapta = right->getLastPos();
			if (right->nullable)
			{
				lastPos.insert(lastPos.end(), stanga.begin(), stanga.end());
				lastPos.insert(lastPos.end(), dreapta.begin(), dreapta.end());
			}
			else
			{
				lastPos.insert(lastPos.end(), dreapta.begin(), dreapta.end());
			}
		}

		if (symbol == '*')
		{
			// compute recursiv
			left->computeLastPos();

			vector<int> stanga = left->getLastPos();
			lastPos.insert(lastPos.end(), stanga.begin(), stanga.end());
		}
	}

	vector<int> getLastPos()
	{
		return lastPos;
	}

	#pragma endregion
};

enum TipToken
{
	caracter,
	text,
	star,
	textstar,
	concat,
	orr,
	error
};

class AuxToken
{
public:
	string textRamas;
	string token;
	TipToken tipToken;

	AuxToken(string text, string tok, TipToken tip)
	{
		textRamas = text;
		token = tok;
		tipToken = tip;
	}
};

AuxToken getToken(string s)
{
	// Caz operator
	if ( s[0] == '|')
	{
		return AuxToken(s.substr(1), string(1, s[0]), TipToken::orr);
	}
	
	if (s[0] == '*')
	{
		return AuxToken(s.substr(1), string(1, s[0]), TipToken::star);
	}
	if (s[0] == '.')
	{
		return AuxToken(s.substr(1), string(1, s[0]), TipToken::concat);
	}

	// verificare caracter
	string caractere = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ#";
	if (caractere.find(s[0]) != string::npos)
	{
		// Verificare token la putere
		if (s.length() > 1)
		{
			if (s[1] == '*')
			{
				return AuxToken(s.substr(2), s.substr(0, 1), TipToken::textstar);
			}
		}
		return AuxToken(s.substr(1), string(1, s[0]), TipToken::caracter);
	}

	if (s[0] == '(')
	{
		// Cautam tot pana la urmatoarea paranteza
		int poz = s.find(')');

		if (string::npos == poz)
		{
			cerr << "Expresia regulata este gresita";
			return AuxToken("","",TipToken::error);
		}

		string bucata = s.substr(1, poz - 1);

		// Verificare token urmator
		if (s.length() > poz+1)
		{
			if (s[poz+1] == '*')
			{
				return AuxToken(s.substr(poz+2), bucata, TipToken::textstar);
			}
		}

		return AuxToken(s.substr(poz+1), bucata,TipToken::text);
	}

	return AuxToken("", "", TipToken::error);
}

Node* stringLaArbore(string s)
{
	// Ultimul caracter
	if (s.length() == 1)
	{
		AuxToken  tok = getToken(s);
		if (tok.tipToken == TipToken::caracter)
		{
			// Caz frunza
			return new Node(s[0], true);
		}

		// Eroare
		return NULL;
	}

	// Luam nodul de stanga(obligatoriu)
	Node* stanga = NULL;
	AuxToken  tok = getToken(s);
	if (tok.tipToken == TipToken::caracter  || tok.tipToken == TipToken::text)
	{
		// Caz frunza
		stanga = stringLaArbore(tok.token);
		
		// Verificam caz eroare
		if (stanga == NULL)
			return NULL;

		s = tok.textRamas;
	}
	else if (tok.tipToken == TipToken::textstar)
	{
		stanga = new Node('*');
		Node* copil = stringLaArbore(tok.token);
		if (copil == NULL)
			return NULL;

		copil->parent = stanga;
		stanga->left = copil;

		s = tok.textRamas;
	}
	else
	{
		return NULL;
	}


	while (s.length() > 0)
	{
		AuxToken token = getToken(s);

		if (token.tipToken == TipToken::error)
		{
			return NULL;
		}

		// Luam operatorul
		Node* op = NULL;
		if (token.tipToken == TipToken::concat || token.tipToken == TipToken::orr)
		{
			op = new Node(token.token[0]);
			s = token.textRamas;
		}
		else
		{
			return NULL;
		}

		// Luam partea dreapta
		Node* dreapta = NULL;
		token = getToken(s);
		if (token.tipToken == TipToken::caracter || token.tipToken == TipToken::text)
		{
			// Caz frunza
			dreapta = stringLaArbore(token.token);

			// Verificam caz eroare
			if (dreapta == NULL)
				return NULL;

			s = token.textRamas;
		}
		else if (token.tipToken == TipToken::textstar)
		{
			dreapta = new Node('*');
			Node* copil = stringLaArbore(token.token);
			if (copil == NULL)
				return NULL;

			copil->parent = dreapta;
			dreapta->left = copil;

			s = token.textRamas;
		}
		else
		{
			return NULL;
		}
		

		// Legaturi intre noduri
		op->left = stanga;
		op->right = dreapta;
		stanga->parent = dreapta->parent = op;

		// Actualizam cine e stanga
		stanga = op;
	}

	return stanga;
}

int setareNumarFrunze(Node* radacina, int n)
{
	if (radacina == NULL)
		return n;

	if (radacina->isLeaf())
	{
		radacina->setNumber(n);
		return n + 1;
	}

	n = setareNumarFrunze(radacina->left, n);
	n = setareNumarFrunze(radacina->right, n);

	return n;
}

void rexToDFA(string rex)
{

	// Adaugare caracter # la final
	rex.append("#");

	// inlocuire + cu |
	replace(rex.begin(), rex.end(), '+', '|');

	// Adaugare .
	for (int i = 0; i < rex.length() - 1; i++)
	{
		if (rex[i + 1] != '*' && rex[i + 1] != ')' && rex[i + 1] != '|' && rex[i] != '|' && rex[i] != '(')
		{
			rex.insert(rex.begin()+i+1, '.');
			i++;
		}
	}

	// construire arbore
	Node* root = stringLaArbore(rex);

	// Dam numarul frunzelor
	setareNumarFrunze(root, 1);

	// Calcule pe aarbore
	root->computeNullable();
	root->computeFirstPos();
	root->computeLastPos();

}


int main()
{

	string rex = "ba(a+b)*ab";

	rexToDFA(rex);

	return 0;

}