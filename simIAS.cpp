// simIAS.cpp
// Christopher Schultz
// COSC 2150
// 27 April 2018
// An IAS Simulator, modeling the fetch-execute cycle

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <cstdlib>
#include <cstdio>

using namespace std;

//
// MEMORY CLASS DEFINITION
//

class Memory {
public:
	Memory() {
		for (int i = 0; i < 1000; i++) {
			memory[i] = "nop";
		}
	}
	string getMem(int x) {
		return memory[x];
	}

	void setMem(int x, string data) {
		memory[x] = data;
	}
private:
	string memory[1000];
};

//
// REGISTERS SLASH GLOBAL VARIABLES
//

Memory newMem;

int programCounter = 0;
string instructionRegister;
string memoryBufferRegister;
int memoryAddressRegister;
string accumulatorRegister = "0";
string multiplierQuotientRegister = "0";

//
// DECODE, EXECUTE, AND STRING-TO-INTEGER FUNCTION DECLARATIONS
//

void decode(string instruction, match_results<string::const_iterator> parse);
void execute(int select, int location);
int stringToInt(string convert);

//
// MAIN FUNCTION - FETCH EXECUTE CYCLE
//

int main() {
	
	// to begin, we need to make sure that we can read a file in properly. We need to load
	//   the file into memory without the line numbers at the beginning
	ifstream inFile;
	string inString;
	match_results<string::const_iterator> matchResults;

	// opening the file, and a simple check to make sure the file is open
	string fileName;
	cout << "Please input the name of the file to read. ";
	getline(cin, fileName);
	inFile.open(fileName);
	if (!inFile.is_open()) {
		cout << "ERROR: Could not open file";
		exit(1);
	}

	// this simply reads the code in the .txt file and repeats it back. This section will
	//   help the user to verify that the code they submitted is accurate before execution happens
	cout << "---------- Checking Code ----------" << endl;
	while (!inFile.eof()) {
		getline(inFile, inString);
		cout << inString << endl;
	}
	inFile.close();

	// here is where we actually load it into memory
	
	// we can reuse the variable "instring" as a temporary variable for instructions. All we
	//   need to do is declare a new integer to hold the "line number" or the PC
	cout << "---------- Loading Into Memory ----------" << endl;
	string instruction;
	int lineNo;


	inFile.open(fileName);
	while (!inFile.eof()) {
		regex memLine("(\\d+) (.*)");
		getline(inFile, inString);
		if (regex_match(inString, matchResults, memLine)) {
			instruction = matchResults[2];
			lineNo = stringToInt(matchResults[1]);
			if (instruction == "begin") {
				programCounter = lineNo;
			}
			newMem.setMem(lineNo, instruction);
		}
	}
	inFile.close();
	cout << endl << "     ...load complete! " << endl;
	
	cout << endl << "---------- Execution of Program ----------" << endl;
	
	// here is where we actually execute the program!

	// pc goes to MAR, memory goes to MBR, PC is incremented by 1, then MBR goes to IR
	while (instructionRegister != "halt"){
		memoryAddressRegister = programCounter;
		memoryBufferRegister = newMem.getMem(programCounter);
		instructionRegister = memoryBufferRegister;
		
		// here, we print the PC and IR, increment the PC, then execute an instruction and print out PC, AC, MQ
		cout << "PC = " << programCounter << "  |  IR = " << instructionRegister << endl;
		
		programCounter++;
		decode(instructionRegister, matchResults);
		
		cout << "PC = " << programCounter << "  |  AC = " << accumulatorRegister <<
			"  |  MQ = " << multiplierQuotientRegister << endl << endl << endl;
	}

	// here is where we print everything that is currently stored in memory!
	cout << endl << "---------- Printing Memory ----------" << endl;
	for (int i = 0; i < 1000; i++) {
		inString = newMem.getMem(i);
		cout << i << " " << inString << endl;
		if (inString == "halt") {
			break;
		}
	}

	return 0;
}

//
// DECODE, EXECUTE, and STRING-TO-INTEGER FUNCTION DEFINITIONS
//

