
//allergo includes
#include <allegro5\allegro.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_image.h>
#include <allegro5\allegro_native_dialog.h>
#include <math.h>
#include <iostream>

//game specific includes
#include "objects.h"

//gloabls
const int WIDTH = 1000;
const int HEIGHT = 600;

const int NumPlatforms = 10;
const int NumArrows = 5;
const int NumEnemies = 8;
int prevState;
bool paused;

bool keys[] = { false, false, false, false, false };
enum KEYS{ UP, DOWN, LEFT, RIGHT, SPACE };
enum STATE{ TITLE, LvlOne, LvlTwo, LvlThree, BossFight, LOST, WON };
enum PLATPOSBOSS{BOTTOM, BOTLEFT,BOTRIGHT,MID,TOPLEFT,TOPRIGHT};

Platform Platforms[NumPlatforms];
Arrow Arrows[NumArrows];
Enemy Enemies[NumEnemies];

int state = -1;
ALLEGRO_FONT* myFont;

//object variables
Player MainChar;
Background BG;
Boss MainBoss;

//prototypes
void InitPlayer(Player &MainChar, ALLEGRO_BITMAP *MainCharImage);
void ResetAnimation(Player &MainChar);
void PlayerJump(Player &MainChar);
void MoveLeft(Player &MainChar);
void MoveRight(Player &MainChar);
void DrawPlayer(Player &MainChar);
void UpdatePlayer(Player &MainChar);

void DrawHUD(Player &MainChar);

void InitBackgrounds(Background &BG, int posX, int posY, int Height, int Width, ALLEGRO_BITMAP * image);
void DrawBackground(Background &BG);

void ChangeState(int &state, int newState);

void InitPlaformImages(Platform Platforms[], int NumPlat, ALLEGRO_BITMAP * Image);
void InitPlatforms(int posX, int posY, int boundX, int boundY, Platform Platforms[], int NumPlat, bool collideable);
void DrawPlatforms(Platform Platforms[], int NumPlatforms);
void PlaformPlayerCollison(Player &MainChar, Platform Platforms[], int NumPlatforms);

void InitBoss(Boss &MainBoss, ALLEGRO_BITMAP *BossImage);
void DrawBoss(Boss &MainBoss);
void BossCollision(Boss &MainBoss, Player &MainChar);
void BossBlastback(Player &MainChar);

void InitArrowImages(Arrow Arrows[], int NumArrows, ALLEGRO_BITMAP *ArrowImage);
void InitArrows(Arrow Arrows[], int NumArrows);
void DrawArrows(Arrow Arrows[], int NumArrows);
void FireArrows(Arrow Arrows[], int NumArrows, Player &MainChar);
void ArrowCollison(Arrow Arrows[], int NumArrows, Player &MainChar);

void InitEnemies(Enemy Enemies[], int NumEnemies, ALLEGRO_BITMAP *EnemyPic);
void DrawEnemies(Enemy Enemies[], int NumEnemies);
void StartEnemies(Enemy Enemies[], int NumEnemies);
void UpdateEnemies(Enemy Enemies[], int NumEnemeies, Platform Platforms[]);
void EnemyCollision(Enemy Enemies[], int NumEnemies, Player &MainChar);

