#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 600
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x00000000
#define COLOR_RED_MAX 0xf05b5b
#define COLOR_RED_MIN 0xff0000
#define COLOR_BLUE 0x1912FF
#define COLOR_GRAY 0x0f0f0f0f
#define CELL_SIZE 20
#define COLUMNS SCREEN_WIDTH/CELL_SIZE 
#define LINE_WIDTH 2
#define ROWS SCREEN_HEIGHT/CELL_SIZE
#define SOLID_TYPE 0
#define WATER_TYPE 1
#define GAS_TYPE 2



struct Cell{
    
    int type;
    double fill_level; //between 0 and 1(full)
    int x;
    int y;
};

double get_interpolated_color(Uint32 min, Uint32 max, double percentage)
{
    Uint32 color1 = min;
    Uint32 color2 = max;
    unsigned char r1 = (color1 >> 16) & 0xff;
    unsigned char r2 = (color2 >> 16) & 0xff;
    unsigned char g1 = (color1 >> 8) & 0xff;
    unsigned char g2 = (color2 >> 8) & 0xff;
    unsigned char b1 =  color1 & 0xff;
    unsigned char b2 =  color2 & 0xff;

    return (int) ((r2-r1) * percentage +r1) << 16 | 
            (int) ((g2-g1) * percentage + g1) << 8 | 
            (int) ((b2-b1) * percentage +b1);
}

void color_cell(SDL_Surface* surface, struct Cell cell, int fill_cell)
{
    int pixel_x= cell.x*CELL_SIZE;
    int pixel_y= cell.y*CELL_SIZE;
    SDL_Rect cell_rect= (SDL_Rect){pixel_x, pixel_y, CELL_SIZE, CELL_SIZE};
    Uint32 color = COLOR_BLACK;
    //background color
    SDL_FillRect(surface, &cell_rect, color);
    //water fill level
    if(cell.type == WATER_TYPE)
    {
        
        int water_height= cell.fill_level > 1 ? CELL_SIZE : cell.fill_level * CELL_SIZE ;
        int empty_height = CELL_SIZE - water_height;
        SDL_Rect water_rect= (SDL_Rect) {pixel_x,pixel_y + empty_height,CELL_SIZE, water_height};
        Uint32 water_color= get_interpolated_color(COLOR_RED_MIN, COLOR_RED_MAX, cell.fill_level);
        if(cell.fill_level < 0.1)
        {
                water_color = COLOR_BLACK;
        }
        if(fill_cell)
        {
            SDL_FillRect(surface, &cell_rect, water_color);
        }
        else
        {
            SDL_FillRect(surface, &water_rect, water_color);
        }
    }
    //Solid Blocks
    if(cell.type == SOLID_TYPE)
    {
        SDL_FillRect(surface, &cell_rect, COLOR_WHITE);
    }
    //gas block
    if (cell.type == GAS_TYPE)
    {
        Uint32 gas_color= get_interpolated_color(COLOR_WHITE, COLOR_BLUE, cell.fill_level);
        int gas_height= cell.fill_level > 1 ? CELL_SIZE : cell.fill_level * CELL_SIZE ;
        int empty_height = CELL_SIZE - gas_height;
        SDL_Rect gas_rect= (SDL_Rect) {pixel_x,pixel_y + empty_height,CELL_SIZE, gas_height};
        SDL_FillRect(surface, &gas_rect, gas_color);
    }
}
void draw_grid(SDL_Surface* surface)
{
    for (int i=0; i<COLUMNS; i++){
        SDL_Rect column=(SDL_Rect) {i*CELL_SIZE, 0, LINE_WIDTH, SCREEN_HEIGHT};
        SDL_FillRect(surface, &column,COLOR_GRAY);
    }
    for (int j=0; j<ROWS ; j++){
        SDL_Rect row=(SDL_Rect) {0, j*CELL_SIZE, SCREEN_WIDTH, LINE_WIDTH};
        SDL_FillRect(surface, &row,COLOR_GRAY);
    }
};

void initialize_environment(struct Cell environment[ROWS *COLUMNS])
{
    for (int i=0; i<ROWS; i++)
    {
        for(int j=0; j<COLUMNS; j++)
        {
            environment[j+COLUMNS*i] = (struct Cell){WATER_TYPE,0,j,i};
        }
    }
}
 void draw_environment(SDL_Surface* surface, struct Cell environment[ROWS*COLUMNS])
{
    for (int i=0; i<ROWS*COLUMNS; i++)
    {
        color_cell(surface, environment[i], 1);
    }
    
}