// decode function definition, which takes a string (the contents of memory) and a match result
void decode(string instruction, match_results<string::const_iterator> parse) {

	// need to have eighteen cases - one each for loads and stores, and for math operations
	regex comment("(.*)(\\.)(.*)");
	regex loadMQ("load\\s?MQ");
	regex loadMQMX("load\\s?MQ,M\\((\\d+)\\)");
	regex storMX("stor\\s?M\\((\\d+)\\)");
	regex loadMX("load\\s?M\\((\\d+)\\)");
	regex loadNegativeMX("load\\s?-M\\((\\d+)\\)");
	regex loadAbsoluteMX("load\\s?\\|M\\((\\d+)\\)\\|");
	regex loadNegativeAbsoluteMX("load\\s?-\\|M\\((\\d+)\\)\\|");
	regex jump("jump\\s?M\\((\\d+)\\)");
	regex jumpPlus("jump\\+\\s?M\\((\\d+)\\)");
	regex addMX("add\\s?M\\((\\d+)\\)");
	regex addAbsoluteMX("add\\s?\\|M\\((\\d+)\\)\\|");
	regex subMX("sub\\s?M\\((\\d+)\\)");
	regex subAbsoluteMX("sub\\s?\\|M\\((\\d+)\\)\\|");
	regex mulMX("mul\\s?M\\((\\d+)\\)");
	regex divMX("div\\s?M\\((\\d+)\\)");
	regex lsh("lsh");
	regex rsh("rsh");

	// now we go through a list to determine, if the instruction is valid, which one to execute!

	// if the instruction is a comment, literally just do nothing
	if (regex_match(instruction, parse, comment)) {
	}

	// if the instruction is "begin" or "halt" or "nop", literally do nothing - can use string compare
	else if ((instruction == "begin") || (instruction == "halt") || (instruction == "nop")) {
	}

	// otherwise, pick the appropriate case to call the execute function with.
	// personal note: find a way to optimize this: this series of else-if statements are a little silly
	//   but they are functional
	else if (regex_match(instruction, parse, loadMQ)) {
		int mem = stringToInt(parse[1]);
		execute(1, mem);
	}
	else if (regex_match(instruction, parse, loadMQMX)) {
		int mem = stringToInt(parse[1]);
		execute(2, mem);
	}
	else if (regex_match(instruction, parse, storMX)) {
		int mem = stringToInt(parse[1]);
		execute(3, mem);
	}
	else if (regex_match(instruction, parse, loadMX)) {
		int mem = stringToInt(parse[1]);
		execute(4, mem);
	}
	else if (regex_match(instruction, parse, loadNegativeMX)) {
		int mem = stringToInt(parse[1]);
		execute(5, mem);
	}
	else if (regex_match(instruction, parse, loadAbsoluteMX)) {
		int mem = stringToInt(parse[1]);
		execute(6, mem);
	}
	else if (regex_match(instruction, parse, loadNegativeAbsoluteMX)) {
		int mem = stringToInt(parse[1]);
		execute(7, mem);
	}
	else if (regex_match(instruction, parse, jump)) {
		int mem = stringToInt(parse[1]);
		execute(8, mem);
	}
	else if (regex_match(instruction, parse, jumpPlus)) {
		int mem = stringToInt(parse[1]);
		execute(9, mem);
	}
	else if (regex_match(instruction, parse, addMX)) {
		int mem = stringToInt(parse[1]);
		execute(10, mem);
	}
	else if (regex_match(instruction, parse, addAbsoluteMX)) {
		int mem = stringToInt(parse[1]);
		execute(11, mem);
	}
	else if (regex_match(instruction, parse, subMX)) {
		int mem = stringToInt(parse[1]);
		execute(12, mem);
	}
	else if (regex_match(instruction, parse, subAbsoluteMX)) {
		int mem = stringToInt(parse[1]);
		execute(13, mem);
	}
	else if (regex_match(instruction, parse, mulMX)) {
		int mem = stringToInt(parse[1]);
		execute(14, mem);
	}
	else if (regex_match(instruction, parse, divMX)) {
		int mem = stringToInt(parse[1]);
		execute(15, mem);
	}
	
	// with these two regex statements, the memory location doesn't matter
	// because we are only working with the AC
	else if (regex_match(instruction, parse, lsh)) {
		execute(16, 0);
	}
	else if (regex_match(instruction, parse, rsh)) {
		execute(17, 0);
	}
	else {
		cout << "ERROR: Could not read a valid instruction!";
		exit(1);
	}
	return;
}

