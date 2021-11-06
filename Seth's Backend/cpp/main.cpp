#include<stdio.h>
#include<iostream>
#include<vector>
#include<string>
#include<conio.h>
#include "Ui.h"
#include <math.h>
#include <stdlib.h>     //exit()
#include <stdio.h>

#include "../../LoRa/lora.h"

using namespace std;

//represents the maximum limits for the module
//use low numbers for debugging purposes, high numbers for actual implementation
int MAXSTRINGLENGTH = 50;
int MAXMESSAGES = 10;
int DISPLAYABLEMESSAGES = 12;

vector<string> FULLSCREEN;

//builds a string from individual key presses
//max length is defined by 
string builtString(char initialChar)
{
    vector<char> charArr;
    charArr.push_back(initialChar);
    cout << initialChar;
    char thisChar;
    while (thisChar != '\n'){
        thisChar = cin.get();
        charArr.push_back(thisChar);
    }
    
    return string(charArr.begin(),charArr.end());

}

//returns the available on screen strings
void screenUpdate(int dataDisplayEnd, vector<string> data){
    
    if (dataDisplayEnd > DISPLAYABLEMESSAGES){
        dataDisplayEnd = DISPLAYABLEMESSAGES;
    } else if (dataDisplayEnd <= data.size()){
        dataDisplayEnd = data.size();
    }
    //cout << "dde " << ": " << dataDisplayEnd << endl;
    for (int i = dataDisplayEnd-DISPLAYABLEMESSAGES; i<dataDisplayEnd; i++){
        //cout << "message " << i << endl;
        if (i>=0){
            cout << "message " << i << ": " <<data[i] << endl;
        }
    }
    
}

void printToScreen(int dataDisplayEnd, vector<string> data, vector<char> typingChars){
    //cout << "DDEIN " << dataDisplayEnd << endl;
    if (dataDisplayEnd > DISPLAYABLEMESSAGES+dataDisplayEnd){
        dataDisplayEnd = DISPLAYABLEMESSAGES+dataDisplayEnd;
    } else if (dataDisplayEnd <= data.size()){
        dataDisplayEnd = data.size();
    }
    FULLSCREEN.clear();
    //cout << "DDEOUT " << dataDisplayEnd << endl;

    for (int i = dataDisplayEnd-DISPLAYABLEMESSAGES; i<dataDisplayEnd; i++){
        //cout << "message " << i << endl;
        if (i>=0){
            //cout << "message " << i << ": " <<data[i] << endl;
            FULLSCREEN.push_back(data[i]);
        }
    }
    //cout << "currently typing " << string(typingChars.begin(),typingChars.end()) << endl;
    FULLSCREEN.push_back(string(typingChars.begin(),typingChars.end()));
}




