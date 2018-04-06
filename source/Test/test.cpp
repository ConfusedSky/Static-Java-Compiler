#include <fstream>
#include <iterator>
#include <vector>
#include <iostream>
#include <iomanip>

#define WIDTH 22
#define COLUMN 5

int main()
{
    std::ifstream input( "../testcases/Test.class", std::ios::binary );
    // copies all data into buffer
    std::vector<char> buffer((
            std::istreambuf_iterator<char>(input)), 
            (std::istreambuf_iterator<char>()));

    for(int i = 0; i < buffer.size(); i++)
    {
    	if(i % WIDTH == 0 && i != 0)
			std::cout << std::endl;
		else if(i % 2 == 0 && i != 0)
			std::cout << " ";
		std::cout << std::setw(2) << std::setbase(16) << std::setfill('0') << (((int)buffer[i])&0xFF);
    }
    std::cout << std::endl;
}
