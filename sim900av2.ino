/************************************************************************************
 *  Created By: Tauseef Ahmad
 *  Created on: 28 November, 2021
 *  Updated on: 2 Decenmer, 2021
 *  YouTube Video: https://youtu.be/IiheQAMT3kA
 *  My Channel: https://www.youtube.com/channel/UCOXYfOHgu-C-UfGyDcu5sYw/
 *  
 **********************************************************************************/

#include <SoftwareSerial.h>
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
#include <EEPROM.h>
//sender phone number with country code
//const String PHONE = "ENTER_PHONE_HERE";
const int totalPhoneNo = 5;
String phoneNo[totalPhoneNo] = {"","","","",""};
int offsetPhone[totalPhoneNo] = {0,15,30,45,60};
String tempPhone = "";
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
//GSM Module RX pin to Arduino 3
//GSM Module TX pin to Arduino 2
#define rxPin 2
#define txPin 3
SoftwareSerial SIM900A(rxPin,txPin);
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
#define RELAY_A 4
#define RELAY_B 5
#define RELAY_C 6
#define RELAY_D 7
boolean STATE_RELAY_A = 0;
boolean STATE_RELAY_B = 0;
boolean STATE_RELAY_C = 0;
boolean STATE_RELAY_D = 0;
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
String smsStatus,senderNumber,receivedDate,msg;
boolean isReply = false;
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM


boolean DEBUG_MODE = 1;
int LED1 = 11;
int LED2 = 12;
int LED3 = 13;

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
int ldr=A0;//Set A0(Analog Input) for LDR.
int value=0;
/*******************************************************************************
 * setup function
 ******************************************************************************/
