#include <Windows.h>
#include <iostream>
#include <thread>
#include <functional>
#include <iomanip>

void* glBuf;

enum DIRECTION
{
	LEFT, RIGHT, DOWN
};

bool collision(const wchar_t* figure, int fx, int fy, DIRECTION direction, const wchar_t* screen, int wWidth)
{
	int cX = 0;
	int cY = 0;
	switch (direction)
	{
	case DIRECTION::LEFT: cX = -1; break; 
	case DIRECTION::RIGHT: cX = 1; break;
	case DIRECTION::DOWN: cY = 1; break;
	}
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (screen[(fy + y + cY) * wWidth + fx + x  + cX] != L' ' && figure[y * 4 + x] == L'X')
			{
				if (figure[(y + cY) * 4 + x + cX] != L'X')
					return true;
			}
		}
	}
	return false;
}

bool collision(const wchar_t* figure, const wchar_t* ignore, int fx, int fy, const wchar_t* screen, int wWidth)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (screen[(fy + y) * wWidth + fx + x] != L' ' && figure[y * 4 + x] == L'X' && ignore[y * 4 + x] != L'X')
			{
				return true;
			}
		}
	}
	return false;
}

void movePiece(int x, int y, DIRECTION direction, wchar_t* screen, int wWidth)
{
	int cX = 0;
	int cY = 0;
	switch (direction)
	{
	case DIRECTION::LEFT: cX = -1; break;
	case DIRECTION::RIGHT: cX = 1; break;
	case DIRECTION::DOWN: cY = 1; break;
	}

	screen[(y + cY) * wWidth + x + cX] = screen[y * wWidth + x];
	screen[y * wWidth + x] = L' ';
}


void move(const wchar_t* figure, DIRECTION direction, int fx, int fy, wchar_t* screen, int wWidth)
{
	std::function<int(int)> getx = [&](int x) { return fx + x; };
	std::function<int(int)> gety = [&](int y) { return fy + y; };
	
	switch (direction)
	{
	case DIRECTION::DOWN:	gety = [&](int y) { return fy + 3 - y; };	break;
	case DIRECTION::RIGHT:	getx = [&](int x) { return fx + 3 - x; };	break;
	}
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (figure[(gety(y) - fy) * 4 + getx(x) - fx] == L'X')
			{
				movePiece(getx(x), gety(y), direction, screen, wWidth);
			}
		}
	}
	
}


bool pieceCollision(int x, int y, DIRECTION direction, const wchar_t* screen, int wWidth)
{
	int cX = 0;
	int cY = 0;
	switch (direction)
	{
	case DIRECTION::LEFT: cX = -1; break;
	case DIRECTION::RIGHT: cX = 1; break;
	case DIRECTION::DOWN: cY = 1; break;
	}
	if (screen[(y + cY) * wWidth + x + cX] != L' ')
		return true;
	return false;
}


void rotate(const wchar_t* figure, wchar_t* res)
{
	//float rad = (float)angle / 180.0f * 3.14159f;
	for (int y = 0; y <= 3; y++)
	{
		for (int x = 0; x <= 3; x++)
		{

			res[x * 4 - y + 3] = figure[y * 4 + x];
			/*int nx = round(float(x - width / 2) * cos(rad) + float(height / 2 - y) * sin(rad) + width / 2);
			int ny = round(-float(x - width / 2) * sin(rad) + float(height / 2 - y) * cos(rad) + height / 2);
			res[ny * width + nx] = figure[y * width + x];*/
		}
	}
}

