#include <iostream>
#include <fstream>
#include <list>
#include <utility>
#include <functional>
#include <string>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <sstream>
using namespace std;

//define N, the number of characters in a sequence
#define N 8
//define the total number of buckets we have in the hash table, a prime number capable of handling files up to 100KB
#define NUM_BUCK 138889
#define maxInLinkedList 100

//use the hash function provided in the STL to compute a hashed number for string values
hash<string> hashNum_func;

//declare and implement a class for the hashTable, which is open
class OpenHashTable
{
public:
    //initialize the count to 0 when constructed
    OpenHashTable()
    {
        for (int i = 0; i<NUM_BUCK; i++)
        {
            m_buckets[i].count = 0;
        }
    }
    void insert (string sequence, int offset);
    bool search (string sequence);
    int copySearch (string& oldFile, string& newFile, int& overlaplength);
    
private:
    inline size_t bucketNum (size_t hashNum) const
    {
        //this returns the bucket number
        return hashNum % NUM_BUCK;
    }
    struct BUCKET
    {
        //a list of pairs of N-character sequence and their offset
        list<pair<string, int>> info;
        //the count contains how many pairs are currently in a list, should be initialize to 0
        int count;
    };
    BUCKET m_buckets[NUM_BUCK];
};

//implement OpenHashTable's insert function, which inserts the pair of sequence and offset into the hashTable, if the count is at the limit, stop inserting.
void OpenHashTable::insert (string sequence, int offset)
{
    //calculate the bucket number for the given sequence
    size_t hashNum = hashNum_func(sequence);
    size_t bucket = bucketNum(hashNum);
    
    //if the count is still less than the limit, allow insertion
    if (m_buckets[bucket].count<=maxInLinkedList)
    {
        //increment the count by one
        (m_buckets[bucket].count)+=1;
        
        //implement the insertion
        pair<string, int> p (sequence, offset);
        ((m_buckets[bucket]).info).push_back(p);
    }
    return;
}

//helper function to count the number of digits in a int
int countDigit(int n)
{
    if (n == 0)
        return 0;
    return 1 + countDigit(n / 10);
}

//simple function to search the corresponding bucket to see if a given sequence is contained in the hashTable. Return true if present, return false otherwise
bool OpenHashTable::search (string sequence)
{
    //calculate the bucket number for the given sequence
    size_t hashNum = hashNum_func(sequence);
    size_t bucket = bucketNum(hashNum);
    
    list<pair<string, int>>::iterator it;
    it = m_buckets[bucket].info.begin();
    int count = m_buckets[bucket].count;
    for (int i = 0; i<count; i++)
    {
        if ((*it).first==sequence)
        {
            return true;
        }
        it++;
    }
    return false;
}


//this function returns the best offset position (longest number of overlap, and fewest number of character) to copy (only called when the sequence is found)
//Passing IN the entire Oldfile and the portion of newFile starting from the point of the first match
int OpenHashTable::copySearch (string& oldFile, string& newFile, int& overlaplength)
{
    //the match is the first N-character sequence and we need to determine what offset in oldFile is the best to be returned
    string sequence = newFile.substr(0,N);
    
    //calculate the bucket number for the given sequence
    size_t hashNum = hashNum_func(sequence);
    size_t bucket = bucketNum(hashNum);
    
    //define variables to be used later
    int maxoverlap = N;
    int bestOffsetSoFar = -1;
    int currentDigit = 1000000;
    
    //using a for-loop to traverse the list and look for all that matches the sequence
    int count = m_buckets[bucket].count;
    list<pair<string, int>>::iterator it;
    it = m_buckets[bucket].info.begin();
    
    for (int i = 0; i<count; i++, it++)
    {
        if ((*it).first==sequence)
        {
            //compare to get the max length at this offset
            int toBeCopied = N;
            int newFileIndex = N; //index in the newFile
            int startingOldOffset = (*it).second;
            int oldFileIndex = startingOldOffset+N; //for the oldfile
            
            while (newFileIndex!= (int)newFile.length() && oldFileIndex!= (int)oldFile.length())
            {
                if (oldFile[oldFileIndex] == newFile[newFileIndex])
                {
                    toBeCopied++;
                    newFileIndex++;
                    oldFileIndex++;
                }
                else
                {
                    break;
                }

            }
            int totalDigit = countDigit(toBeCopied) + countDigit(startingOldOffset);
            //choose the offset that yields the longest copy length, if the copy length are the same, then prefer the one with the shortest instructions
            if (toBeCopied>maxoverlap)
            {
                maxoverlap = toBeCopied;
                bestOffsetSoFar = startingOldOffset;
                currentDigit = totalDigit;
            }
            else if (toBeCopied==maxoverlap && totalDigit<currentDigit)
            {
                bestOffsetSoFar=startingOldOffset;
                currentDigit=totalDigit;
            }
        }
    }
    
    overlaplength = maxoverlap;
    return bestOffsetSoFar;
}

