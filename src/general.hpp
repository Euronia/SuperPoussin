#ifndef GENERAL_HPP
#define GENERAL_HPP


struct Position
{
	int x;
	int y;

	Position() { x = 0; y = 0; }
	Position(int x1, int y1) { x = x1; y = y1; }
	bool operator==(Position pos2) { return x == pos2.x && y == pos2.y; }
	Position droite() { return Position(x+1, y); }
	Position gauche() { return Position(x-1, y); }
	Position haut() { return Position(x, y+1); }
	Position bas() { return Position(x, y-1); }
};

enum DIRECTION
{
	DIRECTION_DROITE=0,
	DIRECTION_GAUCHE,
	DIRECTION_HAUT,
	DIRECTION_BAS
};


template <int L, typename T>
class Tableau
{
private:
    T items[L];
    int num;

public:
    void init()
    {
        num = 0;
    }

    void ajouter(T item)
    {
        if(num < L)
            items[num++] = item;
    }

    void enlever(int index)
    {
        for(int i = index; i < num-1; i++)
            items[i] = items[i+1];
        num--;
    }

	T &get(int index)
	{
		return items[index];
	}

    T &operator[](int index)
    {
        return get(index);
    }

    int taille() const
    {
        return num;
    }
};


#endif