int main(int argc, char **argv)
{
	//============================================== 
	//SHELL VARIABLES 
	//============================================== 
	bool done = false;
	bool render = false;
	float gameTime = 0;
	int frames = 0;
	int gameFPS = 0;
	paused = false;
	srand(time(NULL));

	//============================================== 
	//PROJECT VARIABLES 
	//==============================================


	//============================================== 
	//ALLEGRO VARIABLES
	//============================================== 

	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer;
	ALLEGRO_FONT *font18;
	ALLEGRO_BITMAP *MainCharImage;
	ALLEGRO_BITMAP *BGImage;
	ALLEGRO_BITMAP *Girder;
	ALLEGRO_BITMAP * BossImage;
	ALLEGRO_BITMAP * EnemyPic;
	ALLEGRO_TIMER *BlastbackTimer;
	ALLEGRO_BITMAP *TitleImage;
	ALLEGRO_BITMAP *LostImage;
	ALLEGRO_BITMAP *WonImage;
	ALLEGRO_BITMAP *ArrowImage;

	//============================================== 
	//ALLEGRO INIT FUNCTIONS 
	//==============================================
	if (!al_init())
	{
		//initialize Allegro
		return -1;
	}
	display = al_create_display(WIDTH, HEIGHT);
	//create our display object 
	if (!display)
	{
		//test display object 
		return -1;
	}


	//============================================== 
	//ADDON INSTALL 
	//==============================================
	al_install_keyboard();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();


	//============================================== 
	//PROJECT INIT 
	//============================================== 
	font18 = al_load_font("arial.ttf", 18, 0);
	myFont = font18;
	MainCharImage = al_load_bitmap("character.png");
	BGImage = al_load_bitmap("BackgroundSkyline2.png");
	Girder = al_load_bitmap("GirderGame.png");
	BossImage = al_load_bitmap("Boss.bmp");
	al_convert_mask_to_alpha(BossImage, al_map_rgb(105, 74, 46));
	al_convert_mask_to_alpha(BossImage, al_map_rgb(33, 23, 14));
	EnemyPic = al_load_bitmap("Enemies.png");
	TitleImage = al_load_bitmap("Intro.png") ;
	LostImage = al_load_bitmap("LostScreen.png");
	WonImage = al_load_bitmap("WonScreen.png");
	ArrowImage = al_load_bitmap("Arrow.png");

	ChangeState(state, TITLE);

	InitPlayer(MainChar, MainCharImage);

	InitBackgrounds(BG, 0, 0, 1080, 1920, BGImage);

	InitPlaformImages(Platforms, NumPlatforms, Girder);

	InitEnemies(Enemies, NumEnemies, EnemyPic);

	InitBoss(MainBoss, BossImage);

	InitArrowImages(Arrows, NumArrows, ArrowImage);

	InitArrows(Arrows, NumArrows);

	//============================================== 
	//TIMER INIT AND STARTUP 
	//============================================== 
	event_queue = al_create_event_queue();
	timer = al_create_timer(1.0 / 60);
	BlastbackTimer = al_create_timer(1.0);


	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_timer_event_source(BlastbackTimer));
	al_register_event_source(event_queue, al_get_timer_event_source(MainChar.timer));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_display_event_source(display));
	


	al_start_timer(BlastbackTimer);
	al_start_timer(timer);
	gameTime = al_current_time();
	while (!done)
	{
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);


		//============================================== 
		//INPUT 
		//============================================== 
		if (ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			switch (ev.keyboard.keycode)
			{
			case ALLEGRO_KEY_ESCAPE:
				done = true;
				break;
			case ALLEGRO_KEY_LEFT:
				keys[LEFT] = true;
				break;
			case ALLEGRO_KEY_RIGHT:
				keys[RIGHT] = true;
				break;
			case ALLEGRO_KEY_UP:
				keys[UP] = true;
				PlayerJump(MainChar);
				break;
			case ALLEGRO_KEY_DOWN:
				keys[DOWN] = true;
				break;

			case ALLEGRO_KEY_P:
				if (paused)
				{
					paused = false;
				}
				else
				{
					paused = true;
				}

			case ALLEGRO_KEY_SPACE:
				keys[SPACE] = true;
				if (state == TITLE)
				{
					ChangeState(state, LvlOne);
				}
				else if (state == LvlOne)
				{
					MainChar.EnemyAttacking = true;
				}
				else if (state == LvlTwo)
				{
					MainChar.EnemyAttacking = true;
				}
				else if (state == LvlThree)
				{
					MainChar.EnemyAttacking = true;
				}
				else if (state == BossFight)
				{
					MainChar.EnemyAttacking = true;
				}
				else if (state == LOST)
				{
					ChangeState(state, prevState);
				}
				else if (state == WON)
				{
					ChangeState(state, TITLE);
				}
				break;
			}
		}
		else if (ev.type == ALLEGRO_EVENT_KEY_UP)
		{
			switch (ev.keyboard.keycode)
			{
			case ALLEGRO_KEY_ESCAPE:
				done = true;
				break;
			case ALLEGRO_KEY_LEFT:
				keys[LEFT] = false;
				break;
			case ALLEGRO_KEY_RIGHT:
				keys[RIGHT] = false;
				break;
			case ALLEGRO_KEY_UP:
				keys[UP] = false;
				break;
			case ALLEGRO_KEY_DOWN:
				keys[DOWN] = false;
				break;
			case ALLEGRO_KEY_SPACE:
				keys[SPACE] = false;
				MainChar.EnemyAttacking = false;
				break;
			}
		}

		else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			done = true;
		}


		//============================================== 
		//GAME UPDATE 
		//============================================== 
		else if (ev.type == ALLEGRO_EVENT_TIMER)
		{
			if (!paused)
			{
			if (ev.timer.source == BlastbackTimer)
			{
				if (MainChar.Attacking)
				{
					BossBlastback(MainChar);
				}
			}

			else
			{
				//movement checks 

				if (MainChar.velX == 0)
				{
					if (keys[LEFT])
					{
						MoveLeft(MainChar);
					}
					else
						if (keys[RIGHT])
						{
						MoveRight(MainChar);
						}
						else ResetAnimation(MainChar);
				}

				//Plug in updates here

				if (ev.timer.source == MainChar.timer)
				{
					al_stop_timer(MainChar.timer);
					MainChar.EnemyAttacking = false;
				}

				if (state == TITLE)
				{

				}
				else if (state == LvlOne)
				{
					UpdatePlayer(MainChar);
					PlaformPlayerCollison(MainChar, Platforms, NumPlatforms);
					UpdateEnemies(Enemies, NumEnemies, Platforms);
					EnemyCollision(Enemies, NumEnemies, MainChar);

					//check to see if player has reached goal
					if (MainChar.posY <= HEIGHT / 2 - 220
						&& MainChar.posX <= WIDTH / 2 - 220)
					{
						ChangeState(state, LvlTwo);
					}
				}
				else if (state == LvlTwo)
				{
					UpdatePlayer(MainChar);
					PlaformPlayerCollison(MainChar, Platforms, NumPlatforms);
					UpdateEnemies(Enemies, NumEnemies, Platforms);
					EnemyCollision(Enemies, NumEnemies, MainChar);

					//cheking if player has killed enemies

					if (MainChar.Score == 30)
					{
						ChangeState(state, LvlThree);
					}
				}
				else if (state == LvlThree)
				{
					UpdatePlayer(MainChar);
					PlaformPlayerCollison(MainChar, Platforms, NumPlatforms);
					UpdateEnemies(Enemies, NumEnemies, Platforms);
					EnemyCollision(Enemies, NumEnemies, MainChar);

					//seeing if player has reached goal of this level
					StartEnemies(Enemies, NumEnemies);
					if (MainChar.Score >= 250)
					{
						ChangeState(state, BossFight);
					}
				}
				else if (state == BossFight)
				{

					UpdatePlayer(MainChar);
					PlaformPlayerCollison(MainChar, Platforms, NumPlatforms);
					UpdateEnemies(Enemies, NumEnemies, Platforms);
					EnemyCollision(Enemies, NumEnemies, MainChar);

					StartEnemies(Enemies, NumEnemies);

					int random = rand() % 150;
					if (random == 0)
					{
						FireArrows(Arrows, NumArrows, MainChar);
					}

					ArrowCollison(Arrows, NumArrows, MainChar);

					BossCollision(MainBoss, MainChar);
					if (MainBoss.health <= 0)
					{
						ChangeState(state, WON);
					}
				}
				else if (state == LOST)
				{

				}
				else if (state == WON)
				{

				}

				if (MainChar.Health <= 0)
				{
					ChangeState(state, LOST);
				}
			}


					render = true;
					//UPDATE FPS=========== 
					frames++; if (al_current_time() - gameTime >= 1)
					{
						gameTime = al_current_time();
						gameFPS = frames;
						frames = 0;
					}



					//===================== 
			}
		}

		//============================================== 
		//RENDER 
		//============================================== 
		if (render && al_is_event_queue_empty(event_queue))
		{
			render = false;


			//al_draw_textf(font18, al_map_rgb(255, 0, 255), 5, 5, 0, "FPS: %i", gameFPS);
			//display FPS on screen 


			//BEGIN PROJECT RENDER================ 

			DrawBackground(BG);
			
			if (state == TITLE)
			{
				al_draw_bitmap(TitleImage, 0, 0, 0);
			}
			else if (state == LvlOne)
			{
				DrawPlatforms(Platforms, NumPlatforms);
				DrawPlayer(MainChar);
				DrawHUD(MainChar);
				al_draw_text(myFont, al_map_rgb(0, 0, 0), WIDTH / 2 -245, HEIGHT / 2 - 220, 0, "Reach here!");
				al_draw_text(myFont, al_map_rgb(0, 0, 0), WIDTH/2 - 100, 30, 0, "Use the arrow keys to move.");
				al_draw_text(myFont, al_map_rgb(0, 0, 0), WIDTH / 2 - 100, HEIGHT / 2 - 80, 0, "You can press P to pause the game");
			}
			else if (state == LvlTwo)
			{
				DrawPlatforms(Platforms, NumPlatforms);
				DrawPlayer(MainChar);
				DrawHUD(MainChar);
				DrawEnemies(Enemies, NumEnemies);
				al_draw_text(myFont, al_map_rgb(0, 0, 0), 30, 70, 0, "Use space to kill enemies, Hold space to kill multiple.");
				al_draw_text(myFont, al_map_rgb(0, 0, 0), 30, 90, 0, "Try killing all the enemies on screen.");
				al_draw_text(myFont, al_map_rgb(0, 0, 0), 30, 110, 0, "You can also jump on enemies to kill them.");
			}
			else if (state == LvlThree)
			{
				DrawPlatforms(Platforms, NumPlatforms);
				DrawPlayer(MainChar);
				DrawHUD(MainChar);
				DrawEnemies(Enemies, NumEnemies);
				al_draw_textf(myFont, al_map_rgb(0, 0, 0), WIDTH/2 - 100, 70, 0, "Only %i more points to go", 250 - MainChar.Score);
			}
			else if (state == BossFight)
			{
				DrawPlatforms(Platforms, NumPlatforms);
				DrawPlayer(MainChar);
				DrawHUD(MainChar);
				
				DrawBoss(MainBoss);
				DrawEnemies(Enemies, NumEnemies);
				DrawArrows(Arrows, NumArrows);
			}
			else if (state == LOST)
			{
				al_draw_bitmap(LostImage, 0, 0, 0);
				al_draw_textf(myFont, al_map_rgb(255, 255, 255), WIDTH / 2, HEIGHT / 2 , 0, "%i", MainChar.Score);
				al_draw_text(myFont, al_map_rgb(255, 255, 255), 10, HEIGHT / 2 + 300,0, "Press space to retry level");
			}
			else if (state == WON)
			{
				al_draw_bitmap(WonImage, 0, 0, 0);
				al_draw_textf(myFont, al_map_rgb(255, 255, 255), WIDTH / 2 , HEIGHT / 2 +50, 0, "%i", MainChar.Score);
			}


			//FLIP BUFFERS======================== 
			al_flip_display(); 
			al_clear_to_color(al_map_rgb(0, 0, 0));
		}
	}


	//============================================== 
	//DESTROY PROJECT OBJECTS 
	//============================================== 
	//SHELL OBJECTS=================================
	al_destroy_font(font18);
	al_destroy_timer(timer);
	al_destroy_event_queue(event_queue);
	al_destroy_display(display);
	al_destroy_bitmap(MainCharImage);
	al_destroy_bitmap(BGImage);
	al_destroy_bitmap(Girder);
	al_destroy_bitmap(BossImage);
	al_destroy_bitmap(LostImage);
	al_destroy_bitmap(WonImage);
	al_destroy_bitmap(TitleImage);
	al_destroy_timer(MainChar.timer);
	al_destroy_bitmap(ArrowImage);
	return 0;
}

