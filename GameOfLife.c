#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define GRID_SIZE 16
#define DELAY_MS 500

int	min(int a, int b)
{
	if (a < b)
		return (a);
	return (b);
}

unsigned int	get_live_neighbours(int status[GRID_SIZE][GRID_SIZE], const int x,const int y)
{
	unsigned int count = 0;
	int	currX = x - 1;
	int	currY = y - 1;
	if (currY < 0)
		currY++;
	if (currX < 0)
		currX++;
	while (currY < min((y + 1), GRID_SIZE - 1))
	{
		while (currX < min((x + 1), GRID_SIZE - 1))
		{
			if (currX == x && currY == y)
			{
				currX++;
				continue;
			}
			if (status[currY][currX])
				count++;
			currX++;
		}
		currY++;
	}
	return (count);
}

/*the rules
Any live cell with two or three live neighbours survives.
Any dead cell with three live neighbours becomes a live cell.
All other live cells die in the next generation. Similarly, all other dead cells stay dead.
*/
int	lives_on(unsigned int neighbour_count, int pixel_alive)
{
	if (pixel_alive)
		return (neighbour_count == 2 || neighbour_count == 3);
	else
		return (neighbour_count == 3);
}

void	evolve_grid(int status[GRID_SIZE][GRID_SIZE], int next[GRID_SIZE][GRID_SIZE])
{
	for (int y = 0; y < GRID_SIZE; y++) {
	    for (int x = 0; x < GRID_SIZE; x++) {
		next[y][x] = lives_on(get_live_neighbours(status, x, y), status[y][x]);
	    }
	}
}

void	print_grid(int grid[GRID_SIZE][GRID_SIZE])
{
    printf("\033[H\033[J");
    
    printf("Game of Life - Current Generation:\n");
    printf("--------------------------------\n");
    
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            printf("%c ", grid[y][x] ? '#' : '.');
        }
        printf("\n");
    }
    
    printf("--------------------------------\n");
}

void	copy_grid(int dst[GRID_SIZE][GRID_SIZE], int src[GRID_SIZE][GRID_SIZE])
{
	for (int y = 0; y < GRID_SIZE; y++) {
	    for (int x = 0; x < GRID_SIZE; x++) {
		dst[y][x] = src[y][x];
	    }
	}
}

int main()
{
	int status[16][16] = {
	{0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};
	int	generation_count = 0;
	int	next[GRID_SIZE][GRID_SIZE];
	while (1)
	{
		print_grid(status);
		printf("Generation: %d\n", generation_count++);
		evolve_grid(status, next);
		copy_grid(status, next);
		usleep(DELAY_MS * 1000);
	}
	return (0);
}
