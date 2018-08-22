//3rd party library: TinyGPS++ from: http://arduiniana.org/libraries/tinygpsplus/
//#include <TinyGPS++.h>
//TinyGPSPlus gps;


const int sentenceSize = 80;
char sentence[sentenceSize];
void init_gps() {

  Serial.println(F("Initializing GPS. Please wait..."));
  Serial1.begin(9600);
  Serial.println(F("GPS Initialized!"));
  

//  Serial1.println("
  unsigned int header = 0xB562;
  unsigned int ID = 0x0608;
  unsigned int msgSize = 6;
  unsigned int measRate = 200; //Measurement Rate, GPS measurements are taken every measRate milliseconds
  unsigned int navRate = 1; //Navigation Rate, in number of measurement cycles. On u-blox 5 and u-blox 6, this parameter cannot be changed, and is always equals 1.
  unsigned int timeRef = 1; //Alignment to reference time: 0 = UTC time, 1 = GPS time
  
  unsigned char msg[11];
  msg[0] = header>>8;
  msg[1] = header&0xFF;
  msg[2] = ID>>8;
  msg[3] = ID&0xFF;
  msg[4] = msgSize;
  msg[5] = measRate>>8;
  msg[6] = measRate&0xFF;
  msg[7] = navRate>>8;
  msg[8] = navRate&0xFF;
  msg[9] = timeRef>>8;
  msg[10] = timeRef&0xFF;

  //calculate 8-bit Fletcher Algorithm checksum
  unsigned char CK_A = 0;
  unsigned char CK_B = 0;
  for( int i=0; i<11; i++) {
    CK_A = CK_A + msg[i]; //THG: checksum calculation wrong
    CK_B = CK_B + CK_A;
  }

  for( int i=0; i<11; i++ ) {
    Serial1.write(msg[i]);
  }



    Serial1.write(msg, 11);
    Serial1.write(CK_A);
    Serial1.write(CK_B);

  Serial.print("MSG: ");
  for( int i=0; i<11; i++ ) {
    Serial.print(msg[i], HEX);
    Serial.print(" ");
  }
  Serial.print(CK_A, HEX);
  Serial.println(CK_B, HEX);

//while(true) {
//  while( Serial1.available() > 0 ) {
//    Serial.print(Serial1.read());
//  }
//}

}

void setup(){
Serial.begin(9600);


  char lng_r[11]="11705.8170\0";
  char lngn[2]="N";
  char k[20];
  
  toDegreeDecimalLng(lng_r,lngn,k);
  float testk=atof(k);
  Serial.println(testk,10);
  delay(200000);
//init_gps();
}

void loop()
{

  static int i = 0;
  if (Serial1.available())
  {

    char ch = Serial1.read();
  //Serial.print(ch);
    if (ch != '\n' && i < sentenceSize)
    {
      sentence[i] = ch;
      i++;
    }
    else
    {
     sentence[i] = '\0';
     i = 0;
     displayGPS();
    }
  }
}

void displayGPS()
{

  char field[20];
  char lat_raw[10];
  char lng_raw[11];
  char lat_raw_type[1];
  char lng_raw_type[1];
  char lat_compare[10];
  char lng_compare[11];
  //see: i=https://cdn.sparkfun.com/datasheets/Sensors/GPS/Venus/638/doc/Venus638FLPx_DS_v07.pdf
  //gpgga
  char gga_quality_indicator[3];
  char gga_satellites[2];
  char gga_hdop[4];

  char ggsa_hdop[4];
  char ggsa_vdop[4];
  char ggsa_mode[2];
  char ggsa_mode_fix[2];

 char lat[15];
 char lng[15];

//   getField(field, 0);
//if (strcmp(field, "$GPGSA") == 0){

//    getField(ggsa_mode, 1); //
//    getField(ggsa_mode_fix, 2); //
//    getField(ggsa_hdop, 15); //

//   Serial.print("mode:"); Serial.println(ggsa_mode);
//   Serial.print("modefix:"); Serial.println(ggsa_mode_fix);
//   Serial.print("hdop:"); Serial.println(ggsa_hdop);
//   return;
//  //     Serial.print("vdop:"); Serial.print(ggsa_vdop);
//  //     Serial.println("");
// }


  getField(field, 0);
  if (strcmp(field, "$GPGGA") == 0){
     getField(lat_compare, 2); 
     getField(lng_compare, 4); 
     getField(gga_quality_indicator, 6); //
     getField(gga_satellites, 7); //
     getField(gga_hdop, 8); //

//// Serial.print("hdop:"); Serial.print(gga_hdop);
//// Serial.println("");

// Serial.print("sats:"); Serial.print(gga_satellites); 
 //run some filtering testings
    int gga_qi=(gga_quality_indicator[0]-'0');
    int hdop= +(10*(gga_hdop[0]-'0')) +((gga_hdop[2]-'0'));//1.0 = 10 ,1.2 =12 
//  if(hdop < 15){
//      return;
//  }

//  if(gga_qi < 1){
//       return;
//  }


}

   getField(field, 0);
  if (strcmp(field, "$GPRMC") == 0)
  {
     getField(lat_raw, 3); 
     getField(lat_raw_type, 4);

     getField(lng_raw, 5); 
     getField(lng_raw_type, 6);


     if(strcmp(lat_raw, lat_compare) || strcmp(lng_raw, lng_compare)){
      //  Serial.println("Serial error: ignoring packet:");
       // return;
     }

     toDegreeDecimalLat(lat_raw,lat_raw_type,lat);
     toDegreeDecimalLng(lng_raw,lng_raw_type,lng);
    Serial.print(lat);
    Serial.print(",");
    Serial.print(lng);
    Serial.print("\n");

  }
}