void InitPlayer(Player &MainChar, ALLEGRO_BITMAP *MainCharImage = NULL)
{
	MainChar.ID = PLAYER;
	MainChar.Health = 100;
	MainChar.posX = WIDTH / 2;
	MainChar.posY = HEIGHT - 32;
	MainChar.boundX = 15;
	MainChar.boundY = 16;
	MainChar.Score = 0;
	MainChar.velX = 0;
	MainChar.velY = 0;
	MainChar.Speed = 5;
	MainChar.Attacking = false;
	MainChar.EnemyAttacking = false;
	MainChar.DPS = 10;


	MainChar.animationRow = 0;
	MainChar.animationDirection = 1;
	MainChar.maxFrame = 3;
	MainChar.curFrame = 1;
	MainChar.frameCount = 0;
	MainChar.animationColumns = 3;
	MainChar.frameDelay = 15;
	MainChar.frameWidth = 32;
	MainChar.frameHeight = 32;
	MainChar.OnSurface = true;

	MainChar.MidX =(WIDTH / 2) + (32/2);
	MainChar.MidY =(HEIGHT - 32) + (32 / 2);

	if (MainCharImage != NULL)
	{
		MainChar.image = MainCharImage;
	}

	MainChar.timer = al_create_timer(0.45);

}
void ResetAnimation(Player &MainChar)
{
	MainChar.curFrame = 1;
	MainChar.animationRow = 0;
	
}
void PlayerJump(Player &MainChar)
{
	if (MainChar.OnSurface)
	{
		MainChar.velY += 8;
	}
	MainChar.OnSurface = false;
}
void MoveLeft(Player &MainChar)
{
	MainChar.posX -= MainChar.Speed;
	MainChar.MidX -= MainChar.Speed;
	if (MainChar.posX <= 0)
	{
		MainChar.posX  = 0;
		MainChar.MidX = 0 + MainChar.frameWidth / 2;
	}
	MainChar.animationRow = 1;
}
void MoveRight(Player &MainChar)
{
	MainChar.posX += MainChar.Speed;
	MainChar.MidX += MainChar.Speed;
	if (MainChar.posX >= WIDTH - MainChar.frameWidth)
	{
		MainChar.posX = WIDTH - MainChar.frameWidth;
		MainChar.MidX = (WIDTH - MainChar.frameWidth) + MainChar.frameWidth/2 ;
	}
	MainChar.animationRow = 2;
}
void DrawPlayer(Player &MainChar)
{
	al_draw_bitmap_region(MainChar.image, 32 * MainChar.curFrame, 32 * MainChar.animationRow, 32, 32, MainChar.posX, MainChar.posY, 0);

	//bounding box check
	//al_draw_filled_rectangle(MainChar.MidX - MainChar.boundX, MainChar.MidY - MainChar.boundY, MainChar.MidX + MainChar.boundX, MainChar.MidY + MainChar.boundY, al_map_rgba(255, 0, 255,100));

	if (++MainChar.frameCount >= MainChar.frameDelay)
	{
		MainChar.curFrame += MainChar.animationDirection;


		if (MainChar.curFrame >= MainChar.maxFrame)
		{

			MainChar.curFrame = 0;
		}

		else if (MainChar.curFrame <= 0)
		{
			MainChar.curFrame = MainChar.maxFrame - 1;
		}

		MainChar.frameCount = 0;
	}


}