void simulation_phase_rule1(struct Cell environment[ROWS*COLUMNS])
{
    struct Cell environment_next[ROWS*COLUMNS];
    for (int i=0; i<ROWS*COLUMNS; i++ )
    {
            environment_next[i] = environment[i];
    }
    for (int i=0; i<ROWS; i++)
    {
        for(int j=0; j<COLUMNS; j++)
        {
            
            struct Cell source_cell = environment[j +COLUMNS*i];
            if (source_cell.type == WATER_TYPE && i < ROWS -1)
            {
                struct Cell destination_cell = environment[j+COLUMNS*(i+1)];
                //Rule 1:water flows down
                //water will flow into destination cell
                //will only flow if the destination cell has less liquid than source cell
                if (destination_cell.fill_level < source_cell.fill_level)
                {

                    double free_space_destination = 1 - destination_cell.fill_level;

                    if(free_space_destination >= source_cell.fill_level)
                    {
                    environment_next[j+COLUMNS*i].fill_level = 0;
                    environment_next[j+COLUMNS*(i+1)].fill_level += source_cell.fill_level;
                    }
                    else
                    {
                      environment_next[j+COLUMNS*i].fill_level -= free_space_destination;
                      environment_next[j+COLUMNS*(i+1)].fill_level = 1;  
                    }
                        
                }
            }
        }
    }
    for (int i=0; i<ROWS*COLUMNS; i++)
    {
       environment[i]=environment_next[i];
    }
}

void simulation_phase_rule2(struct Cell environment[ROWS*COLUMNS])
{
    struct Cell environment_next[ROWS*COLUMNS];
    for (int i=0; i<ROWS*COLUMNS; i++ )
    {
            environment_next[i] = environment[i];
    }
    for (int i=0; i<ROWS; i++)
    {
        for(int j=0; j<COLUMNS; j++)
        {
            //Rule 2: water flows into left and right neighboring cells
            //only executes after rule 1
            //check if cell below is full, solid, or the bottom border
            struct Cell source_cell = environment[j +COLUMNS*i];
            if(i+1 == ROWS || environment[j+COLUMNS*(i+1)].type == SOLID_TYPE || environment[j+COLUMNS*(i+1)].fill_level >=environment[j+COLUMNS*i].fill_level)
            {

                if (source_cell.type == WATER_TYPE && j<COLUMNS-1)
                {   
                //how much liquid can flow to the right?
                    struct Cell destination_cell = environment[(j+1)+COLUMNS*i];
                     if (destination_cell.type == GAS_TYPE || source_cell.fill_level <0.1)
                     {
                        destination_cell.fill_level=0.0;
                     }
                    if (destination_cell.type == WATER_TYPE && destination_cell.fill_level < source_cell.fill_level)
                    {
                    double liquid_difference = source_cell.fill_level - destination_cell.fill_level;
                    environment_next[j+COLUMNS*i].fill_level -=liquid_difference/3;
                    environment_next[(j+1)+COLUMNS*i].fill_level += liquid_difference/3;       
                    }
                }
                 if (source_cell.type ==WATER_TYPE && j>0)
                {
                    //how much liquid can flow to the left?
                    struct Cell destination_cell = environment[(j-1)+COLUMNS*i];
                     if (destination_cell.type == GAS_TYPE || source_cell.fill_level <0.1)
                     {
                        destination_cell.fill_level=0.0;
                     }
                    if (destination_cell.type == WATER_TYPE && destination_cell.fill_level < source_cell.fill_level)
                    {
                    double liquid_difference = source_cell.fill_level - destination_cell.fill_level;
                    environment_next[j+COLUMNS*i].fill_level -=liquid_difference/3;
                    environment_next[(j-1)+COLUMNS*i].fill_level += liquid_difference/3;   
                    }
                }
            }
        }      
    }
    for (int i=0; i<ROWS*COLUMNS; i++)
    {
       environment[i]=environment_next[i];
    } 
}  
//Rule 3: Water flows upward with pressure
void simulation_phase_rule3(struct Cell environment[ROWS*COLUMNS])
{
    struct Cell environment_next[ROWS*COLUMNS];
    for (int i=0; i<ROWS*COLUMNS; i++ )
    {
            environment_next[i] = environment[i];
    }
    for (int i=0; i<ROWS; i++)
    {
        for(int j=0; j<COLUMNS; j++)
        {
            //Check if source's cell's fill level is > 1
            //Check if there is a water cell above into which 
            //fluid can be transferred
            struct Cell source_cell = environment[j +COLUMNS*i];
            if (source_cell.type == WATER_TYPE && source_cell.fill_level > 1 && i > 0 && environment[j+COLUMNS*(i-1)].type == WATER_TYPE && source_cell.fill_level > environment[j+COLUMNS*(i-1)].fill_level)
            {
                struct Cell destination_cell = environment[j+COLUMNS*(i-1)];
                double space_above = (source_cell.fill_level-1);
                environment_next[j+COLUMNS*(i)].fill_level -= space_above;
                environment_next[j+COLUMNS*(i-1)].fill_level += space_above;
                 
            }
            
        }  
    }
    for (int i=0; i<ROWS*COLUMNS; i++)
    {
       environment[i]=environment_next[i];
    } 
}