int main(){

    string LoraText;
    vector<string> storedData;
    int messageCount = 0;
    int readingMessage = 0;
    char ch;
    string keystrokes;
    
    bool doneWriting = false;
    vector<char> curChars;
    string curStr;

    LCD_INIT();

    int loraTemp = loraInit("/dev/ttyAMA0", 115200);
    if (loraTemp < 0)
    {
      return loraTemp;
    }

    while (1) {
        if (kbhit()){
            doneWriting = false;
            ch = getch();
            //cout << "char was : " << ch << '\n';
            if (int(ch) == 27)
                break;
	    else if (int(ch) == 127 || int(ch) == 8) {
		curChars.pop_back();
		printToScreen(readingMessage, storedData, curChars);
		LCD_text(FULLSCREEN);
	    }
            else if (int(ch) == 91){
                //[ key moves up
                //need to fix
		LCD_up();
                //readingMessage--;
                //printToScreen(readingMessage, storedData, curChars);
		//LCD_text(FULLSCREEN);
            } 
            else if (int(ch) == 93){
                //] moves down
		LCD_down();
                //readingMessage++;
                //printToScreen(readingMessage, storedData, curChars);
		//LCD_text(FULLSCREEN);
            }
            else if (int(ch) == 10){
                if (curChars.size() != 0){
                    doneWriting = true;
                    //send off to data and lora module
                    curStr = string(curChars.begin(),curChars.end());
                    //cout << "final string was: " << curStr << '\n';
                    int numMessages = ((curChars.size())/(MAXSTRINGLENGTH))+1;
                    int charsLeft = curChars.size();
                    

                    for (int i = 0; i < numMessages; i++){
                        //cout << "letters left " << charsLeft-MAXSTRINGLENGTH;
                        messageCount++;
                        if (charsLeft > MAXSTRINGLENGTH) {
                            
                            //cout << "added this string to vector " << string(curChars.begin()+(i*MAXSTRINGLENGTH),curChars.begin()+((i+1)*MAXSTRINGLENGTH)) <<endl;
                            storedData.push_back(string(curChars.begin()+(i*MAXSTRINGLENGTH),curChars.begin()+((i+1)*MAXSTRINGLENGTH)));
                            charsLeft -= MAXSTRINGLENGTH;
                        } else {
                            //cout << "add this string to vector " << string(curChars.end() - charsLeft,curChars.end()) << endl;
                            storedData.push_back(string(curChars.end() - charsLeft,curChars.end()));
                            readingMessage = messageCount;
                        }
                        if (messageCount > MAXMESSAGES){
                            storedData.pop_back();
                            messageCount--;
                            readingMessage = messageCount;
                        }
                        
                    }
                    curChars.clear();
                }
            } else {
                curChars.push_back(ch);
                //cout << readingMessage << " this" << endl;
                //cout << "current string is " << string(curChars.begin(),curChars.end()) << '\n';
                printToScreen(readingMessage, storedData, curChars);
		LCD_text(FULLSCREEN);
            }
            
        }
        //reads input from LORA module
        
        
    //     //reads keyboard
    //     if (kbhit()){
    //         ch = getch();
    //         //esc terminates program
    //         if (int(ch) == 27)
    //             break;
    //         /*
    //         TODO: FIX PAGEMOVEMENT
    //         if (int(ch) == 13)
    //         {
    //             //cout << "pgup" << endl;
    //             readingMessage--;
    //             screenUpdate(readingMessage, storedData);
    //             //page up
    //         } else if (int(ch) == 221){
    //             //page down
    //             readingMessage++;
    //             screenUpdate(readingMessage, storedData);
                
    //         else {
            
    //             keystrokes = builtString(ch);
    //             keystrokes.pop_back();
    //             cout << "you entered: " << keystrokes << endl;
            
    //             int stringPos = 0;
    //             int numMessages = ((keystrokes.length())/(MAXSTRINGLENGTH))+1;
            
    //             for (int i = 0; i < numMessages-1; i++){
                    
                    
    //                 storedData.push_back(keystrokes.substr(stringPos,MAXSTRINGLENGTH));
    //                 stringPos+=MAXSTRINGLENGTH;
    //                 if (messageCount>=MAXMESSAGES){
    //                     storedData.erase(storedData.begin());
    //                 }
    //                 messageCount++;
                
    //                 //TODO:
    //                 //Send to LORA MODULE HERE
                   
    //             }
    //             //cout << "finishingStrng" << (keystrokes.length())%MAXSTRINGLENGTH <<endl;
    //             if ((keystrokes.length())%MAXSTRINGLENGTH !=0){
    //                 storedData.push_back(keystrokes.substr(stringPos));
    //                 if (messageCount>=MAXMESSAGES){
    //                     storedData.erase(storedData.begin());
    //                 }
    //                 messageCount++;
    //                 //TODO: SEND TO LORA MODULE HERE
    //                 //updateScreen()
    //             }
    //             //cout << "string was split this many ways: " << numMessages << " there are this many messages stored " << messageCount << endl;
    //             readingMessage == messageCount;
    //             screenUpdate(readingMessage, storedData);
    //         /*
    //         for(int i = 0; i < messageCount; i++){
    //             cout << "data set " << i << ": " << storedData[i] << endl;
    //         }
    //         */
    //         }    
    //     }
        
        


       /*
        //TODO: FIX THIS UP
        //DATA INCOMING FROM LORA MODULE
        //if(Data is from lora module and not keyboard){
            getline(cin, LoraText);
            cout << LoraText <<endl;
            messageCount++;
            if (messageCount >= readingMessage-DISPLAYABLEMESSAGES){
                readingMessage++;
            }
            storedData.push_back(LoraText);
            //break;
        //} 
        if (LoraText == "stop"){
            break;
        }

*/

    }	
	LCD_exit();
}