void toDegreeDecimalLat(char* x,char* n,char *buff){
    unsigned long  degTenDec=(100000*(x[2]-'0'))
                   +((unsigned long) 10000*(x[3]-'0'))
                   +(1000*(x[5]-'0'))
                   +(100*(x[6]-'0'))
                   +(10*(x[7]-'0'))
                   +((x[8]-'0'));

    unsigned long deg=(degTenDec * 1000)/6;
    char deg_string[9];
    char deg_string_decimal[9];

    sprintf(deg_string, "%lu", deg);

    //16.6666500
    int length=0;
    for( int i=0; deg_string[i] != 0 ;i++){
         length++;
    }
    for( int i=0; i< (9-length) ;i++){
        deg_string_decimal[i]='0';
    }
    int c=0;
    for(int i=(9-length); i< 9 ;i++){
        deg_string_decimal[i]=deg_string[c];
        c++;
    }
    deg_string_decimal[9]='\0';

    //add first two number to get the non decimal amount
    int DD=(10*(x[0]-'0'))+((x[1]-'0'));
    int MM=((deg_string_decimal[0]-'0'));
    int DD_MM=DD+MM;
    char dd_final[3];
    sprintf(dd_final, "%i", DD_MM);
    //if under ten we pad dd_final
    if(DD_MM < 10){
        dd_final[1]==dd_final[0];
        dd_final[0]=='0';
    }
  if (n[0]=='S'){ 
    buff[0]='-';
    buff[1]=dd_final[0];
    buff[2]=dd_final[1];
    buff[3]='.';
    for(int i=4; i< 12 ;i++){
        buff[i]=deg_string_decimal[i-3];
    }
    buff[12]='\0';
  }else{
    buff[0]=dd_final[0];
    buff[1]=dd_final[1];
    buff[2]='.';
    for(int i=3; i< 11 ;i++){
        buff[i]=deg_string_decimal[i-2];
    }
    buff[11]='\0';
  }
}

void toDegreeDecimalLng(char* x,char* n,char *buff){
    unsigned long  degTenDec=(100000*(x[3]-'0'))
                   +((unsigned long) 10000*(x[4]-'0'))
                   +(1000*(x[6]-'0'))
                   +(100*(x[7]-'0'))
                   +(10*(x[8]-'0'))
                   +((x[9]-'0'));

    unsigned long deg=(degTenDec * 1000)/6;
    char deg_string[9];
    char deg_string_decimal[9];

    sprintf(deg_string, "%lu", deg);


    int length=0;
    for( int i=0; deg_string[i] != 0 ;i++){
         length++;
    }
    for( int i=0; i< (9-length) ;i++){
        deg_string_decimal[i]='0';
    }

    int c=0;
    for(int i=(9-length); i< 9 ;i++){
        deg_string_decimal[i]=deg_string[c];
        c++;
    }
    deg_string_decimal[9]='\0';


    //add first three number to get the non decimal amount
    int DD= (100*(x[0]-'0')) + (10*(x[1]-'0')) + (x[2]-'0');
    int MM=((deg_string_decimal[0]-'0'));
    int DD_MM=DD+MM;
    char dd_final[4];
    sprintf(dd_final, "%i", DD_MM);

    length=0;
    for( int i=0; dd_final[i] != 0 ;i++){
         length++;
    }
    //padd if needed
    if(length==1){
        dd_final[2]=dd_final[0];
        dd_final[1]='0';
        dd_final[0]='0';
    }
    if(length==2){
        dd_final[2]=dd_final[1];
        dd_final[1]=dd_final[0];
        dd_final[0]='0';
    }

   //if under ten we pad dd_final
  if(n[0]=='W' || 1  ){ //just always do this 
    buff[0]='-';
    buff[1]=dd_final[0];
    buff[2]=dd_final[1];
    buff[3]=dd_final[2];
    buff[4]='.';
    for(int i=5; i< 13 ;i++){
        buff[i]=deg_string_decimal[i-4];
    }
    buff[13]='\0';
  }else{
    buff[0]=dd_final[0];
    buff[1]=dd_final[1];
    buff[2]=dd_final[2];
    buff[3]='.';
    for(int i=4; i< 12 ;i++){
        buff[i]=deg_string_decimal[i-3];
    }
    buff[12]='\0';
  }
}



void getField(char* buffer, int index)
{
  int sentencePos = 0;
  int fieldPos = 0;
  int commaCount = 0;
  while (sentencePos < sentenceSize)
  {
    if (sentence[sentencePos] == ',')
    {
      commaCount ++;
      sentencePos ++;
    }
    if (commaCount == index)
    {
      buffer[fieldPos] = sentence[sentencePos];
      fieldPos ++;
    }
    sentencePos ++;
  }
  buffer[fieldPos] = '\0';
} 

