enum IDs
{
	PLAYER,
	ENEMY,
	PLATFORM,
};

struct Player
{
	int ID;
	int Health;
	int Speed;
	int Score;
	bool OnSurface;
	bool Attacking;
	bool EnemyAttacking;
	int DPS;
	bool Falling;

	float posX;
	float posY;

	float velX;
	float velY;

	int boundY;
	int boundX;

	int maxFrame;
	int curFrame;
	int frameCount;
	int frameDelay;
	int frameWidth;
	int frameHeight;
	int animationColumns;
	int animationDirection;

	int animationRow;

	int MidX;
	int MidY;

	ALLEGRO_BITMAP* image;

	ALLEGRO_TIMER* timer;
};

struct Background
{
	int posX;
	int posY;

	int width;
	int height;

	ALLEGRO_BITMAP * image;
};

struct Platform
{
	int posX1;
	int posY1;

	int boundX;
	int boundY;

	bool live;

	bool collidable;

	ALLEGRO_BITMAP * image;
};

struct Arrow
{
	int posx;
	int posy;
	bool live;
	int boundx;
	int boundy;
	int speed;

	ALLEGRO_BITMAP * image;
};

struct Boss
{
	int posx;
	int posy;
	int boundx;
	int boundy;
	int health;
	bool collided;

	int MidX;
	int MidY;

	ALLEGRO_BITMAP * image;
};
struct Enemy
{
	int ID;
	int Speed;
	int DPS;
	bool live;

	float posX;
	float posY;

	int boundY;
	int boundX;

	int maxFrame;
	int curFrame;
	int frameCount;
	int frameDelay;
	int frameWidth;
	int frameHeight;
	int animationColumns;
	int animationDirection;

	int animationRow;

	int MidX;
	int MidY;

	ALLEGRO_BITMAP * image;
};