void UpdatePlayer(Player &MainChar)
{
		MainChar.posY -= MainChar.velY;
		MainChar.MidY = MainChar.posY + (MainChar.frameHeight / 2);
		MainChar.velY -= 0.2;


		MainChar.posX += MainChar.velX;
		MainChar.MidX = MainChar.posX + (MainChar.frameWidth / 2);


		if (MainChar.velX != 0)
		{
			if (MainChar.velX < 0)
			{
				MainChar.velX += 0.5;
				if (MainChar.posX < 0)
				{
					MainChar.posX = 0;
					MainChar.MidX = MainChar.posX + MainChar.frameWidth / 2;
					MainChar.velY = 4;
				}
			}
			else 
			{
				MainChar.velX -= 0.5;
			}
		}
	if (MainChar.posY >= HEIGHT - MainChar.frameHeight)
	{
		MainChar.posY = HEIGHT - MainChar.frameHeight;
		MainChar.MidY = MainChar.posY + (MainChar.frameHeight / 2);
		MainChar.velY = 0;
		MainChar.OnSurface = true;
		MainChar.Falling = false;
	}

	if (MainChar.EnemyAttacking == true)
	{
		al_start_timer(MainChar.timer);
	}
}

void DrawHUD(Player &MainChar)
{	
	al_draw_text(myFont, al_map_rgb(255, 255, 255), 5, 5, 0, "Health:");
	al_draw_filled_rectangle(5, 30, 30 + (MainChar.Health*2), 50 , al_map_rgb(150, 0, 0));
	al_draw_rectangle(5, 30, 230, 50, al_map_rgb(0, 0, 0), 2);

	al_draw_textf(myFont, al_map_rgb(255, 255, 255), WIDTH - 120, 5, 0, "Score = %i", MainChar.Score);
}

void InitBackgrounds(Background &BG, int posX, int posY, int Height, int Width, ALLEGRO_BITMAP * image)
{
	BG.posX = posX;
	BG.posY = posY;
	BG.height = Height;
	BG.width = Width;
	BG.image = image;

}
void DrawBackground(Background &BG)
{
	al_draw_bitmap(BG.image, BG.posX, BG.posY, 0);
}

void ChangeState(int &state, int newState)
{

	//leaving state handling
	if (state == TITLE)
	{

	}
	else if (state == LvlOne)
	{
		prevState = LvlOne;
	}

	else if (state == LvlTwo)
	{
		prevState = LvlTwo;
	}

	else if (state == LvlThree)
	{
		prevState = LvlThree;
	}

	else if (state == BossFight)
	{
		prevState = BossFight;
	}

	else if (state == LOST)
	{

	}

	else if (state == WON)
	{

	}


	for (int i = 0; i < NumEnemies; i++)
	{
		Enemies[i].live = false;
	}

	for (int j = 0; j < NumPlatforms; j++)
	{
		Platforms[j].live = false;

	}

	//change to new state
	state = newState;


	//entering state commands

	if (state == TITLE)
	{

	}
	else if (state == LvlOne)
	{
		MainChar.posX = 0;
		InitPlatforms(WIDTH / 2 - 700 / 2, HEIGHT / 2 + 150, 245, 43, Platforms, NumPlatforms, true);
		InitPlatforms(WIDTH / 2 , HEIGHT / 2 +50, 245, 43, Platforms, NumPlatforms, true);
		InitPlatforms(WIDTH  - 245, HEIGHT / 2 - 100, 245, 43, Platforms, NumPlatforms, true);
		InitPlatforms(WIDTH / 2 - 500 / 2, HEIGHT /2 -200, 245, 43, Platforms, NumPlatforms, true);
	}
	else if (state == LvlTwo)
	{
		if (prevState == LvlOne)
		{
			al_clear_to_color(al_map_rgb(0, 0, 0));
			al_draw_text(myFont, al_map_rgb(255, 255, 255), WIDTH / 2 - 100, HEIGHT / 2, 0, "Congratulations");
			al_flip_display();
			al_rest(2.0);
		}

		InitPlayer(MainChar);
		InitPlatforms(WIDTH - 245, HEIGHT / 2 + 160, 245, 43, Platforms, NumPlatforms, true);
		InitPlatforms(0, HEIGHT / 2 + 160, 245, 43, Platforms, NumPlatforms, true);
		StartEnemies(Enemies, NumEnemies);
	}
	else if (state == LvlThree)
	{
		if (prevState == LvlTwo)
		{
			al_clear_to_color(al_map_rgb(0, 0, 0));
			al_draw_text(myFont, al_map_rgb(255, 255, 255), WIDTH / 2 - 100, HEIGHT / 2, 0, "You're making progress");
			al_flip_display();
			al_rest(2.0);
			al_clear_to_color(al_map_rgb(0, 0, 0));
			al_draw_text(myFont, al_map_rgb(255, 255, 255), WIDTH / 2 - 150, HEIGHT / 2, 0, "Now get a score of 250 by killing enemies");
			al_flip_display();
			al_rest(3.0);
		}

		InitPlayer(MainChar);
		InitPlatforms(WIDTH - 245, HEIGHT / 2 + 160, 245, 43, Platforms, NumPlatforms, true);
		InitPlatforms(0, HEIGHT / 2 + 160, 245, 43, Platforms, NumPlatforms, true);
		InitPlatforms(WIDTH / 2 - 245 / 2, HEIGHT / 2 + 50, 245, 43, Platforms, NumPlatforms, false);
		StartEnemies(Enemies, NumEnemies);

	}
	else if (state == BossFight)
	{
		if (prevState == LvlThree)
		{
			al_clear_to_color(al_map_rgb(0, 0, 0));
			al_draw_text(myFont, al_map_rgb(255, 255, 255), WIDTH / 2 - 100, HEIGHT / 2, 0, "You're making progress");
			al_flip_display();
			al_rest(2.0);
			al_clear_to_color(al_map_rgb(0, 0, 0));
			al_draw_text(myFont, al_map_rgb(255, 255, 255), WIDTH / 2 - 100, HEIGHT / 2, 0, "Next is simple, kill the Boss");
			al_flip_display();
			al_rest(3.0);
		}

		InitPlayer(MainChar);
		MainBoss.health = 200;

		InitArrows(Arrows, NumArrows);

		InitPlatforms(0, HEIGHT / 2 + 160, 245, 43, Platforms, NumPlatforms, true);
		InitPlatforms(WIDTH - 245, HEIGHT / 2 + 160, 245, 43, Platforms, NumPlatforms, true);
		InitPlatforms(WIDTH / 2 - 245 / 2, HEIGHT / 2 + 50, 245, 43, Platforms, NumPlatforms, false);

		InitPlatforms(0, HEIGHT / 2 - 80, 245, 43, Platforms, NumPlatforms, false);
		InitPlatforms(WIDTH - 245, HEIGHT / 2 - 80, 245, 43, Platforms, NumPlatforms, false);

		InitPlatforms(WIDTH / 2 - 245 / 2, 30 + 85, 245, 43, Platforms, NumPlatforms, false);

		StartEnemies(Enemies, NumEnemies);
	}
	else if (state == LOST)
	{

	}
	else if (state == WON)
	{

	}
}


