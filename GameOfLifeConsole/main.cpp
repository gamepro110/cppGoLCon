#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <chrono>
#include <thread>

#include <stdio.h>
#include <Windows.h>

const int m_screenWidth = 120;
const int m_screenHeight = 40;

const short m_onChar = 0x2588;
const short m_offChar = ' ';// 0x256c;

int main()
{
	//enviorment variables
	bool m_run{ true };
	auto m_elapsedTime{ std::chrono::duration<double>() };
	double deltaTime{ 0 };
	float m_delay{ 0.0f };

	// create console buffer, explained by javidx9 on youtube
	wchar_t* screen = new wchar_t[m_screenHeight * m_screenWidth]{};
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten{ 0 };

	// clocking
	auto tp1{ std::chrono::system_clock::now() };
	auto tp2{ std::chrono::system_clock::now() };

	//game of life variables
	bool m_bufferRead[m_screenWidth * m_screenHeight] = { false };
	bool m_bufferWrite[m_screenWidth * m_screenHeight] = { false };
	bool m_bufferTemp[m_screenWidth * m_screenHeight] = { false };

	int m_liveNeigbours{ 0 };
	int m_currentIndex{};

	//initialize random
	std::srand(static_cast<unsigned short int>(time(0)));

	// initializing random patern
	for (int i = 0; i < m_screenHeight * m_screenWidth; i++) m_bufferRead[i] = (rand() % 2 == 0);

	while (m_run)
	{
		// calculating time
		tp2 = std::chrono::system_clock::now();
		m_elapsedTime = tp2 - tp1;
		tp1 = tp2;
		deltaTime = m_elapsedTime.count();

		// kill switch (SPACEBAR)
		if (GetAsyncKeyState((unsigned short)' ') & 0x8000)
		{
			m_run = false;
			break;
		}

		//delay changers
		if (GetAsyncKeyState((unsigned short)'1') & 0x8000)
		{
			m_delay -= 0.1f;
			if (m_delay < 0.0f)
			{
				m_delay += 0.1f;
			}
		}
		if (GetAsyncKeyState((unsigned short)'2') & 0x8000)
		{
			m_delay -= 0.01f;
			if (m_delay < 0.0f)
			{
				m_delay += 0.01f;
			}
		}
		if (GetAsyncKeyState((unsigned short)'3') & 0x8000)
		{
			m_delay += 0.01f;
			if (m_delay > 2.0f)
			{
				m_delay -= 0.01f;
			}
		}
		if (GetAsyncKeyState((unsigned short)'4') & 0x8000)
		{
			m_delay += 0.1f;
			if (m_delay > 2.0f)
			{
				m_delay -= 0.1f;
			}
		}

		// game of life logic
		for (int y = 1; y < m_screenHeight; y++)
		{
			for (int x = 0; x < m_screenWidth; x++)
			{
				m_currentIndex = y * m_screenWidth + x;
				bool& index = m_bufferWrite[m_currentIndex];

				//check live neighbours
				m_liveNeigbours = 0;

				// saving position + offset for preformance
				int xT{};
				int yT{};

				for (int y2 = -1; y2 <= 1; y2++)
				{
					for (int x2 = -1; x2 <= 1; x2++)
					{
						// skip cell thats checking for neighbours
						if (x2 == 0 && y2 == 0)	continue;

						xT = x + x2;
						yT = y + y2;
						auto& cell = m_bufferRead[(yT * m_screenWidth + xT)];

						// bounds check
						if (xT >= 0 && xT < m_screenWidth && yT >= 0 && yT < m_screenHeight)
						{
							if (cell == true)
							{
								m_liveNeigbours++;
							}
						}
					}
				}

				switch (m_liveNeigbours) // game of life rules
				{
				case 2: // if the current cell has 2 live neighbours it stays the same
					index = m_bufferRead[m_currentIndex];
					break;

				case 3: // if the current cell has 3 live neighbours it turns on (or becomes alive)
					index = true;
					break;

				default: // any other amount of live neighbours kills the cell
					index = false;
					break;
				}

				// write to output buffer
				screen[m_currentIndex] = m_bufferRead[m_currentIndex] ? m_onChar : m_offChar;
			}
		}

		//swapping buffers
		for (int i = 0; i < m_screenHeight * m_screenWidth; i++)// write -> temp; read -> write; temp -> read;
		{
			m_bufferTemp[i] = m_bufferWrite[i];
			m_bufferWrite[i] = m_bufferRead[i];
			m_bufferRead[i] = m_bufferTemp[i];
		}

		//print fps (off for preformance test)
		swprintf_s(screen, m_screenWidth, L"\rtime: %3.8f delay: %3.2F", deltaTime, m_delay);

		// setting termination char and writing to screen
		screen[m_screenWidth * m_screenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, m_screenWidth * m_screenHeight, { 0,0 }, &dwBytesWritten);
	}

	// cleaning up the screen pointer. all other variables are on the stack
	delete[] screen;

	return 0;
}