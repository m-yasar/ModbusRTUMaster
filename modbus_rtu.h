#ifndef modbus_rtu_H
#define	modbus_rtu_H
#include <xc.h>

#define ModbusRTU_SendPackage_ByteSIZE      2        //Maximum 38 Ayarlanmalidir.
#define ModbusRTU_ReceivePackage_ByteSIZE   4
//Slave Address+ Function Code + Start Address+ Register Quantity+ Number Of Bytes + Data + CRC
#define FC16ReqPackageSIZE                  (1+1+2+2+1+ModbusRTU_SendPackage_ByteSIZE+2)
//Slave Address+ Function Code + Response Byte Quantity + Data + CRC
#define FC3ResponsePackageSIZE              (1+1+2+ModbusRTU_ReceivePackage_ByteSIZE+2)
#define TransmitRegister                    U1TXREG              


/*
 * Kullaniciya ait arayüz, burada gonderilmek istenilen verilerin bit, byte, word olarak tan?mlar? yap?lmal?d?r
 * Diger c dosyalarinda "extern sendData sendDataPackage;" declaration kullanilarak farkli yerlerde de parametreleri cagirilabilir.
 */
typedef union{
    uint8_t FC16DataArray[ModbusRTU_SendPackage_ByteSIZE];   /*Byte boyutuna gore asagidaki veriler kullanilabilir.*/
    struct{
    uint16_t firstvariable:16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    };
}FC16DataPackage;

typedef union{
    uint8_t  FC3DataArray[ModbusRTU_ReceivePackage_ByteSIZE];   /*Byte boyutuna gore asagidaki veriler kullanilabilir.*/
    struct{
    uint8_t firstvariable:8;
    unsigned :8;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    unsigned :16;
    };
}FC3DataPackage;


/*
 * modbus master mbap bilgilerinin girildigi initiliaze fonksiyonu
 */
void modbus_master_init(int slaveAddress);

/*
 * Modbusa gore crc hesaplayan fonksiyon
 */
uint16_t CRC16IBM( uint8_t *buf, int len);

/*
 *Veriyi gonderdigimiz fonksiyon
 *15 ms'te bir cagirilmali
 */
void sendFC16request(void);
/*
 * Veriyi cekmek icin kullanilan fonksiyon
 * send - receive arasinda bosluk olmalidir
 */
void sendFC3request(void);
/*
 * uart response kontrolu icin interrupt metodu ile calismali
 * Uart Receiver Interrupta bu fonksiyon yazildigi taktirde gerisini fonksiyon kendisi tamamlayacaktir.
 */
void response_RTU(uint8_t receiverRegister);

#endif	/* modbus_rtu_H */