void InitPlaformImages(Platform Platforms[], int NumPlat, ALLEGRO_BITMAP * Image)
{
	for (int i = 0; i < NumPlat; i++)
	{
		Platforms[i].image = Image;
	}
}
void InitPlatforms(int posX, int posY, int boundX, int boundY, Platform Platforms[], int NumPlat, bool collidable)
{
	for (int i = 0; i < NumPlatforms; i++)
	{
		if (Platforms[i].live == false)
		{
			Platforms[i].posX1 = posX;
			Platforms[i].posY1 = posY;
			Platforms[i].boundX = boundX;
			Platforms[i].boundY = boundY;
			Platforms[i].live = true;
			Platforms[i].collidable = collidable;

			break;


		}
	}
}
void DrawPlatforms(Platform Platforms[], int NumPlatforms)
{
	for (int i = 0; i < NumPlatforms; i++)
	{
		if (Platforms[i].live == true)
		{
			al_draw_bitmap(Platforms[i].image, Platforms[i].posX1, Platforms[i].posY1, 0);

			//bounding box check
			//al_draw_filled_rectangle(Platforms[i].posX1, Platforms[i].posY1, Platforms[i].posX1 + Platforms[i].boundX, Platforms[i].posY1 + Platforms[i].boundY, al_map_rgba(255, 0, 255, 100));
		}
	}
}


void PlaformPlayerCollison(Player &MainChar, Platform Platforms[], int NumPlatforms)
{
	for (int i = 0; i < NumPlatforms; i++)
	{
		if (Platforms[i].live)
		{
			if (MainChar.Falling == true)
			{
				if (Platforms[i].collidable == true)
				{
					if (MainChar.MidY + MainChar.boundY > Platforms[i].posY1 && MainChar.MidY + MainChar.boundY < Platforms[i].posY1 + Platforms[i].boundY / 2)
					{
						MainChar.posY = Platforms[i].posY1 - MainChar.frameHeight;
						MainChar.MidY = MainChar.posY + MainChar.frameHeight / 2;
						MainChar.OnSurface = true;
						MainChar.Falling = false;
						MainChar.velY = 0;
					}
				}
			}

			else {

				if (MainChar.MidY + MainChar.boundY > Platforms[i].posY1 &&
					MainChar.MidY - MainChar.boundY < Platforms[i].posY1 + Platforms[i].boundY &&
					MainChar.MidX + MainChar.boundX > Platforms[i].posX1 &&
					MainChar.MidX - MainChar.boundX < Platforms[i].posX1 + Platforms[i].boundX)
				{
					//for debugging
					//al_draw_text(myFont, al_map_rgb(0, 0,0), WIDTH/2, HEIGHT/2, 0, "Collision");

					//checks above collisons
					if (MainChar.MidY + MainChar.boundY > Platforms[i].posY1 && MainChar.MidY + MainChar.boundY < Platforms[i].posY1 + Platforms[i].boundY / 2)
					{
						MainChar.posY = Platforms[i].posY1 - MainChar.frameHeight;
						MainChar.MidY = MainChar.posY + MainChar.frameHeight / 2;
						MainChar.OnSurface = true;
						MainChar.velY = 0;
					}

					//check below collisions
					else if (MainChar.MidY - MainChar.boundY < Platforms[i].posY1 + Platforms[i].boundY &&
						MainChar.MidY - MainChar.boundY > Platforms[i].posY1 + Platforms[i].boundY / 2)
					{
						MainChar.posY = Platforms[i].posY1 + Platforms[i].boundY;
						MainChar.MidY = MainChar.posY - MainChar.frameHeight / 2;
						MainChar.velY = 0;
					}

					//check right collisions
					else if (MainChar.MidX - MainChar.boundX < Platforms[i].posX1 + Platforms[i].boundX && MainChar.MidX > Platforms[i].posX1 + Platforms[i].boundX / 2)
					{
						MainChar.posX = Platforms[i].posX1 + Platforms[i].boundX;
						MainChar.MidX = MainChar.posX + MainChar.frameWidth / 2;
					}

					//checks left collisions
					else if (MainChar.MidX + MainChar.boundX < Platforms[i].posX1)
					{
						MainChar.posX = Platforms[i].posX1 - MainChar.frameWidth;
						MainChar.MidX = MainChar.posX - MainChar.frameWidth / 2;
					}
				}

			}

		}
	}
}

