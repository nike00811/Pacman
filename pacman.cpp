#include <cstdio>
#include <cstdlib>
#include <cmath>
#include<ctime>

#ifdef __APPLE__
#include <OpenGL/gl.h> 
#include <OpenGL/glu.h> 
#include <GLUT/glut.h>
#elif __FreeBSD__ || __linux
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#elif _WIN32
#define GLUT_DISABLE_ATEXIT_HACK
#include <GL\glut.h>
#include <windows.h>
#endif


struct color
{
	float r , g , b ;
};

struct point
{
	float x , y;
};
bool game_end = false ;
bool trigger_left_key = false;
bool trigger_up_key = false;
bool trigger_right_key = false;
bool trigger_down_key = false;

void DrawRectangle(point mid, float width, float height, color c){
    glBegin(GL_QUADS);
        glColor3f(c.r, c.g, c.b);
        glVertex3f(mid.x - width/2, mid.y - height/2, 0.0);
        glVertex3f(mid.x + width/2, mid.y - height/2, 0.0);
        glVertex3f(mid.x + width/2, mid.y + height/2, 0.0);
        glVertex3f(mid.x - width/2, mid.y + height/2, 0.0);
    glEnd();
    return;
}

void DrawCircle(point mid, float radius, color c)
{
    glBegin(GL_POLYGON);
        glColor3f(c.r, c.g, c.b);
        for(float tt = 0; tt < 2*M_PI;tt += 0.09)
            glVertex3f(mid.x + radius*cos(tt), mid.y + radius*sin(tt), 0);
    glEnd();
    return;
}

void DrawPacman(point mid, float radius, float theta, float orient, color c)
{
    float ftheta = theta*M_PI/360.0;
    float tt_mid = orient*2*M_PI/360.0;
    glBegin(GL_POLYGON);
    glColor3f(c.r, c.g, c.b);
    glVertex3f(mid.x, mid.y, 0);
	for(float tt = ftheta; tt < 2*M_PI-ftheta;tt += 0.09)
		glVertex3f(mid.x + radius*cos(tt+tt_mid), mid.y + radius*sin(tt+tt_mid), 0);
    glEnd();
    return;
}

void Draw_ghost( point mid , color c )
{
    float ftheta = 180*M_PI/360.0;
    float tt_mid = 270*2*M_PI/360.0;
    glBegin(GL_POLYGON);
    glColor3f(c.r, c.g, c.b);
    glVertex3f(mid.x, mid.y, 0);
    for(float tt = ftheta; tt < 2*M_PI-ftheta;tt += 0.09)
		glVertex3f(mid.x + 0.1*cos(tt+tt_mid), mid.y + 0.1*sin(tt+tt_mid), 0);
    glBegin(GL_QUADS);
    glVertex3f(mid.x - 0.1 , mid.y - 0.1 , 0.0);
    glVertex3f(mid.x + 0.1 , mid.y - 0.1 , 0.0);
    glVertex3f(mid.x + 0.1 , mid.y , 0.0);
    glVertex3f(mid.x - 0.1 , mid.y , 0.0);
    
    glEnd();
    return;
}

void Draw_ghost_left_eyes( point mid )
{
	glBegin(GL_POLYGON);
    glColor3f( 1 , 1 , 1 );
    for(float tt = 0; tt < 2*M_PI;tt += 0.09)
		glVertex3f(mid.x - 0.04 + 0.025*cos(tt), mid.y + 0.04 + 0.025*sin(tt), 0);
    glEnd();
    return;
}