int main()
{
	std::cout<<"   ***RULES***   \nPRESS 'R' TO ROTATE\nUSE ARROWS TO MOVE FIGURES\nEVERY LINE GIVES YOU ONE POINT\nDON'T RESIZE THE WINDOW!\n";
	std::cout << "---press enter to continue---\n";
	std::cin.get();
	std::cout << "HAVE FUN :)\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	srand(time(0));

	//PARAMETERS============================
	constexpr unsigned int boxWidth = 12;
	constexpr unsigned int boxHeight = 17;
	constexpr unsigned int offsetFromCorner = 7;
	constexpr unsigned int wWidth = 120;
	constexpr unsigned int wHeight = boxHeight + offsetFromCorner;
	//======================================

	wchar_t outputData[wWidth * wHeight];
	
	//THE BOX===============================
	for (int y = 0; y < wHeight; y++)
	{
		for (int x = 0; x < wWidth; x++)
		{
			if ((x == offsetFromCorner || x == boxWidth + offsetFromCorner - 1 || y == boxHeight + offsetFromCorner - 1) && x < boxWidth + offsetFromCorner && x >= offsetFromCorner && y >= offsetFromCorner)
			{
				outputData[y * wWidth + x] = '#';
			}
			else
			{
				outputData[y * wWidth + x] = ' ';
			}
		}
	}
	//======================================

	void* screenBuf = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, NULL, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	glBuf = screenBuf;
	SetConsoleActiveScreenBuffer(screenBuf);
	unsigned long bytesWritten = 0;

	//FIGURES===============================
	const wchar_t figures[7][17]
	{
		L"..X...X...X...X.",
		L"..X..XX...X.....",
		L".....XX..XX.....",
		L"..X..XX..X......",
		L".X...XX...X.....",
		L".X...X...XX.....",
		L"..X...X..XX....."
	};
	//======================================

	//INPUT FLAGS===========================
	bool R = false;
	bool L = false;
	bool D = false;
	bool Rot = false;
	//======================================

	bool newFigure = true;

	bool gameOver = false;

	int colBottomCount = 0;

	wchar_t currentFig[16];
	int nextFigID = rand() % 7;
	int currentFigInd;
	int currentX;
	int currentY;

	unsigned long long int loopsCount = 0;

	int score = 0;

	//DEFINE SIMPLE SYNTAX FOR FUNCTIONS====
#define sCollision(figure, ignoredArea)		collision(figure, ignoredArea, currentX, currentY, outputData, wWidth)
#define dCollision(direction)				collision(currentFig, currentX, currentY, direction, outputData, wWidth)
#define move(direction)						move(currentFig, direction, currentX, currentY, outputData, wWidth)
#define pieceCollision(x, y, direction)		pieceCollision(x, y, direction, outputData, wWidth)
#define movePiece(x, y, direction)			movePiece(x, y, direction, outputData, wWidth)
	//======================================

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(80));
		loopsCount++;

		//-----------------INPUT---------------------
		R = GetAsyncKeyState('\x27');
		L = GetAsyncKeyState('\x25');
		D = GetAsyncKeyState('\x28');
		Rot = GetAsyncKeyState('R');
		if (GetAsyncKeyState('S'))
			break;
		//-------------------------------------------

		if (newFigure)
		{
			currentFigInd = nextFigID;
			nextFigID = rand() % 7;
			for (int i = 0; i < 16; i++)
			{
				currentFig[i] = figures[currentFigInd][i];
			}
			currentX = offsetFromCorner + boxWidth / 2 - 2;
			currentY = offsetFromCorner;

			for (int y = currentY; y < offsetFromCorner + 4; y++)
			{
				for (int x = currentX; x < offsetFromCorner + boxWidth / 2 + 2; x++)
				{
					if (outputData[y * wWidth + x] != L' ')
					{
						gameOver = true;
						break;
					}
					outputData[y * wWidth + x] = currentFig[(y - offsetFromCorner) * 4 + (x - (offsetFromCorner - 2 + boxWidth / 2))];
					if (outputData[y * wWidth + x] == 'X')
						outputData[y * wWidth + x] = L"ABCDEFG"[currentFigInd];
					else
						outputData[y * wWidth + x] = L' ';
				}
				if (gameOver)
					break;
			}
			if (gameOver)
				break;

			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					wchar_t l = figures[nextFigID][y * 4 + x];
					outputData[(y + 1) * wWidth + x + offsetFromCorner + boxWidth / 2 - 2] = (l == L'X' ? L"ABCDEFG"[nextFigID] : L' ');
				}
			}

			newFigure = false;
		}

		//-----------MOVING AND ROTATION-------------
		if (R && !dCollision(DIRECTION::RIGHT))
		{
			move(DIRECTION::RIGHT);
			currentX++;
		}
		if (L && !dCollision(DIRECTION::LEFT))
		{
			move(DIRECTION::LEFT);
			currentX--;
		}
		if (D || loopsCount % 12 == 0)
		{
			if (dCollision(DIRECTION::DOWN))
			{
				newFigure = true;
			}
			else
			{
				move(DIRECTION::DOWN);
				currentY++;

			}
		}
		if (Rot)
		{
			
			wchar_t res[16];
			rotate(currentFig, res);
			if (!sCollision(res, currentFig))
			{
				for (int y = 0; y < 4; y++)
				{
					for (int x = 0; x < 4; x++)
					{
						if (currentFig[y * 4 + x] == L'X')
						{
							outputData[(y + currentY) * wWidth + x + currentX] = L' ';
						}
					}
				}

				for (int i = 0; i < 16; i++)
				{
					currentFig[i] = res[i];
				}
				for (int y = 0; y < 4; y++)
				{
					for (int x = 0; x < 4; x++)
					{
						if (currentFig[y * 4 + x] == L'X')
						{
							outputData[(currentY + y) * wWidth + x + currentX] = L"ABCDEFG"[currentFigInd];
						}
					}
				}
			}
		}
		//-------------------------------------------

		//-------------clearing lines----------------
		if (newFigure)
		{
			for (int y = currentY; y < boxHeight + offsetFromCorner - 1; y++)
			{
				bool clear = true;
				for (int x = offsetFromCorner + 1; x < offsetFromCorner + boxWidth - 1; x++)
				{
					if (outputData[(y)*wWidth + x] == L' ')
					{
						clear = false;
						break;
					}
				}

				if (clear)
				{
					score++;

					for (int x = offsetFromCorner + 1; x < offsetFromCorner + boxWidth - 1; x++)
					{
						outputData[(y)*wWidth + x] = L'*';
					}
					WriteConsoleOutputCharacter(screenBuf, outputData, wWidth * wHeight, { 0, 0 }, &bytesWritten);

					std::this_thread::sleep_for(std::chrono::milliseconds(300));

					for (int x = offsetFromCorner + 1; x < offsetFromCorner + boxWidth - 1; x++)
					{
						outputData[(y)*wWidth + x] = L' ';
					}
					WriteConsoleOutputCharacter(screenBuf, outputData, wWidth * wHeight, { 0, 0 }, &bytesWritten);

					bool moved = true;
					while(moved)
					{

						std::this_thread::sleep_for(std::chrono::milliseconds(300));
						moved = false;
						for (int by = y; by >= offsetFromCorner; by--)
						{
							//move line
							for (int bx = offsetFromCorner + 1; bx < boxWidth + offsetFromCorner - 1; bx++)
							{
								if (outputData[by * wWidth + bx] != L' ')
								{
									if (!pieceCollision(bx, by, DIRECTION::DOWN))
									{
										movePiece(bx, by, DIRECTION::DOWN);
										moved = true;

									}
								}
							}
						}
						WriteConsoleOutputCharacter(screenBuf, outputData, wWidth * wHeight, { 0, 0 }, &bytesWritten);
					} 

					y = currentY++;
				}
			}
		}

		//-------------------------------------------
		/*for (int y = 5; y < 15; y++)
		{
			for (int x = 40; x < 50; x++)
			{
				outputData[y * wWidth + x] = painting[(y - 5) * 10 + x - 40];
			}
		}

		wchar_t pr[101];
		rotate(painting, pr, 0, 10, 10);
		memcpy(painting, pr, 100 * 2);*/

		swprintf_s(&outputData[2 * wWidth + boxWidth + 15], 10, L"SCORE: %2d", score);
		WriteConsoleOutputCharacter(screenBuf, outputData, wWidth * wHeight, { 0, 0 }, &bytesWritten);
	}

	char text[] = " #### |#    #|#     |#   ##|#    #| #### " " #### |#    #|#    #|######|#    #|#    #" "##  ##|# ## #|# ## #|#    #|#    #|#    #" " #####|#     | #### |#     |#     | #####"
		"      |      |      |      |      |      "
		 " #### |#    #|#    #|#    #|#    #| #### " "#    #|#    #|#    #| #  # | #  # |  ##  " " #####|#     | #### |#     |#     | #####" " #### |#    #|#    #|##### |#   # |#    #";
	for (int y = 0; y < wHeight; y++)
	{
		for (int x = 0; x < wWidth; x++)
		{
			outputData[y * wWidth + x] = ' ';
		}
	}

	for (int i = 0; i < 9; i++)
	{
		int sx = 20 + i * 8;
		int sy = 5;
		int o = i * 41;
		for (int y = 0; y < 6; y++)
		{
			for (int x = 0; x < 6; x++)
			{
				wchar_t l = text[y * 6 + x + o];
				if (l == '|') o++;
				l = text[y * 6 + x + o];
				outputData[(y + sy) * wWidth + x + sx] = l;
			}
		}
	}

	WriteConsoleOutputCharacter(screenBuf, outputData, wWidth* wHeight, { 0, 0 }, & bytesWritten);


	int i = 30;
	while(i--)
	{
		std::cout << "\n";
	}
	std::cout << std::setw(50);
	if (gameOver)
		std::cout << "GAME OVER\n";
	else
		std::cout << "GAME STOPPED\n"; 
	std::cout<<std::setw(50)<<"FINAL SCORE : " << score;
	i = 10;
	while (i--)
	{
		std::cout << "\n";
	}
	std::this_thread::sleep_for(std::chrono::seconds(10));
}