void InitBoss(Boss &MainBoss, ALLEGRO_BITMAP *BossImage)
{
	MainBoss.posx = WIDTH / 2 - 96/2;
	MainBoss.posy = 30;
	MainBoss.boundx = 15;
	MainBoss.boundy = 96/2 - 13;
	MainBoss.MidX = WIDTH / 2 - 48 + 96 / 2;
	MainBoss.MidY = 30 + 96/2;
	MainBoss.health = 200;

	MainBoss.image = BossImage;
}
void DrawBoss(Boss &MainBoss)
{
	al_draw_bitmap(MainBoss.image, MainBoss.posx, MainBoss.posy,0);
	al_draw_filled_rectangle(MainBoss.posx , MainBoss.posy, MainBoss.posx + (MainBoss.health/2), MainBoss.posy + 10, al_map_rgb(255,0,0));
	al_draw_rectangle(MainBoss.posx , MainBoss.posy, MainBoss.posx  + 100, MainBoss.posy + 10, al_map_rgb(0, 0, 0),3);

	//boundingboxcheck
	//al_draw_filled_rectangle(MainBoss.MidX - MainBoss.boundx, MainBoss.MidY - MainBoss.boundy, MainBoss.MidX + MainBoss.boundx, MainBoss.MidY + MainBoss.boundy, al_map_rgba(255, 0, 255, 100));
}
void BossCollision(Boss &MainBoss, Player &MainChar)
{
	if (MainChar.MidX + MainChar.boundX > MainBoss.MidX - MainBoss.boundx
		&&
		MainChar.MidX - MainChar.boundX < MainBoss.MidX + MainBoss.boundx
		&&
		MainChar.MidY + MainChar.boundY > MainBoss.MidY - MainBoss.boundy
		&&
		MainChar.MidY - MainChar.boundY < MainBoss.MidY + MainBoss.boundy)
	{
		MainChar.Attacking = true;

		if (keys[SPACE])
		{
			MainBoss.health--;
			MainChar.Score++;
		}
	}
	else MainChar.Attacking = false;
}
void BossBlastback(Player &MainChar)
{
	if (MainChar.MidX + MainChar.boundX > MainBoss.MidX - MainBoss.boundx &&
		MainChar.MidX + MainChar.boundX < MainBoss.MidX + MainBoss.boundx)
	{
		MainChar.velX -= 14;
		MainChar.Falling = true;
	}
	else if (MainChar.MidX - MainChar.boundX < MainBoss.MidX + MainBoss.boundx &&
		MainChar.MidX - MainChar.boundX > MainBoss.MidX - MainBoss.boundx)

	{
		MainChar.velX += 14;
		MainChar.Falling = true;
	}

}

