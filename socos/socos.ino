#include <SPI.h>
#include <SD.h>
//#include <DhcpV2_0.h>
//#include <DnsV2_0.h>
//#include <EthernetClientV2_0.h>
//#include <EthernetServerV2_0.h>
//#include <EthernetUdpV2_0.h>
//#include <EthernetV2_0.h>
//#include <Wire.h>

#define BAUD_RATE 9600
#define MEM_MAX 2048//4096//512
#define CACHE_MAX 2048

#define SPK_PIN 2
#define SDC_PIN 4
#define ETH_PIN 10
#define FAN_PIN 3
#define IN1_PIN 28
#define IN2_PIN 26
#define IN3_PIN 24
#define IN4_PIN 22

String curPath = "/";
String homePath = "/HOME/ROOT";
String userName = "ROOT";

byte switches = 255;

void setup() {
  Serial.begin(BAUD_RATE);//SERIAL_8O1
  while(!Serial);

  //setup speaker
  pinMode(SPK_PIN,OUTPUT);
  tone(SPK_PIN,700,100);
  delay(500);
  tone(SPK_PIN,700,100);
  delay(300);

  //setup switches
  pinMode(IN1_PIN,OUTPUT);
  pinMode(IN2_PIN,OUTPUT);
  pinMode(IN3_PIN,OUTPUT);
  pinMode(IN4_PIN,OUTPUT);

  digitalWrite(IN1_PIN,HIGH);
  digitalWrite(IN2_PIN,HIGH);
  digitalWrite(IN3_PIN,HIGH);
  digitalWrite(IN4_PIN,HIGH);
  
  //start fan
  TOGGLE(1);

  // Disable the Ethernet interface
  pinMode(ETH_PIN, OUTPUT);
  digitalWrite(ETH_PIN, HIGH);
  
  Serial.println(F("\u001b[32m."));
  CLS();
  Serial.println(F("                                                        "));
  Serial.println(F("  ▄██████▄   ▄██████▄   ▄██████▄   ▄██████▄   ▄██████▄  "));
  Serial.println(F(" ███    ███ ███    ███ ███    ███ ███    ███ ███    ███ "));
  Serial.println(F(" ███    █▀  ███    ███ ███    █▀  ███    ███ ███    █▀  "));
  Serial.println(F(" ███▄▄▄▄▄   ███    ███ ███        ███    ███ ███▄▄▄▄▄   "));
  Serial.println(F("   ▀▀▀▀▀███ ███    ███ ███        ███    ███   ▀▀▀▀▀███ "));
  Serial.println(F("  ▄█    ███ ███    ███ ███    █▄  ███    ███  ▄█    ███ "));
  Serial.println(F(" ███    ███ ███    ███ ███    ███ ███    ███ ███    ███ "));
  Serial.println(F("  ▀██████▀   ▀██████▀   ▀██████▀   ▀██████▀   ▀██████▀  "));
  Serial.println(F("                                                        "));

  Serial.print(F("Initializing SD card communications"));
  byte i=0;
  while(!SD.begin(SDC_PIN)){
    i%=4;
    if(i==3)
      Serial.print(F("Initializing SD card communications"));
    else
      Serial.print(((i==2)?F(".\n"):F(".")));
    i++;
  }
  Serial.println();
}

void loop(){
  String input="";
  Serial.print(userName+F(":")+curPath+F("$>"));
  //while(Serial.available()==0);
  char charIn;
  while((Serial.peek()!='\n') && (Serial.peek()!='\r')){
    if((Serial.available()>0)&&(Serial.peek()!='\n') && (Serial.peek()!='\r')){
      charIn = (char)Serial.read();
      //convert to upper case
      if(((byte)charIn)>96&&(((byte)charIn)<123))
        charIn = (charIn-32);
      //handle ansi codes
      if(charIn=='\e'){
        delay(5);
        if(Serial.peek()=='['){
          Serial.read();
          charIn = Serial.read();
        }
      //handle backspaces
      }else if(charIn=='\b' || charIn=='\177'){//backspace
        if(input.length()>0){
          Serial.print(charIn);
          input = input.substring(0,input.length()-1);
        }
        else
          Serial.print('\a');//bell
      }else{
        Serial.print(charIn);
        input.concat(String(charIn));//needs String((char)byte)
      }
    }
    //delay(((10.0/BAUD_RATE)*1000));
  }
  delay(10);
  while((Serial.peek()=='\r')||(Serial.peek()=='\n')){
    Serial.read();
    delay(1);
  }
  input.toUpperCase();
  input.trim();
  Serial.print('\n');
  input.concat(F(" "));
  String commands[] = {F("DIR "),F("LS "),F("CD "),F("CHDIR "),F("RUN "),F("START "),F("EXEC "),F("MEM "),F("FREE "),F("CLS "),F("CLR "),F("TONE "),F("BEEP "),F("TOGGLE ")};
  for(int i=0;i<14;i++)
    if(input.startsWith(commands[i])){
      input.remove(0,input.indexOf(F(" "))+1);
      input.trim();
      switch(i){
        case 0://dir
        case 1://ls
          DIR(input);
          break;
        case 2://cd
        case 3://chdir
          CD(input);
          break;
        case 4://run
        case 5://start
        case 6://exec
          RUN(input);
          break;
        case 7://mem
        case 8://free
          RAM();
          break;
        case 9://cls
        case 10://clr
          CLS();
        break;
        case 11://tone
        case 12://beep
          TONE(input.toInt(),1000);
        break;
        case 13://toggle
          TOGGLE(input.toInt());
        break;
        default: 
          Serial.println(F("ERROR"));
        break;
      }
      return;
    }
  Serial.println(F("COMMAND NOT FOUND"));
}