void setup() {
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  Serial.begin(9600);
  Serial.println("Arduino serial initialize");
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  SIM900A.begin(9600);
  Serial.println("SIM900A software serial initialize");
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  pinMode(RELAY_A, OUTPUT); //Relay A
  digitalWrite(RELAY_A, HIGH);
  pinMode(RELAY_B, OUTPUT); //Relay B
  digitalWrite(RELAY_B, HIGH);
  pinMode(RELAY_C, OUTPUT); //Relay C
  digitalWrite(RELAY_C, HIGH);
  pinMode(RELAY_D, OUTPUT); //Relay D
  digitalWrite(RELAY_D, HIGH);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  Serial.println("List of Registered Phone Numbers");
  for (int i = 0; i < totalPhoneNo; i++){
    phoneNo[i] = readFromEEPROM(offsetPhone[i]);
    if(phoneNo[i].length() != 15)
      {
        phoneNo[i] = "";
        Serial.println(String(i+1)+": empty");
      }
    else
      {
        Serial.println(String(i+1)+": " + phoneNo[i]);
      }
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  delay(7000);
  SIM900A.println("AT+CMGF=1"); //SMS text mode
  delay(1000);
  SIM900A.println("AT+IPR=9600"); //SMS text mode
  delay(1000);
  //delete all sms
  SIM900A.println("AT+CMGD=1,4");
  delay(1000);
  SIM900A.println("AT+CMGDA= \"DEL ALL\"");
  delay(1000);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  //MMMMMMMMMMMMMMMMMM--System ready--MMMMMMMMMMMMMMM
  pinMode(LED3, OUTPUT);
  digitalWrite(LED3, HIGH);
  delay(200);
  ABCON();
}// end setup




/*******************************************************************************
 * Loop Function
 ******************************************************************************/
void loop() {
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
while(SIM900A.available()){
  parseData(SIM900A.readString());
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
while(Serial.available())  {
  SIM900A.println(Serial.readString());
}
ABCON();
LDR();
//MMMMMMMMMMMMMMMMMM
} //main loop ends





/*******************************************************************************
 * parseData function:
 * this function parse the incomming command such as CMTI or CMGR etc.
 * if the sms is received. then this function read that sms and then pass 
 * that sms to "extractSms" function. Then "extractSms" function divide the
 * sms into parts. such as sender_phone, sms_body, received_date etc.
 ******************************************************************************/
void parseData(String buff){
  unsigned int len, index;
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Remove sent "AT Command" from the response string.
  index = buff.indexOf("\r");
  buff.remove(0, index+2);
  buff.trim();
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  
  if(buff != "OK"){
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();
    
    buff.remove(0, index+2);
    Serial.println(buff);
    if(cmd == "+CMT"){
        SIM900A.println("AT+CNMI=2,2,0,0,0"); // AT Command to receive a live SMS
        delay(2000);
        extractSms(buff);
        delay(3000);
        //----------------------------------------------------------------------------
      if(msg.equals("r") && phoneNo[0].length() != 15) {
        writeToEEPROM(offsetPhone[0],senderNumber);
        phoneNo[0] = senderNumber;
        String text = "Number is Registered: ";
        text = text + senderNumber;
        debugPrint(text);
        Reply("Number is Registered", senderNumber);
      }
      //----------------------------------------------------------------------------
      if(comparePhone(senderNumber)){
        doAction(senderNumber);
        //delete all sms
        SIM900A.println("AT+CMGD=1,4");
        delay(1000);
        SIM900A.println("AT+CMGDA= \"DEL ALL\"");
        delay(1000);
      }
      //----------------------------------------------------------------------------
    }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  }
  else{
  //The result of AT Command is "OK"
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
}





/*******************************************************************************
 * extractSms function:
 * This function divide the sms into parts. such as sender_phone, sms_body, 
 * received_date etc.
 ******************************************************************************/
void extractSms(String buff){
   unsigned int index;
   
    index = buff.indexOf(",");
    senderNumber = buff.substring(0, 15);
    Serial.println(senderNumber);
    buff.remove(0,19);
    receivedDate = buff.substring(0, 20);
    buff.remove(0,buff.indexOf("\r"));
    buff.trim();
    
    index =buff.indexOf("\n\r");
    buff = buff.substring(0, index);
    buff.trim();
    msg = buff;
    buff = "";
    msg.toLowerCase();

    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
    String tempcmd = msg.substring(0, 3);
    if(tempcmd.equals("r1=") || tempcmd.equals("r2=") ||
       tempcmd.equals("r3=") || tempcmd.equals("r4=") ||
       tempcmd.equals("r5=")){
        
        tempPhone = msg.substring(3, 18);
        msg = tempcmd;
        //debugPrint(msg);
        //debugPrint(tempPhone);
    }
    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
       
}




/*******************************************************************************
 * doAction function:
 * Performs action according to the received sms
 ******************************************************************************/
void doAction(String phoneNumber){
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  if(msg == "aon"){  
    digitalWrite(RELAY_A, LOW);
    STATE_RELAY_A = 1;
    debugPrint("Relay A is ON");
    Reply("Relay A is ON", phoneNumber);
  }
  else if(msg == "aoff"){
    digitalWrite(RELAY_A, HIGH);
    STATE_RELAY_A = 0;
    debugPrint("Relay A is OFF");
    Reply("Relay A is OFF", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  if(msg == "bon"){  
    digitalWrite(RELAY_B, LOW);
    STATE_RELAY_B = 1;
    debugPrint("Relay B is ON");
    Reply("Relay B is ON", phoneNumber);
  }
  else if(msg == "boff"){
    digitalWrite(RELAY_B, HIGH);
    STATE_RELAY_B = 0;
    debugPrint("Relay B is OFF");
    Reply("Relay B is OFF", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  if(msg == "con"){  
    digitalWrite(RELAY_C, LOW);
    STATE_RELAY_C = 1;
    debugPrint("Relay C is ON");
    Reply("Relay C is ON", phoneNumber);
  }
  else if(msg == "coff"){
    digitalWrite(RELAY_C, HIGH);
    STATE_RELAY_C = 0;
    debugPrint("Relay C is OFF");
    Reply("Relay C is OFF", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  if(msg == "don"){  
    digitalWrite(RELAY_D, LOW);
    STATE_RELAY_D = 1;
    debugPrint("Relay D is ON");
    Reply("Relay D is ON", phoneNumber);
  }
  else if(msg == "doff"){
    digitalWrite(RELAY_D, HIGH);
    STATE_RELAY_D = 0;
    debugPrint("Relay D is OFF");
    Reply("Relay D is OFF", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  if(msg == "abcon"){  
    digitalWrite(RELAY_A, LOW);
    digitalWrite(RELAY_B, LOW);
    digitalWrite(RELAY_C, LOW);
    STATE_RELAY_A = 1;
    STATE_RELAY_B = 1;
    STATE_RELAY_C = 1;
    debugPrint("3 pahases are ON");
    Reply("3 pahases are ON", phoneNumber);
  }
  else if(msg == "abcoff"){
    digitalWrite(RELAY_A, HIGH);
    digitalWrite(RELAY_B, HIGH);
    digitalWrite(RELAY_C, HIGH);
    STATE_RELAY_A = 0;
    STATE_RELAY_B = 0;
    STATE_RELAY_C = 0;
    debugPrint("3 pahases are OFF");
    Reply("3 pahases are OFF", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "stat=a"){
    String text = (STATE_RELAY_A)? "ON" : "OFF";
    debugPrint("Relay A is "+text);
    Reply("Relay A is "+text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "r2="){  
      writeToEEPROM(offsetPhone[1],tempPhone);
      phoneNo[1] = tempPhone;
      String text = "Phone2 is Registered: ";
      text = text + tempPhone;
      debugPrint(text);
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "r3="){  
      writeToEEPROM(offsetPhone[2],tempPhone);
      phoneNo[2] = tempPhone;
      String text = "Phone3 is Registered: ";
      text = text + tempPhone;
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "r4="){  
      writeToEEPROM(offsetPhone[3],tempPhone);
      phoneNo[3] = tempPhone;
      String text = "Phone4 is Registered: ";
      text = text + tempPhone;
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "r5="){  
      writeToEEPROM(offsetPhone[4],tempPhone);
      phoneNo[4] = tempPhone;
      String text = "Phone5 is Registered: ";
      text = text + tempPhone;
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "list"){  
      String text = "";
      if(phoneNo[0])
        text = text + phoneNo[0]+"\r\n";
      if(phoneNo[1])
        text = text + phoneNo[1]+"\r\n";
      if(phoneNo[2])
        text = text + phoneNo[2]+"\r\n";
      if(phoneNo[3])
        text = text + phoneNo[3]+"\r\n";
      if(phoneNo[4])
        text = text + phoneNo[4]+"\r\n";
        
      debugPrint("List of Registered Phone Numbers: \r\n"+text);
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=1"){  
      writeToEEPROM(offsetPhone[0],"");
      phoneNo[0] = "";
      Reply("Phone1 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=2"){  
      writeToEEPROM(offsetPhone[1],"");
      phoneNo[1] = "";
      debugPrint("Phone2 is deleted.");
      Reply("Phone2 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=3"){  
      writeToEEPROM(offsetPhone[2],"");
      phoneNo[2] = "";
      debugPrint("Phone3 is deleted.");
      Reply("Phone3 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=4"){  
      writeToEEPROM(offsetPhone[3],"");
      phoneNo[3] = "";
      debugPrint("Phone4 is deleted.");
      Reply("Phone4 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=5"){  
      writeToEEPROM(offsetPhone[4],"");
      phoneNo[4] = "";
      debugPrint("Phone5 is deleted.");
      Reply("Phone5 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM  
  if(msg == "del=all"){  
      writeToEEPROM(offsetPhone[0],"");
      writeToEEPROM(offsetPhone[1],"");
      writeToEEPROM(offsetPhone[2],"");
      writeToEEPROM(offsetPhone[3],"");
      writeToEEPROM(offsetPhone[4],"");
      phoneNo[0] = "";
      phoneNo[1] = "";
      phoneNo[2] = "";
      phoneNo[3] = "";
      phoneNo[4] = "";
      debugPrint("All phone numbers are deleted.");
      Reply("All phone numbers are deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";
  tempPhone = "";
}





/*******************************************************************************
 * Reply function
 * Send an sms
 ******************************************************************************/
void Reply(String text, String Phone)
{
    SIM900A.print("AT+CMGF=1\r");
    delay(1000);
    SIM900A.print("AT+CMGS="+ Phone + "\r");
    delay(1000);
    SIM900A.print(text);
    delay(1000);
    SIM900A.write(0x1A); //ascii code for ctrl-26 //SIM900A.println((char)26); //ascii code for ctrl-26
    delay(1000);
    Serial.println("SMS Sent Successfully.");
}



/*******************************************************************************
 * comparePhone function:
 * compare phone numbers stored in EEPROM
 ******************************************************************************/
boolean comparePhone(String number)
{
  boolean flag = 0;
  //--------------------------------------------------
  for (int i = 0; i < totalPhoneNo; i++){
    phoneNo[i] = readFromEEPROM(offsetPhone[i]);
    if(phoneNo[i].equals(number)){
      flag = 1;
      break;
    }
  }
  //--------------------------------------------------
  return flag;
}


/*******************************************************************************
 * writeToEEPROM function:
 * Store registered phone numbers in EEPROM
 ******************************************************************************/
void writeToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = 15; //strToWrite.length();
  //EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + i, strToWrite[i]);
  }
}






/*******************************************************************************
 * readFromEEPROM function:
 * Store phone numbers in EEPROM
 ******************************************************************************/
String readFromEEPROM(int addrOffset)
{
  int len = 15;
  char data[len + 1];
  for (int i = 0; i < len; i++)
  {
    data[i] = EEPROM.read(addrOffset + i);
  }
  data[len] = '\0';
  return String(data);
}
/*******************************************************************************
 * ABCON function:
 * Set Red Led LED2 on to show that the three phases  are on 
 ******************************************************************************/
 void ABCON(){
  if(STATE_RELAY_A == 1 && STATE_RELAY_B == 1 && STATE_RELAY_C == 1){
    digitalWrite(LED2, HIGH);
    delay(200);
  }
  else {
    digitalWrite(LED2, LOW);
    delay(200);
  }
 }
/*******************************************************************************
 * LDR function:
 * To verify ldr
 ******************************************************************************/
void LDR() {
value=analogRead(ldr);//Reads the Value of LDR(light).
Serial.println("LDR value is :");//Prints the value of LDR to Serial Monitor.
Serial.println(value);
}
/*******************************************************************************
 * debugPrint function:
 * compare phone numbers stored in EEPROM
 ******************************************************************************/
 void debugPrint(String text){
  if(DEBUG_MODE == 1)
    Serial.println(text);
 }