void InitEnemies(Enemy Enemies[], int NumEnemies, ALLEGRO_BITMAP *EnemyPic)
{
	for (int i = 0; i < NumEnemies; i++)
	{
		Enemies[i].animationColumns = 15;
		Enemies[i].animationDirection = 1;
		Enemies[i].curFrame = 0;
		Enemies[i].frameCount = 0;
		Enemies[i].frameDelay = 3;
		Enemies[i].frameHeight = 64;
		Enemies[i].frameWidth = 64;
		Enemies[i].maxFrame = 15;
		Enemies[i].Speed = rand () % 3 + 1;
		Enemies[i].animationRow = 0;
		Enemies[i].boundX = 25;
		Enemies[i].boundY = 25;

		Enemies[i].image = EnemyPic;
	}
}
void DrawEnemies(Enemy Enemies[], int NumEnemies)
{
	for (int i = 0; i < NumEnemies; i++)
	{
		Enemies[i].ID;
		if (Enemies[i].live == true)
		{
			al_draw_bitmap_region(Enemies[i].image, 64 * Enemies[i].curFrame, 64 * Enemies[i].animationRow, 64, 64, Enemies[i].posX, Enemies[i].posY, 0);

			//bounding box check
			//al_draw_filled_rectangle(Enemies[i].MidX - Enemies[i].boundX, Enemies[i].MidY - Enemies[i].boundY, Enemies[i].MidX + Enemies[i].boundX, Enemies[i].MidY + Enemies[i].boundY, al_map_rgba(255, 0, 255, 100));

			if (++Enemies[i].frameCount >= Enemies[i].frameDelay)
			{
				Enemies[i].curFrame += Enemies[i].animationDirection;


				if (Enemies[i].curFrame >= Enemies[i].maxFrame)
				{

					Enemies[i].curFrame = 0;
				}

				else if (Enemies[i].curFrame <= 0)
				{
					Enemies[i].curFrame = Enemies[i].maxFrame - 1;
				}

				Enemies[i].frameCount = 0;
			}
		}
	}
}
void StartEnemies(Enemy Enemies[], int NumEnemies)
{

	for (int i = 0; i < NumEnemies; i++)
	{
		if (state == BossFight)
		{
			if (Enemies[i].live == false)
			{
				int selection = rand() % 6;
				if (selection == 0)
				{
					Enemies[i].ID = BOTTOM;
					Enemies[i].posX = 0;
					Enemies[i].posY = HEIGHT - 60;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth/2;
					Enemies[i].MidY = Enemies[i].posY + Enemies[i].frameHeight/2;
					Enemies[i].live = true;
				}
				else if (selection == 1)
				{
					Enemies[i].ID = BOTLEFT;
					Enemies[i].posX = 0;
					Enemies[i].posY = HEIGHT / 2 + 160 - 60;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth/2;
					Enemies[i].MidY = Enemies[i].posY + Enemies[i].frameHeight/2;
					Enemies[i].live = true;
				}
				else if (selection == 2)
				{
					Enemies[i].ID = BOTRIGHT;
					Enemies[i].posX = WIDTH - 245;
					Enemies[i].posY = HEIGHT / 2 + 160 - 60;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth/2;
					Enemies[i].MidY = Enemies[i].posY + Enemies[i].frameHeight/2;
					Enemies[i].live = true;
				}
				else if (selection == 3)            
				{
					Enemies[i].ID = MID;
					Enemies[i].posX = WIDTH / 2 - 245 / 2;
					Enemies[i].posY = HEIGHT / 2 + 50 - 60;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth/2;
					Enemies[i].MidY = Enemies[i].posY + Enemies[i].frameHeight/2;
					Enemies[i].live = true;
				}
				else if (selection == 4)
				{
					Enemies[i].ID = TOPLEFT;
					Enemies[i].posX = 0;
					Enemies[i].posY = HEIGHT / 2 - 80 - 60;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth/2;
					Enemies[i].MidY = Enemies[i].posY + Enemies[i].frameHeight/2;
					Enemies[i].live = true;
				}
				else if (selection == 5)
				{
					Enemies[i].ID = TOPRIGHT;
					Enemies[i].posX = WIDTH - 245;
					Enemies[i].posY = HEIGHT / 2 - 80 - 60;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth/2;
					Enemies[i].MidY = Enemies[i].posY + Enemies[i].frameHeight/2;
					Enemies[i].live = true;
				}
				else
				{

				}
				break;
			}
		}

		else if (state == LvlTwo)
		{
			i = 0;
			Enemies[i].ID = BOTLEFT;
			Enemies[i].posX = 0;
			Enemies[i].posY = HEIGHT / 2 + 160 - 60;
			Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth / 2;
			Enemies[i].MidY = Enemies[i].posY + Enemies[i].frameHeight / 2;
			Enemies[i].live = true;

			i++;

			Enemies[i].ID = BOTRIGHT;
			Enemies[i].posX = WIDTH - 200;
			Enemies[i].posY = HEIGHT / 2 + 160 - 60;
			Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth / 2;
			Enemies[i].MidY = Enemies[i].posY + Enemies[i].frameHeight / 2;
			Enemies[i].live = true;
			Enemies[i].animationRow = 1;

			i++;
			Enemies[i].ID = BOTRIGHT;
			Enemies[i].posX = WIDTH - 80;
			Enemies[i].posY = HEIGHT / 2 + 160 - 60;
			Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth / 2;
			Enemies[i].MidY = Enemies[i].posY + Enemies[i].frameHeight / 2;
			Enemies[i].live = true;
			Enemies[i].animationRow = 1;
			break;
		}

		else if (state == LvlThree)
		{
			if (Enemies[i].live == false)
			{
				int selection = rand() % 3 + 1;

				if (selection == 1)
				{
					Enemies[i].ID = BOTLEFT;
					Enemies[i].posX = 0;
					Enemies[i].posY = HEIGHT / 2 + 160 - 60;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth / 2;
					Enemies[i].MidY = Enemies[i].posY + Enemies[i].frameHeight / 2;
					Enemies[i].live = true;
				}
				else if (selection == 2)
				{
					Enemies[i].ID = BOTRIGHT;
					Enemies[i].posX = WIDTH - 245;
					Enemies[i].posY = HEIGHT / 2 + 160 - 60;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth / 2;
					Enemies[i].MidY = Enemies[i].posY + Enemies[i].frameHeight / 2;
					Enemies[i].live = true;	
				}
				else if (selection == 3)
				{
					Enemies[i].ID = MID;
					Enemies[i].posX = WIDTH / 2 - 245 / 2;
					Enemies[i].posY = HEIGHT / 2 + 50 - 60;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth / 2;
					Enemies[i].MidY = Enemies[i].posY + Enemies[i].frameHeight / 2;
					Enemies[i].live = true;
				}

			}
		}
	}
}
void UpdateEnemies(Enemy Enemies[], int NumEnemeies, Platform Platforms[])
{
	if (state == BossFight || state == LvlThree)
	{
		for (int i = 0; i < NumEnemeies; i++)
		{
			if (Enemies[i].live)
			{
				if (Enemies[i].ID == BOTTOM)
				{
					Enemies[i].posX += Enemies[i].Speed;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth/2;
					if (Enemies[i].posX >= WIDTH - Enemies[i].frameWidth)
					{
						Enemies[i].posX = WIDTH - Enemies[i].frameWidth;

						Enemies[i].animationRow = 1;
								
						Enemies[i].Speed = -Enemies[i].Speed;
					}
					else if (Enemies[i].posX <= 0)
					{
						Enemies[i].posX = 0;

						Enemies[i].animationRow = 0;

						Enemies[i].Speed = -Enemies[i].Speed;
					}
				}
				else if (Enemies[i].ID == BOTLEFT)
				{
					Enemies[i].posX += Enemies[i].Speed;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth / 2;
					if (Enemies[i].posX >= 245 - Enemies[i].frameWidth)
					{
						Enemies[i].posX = 245 - Enemies[i].frameWidth;

						Enemies[i].animationRow = 1;

						Enemies[i].Speed = -Enemies[i].Speed;
					}
					else if (Enemies[i].posX <= 0)
					{
						Enemies[i].posX = 0;

						Enemies[i].animationRow = 0;

						Enemies[i].Speed = -Enemies[i].Speed;
					}
					
				}
				else if (Enemies[i].ID == BOTRIGHT)
				{
					Enemies[i].posX += Enemies[i].Speed;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth / 2;
					if (Enemies[i].posX >= WIDTH - Enemies[i].frameWidth)
					{
						Enemies[i].posX = WIDTH - Enemies[i].frameWidth;

						Enemies[i].animationRow = 1;

						Enemies[i].Speed = -Enemies[i].Speed;
					}
					else if (Enemies[i].posX <= WIDTH - 245)
					{
						Enemies[i].posX = WIDTH - 245;

						Enemies[i].animationRow = 0;

						Enemies[i].Speed = -Enemies[i].Speed;
					}
				}
				else if (Enemies[i].ID == MID)
				{
					Enemies[i].posX += Enemies[i].Speed;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth / 2;
					if (Enemies[i].posX >= (WIDTH / 2 - 245 / 2) + 245 - Enemies[i].frameWidth)
					{
						Enemies[i].posX = (WIDTH / 2 - 245 / 2) + 245 - Enemies[i].frameWidth;

						Enemies[i].animationRow = 1;

						Enemies[i].Speed = -Enemies[i].Speed;
					}
					else if (Enemies[i].posX <= WIDTH / 2 - 245 / 2)
					{
						Enemies[i].posX = (WIDTH / 2 - 245 / 2);

						Enemies[i].animationRow = 0;

						Enemies[i].Speed = -Enemies[i].Speed;
					}
				}
				else if (Enemies[i].ID == TOPLEFT)
				{
					Enemies[i].posX += Enemies[i].Speed;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth / 2;
					if (Enemies[i].posX >=  245 - Enemies[i].frameWidth)
					{
						Enemies[i].posX = 245 - Enemies[i].frameWidth;

						Enemies[i].animationRow = 1;

						Enemies[i].Speed = -Enemies[i].Speed;
					}
					else if (Enemies[i].posX <= 0)
					{
						Enemies[i].posX = 0;

						Enemies[i].animationRow = 0;

						Enemies[i].Speed = -Enemies[i].Speed;
					}
				}
				else if (Enemies[i].ID == TOPRIGHT)
				{
					Enemies[i].posX += Enemies[i].Speed;
					Enemies[i].MidX = Enemies[i].posX + Enemies[i].frameWidth / 2;
					if (Enemies[i].posX >= WIDTH - Enemies[i].frameWidth)
					{
						Enemies[i].posX = WIDTH - Enemies[i].frameWidth;

						Enemies[i].animationRow = 1;

						Enemies[i].Speed = -Enemies[i].Speed;
					}
					else if (Enemies[i].posX <= WIDTH -245)
					{
						Enemies[i].posX = (WIDTH - 245);

						Enemies[i].animationRow = 0;

						Enemies[i].Speed = -Enemies[i].Speed;
					}
				}
			}
		}
	}
}
void EnemyCollision(Enemy Enemies[], int NumEnemies, Player &MainChar)
{
	for (int i = 0; i < NumEnemies; i++)
	{
		if (MainChar.MidX - MainChar.boundX < Enemies[i].MidX + Enemies[i].boundX &&
			MainChar.MidX + MainChar.boundX > Enemies[i].MidX - Enemies[i].boundX &&
			MainChar.MidY + MainChar.boundY > Enemies[i].MidY - Enemies[i].boundY &&
			MainChar.MidY - MainChar.boundY < Enemies[i].MidY + Enemies[i].boundY &&
			Enemies[i].live == true)
		{

			if (MainChar.EnemyAttacking == true)
			{
				Enemies[i].live = false;
				MainChar.Score += 10;
				al_stop_timer(MainChar.timer);
			}
			else
			{

				if (MainChar.MidY + MainChar.boundY > Enemies[i].MidY - Enemies[i].boundY &&
					MainChar.MidY + MainChar.boundY < Enemies[i].MidY - Enemies[i].boundY/2)
				{
					Enemies[i].live = false;
					MainChar.velY = 5;
					MainChar.Score += 10;
				}
				else if (MainChar.MidX + MainChar.boundX > Enemies[i].MidX - Enemies[i].boundX &&
					MainChar.MidX + MainChar.boundX < Enemies[i].MidX + Enemies[i].boundX)
				{
					MainChar.velX -= 7;
					MainChar.Health -= 5;
				}
				else if (MainChar.MidX - MainChar.boundX < Enemies[i].MidX + Enemies[i].boundX &&
					MainChar.MidX - MainChar.boundX > Enemies[i].MidX - Enemies[i].boundX)

				{
					MainChar.velX += 7;
					MainChar.Health -= 5;
				}


			}
		}
	}
}