//this function creates the diff file
void createDiff(istream& fold, istream& fnew, ostream& fdiff)
{
    //Read in the entire contents of the old file into a string oldFile. Read the entire contents of the new file into another string newFile, using get so that newline characters are included
    string oldFile, newFile;
    char c;
    while(fold.get(c))
    {
        string temp(1,c);
        oldFile += temp;
    }
    
    while(fnew.get(c))
    {
        string temp(1,c);
        newFile += temp;
    }
    
    //For ALL consecutive N-character sequences in the old file's string, insert that N-character sequence and the offset F where it was found in the old file's string, into a hashtable
    OpenHashTable oht;
    for (int offset = 0; offset<= (int)oldFile.length()-N; offset++)
    {
        string oldSubsequence = oldFile.substr(offset, N);
        oht.insert(oldSubsequence, offset);
    }
    
    //start processing the new file's string, starting from offset j=0, until j reaches the end of the string.
    
    //Look up the N-byte sequence which starts at offset j ([j,j+N-1]) in the new file's string in the table you created from the old file's string.
    for (int j = 0; j<(int)newFile.length(); j++)
    {
        //if at this point, j is past newFile.length-N, then all the remaining has to be added.
        if (j> (int)newFile.length()-N)
        {
            string toBeAdded = newFile.substr(j);
            fdiff<<"A"<< (int)toBeAdded.length()<<":"<<toBeAdded;
            j = (int)newFile.length();
            break;
        }
        
        string newsubsequence = newFile.substr(j, N);
        bool found = oht.search(newsubsequence);

        //if the sequence is found, copy instruction is evoked
        if (found)
        {
            string newFileremaining = newFile.substr(j);
            int copyLength;
            int startingOldOffset = oht.copySearch(oldFile, newFileremaining, copyLength);

            //output copy instruction
            fdiff<<"C"<<copyLength<<","<<startingOldOffset;
            
            //next iteration, we continue at offset j = j + L in the new file's string
            j = j + copyLength -1;
        }
        
        //if the sequence is not found, addition instruction is evoked
        else
        {
            //while the subsequence is not found, we need to add
            string toBeAdded;
            while (!found)
            {
                toBeAdded += newFile.substr(j, 1);
                j++;
                if (j> (int)newFile.length()-N)
                {
                    toBeAdded += newFile.substr(j);
                    j=(int)newFile.length();
                    break;
                }
                newsubsequence = newFile.substr(j,N);
                found = oht.search(newsubsequence);
            }
            fdiff<<"A"<< (int)toBeAdded.length()<<":"<<toBeAdded;
            //move j beack to where it was
            j--;
        }
    }
}

//======================APPLYDIFF=========================================
//whenever an error is detected, i.e. there's a character other than an A or C where an instruction is expected,
// or an offset or length is invalid,
// or If a copy instruction specifies a length of 0 and an offset beyond the last character in the old file
//return false and exit


bool getInt(istream& inf, int& n)
{
    char ch;
    if (!inf.get(ch)  ||  !isascii(ch)  ||  !isdigit(ch))
        return false;
    inf.unget();
    inf >> n;
    return true;
}

//similar to getInt, this function returns a string with the indicated length (used for addition function, to get the string to be added)
bool getString(istream& inf, int length, string& obtained)
{
    
    for (int i = 0; i<length; i++)
    {
        char ch;
        if (!inf.get(ch)  ||  length<0 )
            return false;
        string temp(1, ch);
        obtained+=temp;
    }
    return true;
}

bool getCommand(istream& inf, char& cmd, int& length, int& offset)
{
    if (!inf.get(cmd))
    {
        cmd = 'x';  // signals end of file
        return true;
    }
    char ch;
    switch (cmd)
    {
        case 'A':
            return getInt(inf, length) && inf.get(ch) && ch == ':';
        case 'C':
            return getInt(inf, length) && inf.get(ch) && ch == ',' && getInt(inf, offset);
        case '\r':
        case '\n':
            return true;
    }
    return false;
}

bool applyDiff(istream& fold, istream& fdiff, ostream& fnew)
{
    string oldFile;
    char c;
    while(fold.get(c))
    {
        string temp(1,c);
        oldFile += temp;
    }
    
    char command;
    int length=0;
    int offset = 0;
    
    //if anything is invalid, return false
    if (!getCommand(fdiff, command, length, offset))
    {
        return false;
    }
    
    //keep reading commands as long as the end of file is not reached
    while (command!='x')
    {
        //error checking:
        if (length<0)
            return false;
        if (command=='\r' || command=='\n')
        {
            if (getCommand(fdiff, command, length, offset))
            {
                continue;
            }
            else
            {
                return false;
            }
        }
        
        if (command == 'A')
        {
            string output;
            if (!getString(fdiff, length, output))
                return false;
            //write the string in newFile that is supposed to be added
            fnew<<output;
        }
        
        else if (command == 'C')
        {
            if (offset<0 || offset>= (int)oldFile.length())
                return false;
            //take the string from the oldfile and write it to the new file
            string tobecopied = oldFile.substr(offset,length);
            fnew<<tobecopied;
        }
        
        if (!getCommand(fdiff, command, length, offset))
        {
            return false;
        }
    }
    return true;
}