void Draw_ghost_right_eyes( point mid )
{
	glBegin(GL_POLYGON);
    glColor3f( 1 , 1 , 1 );
    for(float tt = 0; tt < 2*M_PI;tt += 0.09)
		glVertex3f(mid.x + 0.04 + 0.025*cos(tt), mid.y + 0.04 + 0.025*sin(tt), 0);
    glEnd();
    return;
}
int map[10][10] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
	{1, 0, 1, 0, 1, 0, 1, 1, 0, 1},
	{1, 0, 1, 0, 1, 0, 1, 1, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
	{1, 0, 0, 5, 1, 0, 0, 0, 0, 1},
	{1, 0, 1, 0, 1, 1, 0, 1, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

const int DIR_UP = 0;
const int DIR_DOWN = 1;
const int DIR_LEFT = 2;
const int DIR_RIGHT = 3;

struct Lead
{
    float step;
    int dir;
    int x, y;
    color c;
};

Lead pacman ;
Lead ghost_1 ;
Lead ghost_2 ;

color wall_color = { 0.25 , 0.5 , 0 } ;
color star_color = { 1 , 1 , 1 } ;
color feed_color = { 1 , 0 , 0 } ;

point GetRealPoint(int x, int y)
{
    point ret;
    ret.x = -0.9+0.2*x;
    ret.y =  0.9-0.2*y;
    return ret; 
}

void FeedDelete( int x , int y )
{
	if( map[x][y] == 0 )
		map[x][y] = 2 ;
}

int score ;
int count = 0 ;

int life = 3 ;
int super_time = 200 ;
bool ghost_die = false ;
bool st = false ; //super time
bool ghost_1_life = true ;
bool ghost_2_life = true ;

void SystemTimer(int _value)
{
	int dx[] = {0, 0, -1, 1} , dy[] = {-1, 1, 0, 0};
	if( life > 0 )
	{
	    // 硂ㄧ计–25ms碞穦砆㊣Ω
	    // TODO
	    FeedDelete( pacman.x , pacman.y ) ;//eat feed
	    if( pacman.x == 6 && pacman.y == 3 && map[6][3] == 5 )//eat special feed 
		{
			map[6][3] = 2 ;
			st = true ;//super_time start
		}
	    for(int i = 0 ; i < 10 ; i++)		//耞だ计 
			for(int j = 0 ; j < 10 ; j++)
				if( map[i][j] == 2 )
					count++ ;	
		score = count*100 ; //衡だ计 
		count = 0 ;
		if( map[6][3] == 2 )
			score += 200 ;
		if( ghost_die == true )
			score += 300 ;
		if( st == false )
		{
			pacman.c = { 1 , 1 , 0 };
			ghost_1.c = { 1 , 0.5 , 0.5 } ;
			ghost_2.c = { 0.5 , 1 , 0.25 } ;
		}
		
		if( st == true )
		{
			pacman.c = { 0.9 , 0 , 0 } ;
			ghost_1.c = { 0 , 0.1 , 0.8 } ;
			ghost_2.c = { 0 , 0.1 , 0.8 } ;
			
			if( pacman.x == ghost_1.x && pacman.y == ghost_1.y )
			{
				ghost_die = true ;
				ghost_1.x = 0 , ghost_1.y = 0 ;
				ghost_1_life = false ;
				super_time = 1 ;
			}
			else if( pacman.x == ghost_2.x && pacman.y == ghost_2.y )
			{
				ghost_die = true ;
				ghost_2.x = 0 , ghost_2.y = 0 ;
				ghost_2_life = false ;
				super_time = 1 ;
			}
			else if( super_time == 0 )
				st = false ;
			
			super_time-- ;
		}
		
	    if(pacman.step == 0)
		{
			if(trigger_left_key) pacman.dir = DIR_LEFT ;
			if(trigger_right_key) pacman.dir = DIR_RIGHT ;
			if(trigger_down_key) pacman.dir = DIR_DOWN ;
			if(trigger_up_key) pacman.dir = DIR_UP ;
	    }
    
		int next_x = pacman.x + dx[pacman.dir] ;
		int next_y = pacman.y + dy[pacman.dir] ;
		if(map[next_x][next_y] != 1)
			pacman.step++;
	
		if(pacman.step == 4)
		{
			pacman.x += dx[pacman.dir];
			pacman.y += dy[pacman.dir];
			pacman.step = 0;
		}
	}
	if( life > 0 && game_end == false )
	{
		if( ghost_1_life == true )
		{
			int next_g1x = ghost_1.x + dx[ghost_1.dir] ;
	    	int next_g1y = ghost_1.y + dy[ghost_1.dir] ;
		    if(map[next_g1x][next_g1y] != 1 )
		        ghost_1.step = ghost_1.step + 0.5;
	    	if(ghost_1.step == 4)
			{
	    	    ghost_1.x += dx[ghost_1.dir];
	    	    ghost_1.y += dy[ghost_1.dir];
	    	    ghost_1.step = 0;
	    	}
		}
		if( ghost_2_life == true )
		{
		    int next_g2x = ghost_2.x + dx[ghost_2.dir] ;
	    	int next_g2y = ghost_2.y + dy[ghost_2.dir] ;
	    	if(map[next_g2x][next_g2y] != 1 )
        		ghost_2.step = ghost_2.step + 0.5;

		    if(ghost_2.step == 4)
			{
    		    ghost_2.x += dx[ghost_2.dir];
    	    	ghost_2.y += dy[ghost_2.dir];
				ghost_2.step = 0;
			}
		}
	}
	else
	{
		//do nothing
	}
    glutPostRedisplay();
    glutTimerFunc(25, SystemTimer, 1);
    return;
}

int change_time = 0 ;
void change_pacman( point pt , float angle , color c )//pacman糒ぺ秨+よ 
{
	if( change_time < 6 )
    {
		DrawPacman(pt, 0.1,  1, angle, c);
		change_time++ ;
	}
    else if( change_time >= 6 && change_time < 11 )
	{
		DrawPacman(pt, 0.1,  45, angle, c);
		change_time++ ;
	}
	else if( change_time == 11 )
	{
		DrawPacman(pt, 0.1,  45, angle, c);
		change_time = 0 ;
	}
}

int star_change_time = 0 ;
void change_star( point spt , color c ) // special feed
{
	if( star_change_time < 6 )
    {
		DrawCircle( spt , 0.035 , star_color ) ;
		star_change_time++ ;
	}
    else if( star_change_time <= 11 )
	{
		DrawCircle( spt , 0.03 , star_color ) ;
		star_change_time++ ;
	}
	else if( star_change_time <= 17 )
	{
		DrawCircle( spt , 0.025 , star_color ) ;
		star_change_time++ ;
	}
	else if( star_change_time <= 23 )
	{
		DrawCircle( spt , 0.03 , star_color ) ;
		star_change_time++ ;
	}
		else if( star_change_time < 29 )
	{
		DrawCircle( spt , 0.035 , star_color ) ;
		star_change_time++ ;
	}
	else if( star_change_time == 29 )
	{
		DrawCircle( spt , 0.035 , star_color ) ;
		star_change_time = 0 ;
	}
}

int	random_ghost_move(int gptx , int gpty , int gdir )
{
	srand(time(NULL));
	int a = 0 , b = 0 , c = 0 , d = 0 ;
	int ridg ;
	int x = rand() % 4 ;
	
	if( gdir == 0 )
		ridg = 1 ;
	else if( gdir == 1 )
		ridg = 0 ;
	else if( gdir == 2 )
		ridg = 3 ;
	else if( gdir == 3 )
		ridg = 2 ;
	
	if( map[gptx][gpty - 1] != 1 )
		a++ ;
	if( map[gptx][gpty + 1] != 1 )
		b++ ;
	if( map[gptx - 1][gpty] != 1 )
		c++ ;
	if( map[gptx + 1][gpty] != 1 )
		d++ ;
	
	if( a + b + c + d == 4 )
		while(1)
		{
			if( x == ridg )
				x = rand() % 4 ;
			else
				return x ;
		}
	else if( a + b + c + d == 3 )
		while(1)
		{
			if( x == 0 && map[gptx][gpty - 1] != 1 )
				return x ;
			else if( x == 1 && map[gptx][gpty + 1] != 1 )
				return x ;
			else if( x == 2 && map[gptx - 1][gpty] != 1 )
				return x ;
			else if( x == 3 && map[gptx + 1][gpty] != 1 )
				return x ;
			else
				x = rand() % 4 ;
		}
	else if( a == b && c == d )
		return gdir ;
	else if( a + b + c + d == 2 )
		while(1)
		{
			if( x == 0 && map[gptx][gpty - 1] != 1 )
				return x ;
			else if( x == 1 && map[gptx][gpty + 1] != 1 )
				return x ;
			else if( x == 2 && map[gptx - 1][gpty] != 1 )
				return x ;
			else if( x == 3 && map[gptx + 1][gpty] != 1 )
				return x ;
			else
				x = rand() % 4 ;
		}
}

char fraction[5] ;
char lif[3] ;
	
void Display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    // 讽礶惠璶砆穝硂ㄧ计碞穦砆㊣
    // TODO
    for(int lx = 0;lx < 10;lx++)
        for(int ly = 0;ly < 10;ly++)
        {
            if( map[lx][ly] == 1 )
                DrawRectangle(GetRealPoint(lx, ly), 0.18, 0.18, wall_color);
            else if( map[lx][ly] == 0 )
            	DrawCircle( GetRealPoint(lx, ly) , 0.02 , feed_color ) ;
    	}
    if( map[6][3] == 5 )
		change_star( GetRealPoint(6 , 3) , star_color ) ;
		
	if( life > 0 )
	{
	    float dt = pacman.step*0.25;
	    point pt = GetRealPoint(pacman.x, pacman.y);
		
	    if(pacman.dir == DIR_UP)
	    {
			pt.y += dt*0.2;
			change_pacman( pt , 90 , pacman.c );
		}
	    else if(pacman.dir == DIR_DOWN)
		{
			pt.y -= dt*0.2;
			change_pacman( pt , 270 , pacman.c );
		}
	    else if(pacman.dir == DIR_LEFT)
	    {
			pt.x -= dt*0.2;
			change_pacman( pt , 180 , pacman.c );
		}
	    else if(pacman.dir == DIR_RIGHT)
	    {
			pt.x += dt*0.2;
			change_pacman( pt , 0 , pacman.c );
		}
	}
	if( life > 0 && game_end == false )
	{
		if( ghost_1_life == true )
		{
			float gdt_1 = ghost_1.step*0.25 ;
			point gpt_1 = GetRealPoint(ghost_1.x, ghost_1.y);

			ghost_1.dir = random_ghost_move( ghost_1.x , ghost_1.y , ghost_1.dir ) ;
			if(ghost_1.dir == 0)
			{
				gpt_1.y += gdt_1*0.2;
				Draw_ghost( gpt_1 , ghost_1.c) ;
				Draw_ghost_left_eyes( gpt_1 ) ;
				Draw_ghost_right_eyes( gpt_1 ) ;
			}
			else if(ghost_1.dir == 1)
			{
				gpt_1.y -= gdt_1*0.2;
				Draw_ghost( gpt_1 , ghost_1.c) ;
				Draw_ghost_left_eyes( gpt_1 ) ;
				Draw_ghost_right_eyes( gpt_1 ) ;
			}
			else if(ghost_1.dir == 2)
			{
				gpt_1.x -= gdt_1*0.2;
				Draw_ghost( gpt_1 , ghost_1.c) ;
				Draw_ghost_left_eyes( gpt_1 ) ;
				Draw_ghost_right_eyes( gpt_1 ) ;
			}
			else if(ghost_1.dir == 3)
			{
				gpt_1.x += gdt_1*0.2;
				Draw_ghost( gpt_1 , ghost_1.c) ;
				Draw_ghost_left_eyes( gpt_1 ) ;
				Draw_ghost_right_eyes( gpt_1 ) ;
			}
		}
		if( ghost_2_life == true )
		{
			float gdt_2 = ghost_2.step*0.25 ;
			point gpt_2 = GetRealPoint(ghost_2.x, ghost_2.y);
			if( ghost_2.x == 6 && ghost_2.y == 1 ) ghost_2.dir = DIR_DOWN ;
			else if( ghost_2.x == 6 && ghost_2.y == 3 ) ghost_2.dir = DIR_RIGHT ;
			else if( ghost_2.x == 8 && ghost_2.y == 3 ) ghost_2.dir = DIR_UP ;
			else if( ghost_2.x == 8 && ghost_2.y == 1 ) ghost_2.dir = DIR_LEFT ;
			if(ghost_2.dir == 0)
			{
				gpt_2.y += gdt_2*0.2;
				Draw_ghost( gpt_2 , ghost_2.c) ;
				Draw_ghost_left_eyes( gpt_2 ) ;
				Draw_ghost_right_eyes( gpt_2 ) ;
			}
			else if(ghost_2.dir == 1)
			{
				gpt_2.y -= gdt_2*0.2;
				Draw_ghost( gpt_2 , ghost_2.c) ;
				Draw_ghost_left_eyes( gpt_2 ) ;
				Draw_ghost_right_eyes( gpt_2 ) ;
			}
			else if(ghost_2.dir == 2)
			{
				gpt_2.x -= gdt_2*0.2;
				Draw_ghost( gpt_2 , ghost_2.c) ;
				Draw_ghost_left_eyes( gpt_2 ) ;
				Draw_ghost_right_eyes( gpt_2 ) ;
			}
			else if(ghost_2.dir == 3)
			{
				gpt_2.x += gdt_2*0.2;
				Draw_ghost( gpt_2 , ghost_2.c) ;
				Draw_ghost_left_eyes( gpt_2 ) ;
				Draw_ghost_right_eyes( gpt_2 ) ;
			}
		}
	}
	glColor3f(0.5,1,0.5);
	float score_x = -0.9, score_y = 0.9 ;
	char str[] = "score : ";
	
	if( score < 1000 )//锣传だ计 
	{
		fraction[0] = ' ' ;
		fraction[1] = score / 100 + 48 ;
		fraction[2] = '0' ;
		fraction[3] = '0' ;
	}
	else if( score >= 1000 )
	{
		fraction[0] = score / 1000 + 48 ;
		fraction[1] = (score % 1000 ) / 100 + 48 ;
		fraction[2] = '0' ;
		fraction[3] = '0' ;
	}
	strcat( str , fraction ) ;
	glRasterPos2f((GLfloat)score_x,(GLfloat)score_y);
	for(int c = 0; str[c] != 0; c++)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[c]);
		
	glColor3f( 1 , 0.5 , 0.5 );
	float life_x = 0.7 , life_y = 0.9;
	char str_2[] = "Life: ";
	lif[0] = 48 + life ;
	strcat( str_2 , lif ) ;
	glRasterPos2f((GLfloat)life_x,(GLfloat)life_y);
	for(int c = 0; str_2[c] != 0; c++)
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str_2[c]);
	if( st == false )
	{
		if( ghost_1_life == true && pacman.x == ghost_1.x && pacman.y == ghost_1.y )
		{
			life-- ;
			pacman.step = 0 ;
			pacman.x = 1, pacman.y = 8;
			ghost_1.step = 0 ;
			ghost_1.x = 1 , ghost_1.y = 1 ;
			if( ghost_1_life == true )
			{
				ghost_2.step = 0 ;
				ghost_2.x = 8 , ghost_2.y = 1 ;
		    }
		}
		if( ghost_2_life == true &&	pacman.x == ghost_2.x && pacman.y == ghost_2.y )
		{
			life-- ;
			pacman.step = 0 ;
			pacman.x = 1, pacman.y = 8;
			ghost_2.step = 0 ;
			ghost_2.x = 8 , ghost_2.y = 1 ;		
			if( ghost_1_life == true )
			{
				ghost_1.step = 0 ;
				ghost_1.x = 1 , ghost_1.y = 1 ;
			}
		}
	}
	if( life > 0 && score == 5000 )
	{
		glColor3f(1,1,0); 
	    float x = -0.3, y = 0;
	    char game_over[] = "YOU WIN!!";
	    glRasterPos2f((GLfloat)x,(GLfloat)y);
	        for(int c = 0; game_over[c] != 0; c++)
	            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, game_over[c]);
	    ghost_1_life = false ;
	    ghost_2_life = false ;
	    game_end = true ;
	}
	else if(life == 0)
	{
		glColor3f(1,1,0); 
		float x = -0.3, y = 0;
		char game_over[] = "GAME OVER";
		glRasterPos2f((GLfloat)x,(GLfloat)y);
	    for(int c = 0; game_over[c] != 0; c++)
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, game_over[c]);
	}

    glFlush();
    return;
}