//DO NOT TOUCH finaly working
void CD(String &nameIn){
  if(nameIn.equalsIgnoreCase(""))
    if(homePath.equalsIgnoreCase(""))
      return;
    else{
      nameIn=homePath;
    }
  resolvePath(nameIn);
  if(SD.exists(curPath)||curPath.equalsIgnoreCase("/"))
    curPath=nameIn;
}

//DO NOT TOUCH finaly working
void RUN(String &nameIn){
  char pos = nameIn.lastIndexOf("/");
  String path = F("/sbin/");//posibility of not doing check
  if(pos!=-1){
    path = nameIn.substring(0,pos);
    nameIn = nameIn.substring(pos);
    resolvePath(path);
  }
  path.concat(nameIn);
  path.concat(".bf");
  if(SD.exists(path)){
    File file = SD.open(path);
    if(file){
      unsigned short pointer = 0;
      char *memory = new char[MEM_MAX];
      char *cache = new char[CACHE_MAX];
      for(int i=0;i<MEM_MAX;i++)
        memory[i] = 0;
      file.seek(0);
      while(file.available()){
        char command = file.peek();
        //RAM();
        //Serial.print(command);
        //WIP
        //for(int i=0;i<256;i++){
        //  Serial.print((0+memory[i]));
        //  Serial.print(',');
        //}
        //Serial.println();
        //Serial.print(memory[pointer]);
        char chars[] = {'>','<','+','-','.',',','[',']'};
        for(int i=0;i<8;i++)//can be reduced
          if(command == chars[i]){
            switch (i) {
              case 0://>  increment the data pointer (to point to the next cell to the right).
                if(pointer<(MEM_MAX-1))
                  pointer++;
                else
                  pointer=0;
                break;
              case 1://< decrement the data pointer (to point to the next cell to the left).
                if(pointer>0)
                  pointer--;
                else
                  pointer=(MEM_MAX-1);
                break;
              case 2://+ increment (increase by one) the byte at the data pointer.
                if(memory[pointer]!=byte(255))
                  memory[pointer]++;
                else
                  memory[pointer]=byte(0);
                break;
              case 3://- decrement (decrease by one) the byte at the data pointer.
                if(memory[pointer]!=byte(0))
                  memory[pointer]--;
                else
                  memory[pointer]=byte(255);
                break;
              case 4://. output the byte at the data pointer.
                Serial.print(memory[pointer]);
                break;
              case 5://, accept one byte of input, storing its value in the byte at the data pointer.
                while(Serial.available()==0);
                memory[pointer]=(char)Serial.read();
                break;
              case 6://[ if the byte at the data pointer is zero, then instead of moving the instruction pointer forward to the next command, jump it forward to the command after the matching ] command.
                if(memory[pointer]==0){
                  char curChar=0;
                  short curLevel=1;
                  do{
                    if(!file.seek(file.position()+1))
                      break;
                    curChar = file.peek();
                    //Serial.print(curChar);
                    if(curChar=='[')
                      curLevel++;
                    if(curChar==']')
                      curLevel--;
                  }while((curLevel>0));
                }
                break;
              case 7://] if the byte at the data pointer is nonzero, then instead of moving the instruction pointer forward to the next command, jump it back to the command after the matching [ command.
                if(memory[pointer]!=0){
                  char curChar=0;
                  short curLevel=1;
                  do{
                    if(!file.seek(file.position()-1))
                      break;
                    curChar = file.peek();
                    //Serial.print(curChar);
                    if(curChar==']')
                      curLevel++;
                    if(curChar=='[')
                      curLevel--;
                  }while((curLevel>0));
                }
                break;
              default: 
                Serial.println(F("ERROR"));
                break;
            }
            i=8;
          }
          file.read();
      }
      delete memory;
    }
    file.close();
  }
  Serial.println();
}