void simulation_downwardflow(struct Cell environment[ROWS*COLUMNS])
{
    struct Cell environment_next[ROWS*COLUMNS];
    for (int i=0; i<ROWS*COLUMNS; i++ )
    {
            environment_next[i] = environment[i];
    }
    for (int i=0; i<ROWS; i++)
    {
        for(int j=0; j<COLUMNS; j++)
        {
            struct Cell cell_below = environment[j+COLUMNS*(i+1)];
            struct Cell cell_current = environment[j+COLUMNS*i];
            if(i<ROWS-1 && cell_below.type == WATER_TYPE && cell_current.type == WATER_TYPE &&
                cell_current.fill_level > 0 && cell_current.type == WATER_TYPE && cell_current.fill_level > cell_below.fill_level)
            {
                environment[j+COLUMNS*(i+1)].fill_level = 1;
            }
        }
    }
    for (int i=0; i<ROWS*COLUMNS; i++)
    {
       environment[i]=environment_next[i];
    } 
}
//simulates gas moving outwards and to the side
void simulation_gasphase_rule2(struct Cell environment[ROWS*COLUMNS])
{
    struct Cell environment_next[ROWS*COLUMNS];
    for (int i=0; i<ROWS*COLUMNS; i++ )
    {
            environment_next[i] = environment[i];
    }
    for (int i=0; i<ROWS; i++)
    {
        for(int j=0; j<COLUMNS; j++)
        {
            struct Cell cell_current=environment[j+COLUMNS*i];
            if (cell_current.type != GAS_TYPE || cell_current.fill_level <= 0.0)
            {    
                continue;
            }
            if(cell_current.fill_level >0.0)
            {
            //set up parameters
                if(cell_current.type == GAS_TYPE && j<COLUMNS-1)
                {
                    //how much flows to the right
                    struct Cell cell_right=environment[(j+1)+COLUMNS*i];
                    if (environment_next[(j+1)+COLUMNS*i].fill_level < environment_next[j+COLUMNS*i].fill_level && cell_right.type != SOLID_TYPE)
                    {
                        double side_fill= environment_next[j+COLUMNS*i].fill_level/3;
                        if (side_fill > environment_next[j+COLUMNS*i].fill_level)
                            side_fill = environment_next[j+COLUMNS*i].fill_level;

                        // don't overfill the cell (max 1.0)
                        double free_space = 1.0 - environment_next[(j+1)+COLUMNS*i].fill_level;
                        if (side_fill > free_space)
                            side_fill = free_space;
                      if (side_fill > 0.0)
                        {      

                        environment_next[j+1+COLUMNS*i].fill_level += side_fill;
                        environment_next[j+COLUMNS*i].fill_level -= side_fill;
                        environment_next[j+1+COLUMNS*i].type = GAS_TYPE;
                        }
                }

                //how much gas flows to the left
                if (cell_current.type == GAS_TYPE && j > 0 )
                {
                    struct Cell cell_left=environment[(j-1)+COLUMNS*i];
                    if (environment_next[(j-1)+COLUMNS*i].fill_level < environment_next[j+COLUMNS*i].fill_level && cell_left.type != SOLID_TYPE)
                    {
                        double side_fill= environment_next[j+COLUMNS*i].fill_level/3;
                        if (side_fill > environment_next[j+COLUMNS*i].fill_level)
                            side_fill = environment_next[j+COLUMNS*i].fill_level;

                        // don't overfill the cell (max 1.0)
                        double free_space = 1.0 - environment_next[(j-1)+COLUMNS*i].fill_level;
                        if (side_fill > free_space)
                            side_fill = free_space;
                      if (side_fill > 0.0)
                        {      
                        environment_next[j-1+COLUMNS*i].fill_level += side_fill;
                        environment_next[j+COLUMNS*i].fill_level -= side_fill;
                        environment_next[j-1+COLUMNS*i].type = GAS_TYPE;
                        }
                    }
                }
            }
        }
    }
}
    for (int i=0; i<ROWS*COLUMNS; i++)
    {
       environment[i]=environment_next[i];
    } 
}