void Init()//礶3唉碍 
{
    // 硂ㄧ计穦秨﹍㊣
    // TODO
    pacman.step = 0 ;
    pacman.dir = DIR_RIGHT ;
    pacman.x = 1, pacman.y = 8;
    pacman.c = { 1 , 1 , 0 };
    
    ghost_1.step = 0 ;
    ghost_1.dir = DIR_DOWN ;
    ghost_1.x = 1 , ghost_1.y = 1 ;
	ghost_1.c = { 1 , 0.5 , 0.5 } ;
	
    ghost_2.step = 0 ;
    ghost_2.dir = DIR_LEFT ;
    ghost_2.x = 8 , ghost_2.y = 1 ;
	ghost_2.c = { 0.5 , 1 , 0.25 } ;
    
    return ;
}

void SpecialKeyDown(int key, int x, int y)
{
    if(key == GLUT_KEY_LEFT) trigger_left_key = true; 
    if(key == GLUT_KEY_RIGHT) trigger_right_key = true; 
    if(key == GLUT_KEY_UP) trigger_up_key = true; 
    if(key == GLUT_KEY_DOWN) trigger_down_key = true; 
    glutPostRedisplay();
    return;
}

void SpecialKeyUp(int key, int x, int y)
{
    if(key == GLUT_KEY_LEFT) trigger_left_key = false; 
    if(key == GLUT_KEY_RIGHT) trigger_right_key = false; 
    if(key == GLUT_KEY_UP) trigger_up_key = false; 
    if(key == GLUT_KEY_DOWN) trigger_down_key = false; 
    glutPostRedisplay();
    return;
}

int main(int argc, char* argv[])
{
    Init();
    
    glutInit(&argc, argv);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Pacman");
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA); 
    glutDisplayFunc(Display);

    glutTimerFunc(25, SystemTimer, 1);

    glutSpecialFunc(SpecialKeyDown);
    glutSpecialUpFunc(SpecialKeyUp);

    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glutMainLoop();

    return 0;
}