void InitArrowImages(Arrow Arrows[], int NumArrows, ALLEGRO_BITMAP *ArrowImage)
{
	for (int i = 0; i < NumArrows; i++)
	{
		Arrows[i].image = ArrowImage;
	}
}
void InitArrows(Arrow Arrows[], int NumArrows)
{
	for (int i = 0; i < NumArrows; i++)
	{
		Arrows[i].boundx = 6;
		Arrows[i].boundy = 22;
		Arrows[i].live = false;
		Arrows[i].posx = NULL;
		Arrows[i].posy = NULL;
		Arrows[i].speed = rand() % 4 + 1;
	}
}
void DrawArrows(Arrow Arrows[], int NumArrows)
{

	for (int i = 0; i < NumArrows; i++)
	{
		if (Arrows[i].live == true)
		{
			Arrows[i].posy += Arrows[i].speed;

			al_draw_bitmap(Arrows[i].image, Arrows[i].posx, Arrows[i].posy, 0);
		}
		if (Arrows[i].posy >= HEIGHT)
		{
			Arrows[i].live = false;
		}
	}

	
}
void FireArrows(Arrow Arrows[], int NumArrows, Player &MainChar)
{
	for (int i = 0; i < NumArrows; i++)
	{
		if (Arrows[i].live == false)
		{
			Arrows[i].posx = MainChar.MidX;
			Arrows[i].posy = 0 - Arrows[i].boundy;
			Arrows[i].live = true;
		}
	}
}
void ArrowCollison(Arrow Arrows[], int NumArrows, Player &MainChar)
{
	for (int i = 0; i < NumArrows; i++)
	{
		if (MainChar.MidY + MainChar.boundY > Arrows[i].posy &&
			MainChar.MidY - MainChar.boundY < Arrows[i].posy + Arrows[i].boundy &&
			MainChar.MidX + MainChar.boundX > Arrows[i].posx &&
			MainChar.MidX - MainChar.boundX < Arrows[i].posx + Arrows[i].boundx
			&& Arrows[i].live == true)
		{

			Arrows[i].live = false;
			MainChar.Health -= 5;
		}
	}
}