void SAVE(String &nameIn,String &contentsIn){
  if(!nameIn.startsWith("/"))
    nameIn = curPath+"/"+nameIn;
  if(SD.exists(nameIn)){
    File file = SD.open(nameIn, FILE_WRITE);
    if(file){
        file.print(contentsIn);
      file.close();
    }
  }
}

//broken
void DELETE(String &nameIn){
  if(SD.exists(nameIn)){
    File file = SD.open(nameIn);
    rDelete(file);
    Serial.println("File Deleted");
  }else{
    Serial.println("File Not Found");
  }
}

void rDelete(File &fileIn){
  String fileName = fileIn.name();
  if(fileIn.isDirectory()){
    File file;
    do{
      if(file)
        rDelete(file);
      file = fileIn.openNextFile();
    }while(file);
    fileIn.close();
    SD.rmdir(fileName);
  }else{
    fileIn.close();
    SD.remove(fileName);
  }
}

//DIR Command lists current directory or specified directory
void DIR(String &nameIn){
  resolvePath(nameIn);
  //dir code begins here    
  if(SD.exists(nameIn) || nameIn.equalsIgnoreCase("/"))
  {
    File file = SD.open(nameIn);
    if(file && file.isDirectory())
    {
      Serial.println("DIRECTORY OF " + nameIn);
      file.rewindDirectory();
      File subFile;
      do{
        if(!!(subFile)){
          Serial.print("|\t");
          Serial.print(subFile.name());
          if(subFile.isDirectory())
            Serial.println("/");
          else
          {
            Serial.print('\t');
            if(String(subFile.name()).length()<8)
              Serial.print('\t');
            Serial.print(subFile.size()/1024.0);
            Serial.println(F(" kb"));
          }
          subFile.close();
        }
        subFile =  file.openNextFile();
      }while(!!(subFile));
      file.close();
    }
  }
}

void TONE(int pitch,int duration){
  if(pitch<=0)
    noTone(SPK_PIN);
  else if(duration<=0)
    tone(SPK_PIN,pitch);
  else
    tone(SPK_PIN,pitch,duration);
}

void TOGGLE(int num){
  byte switchesIn;
  switch(num){
    case 1:
      switchesIn = 1;
    break;
    case 2:
      switchesIn = 2;
    break;
    case 3:
      switchesIn = 4;
    break;
    case 4:
      switchesIn = 8;
    break;
    case 5:
      switchesIn = 16;
    break;
    case 6:
      switchesIn = 32;
    break;
    case 7:
      switchesIn = 64;
    break;
    case 8:
      switchesIn = 128;
    break;
    default:
      switchesIn = 0;
    break;
  }
  switches=switches^switchesIn;
  digitalWrite(IN1_PIN,((switches>>0)&0x01));
  digitalWrite(IN2_PIN,((switches>>1)&0x01));
  digitalWrite(IN3_PIN,((switches>>2)&0x01));
  digitalWrite(IN4_PIN,((switches>>3)&0x01));
}

void CLS(){
  Serial.println('\f');
  Serial.println('\r');
}

int RAM(){
  extern int __heap_start, *__brkval; 
  int v; 
  Serial.print(F("RAM:\t"));
  Serial.print((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval)); 
  Serial.println(F(" b"));
  return ((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
}

//do not touch
void resolvePath(String &pathIn){
  String tempPath=curPath;
  if(pathIn.startsWith("/")||(pathIn.startsWith(".")||pathIn.startsWith("..")))
  {
    tempPath=pathIn;
    if(!pathIn.startsWith("/"))
      if(curPath.endsWith("/"))
        tempPath=curPath+pathIn;
      else
        tempPath=curPath+"/"+pathIn;
  }else{
    if(!tempPath.endsWith("/"))
      tempPath.concat("/");
    tempPath.concat(pathIn);
  }
  if(!tempPath.endsWith("/"))//make sure string ends in slash
    tempPath.concat("/");
  while(tempPath.indexOf("/./") != -1)//remove cur dirs
    tempPath.remove(tempPath.indexOf("/./")+1,2);
  while(tempPath.indexOf("/../") != -1){//remove up dirs
    tempPath.remove(tempPath.lastIndexOf("/",tempPath.indexOf("/../")-1)+1,tempPath.indexOf("/../")-tempPath.lastIndexOf("/",tempPath.indexOf("/../")-2));
    tempPath.remove(tempPath.indexOf("/../")+1,3);
  }
  if(tempPath.endsWith("/")&&!tempPath.equalsIgnoreCase("/"))//make sure to remove end slashes
    tempPath.remove(tempPath.lastIndexOf("/"));
  if(!(SD.exists(tempPath)||tempPath.equalsIgnoreCase("/")))
    tempPath=curPath;
  pathIn=tempPath;
}
