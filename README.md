# benchP 
benchP is a header file for program testing utilties. You can use it to compare program outputs and times!

### Why use it? 
benchP directly contacts the Windows kernel to run your program. This means we do not use the resources created for powershell to run your program. 
Additionally, all reading and writing is performed before and after your program is executed to give an accurate measurement of your execution time without I/O delay.

To get started its probably easiest to clone this repository and modify bootstrapBenchmarkV4.cpp for your needs.
Alternatively you can copy the benchP.hpp and benchP.cpp into your visual studio project. 

**You should not implement it in your program. **

## Create a benchP Object Then Run it!

You can initialize a benchP object with a string path of the executable file.
An arguments parameter can be passed, if you do not want to concatenate it to the executable file

On a benchP object you can call the runChild() method to have it run! 

*NOTE: If the program does not terminate automatically, the process will hang and run until a interupt is sent (ctrl + c). 
If your program requires userInput to quit, then ensure that you write a quit case into the buffer.*
```cpp    
benchP profProgram("E:\\bootstrapBenchmark\\bootstrapBenchmarkV4\\stockstatsGarth.exe AAPL.csv");
benchP myProgram("E:\\bootstrapBenchmark\\bootstrapBenchmarkV4\\stockstats.exe", "AAPL.csv");

myProgram.runChild();
profProgram.runChild();
```

## Program Output and Comparison

### Writing benchP Object to Ostream
```cpp
std::cout << myProgram << std::endl;
```
printing the object to the ostream will print the executable file, args, cpu time used, and Wall time used.
```bash
E:\bootstrapBenchmark\bootstrapBenchmarkV4\stockstatsGarth.exe AAPL.csv  CPU time: 15.00ms. Wall time: 14.90ms.
```


```cpp
compareOutput(profProgram, myProgram);
```
![Alt text](compareOutputScreenshot.png "Output")

### Write the Program output to std::cout

```cpp
std::cout << myProgram.buffer.str() << std::endl;                 
```

## Write to your program (User Input)
This class writes to your program before it runs, so its good to understand how your program executes for the inputs you give it.
benchP objects will read from their std::stringstream buffer (and empty it) to write into your program. 
*note Windows reads inputs by new lines and it interpets it by \r\n chars. So seperate your inputs with /r/n*
```cpp
std::string userInput{ "53\r\nquit\r\n" };

myProgram.buffer << userInput;
profProgram.buffer << "quit\r\n";
```


### Example main function 
```cpp
#include <iostream>
#include "benchP.hpp"

int main(int argc, char * argv[])
{
    // you can initialize a benchP object with a string path of the executable file
    benchP profProgram("E:\\bootstrapBenchmark\\bootstrapBenchmarkV4\\stockstatsGarth.exe AAPL.csv");

    // an arguements parameter can be passed, if you do not want to concatenate it to the executable file
    benchP myProgram("E:\\bootstrapBenchmark\\bootstrapBenchmarkV4\\stockstats.exe", "AAPL.csv");
    
    // we can create our own user input, \r\n is how windows reads new lines and how we seperate userInputs
    std::string userInput{ "53\r\nquit\r\n" };

    // we can also get a user input from std::cin
    //std::cin >> userInput; 
     
    // then you must write the userInput into our stringstream buffer and it will read as user input
    myProgram.buffer << userInput;

    profProgram.buffer << userInput;

    myProgram.runChild();
    profProgram.runChild();

    // if the program does not terminate automatically, the process will hang and run until a interupt is sent (ctrl + c). 
    // if your program requires userInput to quit, then ensure that you write a quit case into the buffer. 

    std::cout << "myProgram" << std::endl << myProgram << std::endl;
    // program input is read from the buffer, it is cleared, then it is written to the buffer. 
    //std::cout << myProgram.buffer.str() << std::endl;                 

    std::cout << "profProgram" << std::endl << profProgram << std::endl;
    //std::cout << profProgram.buffer.str() << std::endl;
    // we can print the programs output by calling the programs public stringstream buffer instance and printing it as a string. 

    // this is a custom function written to show the output differences between p1, and p2.
    compareOutput(profProgram, myProgram);

    return 0;
}
```