void simulation_gasphase_rule1(struct Cell environment[ROWS*COLUMNS])
{
    struct Cell environment_next[ROWS*COLUMNS];
    for (int i=0; i<ROWS*COLUMNS; i++ )
    {
            environment_next[i] = environment[i];
    }
    for (int i=0; i<ROWS; i++)
    {
        for(int j=0; j<COLUMNS; j++)
        {
           struct Cell cell_above = environment[j+COLUMNS*(i-1)];
           struct Cell cell_current = environment[j+COLUMNS*i];
           double free_space = 1-cell_above.fill_level;

           if (cell_current.type != GAS_TYPE || cell_current.fill_level <= 0.0)
           {
                continue;
           }
           if (cell_current.fill_level > cell_above.fill_level && i>0 && environment_next[j+COLUMNS*i].type == GAS_TYPE)
           {
            if (free_space <= 0.0) 
            {
                continue;
            }
           if(free_space >= cell_current.fill_level)
                {
                    environment_next[j+COLUMNS*i].fill_level = 0;
                    environment_next[j+COLUMNS*(i-1)].type= GAS_TYPE;
                    environment_next[j+COLUMNS*(i-1)].fill_level += cell_current.fill_level;
                }
            else
                {
                environment_next[j+COLUMNS*i].fill_level -= free_space;
                environment_next[j+COLUMNS*(i-1)].type=GAS_TYPE;
                environment_next[j+COLUMNS*(i-1)].fill_level = 1;
                }
           }
        }
    }
    for (int i=0; i<ROWS*COLUMNS; i++ )
    {
            environment[i] = environment_next[i];
    }
}


void simulation_step(struct Cell environment[ROWS*COLUMNS])
{ 
    simulation_phase_rule1(environment);
    simulation_phase_rule2(environment);
    simulation_phase_rule3(environment);
    simulation_gasphase_rule1(environment);
    simulation_gasphase_rule2(environment);   
}


int main(){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window= SDL_CreateWindow("Liquid Simulation",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    
    SDL_Surface* surface=SDL_GetWindowSurface(window);

    struct Cell environment[ROWS * COLUMNS];
    int simulation_running=1;
    SDL_Event event;
    int current_type = SOLID_TYPE;
    int delete_mode = 0;
    initialize_environment(environment);
    while(simulation_running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type ==SDL_QUIT)
            {
                simulation_running=0;
            }
            if (event.type == SDL_MOUSEMOTION)
            {
                if (event.motion.state !=0)
                {
                    int cell_x= event.motion.x/CELL_SIZE;
                    int cell_y= event.motion.y/CELL_SIZE;
                    int fill_level;
                    struct Cell cell;
                    if(delete_mode !=0)
                    {
                        current_type = WATER_TYPE;
                        fill_level = 0; 
                        cell = (struct Cell) {current_type,fill_level,cell_x,cell_y};
                    }
                    else
                    {
                        fill_level = 1;
                        cell = (struct Cell) {current_type,fill_level,cell_x,cell_y};
                    }
                
                environment[cell_x+ COLUMNS*cell_y]=cell;

                }

            }
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_BACKSPACE)
                {
                    delete_mode =!delete_mode;
                }
                if (event.key.keysym.sym == SDLK_SPACE)
                {
                    current_type = current_type+1;
                    if (current_type == 3)
                    {
                        current_type = 0;
                    }
                }
            }

        }
        simulation_step(environment);
        draw_environment(surface, environment);
        draw_grid(surface);
        SDL_UpdateWindowSurface(window);

        SDL_Delay(16);
    
    }
}



