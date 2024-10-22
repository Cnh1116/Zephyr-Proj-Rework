#include <iostream>
#include "Game.hpp"


//#include <unistd.h> // For sleep

int main(int argc, char* argv[])
{
	
	Game my_game;
	
	my_game.RunGame();
	std::cout << "[*] End of main\n";
	
	return 0;
}