// execute function, which takes a "selector" and a memory index (location)
void execute(int select, int location) {
	
	// execute for load MQ, transfer contents of MQ to AC
	if (select == 1) {
		accumulatorRegister = multiplierQuotientRegister;
	}

	// execute for loadMQMX, transfer contents of memory location to MQ
	else if (select == 2) {
		multiplierQuotientRegister = newMem.getMem(location);
	}

	// execute for storMX, store contents of AC to memory location
	else if (select == 3) {
		newMem.setMem(location, accumulatorRegister);
	}

	// execute for loadMX, which transfers the contents of M(X) to AC
	else if (select == 4) {
		accumulatorRegister = newMem.getMem(location);
	}

	// execute for loadNegativeMX, which transfers the negative M(X) to AC
	else if (select == 5) {
		int x = stringToInt(newMem.getMem(location));
		x = -x;
		
		accumulatorRegister = to_string(x);
	}

	// execute for loadAbsoluteMX, which transfers the absolute value of MX to AC
	else if (select == 6) {
		int x = stringToInt(newMem.getMem(location));
		if (x < 0) {
			x = -x;
		}
		
		accumulatorRegister = to_string(x);
	}

	// execute for loadNegativeAbsoluteMX, which transfers the negative absolute value of MX to AC
	
	// personal note: because this is a negative absolute value, I could've flipped the sign of x when
	//   x is greater than 0, conversely to the last case; however, because this is a negative of the
	//   absolute value, and I am very deliberately modeling a fetch-execute cycle, I figured that the
	//   computer would make the value absolute and then make the whole thing negative. I could optimize
	//   this if the computer steps work like that; otherwise, this is absolutely functional
	else if (select == 7) {
		int x = stringToInt(newMem.getMem(location));
		if (x < 0) {
			x = -x;
		}
		
		x = -x;
		accumulatorRegister = to_string(x);
	}

	// execute for the jump statement, which sets the program counter equal to the memory location
	else if (select == 8) {
		programCounter = location;
	}

	// execute for the jump+ statement - if the AC is nonnegative, then PC = location
	else if (select == 9) {
		if (stringToInt(accumulatorRegister) >= 0) {
			programCounter = location;
		}
	}

	// execute for addMX, which adds the contents of the memory location to AC
	else if (select == 10) {
		int aReg = stringToInt(accumulatorRegister);
		int value = stringToInt(newMem.getMem(location));
		
		aReg = aReg + value;
		accumulatorRegister = to_string(aReg);
	}

	// execute for addAbsoluteMX, which adds the absolute value of location to AC
	else if (select == 11) {
		int aReg = stringToInt(accumulatorRegister);
		int value = stringToInt(newMem.getMem(location));
		if (value < 0) {
			value = -value;
		}
		
		aReg = aReg + value;
		accumulatorRegister = to_string(aReg);
	}

	// execute for subMX, which subtracts contents of memory location from AC, same manner as add
	else if (select == 12) {
		int aReg = stringToInt(accumulatorRegister);
		int value = stringToInt(newMem.getMem(location));
		
		aReg = aReg - value;
		accumulatorRegister = to_string(aReg);
	}

	// execute for subAbsoluteMX, which subtracts the absolute value of location from AC
	else if (select == 13) {
		int aReg = stringToInt(accumulatorRegister);
		int value = stringToInt(newMem.getMem(location));
		if (value < 0) {
			value = -value;
		}
		
		aReg = aReg - value;
		accumulatorRegister = to_string(aReg);
	}

	// execute for mulMX, multiply location by MQ and put the result in AC
	else if (select == 14) {
		int mQuot = stringToInt(multiplierQuotientRegister);
		int value = stringToInt(newMem.getMem(location));
		
		mQuot = mQuot * value;
		accumulatorRegister = to_string(mQuot);
	}

	// execute for divMX, divide AC by M(X), put result in MQ and remainder in AC
	else if (select == 15) {
		int aReg = stringToInt(accumulatorRegister);
		int aReg2 = aReg;
		int value = stringToInt(newMem.getMem(location));
		
		aReg = aReg / value;
		aReg2 = aReg2 % value;
		
		multiplierQuotientRegister = to_string(aReg);
		accumulatorRegister = to_string(aReg2);
	}

	// execute for lsh, multiplies AC by 2 (shift left one bit position)
	else if (select == 16) {
		int aReg = stringToInt(accumulatorRegister);
		aReg = aReg * 2;
		accumulatorRegister = to_string(aReg);
	}

	// execute for rsh, divide AC by 2 (shift right one bit position)
	else if (select == 17) {
		int aReg = stringToInt(accumulatorRegister);
		aReg = aReg / 2;
		accumulatorRegister = to_string(aReg);
	}
	return;
}

// function to convert a string to an integer using the sscanf method
int stringToInt(string convert) {
	int intReturn = 0;
	sscanf(convert.c_str(), "%d", &intReturn);
	return (intReturn);
}