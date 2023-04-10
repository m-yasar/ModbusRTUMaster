#include "modbus_rtu.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

FC16DataPackage FC16SendDataPackage;
extern FC16DataPackage FC16SendDataPackage;
FC3DataPackage FC3RespData;
extern FC3DataPackage FC3RespData;
bool FC16ResponseMoment=false;
bool FC3ResponseMoment=false;
#define FC16ResponsePackageSIZE         8
#define FC3RequestPackagePackageSIZE    8
union{
    uint8_t FC16RequestPackage[FC16ReqPackageSIZE];   /*Byte boyutuna gore asagidaki veriler kullanilabilir.*/
       struct{
   uint8_t  SlaveAddress;
   uint8_t  FunctionCode;
   uint16_t destinationAddress; 
   uint16_t QuantityOfSendBit;  
   uint8_t  SendPackageByteSIZE;
   uint8_t  sendData[ModbusRTU_SendPackage_ByteSIZE];
   uint8_t  CRC[2];            
   };
   }FC16Request;
   
union{
   uint8_t FC16ResponsePackage[FC16ResponsePackageSIZE];   /*Byte boyutuna gore asagidaki veriler kullanilabilir.*/
       struct{
   uint8_t  SlaveAddress;
   uint8_t  FunctionCode;
   uint16_t destinationAddress; 
   uint16_t QuantityOfSendBit;  
   uint8_t  CRC[2];            
   };
}FC16Response;           
   
union{
   uint8_t FC3RequestPackage[FC3RequestPackagePackageSIZE];   
       struct{
   uint8_t  SlaveAddress;
   uint8_t  FunctionCode;
   uint16_t destinationAddress; 
   uint16_t QuantityOfWord;     
   uint8_t  CRC[2];            
   };
}FC3Request; 

union{
   uint8_t FC3RequestPackage[FC3RequestPackagePackageSIZE];   
       struct{
   uint8_t  SlaveAddress;
   uint8_t  FunctionCode;
   uint8_t  ResponseByteQuantity; 
   uint8_t  Data[ModbusRTU_ReceivePackage_ByteSIZE];     
   uint8_t  CRC[2];            
   };
}FC3Response; 
void modbus_master_init(int slaveAddress){
    /*Begin Send Package Settings*/
    FC16Request.SlaveAddress=slaveAddress;
    FC16Request.FunctionCode=0x10;      //FC16 
    FC16Request.destinationAddress=0xD007;
    FC16Request.QuantityOfSendBit= (ModbusRTU_SendPackage_ByteSIZE/2)<<8;
    FC16Request.SendPackageByteSIZE=ModbusRTU_SendPackage_ByteSIZE;
    /*End Send Package Settings*/
    
    /*Begin Receiver Package Settings*/
    FC3Request.SlaveAddress=slaveAddress;
    FC3Request.FunctionCode=0x03;
    FC3Request.destinationAddress=0xE803;
    FC3Request.QuantityOfWord=((ModbusRTU_ReceivePackage_ByteSIZE/2)<<8);//8 sola kaydirman gerekebilir
    /*End Receiver Package Settings*/
    
}

void sendFC16request(void){    
    memcpy(FC16Request.sendData,FC16SendDataPackage.FC16DataArray,ModbusRTU_SendPackage_ByteSIZE);   //veriyi gondermeden once gonderim paketine yaziyoruz
    FC16Request.CRC[0]=CRC16IBM(FC16Request.FC16RequestPackage,FC16ReqPackageSIZE-2);        //CRC Hesapliyoruz
    FC16Request.CRC[1]=(CRC16IBM(FC16Request.FC16RequestPackage,FC16ReqPackageSIZE-2)>>8);
    /*Begin Send FC16 Req Packages*/
    for (int i=0; i<FC16ReqPackageSIZE;i++){
        TransmitRegister=(uint8_t)FC16Request.FC16RequestPackage[i];
        while(U1STAbits.UTXBF==1){}
    }
    FC16ResponseMoment=true;    //FC16 Response yazmalari aktif ediliyor.
    /*End Send Packages*/
}
static uint8_t checkFC16response(void){
    /*8 bytelik sabit bir paket kontrolu yapiliyor eger veri dogru gidiyorsa gelen veri de dogru olacaktir*/
    /*verinin yanlis gitmesi durumunda bir daha gonderilebilir.*/
    
    /*Gelen verilerin baslik kontrolu*/
    for(int i=0; i<6;i++){
    if(FC16Request.FC16RequestPackage[i]==FC16Response.FC16ResponsePackage[i]){}
    //else return EXIT_FAILURE;
    }
    /*CRC Kontrolu*/
    uint16_t responseCRC,responcePackageCRC;
    responcePackageCRC=FC16Response.CRC[0]+(FC16Response.CRC[1]<<8);
    responseCRC=CRC16IBM(FC16Response.FC16ResponsePackage,6);
    if(responseCRC!=responcePackageCRC) return EXIT_FAILURE;
    
    return EXIT_SUCCESS;
        
}

void sendFC3request(void){
    FC3Request.CRC[0]=CRC16IBM(FC3Request.FC3RequestPackage,FC3RequestPackagePackageSIZE-2);
    FC3Request.CRC[1]=(CRC16IBM(FC3Request.FC3RequestPackage,FC3RequestPackagePackageSIZE-2)>>8);
    /*Begin Send FC3 Req Packages*/
    for (int i=0; i<FC3RequestPackagePackageSIZE;i++){
        TransmitRegister=(uint8_t)FC3Request.FC3RequestPackage[i];
        while(U1STAbits.UTXBF==1){}
    }
    FC3ResponseMoment=true;    //FC3 Response yazmalari aktif ediliyor.
    /*End Send FC3 Req Packages*/
}
static uint8_t checkFC3response(void){
    return EXIT_SUCCESS;
}

extern void response_RTU(uint8_t receiverRegister); /*Receiver Interrupta cagirilabilmesi icin extern declaration yapildi*/
void response_RTU(uint8_t receiverRegister){
    static uint16_t responseCounter=0;
    if(FC16ResponseMoment){
    FC16Response.FC16ResponsePackage[responseCounter]=receiverRegister;
    if(responseCounter++>=FC16ResponsePackageSIZE){
        responseCounter=0;
        FC16ResponseMoment=false;
        if(checkFC16response()) {/*sendFC16request();*/out1=0; out2=1;}//hatali durum
        else{out1=1; out2=0;}   //Hatasiz durum
    }
    }
    else if(FC3ResponseMoment){
    FC3Response.FC3RequestPackage[responseCounter]=receiverRegister;
    if(responseCounter++>FC3ResponsePackageSIZE){
        responseCounter=0;
        FC3ResponseMoment=false; 
        memcpy(FC3RespData.FC3DataArray,FC3Response.Data,ModbusRTU_ReceivePackage_ByteSIZE);
        if(checkFC16response()) {sendFC3request();}
        else{}
    } 
    }

}


uint16_t CRC16IBM( uint8_t *buf, int len){   
    uint16_t POLY=0xa001;
    uint16_t crc=0xFFFF;

    while (len--) {
        crc ^= *buf++;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        }
        
    return crc;
}
