#include "Windows.h"
#include "iostream"
#include <thread>

enum DIRECTION
{
	LEFT, RIGHT, DOWN
};

bool collision(const wchar_t* figure, int fx, int fy, DIRECTION direction, const wchar_t* screen, int boxWidth, int boxHeight, int wWidth, int wHeight)
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


void move(const wchar_t* figure, DIRECTION direction, wchar_t letter, int fx, int fy, wchar_t* screen, int boxWidth, int boxHeight, int wWidth, int wHeight)
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
			if(figure[y*4 + x] == L'X')
				screen[(fy + y) * wWidth + fx + x] = L' ';
		}
	}

	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (figure[y * 4 + x] == L'X')
			{
				screen[(fy + y + cY) * wWidth + fx + x + cX] = letter;
			}
		}
	}
	
}

int main()
{
	srand(time(0));

	//SETTINGS==========================
	constexpr unsigned int boxWidth = 12;
	constexpr unsigned int boxHeight = 15;
	constexpr unsigned int offsetFromCorner = 3;
	constexpr unsigned int wWidth = 120;
	constexpr unsigned int wHeight = boxHeight + offsetFromCorner;
	//==================================

	wchar_t outputData[wWidth * wHeight];
	
	//THE BOX===========================
	for (int y = 0; y < wHeight; y++)
	{
		for (int x = 0; x < wWidth; x++)
		{
			if ((x == offsetFromCorner || x == boxWidth + offsetFromCorner - 1 || y == boxHeight + offsetFromCorner - 1) && x < boxWidth + offsetFromCorner && x >= offsetFromCorner && y >= offsetFromCorner)
			{
				outputData[y * wWidth + x] = 'X';
			}
			else
			{
				outputData[y * wWidth + x] = ' ';
			}
		}
	}
	//==================================

	void* screenBuf = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, NULL, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(screenBuf);
	unsigned long bytesWritten = 0;

	//FIGURES===========================
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
	//==================================

	//INPUT=============================
	bool R = false;
	bool L = false;
	bool D = false;
	bool Rot = false;
	//==================================

	bool newFigure = true;

	const wchar_t* currentFig = L"";
	int currentFigInd;
	int currentX;
	int currentY;

	unsigned long long int loopsCount = 0;

	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		loopsCount++;

		//-----------------INPUT-------------------
		R = GetAsyncKeyState('\x27');
		L = GetAsyncKeyState('\x25');
		D = GetAsyncKeyState('\x28');
		Rot = GetAsyncKeyState('R') | GetAsyncKeyState('r');
		//-----------------------------------------

		if (newFigure)
		{
			currentFigInd = rand() % 7;
			currentFig = figures[currentFigInd];
			currentX = offsetFromCorner + boxWidth / 2 - 2;
			currentY = offsetFromCorner;

			for (int y = currentY; y < offsetFromCorner + 4; y++)
			{
				for (int x = currentX; x < offsetFromCorner + boxWidth / 2 + 2; x++)
				{
					outputData[y * wWidth + x] = currentFig[(y - offsetFromCorner) * 4 + (x - (offsetFromCorner - 2 + boxWidth / 2))];
					if (outputData[y * wWidth + x] == 'X')
						outputData[y * wWidth + x] = L"ABCDEFG"[currentFigInd];
					else
						outputData[y * wWidth + x] = L' ';
				}
			}

			newFigure = false;
		}

		//-----------------MOVING--------------------
		if (collision(currentFig, currentX, currentY, DIRECTION::DOWN, outputData, boxWidth, boxHeight, wWidth, wHeight))
		{
			newFigure = true;
			continue;
		}
		if (loopsCount % 20 == 0)
		{
			move(currentFig, DIRECTION::DOWN, L"ABCDEFG"[currentFigInd], currentX, currentY, outputData, boxWidth, boxHeight, wWidth, wHeight);
			currentY++;
		}

		if (R && !collision(currentFig, currentX , currentY, DIRECTION::RIGHT, outputData, boxWidth, boxHeight, wWidth, wHeight))
		{
			move(currentFig, DIRECTION::RIGHT, L"ABCDEFG"[currentFigInd], currentX, currentY, outputData, boxWidth, boxHeight, wWidth, wHeight);
			currentX++;
		}
		if (L && !collision(currentFig, currentX, currentY, DIRECTION::LEFT, outputData, boxWidth, boxHeight, wWidth, wHeight))
		{
			move(currentFig, DIRECTION::LEFT, L"ABCDEFG"[currentFigInd], currentX, currentY, outputData, boxWidth, boxHeight, wWidth, wHeight);
			currentX--;
		}
		if (D && !collision(currentFig, currentX, currentY, DIRECTION::DOWN, outputData, boxWidth, boxHeight, wWidth, wHeight))
		{
			move(currentFig, DIRECTION::DOWN, L"ABCDEFG"[currentFigInd], currentX, currentY, outputData, boxWidth, boxHeight, wWidth, wHeight);
			currentY++;
		}
		//-----------------------------------------

		swprintf_s(&outputData[2 * wWidth + boxWidth + 6], 16, L"%15d", currentX);
		swprintf_s(&outputData[3 * wWidth + boxWidth + 6], 16, L"%15d", currentY);

		



		WriteConsoleOutputCharacter(screenBuf, outputData, wWidth * wHeight, { 0, 0 }, &bytesWritten);
	}

}