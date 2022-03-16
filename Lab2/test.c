// CPP program to find largest in an array
// without conditional/bitwise/ternary/ operators
// and without library functions.
#include<iostream>
#include<cstring>
#include<sstream>
using namespace std;
	
// If page found, updates the second chance bit to true
static bool findAndUpdate(int x,int queue[],
				bool referenceBit[],int frames)

{
	int i;
	
	for(i = 0; i < frames; i++)
	{
		
		if(queue[i] == x)
		{
			// Mark that the page deserves a second chance
			referenceBit[i] = true;
			
			// Return 'true', that is there was a hit
			// and so there's no need to replace any page
			return true;
		}
	}
	
	// Return 'false' so that a page for replacement is selected
	// as he reuested page doesn't exist in memory
	return false;
	
}


// Updates the page in memory and returns the referencePointer
static int replaceAndUpdate(int x,int queue[],
			bool referenceBit[],int frames,int referencePointer)
{
	while(true)
	{
		
		// We found the page to replace
		if(!referenceBit[referencePointer])
		{
			// Replace with new page
			queue[referencePointer] = x;
			
			// Return updated referencePointer
			return (referencePointer + 1) % frames;
		}
		
		// Mark it 'false' as it got one chance
		// and will be replaced next time unless accessed again
		referenceBit[referencePointer] = false;
		
		//Pointer is updated in round robin manner
		referencePointer = (referencePointer + 1) % frames;
	}
}

static void printHitsAndFaults(string referenceStream,
											int frames)
{
	int referencePointer, i, referenceSize=0, x, faultCounter;
	
	//initially we consider frame 0 is to be replaced
	referencePointer = 0;
	
	//number of page faults
	faultCounter = 0;
	
	// Create a array to hold page numbers
	int queue[frames];
	
	// No pages initially in frame,
	// which is indicated by -1
	memset(queue, -1, sizeof(queue));
	
	// Create second chance array.
	// Can also be a byte array for optimizing memory
	bool referenceBit[frames];
	
	// Split the string into tokens,
	// that is page numbers, based on space
	
	string str[100];
	string word = "";
	for (auto x : referenceStream)
	{
		if (x == ' ')
		{
			str[referenceSize]=word;
			word = "";
			referenceSize++;
		}
		else
		{
			word = word + x;
		}
	}
	str[referenceSize] = word;
	referenceSize++;
	// referenceSize=the length of array
	
	for(i = 0; i < referenceSize; i++)
	{
		x = stoi(str[i]); // x = numarul
		
		// Finds if there exists a need to replace
		// any page at all
		if(!findAndUpdate(x,queue,referenceBit,frames))
		{
			// Selects and updates a victim page
			referencePointer = replaceAndUpdate(x,queue,
					referenceBit,frames,referencePointer);
			
			// Update page faults
			faultCounter++;
		}
	}
	cout << "Total page faults were " << faultCounter << "\n";
}

// Driver code
int main()
{
	string referenceStream = "";
	int frames = 0;

	// Test 1:
	referenceStream = "0 4 1 4 2 4 3 4 2 4 0 4 1 4 2 4 3 4";
	frames = 3;
	
	// Output is 9
	printHitsAndFaults(referenceStream,frames);
	
	// Test 2:
	referenceStream = "2 5 10 1 2 2 6 9 1 2 10 2 6 1 2 1 6 9 5 1";
	frames = 4;
	
	// Output is 11
	printHitsAndFaults(referenceStream,frames);
	return 0;
}

// This code is contributed by NikhilRathor

