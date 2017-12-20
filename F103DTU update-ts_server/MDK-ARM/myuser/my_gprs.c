#include "my_gprs.h"
#include "my_cc1101.h"
#include "my_globle_extern.h"

/*
GPRS发送数据利用  uint8_t my_at_senddata(uint8_t *string);
GPRS分析接收数据利用
uint8_t my_usart_101frame(uint8_t usart_port);
void my_process_resive_101usart1(void); //M35使用
*/

uint8_t  WDZ_GPRS_101FCB = 0X00;


extern  uint32_t TimingDelay;
extern uint16_t DTU_ADDRESS;

extern	uint16_t USART1_address_first;
extern	uint16_t USART1_address_second;
extern	uint8_t USART1_length;
extern	uint8_t USART1_FRAME_status;
extern	uint8_t USART1_my_frame[256];
extern	uint8_t USART1_TRANSMINT_STATUS;
extern	uint8_t USART1_RESIVER_STATUS;


extern uint8_t rsbuf1[];	  //USART1缓冲器
extern uint8_t txbuf1[];
extern uint16_t rsbuf1pt_write;
extern uint16_t rsbuf1pt_read;
extern uint16_t rsbuf1pt_TEMP_read;
extern uint16_t rsbuf1pt_COMM_read;

//=====
extern	uint16_t USART3_address_first;
extern	uint16_t USART3_address_second;
extern	uint8_t USART3_length;
extern	uint8_t USART3_FRAME_status;
extern	uint8_t USART3_my_frame[256];
extern	uint8_t USART3_TRANSMINT_STATUS;
extern	uint8_t USART3_RESIVER_STATUS;


extern uint8_t rsbuf3[];	  //USART1缓冲器
extern uint8_t txbuf3[];
extern uint16_t rsbuf3pt_write;
extern uint16_t rsbuf3pt_read;
extern uint16_t rsbuf3pt_TEMP_read;
extern uint16_t rsbuf3pt_COMM_read;


//=====



extern struct rtc_time systmtime;  //RTC实时时钟使用
extern u8 rtcbuffer[];

//-------------------
extern uint8_t MY_GPRS_MCU_RsBuf[8];  //存储，周期性电池电压、太阳能电压、温度、湿度共4类8个字节的数据

extern uint8_t MY_GPRS_Call_Single_data_buf[40];
extern uint8_t MY_GPRS_Call_Analog_data_buf[110];
extern uint8_t MY_GPRS_Call_Time_data_buf[7];
extern uint8_t MY_GPRS_Call_Single_data_number;
extern uint8_t MY_GPRS_Call_Analog_data_number;
extern uint8_t MY_GPRS_Call_Status;    //存储获得总召数据状态，为1表示有总召数据，为0表示没有总召数据

extern uint8_t MY_EEPROM_Buf[256];  //用来存储EEPROM中，读出的数据
extern uint8_t MY_GPRS_Cycle_Transmintdata_status;  //用来存储读取到总召数据发送状态，1为已发送，2为未发送


extern uint8_t GPRS_Status;  //标识最终手机模块，GPRS网络状态，1为正常，可以连接服务器，0为有问题，需要处理
extern uint8_t MESS_Status;  //短信网络状态
extern uint8_t NET_Status;  //NET联网状态
extern uint8_t NET_Server_status; //远端服务器server状态

//-----------------
extern	uint8_t MY_433_Alarmdata_NOtimeBuf[256]; //存储 无时标 报警数据
extern	uint8_t MY_433_Alarmdata_TimeBuf[256];  //存储，有时标，报警数据
extern	uint8_t MY_433_ALarmdata_number;  // 存储，报警信息体个数


extern	uint8_t MY_433_ALarmdata_NOtime_status; //为1，表示收到无时标报警数据
extern	uint8_t MY_433_ALarmdata_Time_status;   //为1，表示收到有时标报警数据

//extern uint8_t AT_MESS_telphonenumber[];

extern  uint8_t MY_MCU_RsBuf[];
extern uint8_t MY_433_Call_Single_data_buf[];
extern uint8_t MY_433_Call_Analog_data_buf[];
extern uint8_t MY_433_Call_Time_data_buf[];
extern uint8_t MY_433_Call_Single_data_number;
extern uint8_t MY_433_Call_Analog_data_number;

//------------
extern uint32_t MY_Table1_Alarmdata_StartAddress;
extern uint32_t MY_Table1_Alarmdata_EndAddress;
extern uint32_t MY_Table1_Alarmdata_WriteAddress;
extern uint32_t MY_Table1_Alarmdata_ReadAddress;


extern  uint8_t RE_ALarmData_Status;

extern uint8_t MY_MCU_getdata_status;
extern uint8_t MY_433_Call_Status;

extern	uint8_t my_433_anlag_buf[110];  //存储433模拟量，不进行清0处理，不存储，只要有变化，利用标志位进行转发。
extern	uint8_t my_433_anlag_flag;  //标志位，为0表示已经发送了新数据，为1表示有新数据但是还没有发送



extern uint16_t MY_H_speed_cyc;  //周期10分钟
extern uint16_t MY_H_speed_heart;  //心跳5分钟

extern uint16_t MY_M_speed_cyc;  //周期15分钟
extern uint16_t MY_M_speed_heart;  //心跳9分钟

extern uint16_t MY_L_speed_cyc;  //周期30分钟
extern uint16_t MY_L_speed_heart; //心跳7分钟

extern float MY_Speed_H_Gate;
extern float MY_Speed_L_Gate;

extern uint16_t my_tim6_count;

extern struct indicator_class my_indicator_data[];
extern uint8_t my_usart1_tx_buf1[];
/*
功能：发送命令，等待（特定）反馈命令
发送数据type为类型，1为固定长度，2为非固定长度，txbuf为发送指令数组,address_type为1，rxbuf为接收
（2,txbuf,2,rxbuf)发送非固定长度，(1,txbuf,2,rxbuf)发送固定长度
*/
uint8_t WDZ_GPRS_101transmint_commd_wait_commd(uint8_t type, uint8_t *txbuf, uint8_t address_type, uint8_t *rxbuf) //type为类型，1为固定长度，2为非固定长度，txbuf为发送指令数组
{
    uint8_t FCB = 0x20;
    uint16_t my_dtu_address = DTU_ADDRESS;
    uint8_t my_status = 0;
    uint8_t n = 1; //发送次数
    uint8_t *my_txbuf = txbuf;
//	uint8_t temp=0;

    uint8_t linkcontrol = 0;
    uint8_t type_identification = 0;
    uint8_t  transmit_reason = 0;
    uint16_t my_start_time = 0;
//********************发送非固定长度帧，应答为OK帧
    if(type == 2)
    {
        //控制域码处理
        if(WDZ_GPRS_101FCB == 0x00)
            my_txbuf[4] = my_txbuf[4] & (~FCB);
        else if(WDZ_GPRS_101FCB == 0x20)
            my_txbuf[4] = my_txbuf[4] | (FCB);
        //目的地址处理
        my_txbuf[5] = my_dtu_address & 0xFF;
        my_txbuf[6] = my_dtu_address >> 8;
        if(address_type == 1)
        {
            my_txbuf[10] = my_txbuf[5];
            my_txbuf[11] = my_txbuf[6];
        }
        else if(address_type == 2)
        {
            my_txbuf[10] = my_txbuf[5];
            my_txbuf[11] = my_txbuf[6];
        }

        //校验位产生
        wdz_GPRS_101check_generate(my_txbuf);
        //发送指令
        while((n) > 0 && my_status == 0)
        {

            my_at_senddata(my_txbuf); //利用GPRS发送数据数组

            //Delay_us(300); //测试使用

            my_start_time = my_tim6_count;
            do {

                my_status = WDZ_GPRS_101receive_testactive(0X80, 00, 00, 1500); ///WDZ_GPRS_101recive_OKdata();  //等待0K确认反馈命令 15秒
                if(my_status == 1)break;
                else my_status = 0;
            } while(my_tim6_count - my_start_time <= 3);
            n--;
            if(n != 0) USART_printf(&huart1, "\x1A"); //发一个结束符，防止拥塞

        }



    }


//**************发送固定长度帧，应答为固定长度帧
    else	if(type == 1 && rxbuf[0] == 0x10)
    {
        //产生接收的控制域码，类型，传输原因
        linkcontrol = rxbuf[1];
        type_identification = 0;
        transmit_reason = 0;

        //控制域码处理
        if(WDZ_GPRS_101FCB == 0x00)
            my_txbuf[1] = my_txbuf[1] & (~FCB);
        else if(WDZ_GPRS_101FCB == 0x20)
            my_txbuf[1] = my_txbuf[1] | (FCB);
        //目的地址处理
        my_txbuf[2] = my_dtu_address & 0xFF;
        my_txbuf[3] = my_dtu_address >> 8;

        //校验位产生
        wdz_GPRS_101check_generate(my_txbuf);
        //发送指令
        while((n--) > 0 && my_status == 0)
        {
            //my_UART4_printf(UART4,my_txbuf);
            my_at_senddata(my_txbuf); //利用GPRS发送数据数组

            //Delay_us(300); //测试

            my_status = WDZ_GPRS_101receive_testactive(linkcontrol, type_identification, transmit_reason, 1500); //等待反馈命令
            if(my_status == 1)break;
            else my_status = 0;
        }
    }
//**********发送固定长度帧，应该为非固定长度帧
    else if(type == 1 && rxbuf[0] == 0x68)
    {
        //产生接收的控制域码，类型，传输原因
        linkcontrol = rxbuf[4];
        type_identification = rxbuf[7];
        transmit_reason = rxbuf[9];

        //控制域码处理
        if(WDZ_GPRS_101FCB == 0x00)
            my_txbuf[1] = my_txbuf[1] & (~FCB);
        else if(WDZ_GPRS_101FCB == 0x20)
            my_txbuf[1] = my_txbuf[1] | (FCB);
        //目的地址处理
        my_txbuf[2] = my_dtu_address & 0xFF;
        my_txbuf[3] = my_dtu_address >> 8;

        //校验位产生
        wdz_GPRS_101check_generate(my_txbuf);
        //发送指令
        while((n--) > 0 && my_status == 0)
        {
            //my_UART4_printf(UART4,my_txbuf);
            my_at_senddata(my_txbuf);

            //Delay_us(600); //测试

            my_status = WDZ_GPRS_101receive_testactive(linkcontrol, type_identification, transmit_reason, 1500); //等待反馈命令
            if(my_status == 1)break;
            else my_status = 0;
        }
    }


    //记录发送变化帧
    WDZ_GPRS_101FCB = WDZ_GPRS_101FCB ^ 0x20;
    if(my_status == 1)
    {
        return 1;
    }
    else return 0;

}










/*
校验字检查
*/
uint8_t wdz_GPRS_101char_check(uint8_t *buffer)
{
    uint16_t k = 0;
    uint8_t status68 = 0;
    uint8_t temp = 0;
    uint8_t check_char = 0x00;

    if(buffer[0] == 0x10)
    {
        status68 = 1;
    }
    else if(buffer[0] == 0x68)
    {
        status68 = 2;
    }

    if(status68 == 1) //固定长度校验位检查
    {
        check_char = buffer[1] + buffer[2] + buffer[3];
        if(check_char == buffer[4])
            temp = 1;
        else temp = 0;


    }
    else if(status68 == 2) //非固定长度校验位检查
    {
        for(k = 0; k < buffer[1]; k++)
        {
            check_char = check_char + buffer[k + 4];
        }

        if(check_char == buffer[buffer[1] + 4])
            temp = 1;
        else temp = 0;
    }
    return temp;


}







/*
生成校验字
*/
void wdz_GPRS_101check_generate(uint8_t *buffer)
{
    uint16_t k = 0;
    uint8_t status68 = 0;

    uint8_t check_char = 0x00;

    if(buffer[0] == 0x10)
    {
        status68 = 1;
    }
    else if(buffer[0] == 0x68)
    {
        status68 = 2;
    }


    if(status68 == 1) //固定长度校验位检查
    {
        check_char = buffer[1] + buffer[2] + buffer[3];
        buffer[4] = check_char;
    }
    else if(status68 == 2) //非固定长度校验位检查
    {
        for(k = 0; k < buffer[1]; k++)
        {
            check_char = check_char + buffer[k + 4];
        }
        buffer[buffer[1] + 4] = check_char;
    }


}






/*
功能：测试应答帧，用在主动发送的响应过程中
输入参数：Link_control，域控制码，type_identification-帧类型，transmit_reason-传输原因，time-检测时间
输入结果：应答正确为1，应答错误或没有应答为0
*/
uint8_t WDZ_GPRS_101receive_testactive(uint8_t Link_control, uint8_t type_identification, uint8_t transmit_reason, uint16_t time)
{
    uint8_t temp = 0;
    uint8_t *rsbuf = USART1_my_frame;

    uint8_t status68 = 0; //2代表非固定长度帧，1代表固定长度6的帧，0代表问题或处理完成
    uint16_t my_dtu_address = 0;
    uint16_t my_start_time = 0;



    my_start_time = my_tim6_count;
    while(my_tim6_count - my_start_time <= 3)
    {   my_usart_GPRS_101frame(1);
        if(USART1_FRAME_status > 0)
        {
            //帧头检查
            if(rsbuf[0] == 0x68)
            {
                status68 = 2;
            }

            else if(rsbuf[0] == 0x10)
            {
                status68 = 1;
            }

//************0x10固定长度帧处理
            //站地址检查
            if(status68 == 1)
            {
                my_dtu_address = rsbuf[3];
                my_dtu_address = (my_dtu_address << 8) + rsbuf[2];
                if(my_dtu_address == DTU_ADDRESS)
                    status68 = 1;
                else status68 = 0;
            }
            //校验字节检查
            if(status68 == 1)
            {
                temp = wdz_GPRS_101char_check(rsbuf); //测试时可以考虑先不使用
                if(temp == 1) status68 = 1;
                else status68 = 0;
            }



            //帧类型检查、传输原因检查
            if(status68 == 1)
            {

                if((rsbuf[1] & 0x0f) == (Link_control & 0x0f))
                {
                    temp = 1;
                    //帧的处理过程结束
                    USART1_FRAME_status = 0;
                    break;
                }
                else
                {   temp = 0;
                    //测试，把接收到的帧转发串口3
                    USART_printf(&huart3, "GPRS44");
                    USART_printf(&huart3, USART1_my_frame);
                    USART_printf(&huart3, "\n");
                    my_display_ASCIIdata(USART1_my_frame);
                    //USART_printf(&huart3,"GPRS4");
                    //帧的处理过程结束
                    USART1_FRAME_status = 0;
                }

                //帧的处理过程结束
                USART1_FRAME_status = 0;
            }



//************0x68非固定长度帧处理
            //从站地址检查
            if(status68 == 2)
            {
                my_dtu_address = rsbuf[6];
                my_dtu_address = (my_dtu_address << 8) + rsbuf[5];
                if(my_dtu_address == DTU_ADDRESS)
                    status68 = 2;
                else status68 = 0;
            }
            //校验字节检查

            if(status68 == 2)
            {
                temp = wdz_GPRS_101char_check(rsbuf); //测试时可以考虑先不使用
                if(temp == 1) status68 = 2;
                else status68 = 0;
            }



            //帧类型检查、传输原因检查 68 0D 0D 68 53 01 00 68 01 07 00 01 00 00 00 55 E3 FD 16
            if(status68 == 2)
            {
                if(((rsbuf[4] & 0x0f) == (Link_control & 0x0f)) && (rsbuf[7] == type_identification) && (rsbuf[9] == transmit_reason))
                {
                    temp = 1;
                    //帧的处理过程结束
                    USART1_FRAME_status = 0;
                    break;
                }
                else
                {   temp = 0;
                    //测试，把接收到的帧转发串口3
                    USART_printf(&huart3, "GPRS66-1");
                    USART_printf(&huart3, USART1_my_frame);
                    //USART_printf(&huart3,"GPRS6");
                    //帧的处理过程结束
                    USART1_FRAME_status = 0;
                }
            }
            //********************非固定长度帧处理结束

            //帧的处理过程结束
            USART1_FRAME_status = 0;

        }


    }

    return temp;

}

/*
功能 :被动接收到101指令反应，如果是正确的则处理完成对帧缓冲区标识为0，不是正确的则不处理，帧缓冲区状态保持不变
*/
uint8_t WDZ_GPRS_101receive_testactive2(uint8_t Link_control, uint8_t type_identification, uint8_t transmit_reason, uint16_t time)
{
    uint8_t temp = 0;
    uint8_t *rsbuf = USART1_my_frame;

    uint8_t status68 = 0; //2代表非固定长度帧，1代表固定长度6的帧，0代表问题或处理完成
    uint16_t my_dtu_address = 0;


    if(USART1_FRAME_status > 0)
    {
        //帧头检查
        if(rsbuf[0] == 0x68)
        {
            status68 = 2;
        }

        else if(rsbuf[0] == 0x10)
        {
            status68 = 1;
        }

//************0x10固定长度帧处理
        //站地址检查
        if(status68 == 1)
        {
            my_dtu_address = rsbuf[3];
            my_dtu_address = (my_dtu_address << 8) + rsbuf[2];
            if(my_dtu_address == DTU_ADDRESS)
                status68 = 1;
            else status68 = 0;
        }
        //校验字节检查
        if(status68 == 1)
        {
            //temp=wdz_GPRS_101char_check(rsbuf);  //测试时可以考虑先不使用
            temp = 1; //测试使用，字节校验为真
            if(temp == 1) status68 = 1;
            else status68 = 0;
        }



        //帧类型检查、传输原因检查
        if(status68 == 1)
        {

            if((rsbuf[1] & 0x0f) == (Link_control & 0x0f))
            {
                temp = 1;
                //帧的处理过程结束
                USART1_FRAME_status = 0;
                //break;
            }
            else
            {   temp = 0;
                //测试，把接收到的帧转发串口3
                USART_printf(&huart3, "GPRS44-2");
                USART_printf(&huart3, USART1_my_frame);
                USART_printf(&huart3, "\r\n");
                my_display_ASCIIdata(USART1_my_frame);
                //USART_printf(&huart3,"GPRS4");
                //帧的处理过程结束
                //USART1_FRAME_status=0;
            }

            //帧的处理过程结束
            //	USART1_FRAME_status=0;
        }



//************0x68非固定长度帧处理
        //从站地址检查
        if(status68 == 2)
        {
            my_dtu_address = rsbuf[6];
            my_dtu_address = (my_dtu_address << 8) + rsbuf[5];
            if(my_dtu_address == DTU_ADDRESS)
                status68 = 2;
            else status68 = 0;
        }
        //校验字节检查

        if(status68 == 2)
        {
            temp = wdz_GPRS_101char_check(rsbuf); //测试时可以考虑先不使用
            if(temp == 1) status68 = 2;
            else status68 = 0;
        }



        //帧类型检查、传输原因检查 68 0D 0D 68 53 01 00 68 01 07 00 01 00 00 00 55 E3 FD 16
        if(status68 == 2)
        {
            if(((rsbuf[4] & 0x0f) == (Link_control & 0x0f)) && (rsbuf[7] == type_identification) && (rsbuf[9] == transmit_reason))
            {
                temp = 1;
                //帧的处理过程结束
                USART1_FRAME_status = 0;
                //break;
            }
            else
            {   temp = 0;
                //测试，把接收到的帧转发串口3
                USART_printf(&huart3, "GPRS66-2");
                USART_printf(&huart3, USART1_my_frame);
                //USART_printf(&huart3,"GPRS6");
                //帧的处理过程结束
                //USART1_FRAME_status=0;
            }
        }
        //********************非固定长度帧处理结束

        //帧的处理过程结束
        //USART1_FRAME_status=0;

    }




    return temp;

}







/*
功能：数据接收分析，此部分为分析101协议数据，有帧头，有帧尾，有帧长，这三个参数很重要，取完整的一帧数据。
输入参数：串口号
输出参数： 取帧数据成功返回1，取帧数据失败返回0
*/
uint8_t my_usart_GPRS_101frame(uint8_t usart_port)
{
    uint8_t *rsbuf = 0;
    uint16_t *rsbufpt_read = 0;
    uint16_t *rsbufpt_COMM_read = 0;
    uint16_t *rsbufpt_write = 0;
    uint8_t *my_frame = 0;
    uint8_t *USART_FRAME = 0;


    uint16_t *address_first = 0;
    uint16_t *address_second = 0;
    uint8_t *length = 0;

    uint8_t ch1 = 0;
    uint16_t pt = 0;
    uint8_t tmp_status = 0;
    uint8_t status = 0;
    uint16_t ii = 0;

    uint8_t my_temp_status = 0;



//******串口的选择****************
    if(usart_port == 1)
    {
        rsbuf = rsbuf1;
        rsbufpt_read = &rsbuf1pt_read;
        rsbufpt_COMM_read = &rsbuf1pt_COMM_read;
        rsbufpt_write = &rsbuf1pt_write;
        address_first = &USART1_address_first;
        address_second = &USART1_address_second;
        length = &USART1_length;
        my_frame = USART1_my_frame;
        USART_FRAME = &USART1_FRAME_status;

        *rsbufpt_COMM_read = *rsbufpt_read;

    }

    if(usart_port == 3)
    {
        rsbuf = rsbuf3;
        rsbufpt_read = &rsbuf3pt_read;
        rsbufpt_COMM_read = &rsbuf3pt_COMM_read;
        rsbufpt_write = &rsbuf3pt_write;
        address_first = &USART3_address_first;
        address_second = &USART3_address_second;
        length = &USART3_length;
        my_frame = USART3_my_frame;
        USART_FRAME = &USART3_FRAME_status;

        *rsbufpt_COMM_read = *rsbufpt_read;

    }

////////****串口选择结束***************



    while(*USART_FRAME == 0 && *rsbufpt_COMM_read != *rsbufpt_write) //前一个指令处理完成，还有未处理的字符，则进行处理。结束条件，有完整的一条指令或者所有字符处理完成
    {

        //取一个未处理的字符
        if(*rsbufpt_COMM_read == rsbuf_max - 1)
        {
            ch1 = rsbuf[*rsbufpt_COMM_read];
            *rsbufpt_COMM_read = 0;
            pt = *rsbufpt_COMM_read;
        }
        else
        {
            ch1 = rsbuf[*rsbufpt_COMM_read];
            *rsbufpt_COMM_read = *rsbufpt_COMM_read + 1;
            pt = *rsbufpt_COMM_read;
        }

        //进行0X68帧头和帧尾标记
        if(ch1 == 0x68)
        {
            tmp_status = 1;
        }
        else if(ch1 == 0x16)
        {
            tmp_status = 2;
        }
        else
        {
            *USART_FRAME = 0;
            my_temp_status = 0;
            tmp_status = 0;
        }


        //进行0X68帧头分析
        if(tmp_status == 1)
        {

            //
            if((pt > 0 && pt < 4) && ch1 == rsbuf[rsbuf_max + pt - 4] && ch1 == 0x68)
            {

                *address_first = rsbuf_max + pt - 4;
                if(pt - 3 == 0)*length = rsbuf[pt - 3];
                else *length = rsbuf[rsbuf_max + pt - 3];

            }
            else if(pt >= 4 && ch1 == rsbuf[pt - 4] && ch1 == 0x68)
            {

                *address_first = pt - 4;
                *length = rsbuf[pt - 3];
            }
            else if(pt == 0 && ch1 == rsbuf[rsbuf_max - 4] && ch1 == 0x68)
            {

                *address_first = rsbuf_max - 4;
                *length = rsbuf[rsbuf_max - 3];
            }

            else
            {
                *USART_FRAME = 0;
                my_temp_status = 0;
                tmp_status = 0;
            }

        }
        //进行由帧尾到帧头的分析
        if(tmp_status == 2)
        {

            //固定长度帧
            if(pt > 0 && pt < 6 && rsbuf[rsbuf_max + pt - 6] == 0x10)
            {

                *address_first = rsbuf_max + pt - 6;
                *length = 6;
                *address_second = pt - 1;
                *USART_FRAME = 1;
                my_temp_status = 1;

            }
            else if(pt >= 6 && rsbuf[pt - 6] == 0x10)
            {

                *address_first = pt - 6;
                *length = 6;
                *address_second = pt - 1;
                *USART_FRAME = 1;
                my_temp_status = 1;
            }
            else if(pt == 0 && rsbuf[rsbuf_max - 6] == 0x10)
            {

                *address_first = rsbuf_max - 6;
                *length = 6;
                *address_second = pt - 1;
                *USART_FRAME = 1;
                my_temp_status = 1;
            }
            //非固定长度帧
            if(pt - 6 - *address_first == (*length) && *address_first < pt)
            {
                *address_second = pt - 1;
                *USART_FRAME = 2;
                my_temp_status = 2;
            }
            else if(*address_first > pt && pt != 0)
            {
                if((pt + rsbuf_max - *address_first - 6) == (*length))
                {   *address_second = pt - 1;
                    *USART_FRAME = 2;
                    my_temp_status = 2;
                }
            }
            else if(pt == 0)
            {
                if((rsbuf_max - *address_first - 6) == (*length))
                {   *address_second = rsbuf_max - 1;
                    *USART_FRAME = 2;
                    my_temp_status = 2;
                }
            }
            //


        }
    }
//取一帧数据存入到指令数组中



    if(my_temp_status > 0)	 //如果有完整一帧数据，就开始处理，否则返回，不处理
    {
        //清空命令数组区
        for(ii = 0; ii < 256; ii++)
        {
            my_frame[ii] = 0;
        }

        //取固定长度指令
        if(rsbuf[*address_first] == 0x10)
        {
            for(ii = 0; ii < 6; ii++)
            {
                my_frame[ii] = rsbuf[*address_first];
                *address_first = *address_first + 1;
                if(*address_first >= rsbuf_max) *address_first = 0;
            }

            *rsbufpt_read = *address_second + 1;
            if(*rsbufpt_read >= rsbuf_max)*rsbufpt_read = 0;

            status = 1;
        }
        //取非固定长度指令
        else if(rsbuf[*address_first] == 0x68)
        {
            for(ii = 0; ii < 6 + *length; ii++)
            {
                my_frame[ii] = rsbuf[*address_first];
                *address_first = *address_first + 1;
                if(*address_first >= rsbuf_max) *address_first = 0;
            }

            *rsbufpt_read = *address_second + 1;
            if(*rsbufpt_read >= rsbuf_max)*rsbufpt_read = 0;
            status = 1;
        }
        else
        {
            status = 0;
        }
    }

    //进行返回处理
    if(status == 1)
    {
        //*USART_FRAME=0;  //取完一帧指令进行标记
        //printf("\r\nOK=%s",my_frame);
        //USART_printf(USARTx,"\r\nOK=%s",my_frame);
        //USART_printf(USARTx,"%s",my_frame);
        return(1);
    }
    else
    {
        //USART_FRAME=0;  //取完一帧指令进行标记
        //printf("\r\nERROR");
        return(0);
    }

}








/*
发送OK肯定确认指令

*/
void WDZ_GPRS_101Transmit_OKdata(void)
{
    uint16_t my_dtu_address = DTU_ADDRESS;
    uint8_t commd[] = TX_GPRS_101_OKdata;
    //my_UART4_printf(UART4,commd);

//目的地址处理
    commd[2] = my_dtu_address & 0xFF;
    commd[3] = my_dtu_address >> 8;

    //校验位产生
    wdz_GPRS_101check_generate(commd);

    my_at_senddata(commd);

}






/*
命令字符串复制到数组中
*/
void wdz_GPRS_string_to_array(uint8_t *my_string, uint8_t *txbuf)
{
    uint32_t k = 0;
    uint32_t length = 0;
    if(my_string[0] == 0x10)
    {
        length = 6;
    }
    else if(my_string[0] == 0x68)
    {
        length = my_string[1] + 6;
    }
    //=================

    for(k = 0; k < length; k++)
    {
        txbuf[k] = my_string[k];
    }
    //===========
    if(my_string[0] == 0x10)
    {
        txbuf[2] = DTU_ADDRESS;
        txbuf[3] = (DTU_ADDRESS >> 8);
    }
    else if(my_string[0] == 0x68)
    {
        txbuf[5] = DTU_ADDRESS;
        txbuf[6] = (DTU_ADDRESS >> 8);

        txbuf[10] = DTU_ADDRESS; //
        txbuf[11] = (DTU_ADDRESS >> 8);


    }
    txbuf[length - 2] = my_fun_101check_generate(txbuf, 0);
    txbuf[k] = 0;
}








//*************主动发送命令部分*******
/*
功能：发送心跳包，等待应答程序

发送心跳包，10 D2 01 00 D3 16 或者 10 F2 01 00 F3 16
接收确认包  10 80 01 00 81 16 或者 10 82 01 00 83 16
*/
extern uint8_t link_status_GPRS;
extern uint8_t GPRS_Heartdata_error_count;  //判断心跳包失败的次数，如果到5次了，就标识GPRS网络故障，然后利用这个计数值进行M35的重启判断
//uint8_t WDZ_GPRS_101transmit_heartdata(void)
//{
//    uint8_t my_status = 0;
//    uint8_t my_rxbuf[7] = "\x10\x80\x01\x00\x81\x16";
//    uint8_t my_txbuf[7] = TX_GPRS_101_heartdata;


//    my_status = WDZ_GPRS_101transmint_commd_wait_commd(1, my_txbuf, 1, my_rxbuf);

//    if(my_status == 0)
//    {
//        USART_printf(&huart3, " Server heart-\r\n");
//        GPRS_Heartdata_error_count++;
//    }
//    else if(my_status == 1)
//    {
//        USART_printf(&huart3, " Server heart*\r\n");
//        GPRS_Heartdata_error_count = 0;
//    }
//    //网络连接状态判断，如果5次失败，就标识网络有问题，重新启动M35
//    if(GPRS_Heartdata_error_count >= 3)
//    {
//        GPRS_Status = 0;
//        NET_Server_status = 0;
//        //GPRS_Heartdata_error_count=0;

//        link_status_GPRS = 0; //GPRS网络如果断开了，则认为101链路断了
//    }

//    return my_status;

//}








//*************发送测试周期数据命令*************

/*
68 0C 0C 68 73 01 00 68 01 06 00 01 00 00 AA 55 E3 16测试命令(激活)
        （肯定确认）10 80 01 00 81 16
 68 23 23 68 73 01 00 01 98 14 00 01 00 01 00 00 00 00 00 00 00 00 23 16(遥信数据包)
 （遥测数据包）
  （环境数据包）
				（肯定确认）10 80 01 00 81 16
*/

extern uint32_t MY_Table2_Cycledata_StartAddress;
extern uint32_t MY_Table2_Cycledata_EndAddress;
extern uint32_t MY_Table2_Cycledata_WriteAddress;
extern uint32_t MY_Table2_Cycledata_ReadAddress;
extern uint8_t MY_GPRS_Call_Single_data_number;
extern uint8_t MY_GPRS_Call_Analog_data_number;

//uint8_t WDZ_GPRS_101transmit_Cycle_data(void)  //发送历史数据，修改方案2016-05-27，历史数据只发送一天的。10*24=240条数据
//{
//    uint8_t tempbuf[12] = {0};
//    uint32_t startaddressPT = MY_Table2_Cycledata_StartAddress;
//    uint32_t endaddressPT = MY_Table2_Cycledata_EndAddress;
//    uint32_t writeaddressPT = MY_Table2_Cycledata_WriteAddress;
//    uint32_t readaddressPT = MY_Table2_Cycledata_ReadAddress;
//    uint32_t tableaddress = 0;

//    uint8_t my_status = 0;
//    uint8_t tempstaus = 0;
//    uint8_t my_rxbuf[7] = "\x10\x80\x01\x00\x81\x16";
//    uint8_t my_txbuf[256] = TX_GPRS_101_testdata;
//    uint16_t my_tx_cn = 0;


////***处理获得最新的表指针地址
//    SPI_EE_BufferRead2(tempbuf, 213, 12);

//    my_buf_writeto_val(tempbuf, &startaddressPT, &endaddressPT, &writeaddressPT, &readaddressPT);
//    MY_Table2_Cycledata_StartAddress = startaddressPT;
//    MY_Table2_Cycledata_EndAddress = endaddressPT;
//    MY_Table2_Cycledata_WriteAddress = writeaddressPT;
//    MY_Table2_Cycledata_ReadAddress = readaddressPT;

////--------------
////进行读指针末尾校验，如果到了末尾，就返回起始地址，然后写入表中
//    if(readaddressPT + 144 > endaddressPT)
//    {   readaddressPT = startaddressPT;
//        MY_Table2_Cycledata_ReadAddress = readaddressPT;
//        tempbuf[0] = readaddressPT & 0x0000ff;
//        tempbuf[1] = (readaddressPT >> 8) & 0x0000ff;
//        tempbuf[2] = (readaddressPT >> 16) & 0x0000ff;
//        SPI_EE_BufferWrite2(tempbuf, 222, 3);
//    }


////----------------------

//    if(readaddressPT > writeaddressPT)
//    {
//        while(readaddressPT > writeaddressPT)
//        {
//            if(GPRS_Status == 1 && link_status_GPRS == 1)
//                my_status = 1;
//            else
//                my_status = 0;

//            wdz_GPRS_string_to_array(TX_GPRS_101_testdata, my_txbuf); //密码指令
//            //my_status=WDZ_GPRS_101transmint_commd(2,my_txbuf,2);
//            my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
//            if(my_status == 1)
//            {
//                //my_status=WDZ_101receive_testactive(0x03,0x68,0x07,300);
////***********重要****发送周期性数据****
////先从EEPROM中读出来，放到3个特定数组中，然后发送3个101信息包，遥信，遥测、环境，带时标。
////等待Server肯定确认，确认后修改EEPROM中数据包的GPRS发送状态，同时修改读指针地址

//                tempstaus = my_eeprom_data_to_cycle_array(); //把数据存入到GPRS发送数组中*********
//                if(tempstaus == 0)
//                {
//                    if((MY_GPRS_Cycle_Transmintdata_status != 1 && MY_GPRS_Cycle_Transmintdata_status != 2) | (MY_GPRS_Call_Single_data_number != 18) || (MY_GPRS_Call_Analog_data_number != 36))
//                    {   //移动读指针
//                        readaddressPT = writeaddressPT;	 //因为读指针已经跑偏，直接把它校正到写指针2015-11.16
//                        tempbuf[0] = readaddressPT & 0x0000ff;
//                        tempbuf[1] = (readaddressPT >> 8) & 0x0000ff;
//                        tempbuf[2] = (readaddressPT >> 16) & 0x0000ff;
//                        tableaddress = 222;
//                        SPI_EE_BufferWrite2(tempbuf, tableaddress, 3);
//                        my_status = 0;
//                        return my_status;
//                    }
//                }


//                if(tempstaus == 1)
//                {
//                    //生成发送数据包，发送数据，先发遥信、再发遥测、最后发环境
//                    //遥信数据包 68 23 23 68 73 01 00 01 98 14 00 01 00 01 00 00 00 00 00 00 00 00 23 16

//                    my_gprs_generate_101single_data(1, my_txbuf);
//                    //my_at_senddata(my_txbuf);

//                    //新增
//                    //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500);

//                    my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
//                    if(my_status == 1)
//                    {

//                        //遥测数据包
//                        //68 53 53 68 53 01 00 09 98 14 00 01 00 01 40 00 00 00 00 00 00 00 00 00 00 4B 16
//                        my_gprs_generate_101analog_data(1, my_txbuf);
//                        //my_at_senddata(my_txbuf);

//                        //新增
//                        //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500);
//                        my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
//                    }
//                    else
//                    {
//                        my_status = 0;
//                        USART_printf(&huart3, "GPRS Sing data error"); //调试使用
//                    }


//                    if(my_status == 1)
//                    {
//                        //环境数据包
//                        //68 53 53 68 53 01 00 09 98 14 00 01 00 00 41 00 00 00 00 00 00 00 00 00 00 4B 16
//                        my_gprs_generate_101MCU_data(1, my_txbuf);
//                        //my_at_senddata(my_txbuf);
//                        //新增
//                        //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500);
//                        my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

//                    }
//                    else
//                    {
//                        my_status = 0;
//                        USART_printf(&huart3, "GPRS angale data error"); //调试使用
//                    }

//                    if(my_status == 0)
//                    {
//                        USART_printf(&huart3, "GPRS MCU data error"); //调试使用
//                    }

//                }
//                else
//                {
//                    //生产空数据包，发送数据包
//                    my_gprs_generate_101single_data(0, my_txbuf);
//                    my_at_senddata(my_txbuf);
//                    my_gprs_generate_101analog_data(0, my_txbuf);
//                    my_at_senddata(my_txbuf);
//                    my_gprs_generate_101MCU_data(0, my_txbuf);
//                    my_at_senddata(my_txbuf);

//                }

//                //接收server发来的肯定确认数据包
//                //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500); //旧的方案，不要确认的方案

//            }


//            else if(my_status == 0)
//            {   //Delay_us(100);
//                USART_printf(&huart3, " Server Cycle data error1\r\n");
//                break;
//                //LED3_TOGGLE;
//            }



//            if(my_status == 1) //发送成功了，最终
//            {
//                //进行发送帧状态字节修改，和移动EEPROM的表读指针
//                if(tempstaus == 1)
//                    my_GPRS_chang_tablereadpt(2, 1); //成功，修改此帧状态为1，表示发送出去了，移动读指针
//                else if(tempstaus == 0)
//                {
//                    my_GPRS_chang_tablereadpt(2, 0);  //失败，也修改为1，让指针动起来，//修改为2，未发送，不移动指针
//                    break;
//                }

//                //Delay_us(100);
//                USART_printf(&huart3, " Server Cycle data OK1*--%d\r\n", my_tx_cn);


//                //--2016-5-27 发送周期数据控制，只发送一天的周期历史数据，防止长时间占用。10*24=240
//                my_tx_cn++;  //发送数据条数计数
//                if(my_tx_cn > TX_History_cyc_data_record)
//                {
//                    readaddressPT = writeaddressPT;	 //因为读指针已经跑偏，直接把它校正到写指针2016-5-27
//                    tempbuf[0] = readaddressPT & 0x0000ff;
//                    tempbuf[1] = (readaddressPT >> 8) & 0x0000ff;
//                    tempbuf[2] = (readaddressPT >> 16) & 0x0000ff;
//                    tableaddress = 222;
//                    SPI_EE_BufferWrite2(tempbuf, tableaddress, 3);

//                }


//                // LED2_TOGGLE;
//            }
//            else if(my_status == 0)
//            {   //Delay_us(100);
//                USART_printf(&huart3, " Server Cycle data errort2 \r\n");
//                //LED3_TOGGLE;
//            }
//            //***处理获得最新的表指针地址
//            SPI_EE_BufferRead2(tempbuf, 213, 12);
//            my_buf_writeto_val(tempbuf, &startaddressPT, &endaddressPT, &writeaddressPT, &readaddressPT);
//            MY_Table2_Cycledata_StartAddress = startaddressPT;
//            MY_Table2_Cycledata_EndAddress = endaddressPT;
//            MY_Table2_Cycledata_WriteAddress = writeaddressPT;
//            MY_Table2_Cycledata_ReadAddress = readaddressPT;


//        }
//        //进行读指针移动
//        if(readaddressPT > endaddressPT || (readaddressPT + 144) > endaddressPT) //进行读指针校正
//        {   readaddressPT = startaddressPT;

//            tempbuf[0] = readaddressPT & 0x0000ff;
//            tempbuf[1] = (readaddressPT >> 8) & 0x0000ff;
//            tempbuf[2] = (readaddressPT >> 16) & 0x0000ff;
//            tableaddress = 222;
//            SPI_EE_BufferWrite2(tempbuf, tableaddress, 3);

//        }


//    }
//    else if(readaddressPT < writeaddressPT)
//    {
//        while(readaddressPT < writeaddressPT)
//        {
//            if(GPRS_Status == 1 && link_status_GPRS == 1)
//                my_status = 1;
//            else
//                my_status = 0;

//            wdz_GPRS_string_to_array(TX_GPRS_101_testdata, my_txbuf); //密码指令
//            //my_status=WDZ_GPRS_101transmint_commd(2,my_txbuf,2);
//            my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
//            if(my_status == 1)
//            {
//                //my_status=WDZ_101receive_testactive(0x03,0x68,0x07,300);
////***********重要****发送周期性数据****
////先从EEPROM中读出来，放到3个特定数组中，然后发送3个101信息包，遥信，遥测、环境，带时标。
////等待Server肯定确认，确认后修改EEPROM中数据包的GPRS发送状态，同时修改读指针地址

//                tempstaus = my_eeprom_data_to_cycle_array(); //把数据存入到GPRS发送数组中*********
//                //强制校验指针环节，从EEPROM中读的数据是错的,2015-11.16
//                if(tempstaus == 0)
//                {
//                    if((MY_GPRS_Cycle_Transmintdata_status != 1 && MY_GPRS_Cycle_Transmintdata_status != 2) | (MY_GPRS_Call_Single_data_number != 18) || (MY_GPRS_Call_Analog_data_number != 36))
//                    {   //移动读指针
//                        readaddressPT = writeaddressPT;	 //因为读指针已经跑偏，直接把它校正到写指针2015-11.16
//                        tempbuf[0] = readaddressPT & 0x0000ff;
//                        tempbuf[1] = (readaddressPT >> 8) & 0x0000ff;
//                        tempbuf[2] = (readaddressPT >> 16) & 0x0000ff;
//                        tableaddress = 222;
//                        SPI_EE_BufferWrite2(tempbuf, tableaddress, 3);
//                        my_status = 0;
//                        return my_status;
//                    }
//                }

//                if(tempstaus == 1)
//                {
//                    //生成发送数据包，发送数据，先发遥信、再发遥测、最后发环境
//                    //遥信数据包 68 23 23 68 73 01 00 01 98 14 00 01 00 01 00 00 00 00 00 00 00 00 23 16

//                    my_gprs_generate_101single_data(1, my_txbuf);
//                    //my_at_senddata(my_txbuf);

//                    //新增
//                    //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500);

//                    my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
//                    if(my_status == 1)
//                    {

//                        //遥测数据包
//                        //68 53 53 68 53 01 00 09 98 14 00 01 00 01 40 00 00 00 00 00 00 00 00 00 00 4B 16
//                        my_gprs_generate_101analog_data(1, my_txbuf);
//                        //my_at_senddata(my_txbuf);

//                        //新增
//                        //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500);
//                        my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
//                    }
//                    else
//                    {
//                        my_status = 0;
//                        USART_printf(&huart3, "GPRS Sing data error"); //调试使用
//                    }


//                    if(my_status == 1)
//                    {
//                        //环境数据包
//                        //68 53 53 68 53 01 00 09 98 14 00 01 00 00 41 00 00 00 00 00 00 00 00 00 00 4B 16
//                        my_gprs_generate_101MCU_data(1, my_txbuf);
//                        //my_at_senddata(my_txbuf);
//                        //新增
//                        //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500);
//                        my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

//                    }
//                    else
//                    {
//                        my_status = 0;
//                        USART_printf(&huart3, "GPRS angale data error"); //调试使用
//                    }

//                    if(my_status == 0)
//                    {
//                        USART_printf(&huart3, "GPRS MCU data error"); //调试使用
//                    }

//                }
//                else
//                {
//                    //生产空数据包，发送数据包
//                    my_gprs_generate_101single_data(0, my_txbuf);
//                    my_at_senddata(my_txbuf);
//                    my_gprs_generate_101analog_data(0, my_txbuf);
//                    my_at_senddata(my_txbuf);
//                    my_gprs_generate_101MCU_data(0, my_txbuf);
//                    my_at_senddata(my_txbuf);

//                }

//                //接收server发来的肯定确认数据包
//                //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500); //旧的方案，不要确认的方案

//            }


//            else if(my_status == 0)
//            {
//                USART_printf(&huart3, " Server Cycle data error3\r\n");
//                break;
//            }



//            if(my_status == 1) //发送成功了，最终
//            {
//                //进行发送帧状态字节修改，和移动EEPROM的表读指针
//                if(tempstaus == 1)
//                {
//                    my_GPRS_chang_tablereadpt(2, 1); //成功，修改此帧状态为1，表示发送出去了，移动读指针
//                }
//                else if(tempstaus == 0)
//                {
//                    my_GPRS_chang_tablereadpt(2, 0);  //失败，修改为2，未发送，不移动指针
//                    USART_printf(&huart3, " Server Cycle data errort4-1 \r\n");
//                    break;
//                }

//                //Delay_us(100);
//                USART_printf(&huart3, " Server Cycle data OK2*--%d\r\n", my_tx_cn);

//                //--2016-5-27 发送周期数据控制，只发送一天的周期历史数据，防止长时间占用。10*24=240
//                my_tx_cn++;  //发送数据条数计数
//                if(my_tx_cn > TX_History_cyc_data_record)
//                {
//                    readaddressPT = writeaddressPT;	 //因为读指针已经跑偏，直接把它校正到写指针2016-5-27
//                    tempbuf[0] = readaddressPT & 0x0000ff;
//                    tempbuf[1] = (readaddressPT >> 8) & 0x0000ff;
//                    tempbuf[2] = (readaddressPT >> 16) & 0x0000ff;
//                    tableaddress = 222;
//                    SPI_EE_BufferWrite2(tempbuf, tableaddress, 3);

//                }
//                //---2016-05-27 修改结束
//                // LED2_TOGGLE;
//            }
//            else if(my_status == 0)
//            {   //Delay_us(100);
//                USART_printf(&huart3, " Server Cycle data errort4 \r\n");
//                //LED3_TOGGLE;
//            }
//            //***处理获得最新的表指针地址
//            SPI_EE_BufferRead2(tempbuf, 213, 12);
//            my_buf_writeto_val(tempbuf, &startaddressPT, &endaddressPT, &writeaddressPT, &readaddressPT);
//            MY_Table2_Cycledata_StartAddress = startaddressPT;
//            MY_Table2_Cycledata_EndAddress = endaddressPT;
//            MY_Table2_Cycledata_WriteAddress = writeaddressPT;
//            MY_Table2_Cycledata_ReadAddress = readaddressPT;


//        }
//        //进行读指针移动
//        if(readaddressPT > writeaddressPT) //进行读指针校正
//        {   readaddressPT = writeaddressPT;

//            tempbuf[0] = readaddressPT & 0x0000ff;
//            tempbuf[1] = (readaddressPT >> 8) & 0x0000ff;
//            tempbuf[2] = (readaddressPT >> 16) & 0x0000ff;
//            tableaddress = 222;
//            SPI_EE_BufferWrite2(tempbuf, tableaddress, 3);

//        }


//    }










//    return my_status;

//}





/*
功能：生成遥信数据包  68 23 23 68 73 01 00 02 ?? 14 01 00 01 00 ?? ??  ??  ** 16
*/
void my_gprs_generate_101single_data(uint8_t temp, uint8_t *my_rsbuf)
{
    uint8_t length = 0;


    if(temp == 1) //生成数据包
    {
        length = MY_GPRS_Call_Single_data_number;
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length + 10 + 7; //18+10+7
        my_rsbuf[2] = length + 10 + 7;

        //控制域码处理
        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        //my_rsbuf[4]=0x73&WDZ_GPRS_101FCB;  //控制域码为53/73

        my_rsbuf[5] = DTU_ADDRESS;
        my_rsbuf[6] = (DTU_ADDRESS >> 8);

        my_rsbuf[7] = 0X02; //类型标识，带时标的单点信息，
        my_rsbuf[8] = length + 0x80; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8);

        my_rsbuf[12] = 0x01; //遥信信息体首地址
        my_rsbuf[13] = 0x00;

        my_buf1_to_buf2(MY_GPRS_Call_Single_data_buf, 0, my_rsbuf, 14, length); //存储遥信数据
				
        //my_buf1_to_buf2(MY_GPRS_Call_Time_data_buf, 0, my_rsbuf, 14 + length, 7); //存储时标

        my_rsbuf[13 + length + 7 + 1] = 0XFF;
        my_rsbuf[13 + length + 7 + 1 + 1] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节



    }
    //测试使用
    else if(temp == 0) //生成0数据体数据包
    {
        length = 2;
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length + 10;
        my_rsbuf[2] = length + 10;

        //控制域码处理
        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X02; //类型标识，带时标的单点信息，
        my_rsbuf[8] = 0x92; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[12] = 0x01; //遥信信息体首地址
        my_rsbuf[13] = 0x00;

        my_rsbuf[14] = 0x00;
        my_rsbuf[15] = 0x00;

        my_rsbuf[16] = 0XFF;
        my_rsbuf[17] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节
    }


}








/*
功能;生成遥测数据包

//68 53 53 68 53 01 00 09 98 14 00 01 00 01 40 00 00 00 00 00 00 00 00 00 00 4B 16
*/
void my_gprs_generate_101analog_data(uint8_t temp, uint8_t *my_rsbuf)
{
    uint8_t length = 0;


    if(temp == 1) //生成数据包
    {
        length = MY_GPRS_Call_Analog_data_number;

        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length * 3 + 10; //0X24 共36个信息体，每个信息体3个字节
        my_rsbuf[2] = length * 3 + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS;
        my_rsbuf[6] = (DTU_ADDRESS >> 8);

        my_rsbuf[7] = 0X09; //类型标识，带时标的遥测数据信息，
        my_rsbuf[8] = length + 0x80; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8);;

        my_rsbuf[12] = 0x01; //遥信信息体首地址
        my_rsbuf[13] = 0x40;

        my_buf1_to_buf2(MY_GPRS_Call_Analog_data_buf, 0, my_rsbuf, 14, length * 3);


        my_rsbuf[13 + length * 3 + 1] = 0XFF;
        my_rsbuf[13 + length * 3 + 1 + 1] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节



    }
    else if(temp == 0) //生成0数据体数据包
    {
        length = 2;
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length + 10;
        my_rsbuf[2] = length + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X09; //类型标识，带时标的单点信息，
        my_rsbuf[8] = 0xA4; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;;

        my_rsbuf[12] = 0x01; //遥信信息体首地址
        my_rsbuf[13] = 0x40;

        my_rsbuf[14] = 0x00;
        my_rsbuf[15] = 0x00;

        my_rsbuf[16] = 0XFF;
        my_rsbuf[17] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节
    }



}







/*
功能;生成环境数据包,temp为1生成有数据的数据包，0生成0数据的数据包
*/
void my_gprs_generate_101MCU_data(uint8_t temp, uint8_t *my_rsbuf)
{
    uint8_t length = 0;

    if(temp == 1) //生成数据包
    {
        length = 4;

        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length * 2 + 10;
        my_rsbuf[2] = length * 2 + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);


        my_rsbuf[5] = DTU_ADDRESS;
        my_rsbuf[6] = (DTU_ADDRESS >> 8);

        my_rsbuf[7] = 0X09; //类型标识，带时标的单点信息，
        my_rsbuf[8] = length + 0x80; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8);;

        my_rsbuf[12] = 0x00; //遥信信息体首地址
        my_rsbuf[13] = 0x41;

        my_buf1_to_buf2(MY_GPRS_MCU_RsBuf, 0, my_rsbuf, 14, length * 2);

        my_rsbuf[13 + length * 2 + 1] = 0XFF;
        my_rsbuf[13 + length * 2 + 1 + 1] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节



    }
    else if(temp == 0) //生成0数据体数据包
    {
        length = 2;
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length + 10;
        my_rsbuf[2] = length + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X09; //类型标识，带时标的单点信息，
        my_rsbuf[8] = 0x84; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;;

        my_rsbuf[12] = 0x00; //遥信信息体首地址
        my_rsbuf[13] = 0x41;

        my_rsbuf[14] = 0x00;
        my_rsbuf[15] = 0x00;

        my_rsbuf[16] = 0XFF;
        my_rsbuf[17] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节
    }


}







/*
产生控制域码
*/
void my_GPRS_101_geneate_FCBword(uint8_t *my_rsbuf)
{   uint8_t FCB = 0X20;

    if(my_rsbuf[0] == 0x68)
    {
        //控制域码处理
        if(WDZ_GPRS_101FCB == 0x00)
            my_rsbuf[4] = my_rsbuf[4] & (~FCB);
        else if(WDZ_GPRS_101FCB == 0x20)
            my_rsbuf[4] = my_rsbuf[4] | (FCB);
    }
    else if(my_rsbuf[0] == 0x10)
    {
        //控制域码处理
        if(WDZ_GPRS_101FCB == 0x00)
            my_rsbuf[1] = my_rsbuf[1] & (~FCB);
        else if(WDZ_GPRS_101FCB == 0x20)
            my_rsbuf[1] = my_rsbuf[1] | (FCB);
    }

    //记录发送变化帧
    WDZ_GPRS_101FCB = WDZ_GPRS_101FCB ^ 0x20;

}











/*
功能：建立链路连接
*/
uint8_t WDZ_GPRS_101Transmit_Link_data(void)
{
    uint8_t my_status = 0;
    uint8_t my_rxbuf[256] = "\x10\x8B\x01\x00\x8C\x16";
    uint8_t my_txbuf[256] = TX_GPRS_101_Linkquire_data;



    //第1步，发送请求建立链路，收到链路响应
    WDZ_GPRS_101FCB = 0; //对FCB清零，不翻转
    my_status = WDZ_GPRS_101transmint_commd_wait_commd(1, my_txbuf, 1, my_rxbuf);


    //第2步，发送复位远方链路，收到肯定确认
    if(my_status == 1)
    {
        wdz_GPRS_string_to_array(TX_GPRS_101_Linkrest_data, my_txbuf);
        wdz_GPRS_string_to_array(RX_GPRS_101_OKdata, my_rxbuf);
        WDZ_GPRS_101FCB = 0; //对FCB清零，不翻转
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(1, my_txbuf, 1, my_rxbuf);
        //10 C9 01 00 CA 16
        //my_rxbuf[1]=0XC9;
        //my_rxbuf[2]=DTU_ADDRESS&0X00FF;
        //my_rxbuf[3]=(DTU_ADDRESS>>8)&0X00FF;
        //wdz_GPRS_101char_check(my_rxbuf);

        //my_status=WDZ_GPRS_101transmint_commd_wait_commd(1,my_txbuf,1,my_rxbuf);
    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server LINK-1\r\n");

    }
    //第3步，收到请求链路
    if(my_status == 1)
    {   //10 C9 01 00 CA 16
        my_status = WDZ_GPRS_101receive_testactive(0XC9, 0, 0, 600);

    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server LINK-2\r\n");
    }
    //第4步，发送响应链路状态,收到复位远方链路
    if(my_status == 1)
    {
        wdz_GPRS_string_to_array(TX_GPRS_101_Linkconfirm_data, my_txbuf);
        wdz_GPRS_string_to_array(RX_GPRS_101_Linkrest_data, my_rxbuf);
        WDZ_GPRS_101FCB = 0; //对FCB清零，不翻转
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(1, my_txbuf, 1, my_rxbuf);

    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server LINK-3\r\n");
    }
    //第5步，发送肯定确认,收到初始化结束
    if(my_status == 1)
    {
        wdz_GPRS_string_to_array(TX_GPRS_101_OKdata, my_txbuf);
        wdz_GPRS_string_to_array(RX_GPRS_101_Linkover_data, my_rxbuf);
        WDZ_GPRS_101FCB = 0; //对FCB清零，不翻转
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(1, my_txbuf, 1, my_rxbuf);

    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server LINK-4\r\n");
    }
    //第6步，发送肯定确认
    if(my_status == 1)
    {
        WDZ_GPRS_101Transmit_OKdata();
        USART_printf(&huart3, " Server LINK*\r\n");

    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server LINK-5\r\n");
    }


    return my_status;
}





//-------------被动接收程序，校时、总召----
/*
功能：校时功能
*/
uint8_t WDZ_GPRS_101Resiver_AdjustTime_data(void)
{
    uint8_t my_status = 0;
    uint8_t my_rtcbuffer1[7] = {0};

    uint8_t k = 0;
    uint8_t my_rxbuf[256] = "\x10\x8B\x01\x00\x8C\x16";
    uint8_t my_txbuf[256] = TX_GPRS_101_Linkquire_data;

    //第1步：获得延时获得命令，存储毫秒值，发送肯定确认，并校正本地毫秒值,取最新毫秒值，发送延时获得命令(激活确认)
    //68 0C 0C 68 53 01 00 6A 01 06 01 00 00 00 3D C3 C6 16
    my_status = WDZ_GPRS_101receive_testactive2(0X53, 0X6A, 0X06, 300);
    if(my_status == 0)  return my_status; //   如果指令错误，返回，检查下一条指令

    if(my_status == 0)  return my_status; //   如果指令错误，返回，检查下一条指令
    if(my_status == 1)
    {
        //发送肯定确认指令
        WDZ_GPRS_101Transmit_OKdata();

        //校正本地值，取值，发送最新数据
        my_RTCtime_to_array(my_rtcbuffer1);//读取RTC
        my_rtcbuffer1[0] = USART1_my_frame[14];
        my_rtcbuffer1[1] = USART1_my_frame[15];
        my_array_to_RTCtime(my_rtcbuffer1); //校时
        my_RTCtime_to_array(my_rtcbuffer1);//读取RTC


        //发送延时获得命令,收到肯定确认
        //68 0C 0C 68 73 01 00 6A 01 07 01 00 00 00 5B C3 05 16

        wdz_GPRS_string_to_array(TX_GPRS_101_delaytime_data, my_txbuf);
        my_txbuf[14] = my_rtcbuffer1[0];
        my_txbuf[15] = my_rtcbuffer1[1];
        wdz_GPRS_string_to_array(RX_GPRS_101_OKdata, my_rxbuf);
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server Time-1\r\n");
    }
    //第2步，收到延时获得命令
    if(my_status == 1)
    {   //68 0C 0C 68 73 01 00 6A 01 03 01 00 00 00 C4 01 A8 16
        my_status = WDZ_GPRS_101receive_testactive(0X73, 0x6A, 0X03, 300);

    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server Time-2\r\n");
    }
    //第3步，发送肯定确认,	收到时钟同步激活
    if(my_status == 1)
    {
        WDZ_GPRS_101Transmit_OKdata();//发送肯定确认

        //收到时钟同步激活 68 11 11 68 53 01 00 67 01 06 01 00 00 00 8B D4 0B 0F 05 07 0E 5616

        my_status = WDZ_GPRS_101receive_testactive(0X53, 0x67, 0X06, 300);


    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server Time-3\r\n");
    }

    //第4步，发送肯定确认,	时钟同步命令(激活确认),接收肯定确认
    if(my_status == 1)
    {
        WDZ_GPRS_101Transmit_OKdata();//发送肯定确认

        //收到时钟同步激活 68 11 11 68 53 01 00 67 01 06 01 00   00 00     8B D4 0B 0F 05 07 0E    56 16

        for(k = 0; k < 7; k++)
        {
            my_rtcbuffer1[k] = USART1_my_frame[k + 14];
        }
        my_array_to_RTCtime(my_rtcbuffer1); //校时
        my_RTC_TO_EEPROM(my_rtcbuffer1, 0); //存入EEPROM中

        wdz_GPRS_string_to_array(TX_GPRS_101_time_synchronization_data, my_txbuf); //生成发送数据包
        my_RTCtime_to_array(my_rtcbuffer1); //读取RTC
        for(k = 0; k < 7; k++)
        {
            my_txbuf[k + 14] = my_rtcbuffer1[k]; //USART1_my_frame
        }

        wdz_GPRS_string_to_array(RX_GPRS_101_OKdata, my_rxbuf);
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server Time-4\r\n");
    }

    if(my_status == 1)
    {
        USART_printf(&huart3, " Server Time*\r\n");
    }

    if(my_status == 0)
    {
        USART_printf(&huart3, " Server Time-5\r\n");
    }

    return my_status;
}







/*
功能：接收总召指令，进行响应，发送数据
*/
uint8_t WDZ_GPRS_101Resiver_Call_Data(void)
{
    uint8_t my_status = 0;
    uint8_t my_rxbuf[256] = "\x10\x80\x01\x00\x81\x16";
    uint8_t my_txbuf[256] = {0};



    //第1步：接收到总召指令	 主站发送：68 0B 0B 68 73 01 00 64 01 06 01 00 00 00 14 F4 16
    my_status = WDZ_GPRS_101receive_testactive2(0X73, 0X64, 0X06, 300);
    if(my_status == 0)  return my_status; //   如果指令错误，返回，检查下一条指令

    if(my_status == 1)
    {
        //发送肯定确认指令
        WDZ_GPRS_101Transmit_OKdata();

        WDZ_101transmit_Calldata(2);//433模块总召
        WDZ_MCUtransmit_Calldata();		//MCU模块总召


        //生成发送数据包，发送数据，先发遥信、再发遥测、最后发环境
        //遥信数据包 68 23 23 68 73 01 00 01 98 14 00 01 00 01 00 00 00 00 00 00 00 00 23 16

        my_gprs_generate_101single_data(1, my_txbuf);
        my_at_senddata(my_txbuf);

        //遥测数据包
        //68 53 53 68 53 01 00 09 98 14 00 01 00 01 40 00 00 00 00 00 00 00 00 00 00 4B 16
        my_gprs_generate_101analog_data(1, my_txbuf);
        my_at_senddata(my_txbuf);
        //环境数据包
        //68 53 53 68 53 01 00 09 98 14 00 01 00 00 41 00 00 00 00 00 00 00 00 00 00 4B 16
        my_gprs_generate_101MCU_data(1, my_txbuf);
        my_at_senddata(my_txbuf);

        //激活终止;从站发送：68 12 12 68 73 01 00 09 84 14 01 00 00 41 68 05 A1 00 00 6C 00 2D FE 16,收到肯定确认
        wdz_GPRS_string_to_array(TX_GPRS_101_Calldata_over_data, my_txbuf);
        wdz_GPRS_string_to_array(RX_GPRS_101_OKdata, my_rxbuf);
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

        USART_printf(&huart3, " Server Call*\r\n");

    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server Call-1\r\n");
    }



    return my_status;
}



/*
功能：接收进程复位，重启MCU
*/

uint8_t WDZ_GPRS_101Resiver_ResetMCU_Data(void)
{
    uint8_t my_status = 0;
//	uint8_t k=0;
    uint8_t my_rxbuf[256] = "\x10\x8B\x01\x00\x8C\x16";
    uint8_t my_txbuf[256] = TX_GPRS_101_Linkquire_data;

    //第1步：接收到总召指令	 主站发送：68 0B 0B 68 73 01 00 69 01 06 01 00 00 00 01 E6 16
    my_status = WDZ_GPRS_101receive_testactive2(0X73, 0X69, 0X06, 300);
    if(my_status == 0)  return my_status; //   如果指令错误，返回，检查下一条指令

    if(my_status == 1)
    {
        //发送肯定确认指令
        WDZ_GPRS_101Transmit_OKdata();

        //复位进程命令(激活确认)：从站发 68 0B 0B 68 53 01 00 69 01 07 01 00 00 00 01 C7 16,收到肯定确认
        wdz_GPRS_string_to_array(TX_GPRS_101_ResetActive_data, my_txbuf);
        wdz_GPRS_string_to_array(RX_GPRS_101_OKdata, my_rxbuf);
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

        USART_printf(&huart3, " Server reset*\r\n");

    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server reset-1\r\n");
    }
    //第2步，进行系统重启，通过关闭硬件看门口
    if(my_status == 1)
    {
        my_reset_mcu(); //软重启
    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server reset-2\r\n");
    }


    return my_status;
}

/*
功能;生成无时标的报警数据帧，temp=1生成有效的数据包，0生成数据位0的空数据包

68 0B 0B 68 53 01 00 01 01 03 01 00 05 00 01 60 16
*/
void my_gprs_generate_101Alarm_Notime_data(uint8_t temp, uint8_t *my_rsbuf) //生成，无时标的报警数据包
{
    uint8_t length = 0;
    //uint8_t k=0;
    length = MY_433_ALarmdata_number; //信息体个数
    if(temp == 1)
    {
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length * 3 + 8;
        my_rsbuf[2] = length * 3 + 8;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X01; //类型标识，不带时标的单点信息，
        my_rsbuf[8] = length; //信息体个数
        my_rsbuf[9] = 0x03; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_buf1_to_buf2(MY_433_Alarmdata_NOtimeBuf, 0, my_rsbuf, 12, length * 3); //报警数据保存在MY_433_Alarmdata_NOtimeBuf数组中

        my_rsbuf[11 + length * 3 + 1] = 0XFF;
        my_rsbuf[11 + length * 3 + 1 + 1] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节


    }
    else if(temp == 0)
    {
        length = 2;
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length + 10;
        my_rsbuf[2] = length + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X01; //类型标识，带时标的单点信息，
        my_rsbuf[8] = 0x00; //信息体个数
        my_rsbuf[9] = 0x03; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;;

        my_rsbuf[12] = 0x00; //遥信信息体首地址
        my_rsbuf[13] = 0x00;

        my_rsbuf[14] = 0x00;
        my_rsbuf[15] = 0x00;

        my_rsbuf[16] = 0XFF;
        my_rsbuf[17] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节

    }

}

/*
功能;生成有时标的报警数据帧，temp=1生成有效的数据包，0生成数据位0的空数据包
68 1C 1C 68 73 01 00 1E 02 03 01 00 08 00 01 20 BA 16 0F 05 07 0D 05 00 01 49 BC 16 0F 05 07 0D 02 16
*/
void my_gprs_generate_101Alarm_Time_data(uint8_t temp, uint8_t *my_rsbuf) // 生成，有时标的报警数据包
{
    uint8_t length = 0;
    //uint8_t k=0;
    length = MY_433_ALarmdata_number;
    if(temp == 1)
    {
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length * 10 + 8;
        my_rsbuf[2] = length * 10 + 8;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X1E; //类型标识，不带时标的单点信息，
        my_rsbuf[8] = length; //信息体个数
        my_rsbuf[9] = 0x03; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_buf1_to_buf2(MY_433_Alarmdata_TimeBuf, 0, my_rsbuf, 12, length * 10);

        my_rsbuf[11 + length * 10 + 1] = 0XFF;
        my_rsbuf[11 + length * 10 + 1 + 1] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节


    }
    else if(temp == 0)
    {
        length = 2;
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length + 10;
        my_rsbuf[2] = length + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X1E; //类型标识，带时标的单点信息，
        my_rsbuf[8] = 0x00; //信息体个数
        my_rsbuf[9] = 0x03; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;;

        my_rsbuf[12] = 0x00; //遥信信息体首地址
        my_rsbuf[13] = 0x00;

        my_rsbuf[14] = 0x00;
        my_rsbuf[15] = 0x00;

        my_rsbuf[16] = 0XFF;
        my_rsbuf[17] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节

    }



}



/*
功能：从站主动发送报警数据，先发送无时标数据，后发送带时标数据
*/
uint8_t WDZ_GPRS_101Transmit_Alarm_Data(void)
{
    uint8_t my_status = 0;
    uint8_t my_rxbuf[256] = "\x10\x80\x01\x00\x81\x16";
    uint8_t my_txbuf[256] = {0};

    my_gprs_generate_101Alarm_Notime_data(1, my_txbuf);

    my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
    if(my_status == 1)
    {
        my_gprs_generate_101Alarm_Time_data(1, my_txbuf);
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
        USART_printf(&huart3, " Server Alarm*\r\n");
    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server Alarm-1\r\n");
    }

    return my_status;
}





/*
功能：被动接收 遥测 参数
*/
uint8_t WDZ_GPRS_101Resiver_Adjustvalue_data(void)
{
    uint8_t my_status = 0;
    uint8_t my_rxbuf[256] = "\x68\x0C\x0C\x68\x00\x03\x00\x30\x01\x07\x03\x00\x02\x50\x0C\x00\x0E\x16";
    uint8_t my_databuf[256] = {0};
    uint8_t length = 0;
    uint8_t k = 0;
    //uint8_t my_txbuf[256]=TX_GPRS_101_Linkquire_data;
    my_status = WDZ_GPRS_101receive_testactive2(0X73, 0X30, 0X06, 300);
    if(my_status == 0)  return my_status; //   如果指令错误，返回，检查下一条指令

    if(my_status == 1)
    {
        length = USART1_my_frame[1];
        for(k = 0; k < length - 8; k++)
        {
            my_databuf[k] = USART1_my_frame[k + 12];
        }
        //调用函数，进行响应的参数设置，my_databuf数组中0,1两个字节存放信息体地址，后面为数据，利用地址来判断对应的功能，进行响应的处理
        WDZ_GPRS_Adjust_value(my_databuf);  //设计系统参数*******

        my_rxbuf[4] = 0x00;
        my_rxbuf[9] = 0x07;
        my_rxbuf[5] = DTU_ADDRESS & 0x00ff;
        my_rxbuf[6] = (DTU_ADDRESS >> 8) & 0x00ff;

        my_rxbuf[10] = DTU_ADDRESS & 0x00ff;
        my_rxbuf[11] = (DTU_ADDRESS >> 8) & 0x00ff;


        for(k = 0; k < length - 8; k++)
        {
            //my_databuf[k]=USART1_my_frame[k+12];
            my_rxbuf[12 + k] = my_databuf[k];
        }

        //my_rxbuf[12]=my_databuf[0];
        //my_rxbuf[13]=my_databuf[1];
        //my_rxbuf[14]=my_databuf[2];
        //my_rxbuf[15]=my_databuf[3];


        wdz_GPRS_101check_generate(my_rxbuf);
        my_at_senddata(my_rxbuf);


    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server Adjustdata-1\r\n");
    }



    return my_status;
}

/*
功能：对收到的参数设置命令进行响应，设置相关的参数，主要包括DTU地址5001，心跳包时间5002，周期数据发送时间5003，Server地址6字节5004，
      增加手机号8字节5005，删除手机号码5006
*/
extern uint16_t Transmint_to_GPRS_hearttime;
extern uint16_t Transmint_to_GPRS_Cycledata;

extern uint8_t MY_IP[4]; //222.222.118.3
extern uint16_t MY_PORT;  //8080  16位数据

void WDZ_GPRS_Adjust_value(uint8_t *rsbuf)
{
    uint16_t my_address = 0;
    // uint8_t k = 0;
    uint16_t my_temp_val16 = 0;
    //uint8_t my_buf[100] = {0};
    //uint8_t my_phonebuf[16] = {0};
    //uint8_t my_firstaddress = 0;
    uint8_t my_status = 0;
    //uint8_t my_temp = 0;
    uint8_t time_buf[7] = {0};
    uint16_t my_temp_dtu = 0;
    uint8_t my_temp_rxbuf[256] = RX_GPRS_101_OKdata;
    uint8_t my_temp_txbuf[256] = TX_GPRS_101_Chaxun_data;




    my_address = rsbuf[1];
    my_address = ((my_address << 8) & 0xff00) + rsbuf[0];
    //*******DTU地址
    if(my_address == 0x5001)
    {
        my_temp_dtu = DTU_ADDRESS;
        my_temp_val16 = rsbuf[3];
        my_temp_val16 = ((my_temp_val16 << 8) & 0xff00) + rsbuf[2];
        DTU_ADDRESS = my_temp_val16;
        AT25_WriteByte(rsbuf[2], EEPROM_DTU_Address); //把新的DTU地址写入到EEPROM中
        AT25_WriteByte(rsbuf[3], EEPROM_DTU_Address + 1);


        //发送DTU地址
        my_temp_txbuf[5] = my_temp_dtu & 0xff;
        my_temp_txbuf[6] = (my_temp_dtu >> 8) & 0xff;
        my_temp_txbuf[10] = my_temp_dtu & 0xff;
        my_temp_txbuf[11] = (my_temp_dtu >> 8) & 0xff;

        my_temp_txbuf[12] = 0x01;
        my_temp_txbuf[13] = 0x50;
        my_temp_txbuf[14] = DTU_ADDRESS & 0xff;
        my_temp_txbuf[15] = (DTU_ADDRESS >> 8) & 0xff;
        wdz_GPRS_101check_generate(my_temp_txbuf);

        my_status = WDZ_GPRS_101transmint_commd_wait_commd2(2, my_temp_txbuf, 2, my_temp_rxbuf, my_temp_dtu);

        Delay_ms(3);
        my_reset_mcu();//重新启动MCU



    }
    //******心跳包时间
    else if(my_address == 0x5002)
    {
        my_temp_val16 = rsbuf[3];
        my_temp_val16 = ((my_temp_val16 << 8) & 0xff00) + rsbuf[2];
        Transmint_to_GPRS_hearttime = my_temp_val16;
        MY_M_speed_heart = my_temp_val16; //20160621

        AT25_WriteByte(rsbuf[2], EEPROM_Hearttime_Address); //把新的心跳包时间地址写入到EEPROM中
        AT25_WriteByte(rsbuf[3], EEPROM_Hearttime_Address + 1);
    }
    //******周期数据发送时间
    else if(my_address == 0x5003)
    {
        my_temp_val16 = rsbuf[3];
        my_temp_val16 = ((my_temp_val16 << 8) & 0xff00) + rsbuf[2];
        Transmint_to_GPRS_Cycledata = my_temp_val16;
        MY_M_speed_cyc = my_temp_val16; //20160621

        AT25_WriteByte(rsbuf[2], EEPROM_Cycetime_Address); //把新的周期数据时间地址写入到EEPROM中
        AT25_WriteByte(rsbuf[3], EEPROM_Cycetime_Address + 1);

    }

    //20160621****阀值上线 下线
    else if(my_address == 0x5010)
    {
        my_temp_val16 = rsbuf[3];
        my_temp_val16 = ((my_temp_val16 << 8) & 0xff00) + rsbuf[2];
        MY_Speed_H_Gate = my_temp_val16 / 65535 * 15.0;
        AT25_WriteByte(rsbuf[2], EEPROM_SPEED_Gate_H_Address); //把新的周期数据时间地址写入到EEPROM中
        AT25_WriteByte(rsbuf[3], EEPROM_SPEED_Gate_H_Address + 1);




    }
    else if(my_address == 0x5011)
    {
        my_temp_val16 = rsbuf[3];
        my_temp_val16 = ((my_temp_val16 << 8) & 0xff00) + rsbuf[2];
        MY_H_speed_cyc = my_temp_val16;
        AT25_WriteByte(rsbuf[2], EEPROM_SPEED_H_Cyc_Address); //把新的周期数据时间地址写入到EEPROM中
        AT25_WriteByte(rsbuf[3], EEPROM_SPEED_H_Cyc_Address + 1);

        my_temp_val16 = rsbuf[5];
        my_temp_val16 = ((my_temp_val16 << 8) & 0xff00) + rsbuf[4];
        MY_H_speed_heart = my_temp_val16;
        //AT25_WriteByte(rsbuf[4], EEPROM_SPEED_H_Heart_Address); //把新的周期数据时间地址写入到EEPROM中
        //AT25_WriteByte(rsbuf[5], EEPROM_SPEED_H_Heart_Address + 1);
    }

    else if(my_address == 0x5012)
    {
        my_temp_val16 = rsbuf[3];
        my_temp_val16 = ((my_temp_val16 << 8) & 0xff00) + rsbuf[2];
        MY_L_speed_cyc = my_temp_val16;
        AT25_WriteByte(rsbuf[2], EEPROM_SPEED_H_Cyc_Address); //把新的周期数据时间地址写入到EEPROM中
        AT25_WriteByte(rsbuf[3], EEPROM_SPEED_H_Cyc_Address + 1);

        my_temp_val16 = rsbuf[5];
        my_temp_val16 = ((my_temp_val16 << 8) & 0xff00) + rsbuf[4];
        MY_L_speed_heart = my_temp_val16;
        AT25_WriteByte(rsbuf[4], EEPROM_SPEED_L_Cyc_Address); //把新的周期数据时间地址写入到EEPROM中
        AT25_WriteByte(rsbuf[5], EEPROM_SPEED_L_Cyc_Address + 1);
    }




    //********Server地址
    else if(my_address == 0x5004)
    {
        MY_IP[0] = rsbuf[2];
        MY_IP[1] = rsbuf[3];
        MY_IP[2] = rsbuf[4];
        MY_IP[3] = rsbuf[5];

        my_temp_val16 = rsbuf[7];
        my_temp_val16 = ((my_temp_val16 << 8) & 0xff00) + rsbuf[6];
        MY_PORT = my_temp_val16;

        SPI_EE_BufferWrite2(MY_IP, EEPROM_IP_Address, 4); //把新的 IP及 端口号 写入到EEPROM中
        AT25_WriteByte(rsbuf[6], EEPROM_IPPort_Address);
        AT25_WriteByte(rsbuf[7], EEPROM_IPPort_Address + 1);

        //返回确认指令
        my_temp_txbuf[1] = 0X0C + 4;
        my_temp_txbuf[2] = 0X0C + 4;

        my_temp_txbuf[5] = DTU_ADDRESS & 0xff;
        my_temp_txbuf[6] = (DTU_ADDRESS >> 8) & 0xff;
        my_temp_txbuf[10] = DTU_ADDRESS & 0xff;
        my_temp_txbuf[11] = (DTU_ADDRESS >> 8) & 0xff;

        my_temp_txbuf[12] = 0x04;
        my_temp_txbuf[13] = 0x50;

        my_temp_txbuf[14] = MY_IP[0];
        my_temp_txbuf[15] = MY_IP[1];
        my_temp_txbuf[16] = MY_IP[2];
        my_temp_txbuf[17] = MY_IP[3];

        my_temp_txbuf[18] = MY_PORT & 0x00ff;
        my_temp_txbuf[19] = (MY_PORT >> 8) & 0x00ff;

        my_temp_txbuf[20] = 0X00;
        my_temp_txbuf[21] = 0X16;

        wdz_GPRS_101check_generate(my_temp_txbuf);
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_temp_txbuf, 2, my_temp_rxbuf);

        Delay_ms(3);
        my_reset_mcu();//重新启动MCU

    }
    //*****查询指令，返回DTU地址，心跳包、周期时间、IP、PORT、time

    else if(my_address == 0x5020)
    {   my_status = 0;
        WDZ_GPRS_101Transmit_OKdata();
        //发送DTU地址
        my_temp_txbuf[5] = DTU_ADDRESS & 0xff;
        my_temp_txbuf[6] = (DTU_ADDRESS >> 8) & 0xff;
        my_temp_txbuf[10] = DTU_ADDRESS & 0xff;
        my_temp_txbuf[11] = (DTU_ADDRESS >> 8) & 0xff;

        my_temp_txbuf[12] = 0x01;
        my_temp_txbuf[13] = 0x50;
        my_temp_txbuf[14] = DTU_ADDRESS & 0xff;
        my_temp_txbuf[15] = (DTU_ADDRESS >> 8) & 0xff;
        wdz_GPRS_101check_generate(my_temp_txbuf);

        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_temp_txbuf, 2, my_temp_rxbuf);
        //发送心跳时间
        if(my_status == 1)
        {
            my_temp_txbuf[12] = 0x02;
            my_temp_txbuf[13] = 0x50;
            my_temp_txbuf[14] = Transmint_to_GPRS_hearttime & 0xff;
            my_temp_txbuf[15] = (Transmint_to_GPRS_hearttime >> 8) & 0xff;
            wdz_GPRS_101check_generate(my_temp_txbuf);
            my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_temp_txbuf, 2, my_temp_rxbuf);
        }
        else if(my_status == 0)
        {
            USART_printf(&huart3, " chaxun error-1!\r\n");
        }
        //发送周期时间
        if(my_status == 1)
        {
            my_temp_txbuf[12] = 0x03;
            my_temp_txbuf[13] = 0x50;
            my_temp_txbuf[14] = Transmint_to_GPRS_Cycledata & 0xff;
            my_temp_txbuf[15] = (Transmint_to_GPRS_Cycledata >> 8) & 0xff;
            wdz_GPRS_101check_generate(my_temp_txbuf);
            my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_temp_txbuf, 2, my_temp_rxbuf);
        }
        else if(my_status == 0)
        {
            USART_printf(&huart3, " chaxun error-2!\r\n");
        }
        //IP地址及PORT
        if(my_status == 1)
        {
            my_temp_txbuf[1] = 0X0C + 4;
            my_temp_txbuf[2] = 0X0C + 4;
            my_temp_txbuf[12] = 0x04;
            my_temp_txbuf[13] = 0x50;

            my_temp_txbuf[14] = MY_IP[0];
            my_temp_txbuf[15] = MY_IP[1];
            my_temp_txbuf[16] = MY_IP[2];
            my_temp_txbuf[17] = MY_IP[3];

            my_temp_txbuf[18] = MY_PORT & 0x00ff;
            my_temp_txbuf[19] = (MY_PORT >> 8) & 0x00ff;

            my_temp_txbuf[20] = 0X00;
            my_temp_txbuf[21] = 0X16;

            wdz_GPRS_101check_generate(my_temp_txbuf);
            my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_temp_txbuf, 2, my_temp_rxbuf);
        }
        else if(my_status == 0)
        {
            USART_printf(&huart3, " chaxun error-3!\r\n");
        }
        //DTU的时间
        if(my_status == 1)
        {
            my_temp_txbuf[1] = 0X0C + 5;
            my_temp_txbuf[2] = 0X0C + 5;
            my_temp_txbuf[12] = 0x19;
            my_temp_txbuf[13] = 0x50;

            my_RTCtime_to_array(time_buf);  //读取RTC时间
            my_temp_txbuf[14] = time_buf[0];
            my_temp_txbuf[15] = time_buf[1];
            my_temp_txbuf[16] = time_buf[2];
            my_temp_txbuf[17] = time_buf[3];
            my_temp_txbuf[18] = time_buf[4];
            my_temp_txbuf[19] = time_buf[5];
            my_temp_txbuf[20] = time_buf[6];

            my_temp_txbuf[21] = 0X00;
            my_temp_txbuf[22] = 0X16;

            wdz_GPRS_101check_generate(my_temp_txbuf);
            my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_temp_txbuf, 2, my_temp_rxbuf);
        }
        else if(my_status == 0)
        {
            USART_printf(&huart3, " chaxun error-4!\r\n");
        }

    }




}

/*
功能：电话号码检查，电话号码簿中是否含有这个电话号码
*/
uint8_t WDZ_GPRS_Phonenumber_check(uint8_t *phone_source_buf1, uint8_t *phone_getnew_buf2)
{
    uint8_t my_status = 0;
    uint8_t k = 0;
    uint8_t mm = 0;
    uint8_t my_firstaddress = 0;


    for(k = 0; k < 100; k++)
    {
        if(phone_source_buf1[k] == phone_getnew_buf2[mm])
        {
            if(phone_getnew_buf2[mm] == 0x00)
            {
                my_firstaddress = k;
                break;
            }
            mm++;


        }
        else mm = 0;

    }

    if(my_firstaddress > 0) my_status = (my_firstaddress / 16) * 16;
    else my_status = 0;



    return my_status;

}

/*
功能：被动接收到的101指令进行分析，产生相应的动作
*/
void WDZ_GPRS_101Resiver_Analyse(void)
{
    uint8_t my_status = 0;
    my_usart_GPRS_101frame(1);  //取一个完整的101帧
    if(USART1_FRAME_status > 0)
    {
        my_status = WDZ_GPRS_101Resiver_AdjustTimeSimple_data(); //1  测试,简单校时

        if(my_status == 0) my_status = WDZ_GPRS_101Resiver_ResetMCU_Data(); //2  测试，进程复位，重启主MCU

        if(my_status == 0) my_status = WDZ_GPRS_101Resiver_Adjustvalue_data(); //3 调试通过,调试DTU参数，心跳、周期、IP，port

        if(my_status == 0) my_status == WDZ_GPRS_101Resiver_AdjustTime_data(); //4

        if(my_status == 0) my_status = WDZ_GPRS_101Resiver_CallHistory_data(); //5

        if(my_status == 0) my_status = WDZ_GPRS_101Resiver_Call_Data(); //6  通过 总召数据

        if(my_status == 0) my_status = WDZ_GPRS_101Resiver_ControlMonitor_Data(); //7通过，翻牌

        if(my_status == 0) my_status = WDZ_GPRS_101Resiver_Cycle_analog_time_Data(); //8 通过，设置指示器时间

        //9设置门限阀值，上线
        //10设置门限阀值，下线
        //11设置高速周期值
        //12设置高速心跳值
        //13设置低速周期值
        //14设置低速心跳值




    }
    USART1_FRAME_status = 0;
}







/*
功能：总召历史数据，把以前没有发送出去的数据补发出去
原理：查询EEPROM中遥信和遥测表，对表中数据帧的发送状态帧进行分析，没有发送的数据就发送出去，然后修改状态帧。先发周期遥信、遥测、环境，再发报警数据。
			所有数据都发送完后，在发送总召结束帧。
			**比较麻烦的一个帧,使用效果有待检验*****
*/
uint8_t WDZ_GPRS_101Resiver_CallHistory_data() //
{
    uint8_t my_status = 0;
    uint8_t my_rxbuf[256] = "\x10\x80\x01\x00\x81\x16";
    uint8_t my_txbuf[256] = {0};
    uint8_t my_buf[256] = {0};
//	uint8_t my_getdata_status=1;
    uint32_t n = 0;
    uint32_t first_address = EEPROM_Cycle_Table2_StartAddress;
    uint32_t second_address = EEPROM_Cycle_Table2_EndAddress + 1;
    uint8_t single_data_number = 0;
    uint8_t cycle_data_number = 0;
    uint8_t alarm_data_number = 0;
    uint8_t rd_status = 0;
    uint8_t length = 0;
    uint32_t my_count = 0;

    //第1步，总召历史数据查询命令，68 0B 0B 68 53 01 00 64 01 2C 01 00 00 00 14 FF 16
    my_status = WDZ_GPRS_101receive_testactive2(0X73, 0X64, 0X2C, 300);
    if(my_status == 0)  return my_status; //   如果指令错误，返回，检查下一条指令

    if(my_status == 1)
    {
        //发送肯定确认指令
        WDZ_GPRS_101Transmit_OKdata();
        //查表 发送周期数据，每发送一条周期数据，等待一个确认OK帧
        for(n = first_address; n < second_address;)
        {
            SPI_EE_BufferRead2(my_buf, n, 3); //读取前一帧数据的前3个字符
            rd_status = my_buf[0];
            single_data_number = my_buf[1];
            cycle_data_number = my_buf[2];
            length = 3 + 8 + single_data_number + cycle_data_number * 3 + 7;
            if(rd_status == 2) //此帧数据没有发送出去,进行发送处理
            {
                SPI_EE_BufferRead2(my_buf, n, length);
                my_gprs_generate_101single_history_data(1, my_buf, my_txbuf); //生成一帧遥信数据
                my_at_senddata(my_txbuf);

                my_gprs_generate_101analog_history_data(1, my_buf, my_txbuf); //生成一帧遥测数据
                my_at_senddata(my_txbuf);

                my_gprs_generate_101MCU_history_data(1, my_buf, my_txbuf); //生成一帧环境数据
                my_at_senddata(my_txbuf);

                my_status = WDZ_GPRS_101receive_testactive(0x80, 00, 00, 300); //确认收到OK帧

                if(my_status == 1)
                {
                    AT25_WriteByte(0x01, n);
                    my_count++;  //发送成功一个,计数+1
                    n = n + length; //此帧数据处理完了，指针移动到下一帧数据
                }
                else if(my_status == 0)
                {
                    break;
                }
            }
            else if(rd_status == 1) //此帧数据已经发送完成了，跳到下一帧数据进行检查
            {
                n = n + length; //指针移动到下一帧数据
            }
            else if(rd_status == 0) //表格的所有数据检查完毕了，可以退出了
            {
                break;
            }



        }



    }
    if(my_status == 1) //查询报警数据表，发送未发送成功的报警数据帧
    {
        first_address = EEPROM_Alarm_Table1_StartAddress;
        second_address = EEPROM_Alarm_Table1_EndAddress;

        //查询Table1 ，获得报警数据
        //查表 发送周期数据，每发送一条周期数据，等待一个确认OK帧
        for(n = first_address; n < second_address;)
        {
            SPI_EE_BufferRead2(my_buf, n, 3); //读取前一帧数据的前3个字符
            rd_status = my_buf[0];
            alarm_data_number = my_buf[1];  //此帧数据中含有的信息体个数
            length = 2 + alarm_data_number * 10; //算出此帧数据的总长度
            if(rd_status == 2) //此帧数据没有发送出去,进行发送处理
            {
                SPI_EE_BufferRead2(my_buf, n, length);
                my_gprs_generate_101Alarm_Time_history_data(1, my_buf, my_txbuf); //生成一帧遥信数据
                my_at_senddata(my_txbuf);

                my_status = WDZ_GPRS_101receive_testactive(0x80, 00, 00, 300); //确认收到OK帧

                if(my_status == 1)
                {
                    AT25_WriteByte(0x01, n);
                    my_count++;  //发送成功一个,计数+1
                    n = n + length; //此帧数据处理完了，指针移动到下一帧数据
                }
                else if(my_status == 0)
                {
                    break;
                }
            }
            else if(rd_status == 1) //此帧数据已经发送完成了，跳到下一帧数据进行检查
            {
                n = n + length; //指针移动到下一帧数据
            }
            else if(rd_status == 0) //表格的所有数据检查完毕了，可以退出了
            {
                break;
            }



        }

    }
    //发送终止总召数据帧
    if(my_status == 1)
    {
        //激活终止;从站发送：68 0B 0B 68 53 01 00 64 01 0A 01 00 00 00 14 FF 16,收到肯定确认
        //									68 0B 0B 68 53 01 00 64 01 0A 01 00 00 00 14 FF 16
        wdz_GPRS_string_to_array(TX_GPRS_101_Callhistory_over_data, my_txbuf);
        wdz_GPRS_string_to_array(RX_GPRS_101_OKdata, my_rxbuf);
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

        USART_printf(&huart3, " Server Callhistorydata*\r\n");

    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server Callhistorydata--\r\n");
    }

    return my_status;

}


/*
功能;简化版 时钟同步 指令

（19）时钟同步激活：主站发：68 11 11 68 53 01 00 67 01 06 01 00 00 00 8B D4 0B 0F 05 07 0E 5616
（20）肯定确认：从站发；10 80 01 00 81 16
（21）时钟同步命令(激活确认)：从站发；68 11 11 68 73 01 00 67 01 07 00 01 00 00 B2 9D 0E 0F 05 07 0E 6A 16
（22）肯定确认：主站发：10 80 01 00 81 16
*/
uint8_t WDZ_GPRS_101Resiver_AdjustTimeSimple_data(void)
{
    uint8_t my_status = 0;
    uint8_t my_rtcbuffer1[7] = {0};

    uint8_t k = 0;
    uint8_t my_rxbuf[256] = "\x10\x8B\x01\x00\x8C\x16";
    uint8_t my_txbuf[256] = TX_GPRS_101_Linkquire_data;

    //第1步：获得延时获得命令，存储毫秒值，发送肯定确认，并校正本地毫秒值,取最新毫秒值，发送延时获得命令(激活确认)
    //68 0C 0C 68 53 01 00 6A 01 06 01 00 00 00 3D C3 C6 16
    my_status = WDZ_GPRS_101receive_testactive2(0X53, 0X67, 0X06, 300);
    if(my_status == 0)  return my_status; //   如果指令错误，返回，检查下一条指令

    //第4步，发送肯定确认,	时钟同步命令(激活确认),接收肯定确认
    if(my_status == 1)
    {
        WDZ_GPRS_101Transmit_OKdata();//发送肯定确认

        //收到时钟同步激活 68 11 11 68 53 01 00 67 01 06 01 00   00 00     8B D4 0B 0F 05 07 0E    56 16

        for(k = 0; k < 7; k++)
        {
            my_rtcbuffer1[k] = USART1_my_frame[k + 14];
        }
        my_array_to_RTCtime(my_rtcbuffer1); //校时
        my_RTC_TO_EEPROM(my_rtcbuffer1, 0); //存入EEPROM中

        wdz_GPRS_string_to_array(TX_GPRS_101_time_synchronization_data, my_txbuf); //生成发送数据包
        my_RTCtime_to_array(my_rtcbuffer1); //读取RTC
        for(k = 0; k < 7; k++)
        {
            my_txbuf[k + 14] = my_rtcbuffer1[k];
        }

        wdz_GPRS_string_to_array(RX_GPRS_101_OKdata, my_rxbuf);
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server Simple Time-4\r\n");
    }

    if(my_status == 1)
    {
        USART_printf(&huart3, " Server Simple Time*\r\n");
    }

    if(my_status == 0)
    {
        USART_printf(&huart3, " Server  Simple Time-5\r\n");
    }

    return my_status;

}








/*
功能：生成遥信数据包  68 23 23 68 73 01 00 02 ?? 14 01 00 01 00 ?? ??  ??  ** 16
*/
void my_gprs_generate_101single_history_data(uint8_t temp, uint8_t *my_buf, uint8_t *my_rsbuf)
{
    uint8_t length = 0;
    uint8_t alarm_data_number = 0;
    uint8_t cycle_data_number = 0;


    if(temp == 1) //生成数据包
    {
        length = my_buf[1];
        alarm_data_number = my_buf[1];
        cycle_data_number = my_buf[2];


        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length + 10 + 7;
        my_rsbuf[2] = length + 10 + 7;

        //控制域码处理
        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        //my_rsbuf[4]=0x73&WDZ_GPRS_101FCB;  //控制域码为53/73

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X02; //类型标识，带时标的单点信息，
        my_rsbuf[8] = length + 0x80; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[12] = 0x01; //遥信信息体首地址
        my_rsbuf[13] = 0x00;

        my_buf1_to_buf2(my_buf, 3 + 8, my_rsbuf, 14, length); //存储遥信数据

        my_buf1_to_buf2(my_buf, 3 + 8 + alarm_data_number + cycle_data_number * 3, my_rsbuf, 14 + length, 7); //存储时标

        my_rsbuf[13 + length + 7 + 1] = 0XFF;
        my_rsbuf[13 + length + 7 + 1 + 1] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节



    }
    else if(temp == 0) //生成0数据体数据包
    {
        length = 2;
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length + 10;
        my_rsbuf[2] = length + 10;

        //控制域码处理
        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X02; //类型标识，带时标的单点信息，
        my_rsbuf[8] = 0x00; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[12] = 0x01; //遥信信息体首地址
        my_rsbuf[13] = 0x00;

        my_rsbuf[14] = 0x00;
        my_rsbuf[15] = 0x00;

        my_rsbuf[16] = 0XFF;
        my_rsbuf[17] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节
    }


}

/*
功能;生成遥测数据包

//68 53 53 68 53 01 00 09 98 14 00 01 00 01 40 00 00 00 00 00 00 00 00 00 00 4B 16
*/
void my_gprs_generate_101analog_history_data(uint8_t temp, uint8_t *my_buf, uint8_t *my_rsbuf)
{
    uint8_t length = 0;
    uint8_t single_data_number = 0;
//	uint8_t cycle_data_number=0;


    if(temp == 1) //生成数据包
    {
        length = my_buf[2];
        single_data_number = my_buf[1];
//		cycle_data_number=my_buf[2];

        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length * 3 + 10;
        my_rsbuf[2] = length * 3 + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X09; //类型标识，带时标的单点信息，
        my_rsbuf[8] = length + 0x80; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;;

        my_rsbuf[12] = 0x01; //遥信信息体首地址
        my_rsbuf[13] = 0x40;

        my_buf1_to_buf2(my_buf, 3 + 8 + single_data_number, my_rsbuf, 14, length * 3);


        my_rsbuf[13 + length * 3 + 1] = 0XFF;
        my_rsbuf[13 + length * 3 + 1 + 1] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节



    }
    else if(temp == 0) //生成0数据体数据包
    {
        length = 2;
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length + 10;
        my_rsbuf[2] = length + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X09; //类型标识，带时标的单点信息，
        my_rsbuf[8] = 0x00; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;;

        my_rsbuf[12] = 0x01; //遥信信息体首地址
        my_rsbuf[13] = 0x40;

        my_rsbuf[14] = 0x00;
        my_rsbuf[15] = 0x00;

        my_rsbuf[16] = 0XFF;
        my_rsbuf[17] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节
    }



}







/*
功能;生成环境数据包,temp为1生成有数据的数据包，0生成0数据的数据包
*/
void my_gprs_generate_101MCU_history_data(uint8_t temp, uint8_t *my_buf, uint8_t *my_rsbuf)
{
    uint8_t length = 0;

    if(temp == 1) //生成数据包
    {
        length = 4;

        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length * 2 + 10;
        my_rsbuf[2] = length * 2 + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);


        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X09; //类型标识，带时标的单点信息，
        my_rsbuf[8] = length + 0x80; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;;

        my_rsbuf[12] = 0x00; //遥信信息体首地址
        my_rsbuf[13] = 0x41;

        my_buf1_to_buf2(my_buf, 3, my_rsbuf, 14, length * 2);



        my_rsbuf[13 + length * 2 + 1] = 0XFF;
        my_rsbuf[13 + length * 2 + 1 + 1] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节



    }
    else if(temp == 0) //生成0数据体数据包
    {
        length = 2;
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length + 10;
        my_rsbuf[2] = length + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X09; //类型标识，带时标的单点信息，
        my_rsbuf[8] = 0x00; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;;

        my_rsbuf[12] = 0x00; //遥信信息体首地址
        my_rsbuf[13] = 0x41;

        my_rsbuf[14] = 0x00;
        my_rsbuf[15] = 0x00;

        my_rsbuf[16] = 0XFF;
        my_rsbuf[17] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节
    }


}

/*
功能;生成有时标的报警数据帧，temp=1生成有效的数据包，0生成数据位0的空数据包
68 1C 1C 68 73 01 00 1E 02 03 01 00 08 00 01 20 BA 16 0F 05 07 0D 05 00 01 49 BC 16 0F 05 07 0D 02 16
*/
void my_gprs_generate_101Alarm_Time_history_data(uint8_t temp, uint8_t *my_buf, uint8_t *my_rsbuf) // 生成，有时标的报警数据包
{
    uint8_t length = 0;
    //uint8_t k=0;
    length = my_buf[1];
    if(temp == 1)
    {
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length * 10 + 8;
        my_rsbuf[2] = length * 10 + 8;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X1E; //类型标识，不带时标的单点信息，
        my_rsbuf[8] = length; //信息体个数
        my_rsbuf[9] = 0x03; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_buf1_to_buf2(my_buf, 2, my_rsbuf, 12, length * 10);

        my_rsbuf[11 + length * 10 + 1] = 0XFF;
        my_rsbuf[11 + length * 10 + 1 + 1] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节


    }
    else if(temp == 0)
    {
        length = 2;
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length + 10;
        my_rsbuf[2] = length + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X1E; //类型标识，带时标的单点信息，
        my_rsbuf[8] = 0x00; //信息体个数
        my_rsbuf[9] = 0x03; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;;

        my_rsbuf[12] = 0x00; //遥信信息体首地址
        my_rsbuf[13] = 0x00;

        my_rsbuf[14] = 0x00;
        my_rsbuf[15] = 0x00;

        my_rsbuf[16] = 0XFF;
        my_rsbuf[17] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节

    }



}
//--------测试用程序--------------------------
/*
功能：测试，把总召数据写入到GPRS发送数组中，并生成遥信帧，遥测帧、环境数据 中国
*/
void my_test_calldata_to_GPRS_array(void)
{
    uint8_t my_status = 0;
    uint8_t my_txbuf[256] = {0};

    my_status = my_eeprom_data_to_cycle_array();

    if(my_status == 1)
    {
        //生成发送数据包，发送数据，先发遥信、再发遥测、最后发环境
        //遥信数据包 68 23 23 68 73 01 00 01 98 14 00 01 00 01 00 00 00 00 00 00 00 00 23 16

        my_gprs_generate_101single_data(1, my_txbuf);
        USART_printf(&huart3, "\r\n");
        USART_printf(&huart3, my_txbuf);
        USART_printf(&huart3, "\r\n");

        //遥测数据包
        //68 53 53 68 53 01 00 09 98 14 00 01 00 01 40 00 00 00 00 00 00 00 00 00 00 4B 16
        my_gprs_generate_101analog_data(1, my_txbuf);
        USART_printf(&huart3, "\r\n");
        USART_printf(&huart3, my_txbuf);
        USART_printf(&huart3, "\r\n");
        //环境数据包
        //68 53 53 68 53 01 00 09 98 14 00 01 00 00 41 00 00 00 00 00 00 00 00 00 00 4B 16
        my_gprs_generate_101MCU_data(1, my_txbuf);
        USART_printf(&huart3, "\r\n");
        USART_printf(&huart3, my_txbuf);
        USART_printf(&huart3, "\r\n");
    }

}

/*
功能：进行补发报警数据监测及发送

需要的处理，把MY_433_Alarmdata_NOtimeBuf，MY_433_Alarmdata_timeBuf复制报警数据，MY_433_ALarmdata_number复制信息体个数
然后利用已有函数进行发送
      my_status=WDZ_GPRS_101Transmit_Alarm_Data();//利用GPRS网络发送报警数据，
			if(my_status==1)
			my_GPRS_chang_tablereadpt(1,1); //同时修改eeprom中，读写状态和写指针。
			else my_GPRS_chang_tablereadpt(1,0);  //只修改读指针地址，而不修改帧发送状态字节。
*/
void my_re_transmit_alarmdata()
{
    int transmint_alarmdata_status = 0;
    int my_status = 0;

    uint8_t my_buf[256] = {0};

    uint32_t first_address = 0;
    uint32_t second_address = 0;
    uint8_t alarm_data_number = 0;
    uint8_t rd_status = 0;
    uint8_t length = 0;
    int k = 0;
    uint8_t tempbuf[12] = {0};
    uint32_t startaddressPT = MY_Table1_Alarmdata_StartAddress;
    uint32_t endaddressPT = MY_Table1_Alarmdata_EndAddress;
    uint32_t writeaddressPT = MY_Table1_Alarmdata_WriteAddress;
    uint32_t readaddressPT = MY_Table1_Alarmdata_ReadAddress;
    uint32_t tableaddress = 210; //TABLE1读指针首地址
    int temp_address1 = 0;
    int temp_address2 = 0;

    transmint_alarmdata_status = AT25_ReadByte(200); //读取补发状态位

    //transmint_alarmdata_status=1;//测试使用，强制为1，正常运行时屏蔽掉@@@@@

    if(transmint_alarmdata_status == 1) //为1表示，有数据需要补发
    {

        //***处理获得最新的表指针地址
        SPI_EE_BufferRead2(tempbuf, 201, 12);
        if(tempbuf[0] == tempbuf[1] && tempbuf[1] == tempbuf[2] && tempbuf[2] == tempbuf[3] && tempbuf[3] == tempbuf[4] && tempbuf[4] == tempbuf[5])
        {

            my_val_writeto_eeprom(tempbuf, startaddressPT, endaddressPT, writeaddressPT, readaddressPT, 201);

        }
        else
        {

            my_buf_writeto_val(tempbuf, &startaddressPT, &endaddressPT, &writeaddressPT, &readaddressPT);

            MY_Table1_Alarmdata_StartAddress = startaddressPT;
            MY_Table1_Alarmdata_EndAddress = endaddressPT;
            MY_Table1_Alarmdata_WriteAddress = writeaddressPT;
            MY_Table1_Alarmdata_ReadAddress = readaddressPT;

        }
        //-----需要进行读指针校验，如果到末尾了从新赋值，变为起始地址，然后开始读
        if(readaddressPT + 12 > endaddressPT)
        {   readaddressPT = startaddressPT;
            MY_Table1_Alarmdata_ReadAddress = readaddressPT;
            tempbuf[0] = readaddressPT & 0x0000ff;
            tempbuf[1] = (readaddressPT >> 8) & 0x0000ff;
            tempbuf[2] = (readaddressPT >> 16) & 0x0000ff;
            SPI_EE_BufferWrite2(tempbuf, 210, 3);
        }
        //-------------------------



        first_address = readaddressPT;
        //first_address=startaddressPT;  //测试用，正常运行时删除 @@@@@
        second_address = writeaddressPT;

        //查询Table1 ，获得报警数据，生成无时标报警数据帧和有时标报警数据帧

        if(first_address < second_address) //1-写指针在高地址，读指针在低地址
        {
            while(first_address < second_address)
            {   my_status = 0;
                SPI_EE_BufferRead2(my_buf, first_address, 252); //读取前一帧数据的前3个字符
                rd_status = my_buf[0]; //GPRS发送状态，2为未发送，1为已发送
                alarm_data_number = my_buf[1];  //此帧数据中含有的信息体个数
                //读表指针进行强制校验 2015-11-16
                if((rd_status != 1 && rd_status != 2) || alarm_data_number != 1)
                {
                    first_address = second_address; // 进行指针校正，不一定有用
                    MY_Table1_Alarmdata_ReadAddress = first_address;
                    my_save_PTTO_EEROM(first_address, tableaddress);
                    return ;
                }


                length = 2 + alarm_data_number * 10; //算出此帧数据的总长度
                length = 12; //固定长度，考虑没有并发性

                if(rd_status == 2 && alarm_data_number == 1 ) //此帧数据没有发送出去,进行发送处理
                {
                    SPI_EE_BufferRead2(my_buf, first_address, length); //1字节为GPRS发送状态，2字节为个数，3字节为地址地位，4字节为地址高位，5字节为值，6-12为时标
                    for(k = 0; k < alarm_data_number; k++) //每个信息体占10个字节，地址2个，值一个，时标7个
                    {   temp_address1 = k * 10;
                        temp_address2 = k * 12;
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 0] = my_buf[2 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 1] = my_buf[3 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 2] = my_buf[4 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 3] = my_buf[5 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 4] = my_buf[6 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 5] = my_buf[7 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 6] = my_buf[8 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 7] = my_buf[9 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 8] = my_buf[10 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 9] = my_buf[11 + temp_address2];
                    }
                    for(k = 0; k < alarm_data_number; k++) //每个信息体占3个字节，地址2个，值一个
                    {   temp_address1 = k * 3;
                        temp_address2 = k * 12;
                        MY_433_Alarmdata_NOtimeBuf[temp_address1 + 0] = my_buf[2 + temp_address2];
                        MY_433_Alarmdata_NOtimeBuf[temp_address1 + 1] = my_buf[3 + temp_address2];
                        MY_433_Alarmdata_NOtimeBuf[temp_address1 + 2] = my_buf[4 + temp_address2];
                    }
                    MY_433_ALarmdata_number = alarm_data_number; //信息体个数

                    my_status = WDZ_GPRS_101Transmit_Alarm_Data(); //利用GPRS网络发送报警数据，
                    if(my_status == 1)
                    {
                        my_GPRS_chang_tablereadpt(1, 1); //同时修改eeprom中，读写状态和读指针地址。
                        first_address = first_address + length; //指针移动到下一帧数据
                        //读指针记录到rom中
                        // tempbuf[0]=first_address&0x0000ff;
                        // tempbuf[1]=(first_address>>8)&0x0000ff;
                        //  tempbuf[2]=(first_address>>16)&0x0000ff;
                        //  SPI_EE_BufferWrite2(tempbuf,tableaddress,3);
                    }
                    else
                    {
                        my_GPRS_chang_tablereadpt(1, 0); //读指针不移动，保存此帧发送状态为2，修改补发状态为1
                        //为了防止读指针不移动，死机的问题，抛弃这个有问题的帧，等待到下一个time扫描周期发送下一个帧
                        //	first_address=first_address+length;   //指针移动到下一帧数据??

                        //读指针记录到rom中
                        // tempbuf[0]=first_address&0x0000ff;
                        // tempbuf[1]=(first_address>>8)&0x0000ff;
                        //tempbuf[2]=(first_address>>16)&0x0000ff;
                        //SPI_EE_BufferWrite2(tempbuf,tableaddress,3);
                        //	my_save_PTTO_EEROM(first_address,tableaddress);
                        break;  //发送失败了，就不要在继续发了
                    }
                }
                else if(rd_status == 1 && alarm_data_number == 1) //此帧数据已经发送完成了，跳到下一帧数据进行检查
                {

                    first_address = first_address + length; //指针移动到下一帧数据

                    //读指针记录到rom中
                    //tempbuf[0]=first_address&0x0000ff;
                    //tempbuf[1]=(first_address>>8)&0x0000ff;
                    // tempbuf[2]=(first_address>>16)&0x0000ff;
                    // SPI_EE_BufferWrite2(tempbuf,tableaddress,3);
                    MY_Table1_Alarmdata_ReadAddress = first_address;
                    my_save_PTTO_EEROM(first_address, tableaddress);
                }
                else if(rd_status == 0 || rd_status > 2 || alarm_data_number != 1) //如果指针出现问题，发生漂移了,读指针移动到写指针处
                {
                    first_address = first_address + 12;
                }
                //判断读指针是否溢出区域
                if(first_address + 12 >= endaddressPT) //
                {
                    first_address = startaddressPT;
                }

            }

            if(my_status == 1) //为1表示成功了，所有补发数据都出去了
            {
                AT25_WriteByte(0x00, 200);
                RE_ALarmData_Status = 0x00;
                first_address = second_address; // 进行指针校正，不一定有用

                //读指针记录到rom中
                // tempbuf[0]=first_address&0x0000ff;
                //tempbuf[1]=(first_address>>8)&0x0000ff;
                //tempbuf[2]=(first_address>>16)&0x0000ff;
                //SPI_EE_BufferWrite2(tempbuf,tableaddress,3);
                MY_Table1_Alarmdata_ReadAddress = first_address;
                my_save_PTTO_EEROM(first_address, tableaddress);
            }
            else if(my_status == 0 && first_address >= second_address)
            {
                AT25_WriteByte(0x00, 200); //出问题的帧就抛弃了,撤销了补发报警数据状态
                RE_ALarmData_Status = 00;
                //指针校正
                first_address = second_address; // 进行指针校正，不一定有用

                //读指针记录到rom中
                //tempbuf[0]=first_address&0x0000ff;
                //tempbuf[1]=(first_address>>8)&0x0000ff;
                //tempbuf[2]=(first_address>>16)&0x0000ff;
                //SPI_EE_BufferWrite2(tempbuf,tableaddress,3);
                MY_Table1_Alarmdata_ReadAddress = first_address;
                my_save_PTTO_EEROM(first_address, tableaddress);

            }


        }
        else if(first_address > second_address) //2-写指针在低地址，读指针在高地址
        {
            while((first_address + 12) < endaddressPT)
            {   my_status = 0;
                SPI_EE_BufferRead2(my_buf, first_address, 3); //读取前一帧数据的前3个字符
                rd_status = my_buf[0]; //GPRS发送状态，2为未发送，1为已发送
                alarm_data_number = my_buf[1];  //此帧数据中含有的信息体个数
                //读表指针进行强制校验 2015-11-16
                if((rd_status != 1 && rd_status != 2) || alarm_data_number != 1)
                {
                    first_address = second_address; // 进行指针校正，不一定有用
                    MY_Table1_Alarmdata_ReadAddress = first_address;
                    my_save_PTTO_EEROM(first_address, tableaddress);
                    return ;
                }



                length = 2 + alarm_data_number * 10; //算出此帧数据的总长度
                length = 12; //固定长度，考虑没有并发性

                if(rd_status == 2 && alarm_data_number == 1) //此帧数据没有发送出去,进行发送处理
                {
                    SPI_EE_BufferRead2(my_buf, first_address, length); //1字节为GPRS发送状态，2字节为个数，3字节为地址地位，4字节为地址高位，5字节为值，6-12为时标
                    for(k = 0; k < alarm_data_number; k++) //每个信息体占10个字节，地址2个，值一个，时标7个
                    {
                        temp_address1 = k * 10;
                        temp_address2 = k * 12;
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 0] = my_buf[2 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 1] = my_buf[3 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 2] = my_buf[4 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 3] = my_buf[5 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 4] = my_buf[6 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 5] = my_buf[7 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 6] = my_buf[8 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 7] = my_buf[9 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 8] = my_buf[10 + temp_address2];
                        MY_433_Alarmdata_TimeBuf[temp_address1 + 9] = my_buf[11 + temp_address2];
                    }
                    for(k = 0; k < alarm_data_number; k++) //每个信息体占3个字节，地址2个，值一个
                    {
                        temp_address1 = k * 3;
                        temp_address2 = k * 12;
                        MY_433_Alarmdata_NOtimeBuf[temp_address1 + 0] = my_buf[2 + temp_address2];
                        MY_433_Alarmdata_NOtimeBuf[temp_address1 + 1] = my_buf[3 + temp_address2];
                        MY_433_Alarmdata_NOtimeBuf[temp_address1 + 2] = my_buf[4 + temp_address2];
                    }
                    MY_433_ALarmdata_number = alarm_data_number; //信息体个数

                    my_status = WDZ_GPRS_101Transmit_Alarm_Data(); //利用GPRS网络发送报警数据，
                    if(my_status == 1 )
                    {
                        my_GPRS_chang_tablereadpt(1, 1); //同时修改eeprom中，读写状态和读指针地址。
                        first_address = first_address + length; //指针移动到下一帧数据
                        //读指针记录到rom中
                        //tempbuf[0]=first_address&0x0000ff;
                        //tempbuf[1]=(first_address>>8)&0x0000ff;
                        //tempbuf[2]=(first_address>>16)&0x0000ff;
                        //SPI_EE_BufferWrite2(tempbuf,tableaddress,3);
                    }
                    else
                    {
                        my_GPRS_chang_tablereadpt(1, 0); //读指针不移动，保存此帧发送状态为2，修改补发状态为1
                        //为了防止读指针不移动，死机的问题，抛弃这个有问题的帧，等待到下一个time扫描周期发送下一个帧
                        //first_address=first_address+length;   //指针移动到下一帧数据

                        //读指针记录到rom中
                        //tempbuf[0]=first_address&0x0000ff;
                        //tempbuf[1]=(first_address>>8)&0x0000ff;
                        //tempbuf[2]=(first_address>>16)&0x0000ff;
                        //SPI_EE_BufferWrite2(tempbuf,tableaddress,3);
                        //  my_save_PTTO_EEROM(first_address,tableaddress);

                        break;  //发送失败了，就不要在继续发了
                    }
                }
                else if(rd_status == 1 && alarm_data_number == 1) //此帧数据已经发送完成了，跳到下一帧数据进行检查
                {
                    first_address = first_address + length; //指针移动到下一帧数据

                    //读指针记录到rom中
                    //tempbuf[0]=first_address&0x0000ff;
                    //tempbuf[1]=(first_address>>8)&0x0000ff;
                    //tempbuf[2]=(first_address>>16)&0x0000ff;
                    //SPI_EE_BufferWrite2(tempbuf,tableaddress,3);
                    MY_Table1_Alarmdata_ReadAddress = first_address;
                    my_save_PTTO_EEROM(first_address, tableaddress);
                }
                else if(rd_status == 0 || alarm_data_number == 0 || alarm_data_number > 1) //如果指针出现问题，发生漂移了,读指针复位为1
                {
                    first_address = endaddressPT;
                }

            }

            if(my_status == 1) //为1表示成功了，所有补发数据都出去了
            {
                //AT25_WriteByte(0x00,200);
                first_address = startaddressPT; // 进行指针校正，不一定有用

                //读指针记录到rom中
                //tempbuf[0]=first_address&0x0000ff;
                //tempbuf[1]=(first_address>>8)&0x0000ff;
                //tempbuf[2]=(first_address>>16)&0x0000ff;
                //SPI_EE_BufferWrite2(tempbuf,tableaddress,3);
                MY_Table1_Alarmdata_ReadAddress = first_address;
                my_save_PTTO_EEROM(first_address, tableaddress);
            }
            else if(my_status == 0 && (first_address + 12) > endaddressPT)
            {
                //指针校正
                first_address = startaddressPT; // 进行指针校正，不一定有用

                //读指针记录到rom中
                //tempbuf[0]=first_address&0x0000ff;
                //tempbuf[1]=(first_address>>8)&0x0000ff;
                //tempbuf[2]=(first_address>>16)&0x0000ff;
                // SPI_EE_BufferWrite2(tempbuf,tableaddress,3);
                MY_Table1_Alarmdata_ReadAddress = first_address;
                my_save_PTTO_EEROM(first_address, tableaddress);

            }



        }




    }

}

/*
	功能：存储mypt地址值的低3个字节到EEPROM中tabladdress开始的地址
	*/
void my_save_PTTO_EEROM(uint32_t mypt, uint32_t tableaddress)
{   uint8_t tempbuf[3] = {0};
    uint32_t first_address = mypt;
    tempbuf[0] = first_address & 0x0000ff;
    tempbuf[1] = (first_address >> 8) & 0x0000ff;
    tempbuf[2] = (first_address >> 16) & 0x0000ff;
    SPI_EE_BufferWrite2(tempbuf, tableaddress, 3);
}


/*
功能：接收  翻牌  指令，进行响应，发送数据
*/
uint8_t WDZ_GPRS_101Resiver_ControlMonitor_Data(void)
{
    uint8_t my_status = 0;
//	uint8_t my_rxbuf[256]="\x10\x80\x01\x00\x81\x16";
    uint8_t my_txbuf[256] = {0X68, 0X0E, 0X0E, 0X68, 0X00, 0X01, 0X00, 0X2D, 0X02, 0X07, 0X01, 0X00, 0X01, 0X60, 0X01, 0X02, 0X60, 0X00, 0XFF, 0X16};
    uint8_t *myrsbuf1 = USART1_my_frame;
    uint8_t status_value = 0;
    uint8_t my_address = 0;


    //第1步：接收到总召指令	 主站发送：68 0B 0B 68 73 01 00 64 01 06 01 00 00 00 14 F4 16
    my_status = WDZ_GPRS_101receive_testactive2(0X73, 0X2D, 0X06, 300);
    if(my_status == 0)  return my_status; //   如果指令错误，返回，检查下一条指令

    if(my_status == 1)
    {
        //发送肯定确认指令
        //	WDZ_GPRS_101Transmit_OKdata();
        my_txbuf[5] = DTU_ADDRESS & 0X00FF;
        my_txbuf[6] = DTU_ADDRESS >> 8;
        my_txbuf[8] = 2;
        my_txbuf[10] = DTU_ADDRESS & 0X00FF;
        my_txbuf[11] = DTU_ADDRESS >> 8;
        my_txbuf[12] = myrsbuf1[12]; //01
        my_txbuf[13] = myrsbuf1[13]; //60
        my_txbuf[14] = myrsbuf1[14]; //指示器地址，01,02,03，04，05,06,07,08,09
        my_txbuf[15] = myrsbuf1[15]; //02,翻牌，03复位
        my_txbuf[16] = myrsbuf1[16]; //60
        my_txbuf[17] = myrsbuf1[17]; //00
        wdz_GPRS_101check_generate(my_txbuf); //生成校验字节

        //my_at_senddata(my_txbuf);  //利用GPRS返回确认指令
        //控制433工作，进行翻牌操作
        if(my_txbuf[15] == 0X02) //翻牌
            status_value = 0x81;
        else if(my_txbuf[15] == 0X03)
            status_value = 0x80;
        if(my_txbuf[14] >= 4 && my_txbuf[14] <= 6)
            my_address = my_txbuf[14] + 1;
        else if(my_txbuf[14] >= 7 && my_txbuf[14] <= 9)
            my_address = my_txbuf[14] + 2;
        else if(my_txbuf[14] >= 1 && my_txbuf[14] <= 3)
            my_address = my_txbuf[14] + 0;

        my_status = WDZ_101Transmit_Control_monitor(my_address, status_value);		//对433模块发送指令，动作指示器
    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server ControlMonitor fail-1\r\n");
    }

    if(my_status == 1)
    {
        USART_printf(&huart3, " Server ControlMonitor* %d\r\n", my_address);
        my_at_senddata(my_txbuf);  //利用GPRS返回确认指令
    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server ControlMonitor fail-2\r\n");
    }



    return my_status;
}

void my_reset_mcu()  //重启MCU通过软命令
{
    __disable_fault_irq();
    NVIC_SystemReset();
}

/*
功能：发送命令，等待（特定）反馈命令,更换DTU地址使用�
发送数据type为类型，1为固定长度，2为非固定长度，txbuf为发送指令数组,address_type为1，rxbuf为接收，DTU地址
（2,txbuf,2,rxbuf)发送非固定长度，(1,txbuf,2,rxbuf)发送固定长度
*/
uint8_t WDZ_GPRS_101transmint_commd_wait_commd2(uint8_t type, uint8_t *txbuf, uint8_t address_type, uint8_t *rxbuf, uint16_t my_temp_dtu_address) //type为类型，1为固定长度，2为非固定长度，txbuf为发送指令数组
{
    uint8_t FCB = 0x20;
    uint16_t my_dtu_address = my_temp_dtu_address;
    uint8_t my_status = 0;
    uint8_t n = 1; //发送次数
    uint8_t *my_txbuf = txbuf;
//	uint8_t temp=0;

    uint8_t linkcontrol = 0;
    uint8_t type_identification = 0;
    uint8_t  transmit_reason = 0;
    uint16_t my_start_time = 0;
//********************发送非固定长度帧，应答为OK帧
    if(type == 2)
    {
        //控制域码处理
        if(WDZ_GPRS_101FCB == 0x00)
            my_txbuf[4] = my_txbuf[4] & (~FCB);
        else if(WDZ_GPRS_101FCB == 0x20)
            my_txbuf[4] = my_txbuf[4] | (FCB);
        //目的地址处理
        my_txbuf[5] = my_temp_dtu_address & 0xFF;
        my_txbuf[6] = my_temp_dtu_address >> 8;
        if(address_type == 1)
        {
            my_txbuf[10] = my_txbuf[5];
            my_txbuf[11] = my_txbuf[6];
        }
        else if(address_type == 2)
        {
            my_txbuf[10] = my_txbuf[5];
            my_txbuf[11] = my_txbuf[6];
        }

        //校验位产生
        wdz_GPRS_101check_generate(my_txbuf);
        //发送指令
        while((n) > 0 && my_status == 0)
        {

            my_at_senddata(my_txbuf); //利用GPRS发送数据数组

            //Delay_us(300); //测试使用


            my_start_time = my_tim6_count;
            do {

                my_status = WDZ_GPRS_101receive_testactive(0X80, 00, 00, 1500); ///WDZ_GPRS_101recive_OKdata();  //等待0K确认反馈命令 15秒
                if(my_status == 1)break;
                else my_status = 0;
            } while(my_tim6_count - my_start_time <= 3);
            n--;
            if(n != 0) USART_printf(&huart1, "\x1A"); //发一个结束符，防止拥塞

        }



    }


//**************发送固定长度帧，应答为固定长度帧
    else	if(type == 1 && rxbuf[0] == 0x10)
    {
        //产生接收的控制域码，类型，传输原因
        linkcontrol = rxbuf[1];
        type_identification = 0;
        transmit_reason = 0;

        //控制域码处理
        if(WDZ_GPRS_101FCB == 0x00)
            my_txbuf[1] = my_txbuf[1] & (~FCB);
        else if(WDZ_GPRS_101FCB == 0x20)
            my_txbuf[1] = my_txbuf[1] | (FCB);
        //目的地址处理
        my_txbuf[2] = my_dtu_address & 0xFF;
        my_txbuf[3] = my_dtu_address >> 8;

        //校验位产生
        wdz_GPRS_101check_generate(my_txbuf);
        //发送指令
        while((n--) > 0 && my_status == 0)
        {
            //my_UART4_printf(UART4,my_txbuf);
            my_at_senddata(my_txbuf); //利用GPRS发送数据数组

            //Delay_us(300); //测试

            my_status = WDZ_GPRS_101receive_testactive(linkcontrol, type_identification, transmit_reason, 1500); //等待反馈命令
            if(my_status == 1)break;
            else my_status = 0;
        }
    }
//**********发送固定长度帧，应该为非固定长度帧
    else if(type == 1 && rxbuf[0] == 0x68)
    {
        //产生接收的控制域码，类型，传输原因
        linkcontrol = rxbuf[4];
        type_identification = rxbuf[7];
        transmit_reason = rxbuf[9];

        //控制域码处理
        if(WDZ_GPRS_101FCB == 0x00)
            my_txbuf[1] = my_txbuf[1] & (~FCB);
        else if(WDZ_GPRS_101FCB == 0x20)
            my_txbuf[1] = my_txbuf[1] | (FCB);
        //目的地址处理
        my_txbuf[2] = my_temp_dtu_address & 0xFF;
        my_txbuf[3] = my_temp_dtu_address >> 8;

        //校验位产生
        wdz_GPRS_101check_generate(my_txbuf);
        //发送指令
        while((n--) > 0 && my_status == 0)
        {
            //my_UART4_printf(UART4,my_txbuf);
            my_at_senddata(my_txbuf);

            //Delay_us(600); //测试

            my_status = WDZ_GPRS_101receive_testactive(linkcontrol, type_identification, transmit_reason, 1500); //等待反馈命令
            if(my_status == 1)break;
            else my_status = 0;
        }
    }


    //记录发送变化帧
    WDZ_GPRS_101FCB = WDZ_GPRS_101FCB ^ 0x20;
    if(my_status == 1)
    {
        return 1;
    }
    else return 0;

}

/*
功能：调整指示器遥测数据周期时间，一次只能调试一个指示器。
*/
uint8_t WDZ_GPRS_101Resiver_Cycle_analog_time_Data(void)
{
    uint8_t my_status = 0;
//	uint8_t my_rxbuf[256]="\x10\x80\x01\x00\x81\x16";
//68 0E 0E 68 73 03 00 3D 02 07 03 00 01 70 01 02 70 01 72 16

    uint8_t my_txbuf[256] = {0X68, 0X0E, 0X0E, 0X68, 0X73, 0X03, 0X00, 0X3D, 0X02, 0X07, 0X03, 0X00, 0X01, 0X70, 0X01, 0X02, 0X70, 0X01, 0XFF, 0X16};
    uint8_t *myrsbuf1 = USART1_my_frame;
    uint8_t status_value = 0;
    uint8_t my_address = 0;


    //第1步：接收到总召指令	 主站发送：68 0E 0E 68 73 03 00 3D 02 06 03 00 01 70 01 02 70 01 72 16
    my_status = WDZ_GPRS_101receive_testactive2(0X73, 0X3D, 0X06, 300);
    if(my_status == 0)  return my_status; //   如果指令错误，返回，检查下一条指令

    if(my_status == 1)
    {
        //发送肯定确认指令
        //	WDZ_GPRS_101Transmit_OKdata();
        my_txbuf[5] = DTU_ADDRESS & 0X00FF;
        my_txbuf[6] = DTU_ADDRESS >> 8;
        my_txbuf[8] = 2;
        my_txbuf[10] = DTU_ADDRESS & 0X00FF;
        my_txbuf[11] = DTU_ADDRESS >> 8;
        my_txbuf[12] = myrsbuf1[12]; //01
        my_txbuf[13] = myrsbuf1[13]; //70
        my_txbuf[14] = myrsbuf1[14]; //指示器地址，01,02,03，04，05,06,07,08,09
        my_txbuf[15] = myrsbuf1[15]; //02,
        my_txbuf[16] = myrsbuf1[16]; //70
        my_txbuf[17] = myrsbuf1[17]; //01设置时间
        wdz_GPRS_101check_generate(my_txbuf); //生成校验字节

        //my_at_senddata(my_txbuf);  //利用GPRS返回确认指令
        //控制433工作，进行指示器时间操作

        status_value = my_txbuf[17];

        if(my_txbuf[14] >= 4 && my_txbuf[14] <= 6)
            my_address = my_txbuf[14] + 1;
        else if(my_txbuf[14] >= 7 && my_txbuf[14] <= 9)
            my_address = my_txbuf[14] + 2;
        else if(my_txbuf[14] >= 1 && my_txbuf[14] <= 3)
            my_address = my_txbuf[14] + 0;

        my_status = WDZ_101Transmit_Cycle_analog_time(my_address, status_value);		//对433模块发送指令，动作指示器
    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server CycleAnalogTime fail-1\r\n");
    }

    if(my_status == 1)
    {
        USART_printf(&huart3, " Server CycleAnalogTime* %d\r\n", my_address);
        my_at_senddata(my_txbuf);  //利用GPRS返回确认指令
    }
    else if(my_status == 0)
    {
        USART_printf(&huart3, " Server CycleAnalogTime fail-2\r\n");
    }



    return my_status;
}

/*

功能：实时发送周期数据，直接发送过程数据，不从EEPROM中进行读取，发送完数据后修改EEPROM中记录的状态，条件是无历史数据发送
*/
uint8_t WDZ_GPRS_101transmit_Cycle_realtime_data(void)
{
    uint8_t tempbuf[12] = {0};
    uint32_t startaddressPT = MY_Table2_Cycledata_StartAddress;
    uint32_t endaddressPT = MY_Table2_Cycledata_EndAddress;
    uint32_t writeaddressPT = MY_Table2_Cycledata_WriteAddress;
    uint32_t readaddressPT = MY_Table2_Cycledata_ReadAddress;
    uint32_t tableaddress = 0;
    uint8_t rs_temp_buf22[512] = {0}; //测试使用
    uint8_t my_status = 0;
    uint8_t tempstaus = 0;
    uint8_t my_rxbuf[256] = "\x10\x80\x01\x00\x81\x16";
    uint8_t my_txbuf[256] = TX_GPRS_101_testdata;



//***处理获得最新的表指针地址
    SPI_EE_BufferRead2(tempbuf, 213, 12);

    my_buf_writeto_val(tempbuf, &startaddressPT, &endaddressPT, &writeaddressPT, &readaddressPT);
    MY_Table2_Cycledata_StartAddress = startaddressPT;
    MY_Table2_Cycledata_EndAddress = endaddressPT;
    MY_Table2_Cycledata_WriteAddress = writeaddressPT;
    MY_Table2_Cycledata_ReadAddress = readaddressPT;

//--------------
//进行读指针末尾校验，如果到了末尾，就返回起始地址，然后写入表中
    if(readaddressPT + 144 > endaddressPT)
    {   readaddressPT = startaddressPT;
        MY_Table2_Cycledata_ReadAddress = readaddressPT;
        tempbuf[0] = readaddressPT & 0x0000ff;
        tempbuf[1] = (readaddressPT >> 8) & 0x0000ff;
        tempbuf[2] = (readaddressPT >> 16) & 0x0000ff;
        SPI_EE_BufferWrite2(tempbuf, 222, 3);
    }


//----------------------


//----------------
    if(readaddressPT + 144 == writeaddressPT) //此为发送条件，无历史数据，读指针已经移到最后一条记录的首地址
    {

        if(GPRS_Status == 1 && link_status_GPRS == 1)
            my_status = 1;
        else
            my_status = 0;

        wdz_GPRS_string_to_array(TX_GPRS_101_testdata, my_txbuf); //密码指令
        //my_status=WDZ_GPRS_101transmint_commd(2,my_txbuf,2);
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
        if(my_status == 1)
        {
            //my_status=WDZ_101receive_testactive(0x03,0x68,0x07,300);
//***********重要****发送周期性数据****
//直接从433的过程数据存储单元中读取，放到3个特定数组中，然后发送3个101信息包，遥信，遥测、环境，带时标。
//等待Server肯定确认，确认后修改EEPROM中数据包的GPRS发送状态，同时修改读指针地址

            //tempstaus=my_eeprom_data_to_cycle_array(); //把数据存入到GPRS发送数组中*********
            //MY_433_Call_Single_data_number
            //MY_433_Call_Analog_data_number
            //MY_MCU_RsBuf  //8个字节
            //MY_433_Call_Single_data_buf   2*9=18
            //MY_433_Call_Analog_data_buf  3*4*9=108    8+18+108=134  134+7=141加时标， 141+3=144加状态、遥信个数，遥测个数
            //MY_433_Call_Time_data_buf  7个字节

            if(MY_MCU_getdata_status == 1)
                my_buf1_to_buf2(MY_MCU_RsBuf, 0, MY_GPRS_MCU_RsBuf, 0, 8); //环境参数

            if(MY_433_Call_Status == 1) //产生遥信、遥测、时标数据
            {
                MY_GPRS_Call_Single_data_number = MY_433_Call_Single_data_number;
                MY_GPRS_Call_Analog_data_number = MY_433_Call_Analog_data_number;
                my_buf1_to_buf2(MY_433_Call_Single_data_buf, 0, MY_GPRS_Call_Single_data_buf, 0, MY_GPRS_Call_Single_data_number); //遥信数据
                my_buf1_to_buf2(MY_433_Call_Analog_data_buf, 0, MY_GPRS_Call_Analog_data_buf, 0, MY_GPRS_Call_Analog_data_number * 3); //遥测数据
                my_buf1_to_buf2(MY_433_Call_Time_data_buf, 0, MY_GPRS_Call_Time_data_buf, 0, 7); //时标数据
            }

            tempstaus = 1;
            if(tempstaus == 1)
            {
                if(MY_433_Call_Status == 1)
                {
                    //生成发送数据包，发送数据，先发遥信、再发遥测、最后发环境
                    //遥信数据包 68 23 23 68 73 01 00 01 98 14 00 01 00 01 00 00 00 00 00 00 00 00 23 16
                    my_gprs_generate_101single_data(1, my_txbuf);
                    //my_at_senddata(my_txbuf);

                    //新增
                    //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500);
                    //发送遥信数据
                    my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
                    if(my_status == 1)
                    {

                        //遥测数据包
                        //68 53 53 68 53 01 00 09 98 14 00 01 00 01 40 00 00 00 00 00 00 00 00 00 00 4B 16
                        my_gprs_generate_101analog_data(1, my_txbuf);
                        //my_at_senddata(my_txbuf);

                        //新增
                        //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500);
                        //发送遥测数据
                        my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);


                        //测试使用，过程数据@@@@@@@

                        USART_printf(&huart3, "real cycle data to show-433M data\r\n");
                        HexToStr2(rs_temp_buf22, MY_GPRS_Call_Single_data_buf, 18);
                        USART_printf(&huart3, rs_temp_buf22);
                        USART_printf(&huart3, "\r\n");
                        HexToStr2(rs_temp_buf22, MY_GPRS_Call_Analog_data_buf, 108);
                        USART_printf(&huart3, rs_temp_buf22);
                        USART_printf(&huart3, "\r\n");
                        HexToStr2(rs_temp_buf22, MY_GPRS_Call_Time_data_buf, 7);
                        USART_printf(&huart3, rs_temp_buf22);
                        USART_printf(&huart3, "\r\n");

                    }
                    else
                    {
                        my_status = 0;
                        USART_printf(&huart3, "GPRS real Sing data error\r\n"); //调试使用
                    }
                }
                //发送环境数据
                if(my_status == 1 && MY_MCU_getdata_status == 1)
                {
                    //环境数据包
                    //68 53 53 68 53 01 00 09 98 14 00 01 00 00 41 00 00 00 00 00 00 00 00 00 00 4B 16
                    my_gprs_generate_101MCU_data(1, my_txbuf);
                    //my_at_senddata(my_txbuf);
                    //新增
                    //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500);
                    my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

                    //测试使用，过程数据@@@@@@@
                    USART_printf(&huart3, "real cycle data to show-MCU data\r\n");
                    HexToStr2(rs_temp_buf22, MY_GPRS_MCU_RsBuf, 8);
                    USART_printf(&huart3, rs_temp_buf22);
                    USART_printf(&huart3, "\r\n");

                }
                else
                {
                    my_status = 0;
                    USART_printf(&huart3, "GPRS real angale data error\r\n"); //调试使用
                }

                if(my_status == 0)
                {
                    USART_printf(&huart3, "GPRS MCU data error\r\n"); //调试使用
                }

            }
            else
            {
                //生产空数据包，发送数据包
                my_gprs_generate_101single_data(0, my_txbuf);
                my_at_senddata(my_txbuf);
                my_gprs_generate_101analog_data(0, my_txbuf);
                my_at_senddata(my_txbuf);
                my_gprs_generate_101MCU_data(0, my_txbuf);
                my_at_senddata(my_txbuf);

            }

            //接收server发来的肯定确认数据包
            //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500); //旧的方案，不要确认的方案

        }


        else if(my_status == 0)
        {
            USART_printf(&huart3, " Server real Cycle data error3\r\n");

        }



        if(my_status == 1) //发送成功了，最终
        {
            //进行发送帧状态字节修改，和移动EEPROM的表读指针
            if(tempstaus == 1)
                my_GPRS_chang_tablereadpt(2, 1); //成功，修改此帧状态为1，表示发送出去了，移动读指针
            else if(tempstaus == 0)
            {
                my_GPRS_chang_tablereadpt(2, 0);  //失败，修改为2，未发送，不移动指针
                USART_printf(&huart3, " Server real Cycle data errort4-1 \r\n");

            }

            //Delay_us(100);
            USART_printf(&huart3, " Server real Cycle data OK2*\r\n");

            // LED2_TOGGLE;
        }
        else if(my_status == 0)
        {   //Delay_us(100);
            USART_printf(&huart3, " Server real Cycle data errort4 \r\n");
            //LED3_TOGGLE;
        }
        //***处理获得最新的表指针地址
        SPI_EE_BufferRead2(tempbuf, 213, 12);
        my_buf_writeto_val(tempbuf, &startaddressPT, &endaddressPT, &writeaddressPT, &readaddressPT);
        MY_Table2_Cycledata_StartAddress = startaddressPT;
        MY_Table2_Cycledata_EndAddress = endaddressPT;
        MY_Table2_Cycledata_WriteAddress = writeaddressPT;
        MY_Table2_Cycledata_ReadAddress = readaddressPT;

        //进行读指针移动
        if(readaddressPT > writeaddressPT) //进行读指针校正
        {   readaddressPT = writeaddressPT;

            tempbuf[0] = readaddressPT & 0x0000ff;
            tempbuf[1] = (readaddressPT >> 8) & 0x0000ff;
            tempbuf[2] = (readaddressPT >> 16) & 0x0000ff;
            tableaddress = 222;
            SPI_EE_BufferWrite2(tempbuf, tableaddress, 3);

        }


    }

//结束，进行返回状态处理
    MY_MCU_getdata_status = 0; //PW环境总召数据处理完成
    MY_433_Call_Status = 0;  //433模块总召数据处理完成

    return my_status;

}


/*

功能：只发送周期数据，不发送433模块数据
*/
uint8_t WDZ_GPRS_101transmit_Cycle_TH_data(void)
{
    uint8_t rs_temp_buf22[512] = {0}; //测试使用
    uint8_t my_status = 0;
    uint8_t my_rxbuf[256] = "\x10\x80\x01\x00\x81\x16";
    uint8_t my_txbuf[256] = TX_GPRS_101_testdata;
    int ii = 0;

//----------------

    if(GPRS_Status == 1 && MY_MCU_getdata_status == 1)
        my_status = 1;
    else
        my_status = 0;

    if(my_status == 1)
    {
        wdz_GPRS_string_to_array(TX_GPRS_101_testdata, my_txbuf); //密码指令
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
    }
    if(my_status == 1)
    {



        my_buf1_to_buf2(MY_MCU_RsBuf, 0, MY_GPRS_MCU_RsBuf, 0, 8); //环境参数

        MY_GPRS_Call_Single_data_number = 0X12;
        MY_GPRS_Call_Analog_data_number = 0X24;
        for(ii = 0; ii < 40; ii++)
            MY_GPRS_Call_Single_data_buf[ii] = 0XEF; //模拟数据，用0XEF来表示
        for(ii = 0; ii < 110; ii++)
            MY_GPRS_Call_Analog_data_buf[ii] = 0XEF;
        my_RTCtime_to_array(MY_GPRS_Call_Time_data_buf);

        //生成发送数据包，发送数据，先发遥信、再发遥测、最后发环境
        //遥信数据包 68 23 23 68 73 01 00 01 98 14 00 01 00 01 00 00 00 00 00 00 00 00 23 16
        my_gprs_generate_101single_data(1, my_txbuf);
        //发送遥信数据
        my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

    }
    if(my_status == 1)
    {

        //遥测数据包
        //68 53 53 68 53 01 00 09 98 14 00 01 00 01 40 00 00 00 00 00 00 00 00 00 00 4B 16
        my_gprs_generate_101analog_data(1, my_txbuf);
        //发送遥测数据
        my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);


        //测试使用，过程数据@@@@@@@
        HexToStr2(rs_temp_buf22, MY_GPRS_Call_Single_data_buf, 18);
        USART_printf(&huart3, rs_temp_buf22);
        USART_printf(&huart3, "\r\n");
        HexToStr2(rs_temp_buf22, MY_GPRS_Call_Analog_data_buf, 108);
        USART_printf(&huart3, rs_temp_buf22);
        USART_printf(&huart3, "\r\n");
        HexToStr2(rs_temp_buf22, MY_GPRS_Call_Time_data_buf, 7);
        USART_printf(&huart3, rs_temp_buf22);
        USART_printf(&huart3, "\r\n");

    }
    else
    {
        my_status = 0;
        USART_printf(&huart3, "GPRS TH Sing data error\r\n"); //调试使用
    }

    //发送环境数据
    if(my_status == 1)
    {
        //环境数据包
        //68 53 53 68 53 01 00 09 98 14 00 01 00 00 41 00 00 00 00 00 00 00 00 00 00 4B 16
        my_gprs_generate_101MCU_data(1, my_txbuf);
        //my_at_senddata(my_txbuf);
        //新增
        //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500);
        my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

        //测试使用，过程数据@@@@@@@
        USART_printf(&huart3, "real TH cycle data to show-MCU data\r\n");
        HexToStr2(rs_temp_buf22, MY_GPRS_MCU_RsBuf, 8);
        USART_printf(&huart3, rs_temp_buf22);
        USART_printf(&huart3, "\r\n");

    }
    else
    {
        my_status = 0;
        USART_printf(&huart3, "GPRS real TH angale data error\r\n"); //调试使用
    }

    if(my_status == 0)
    {
        USART_printf(&huart3, "GPRS MCU TH data error\r\n"); //调试使用
    }

    if(my_status == 1) //发送成功了，最终
    {

        USART_printf(&huart3, " Server real TH Cycle data OK2*\r\n");

        // LED2_TOGGLE;
    }

//结束，进行返回状态处理
    MY_MCU_getdata_status = 0; //PW环境总召数据处理完成
    MY_433_Call_Status = 0;  //433模块总召数据处理完成

    return my_status;

}

/*
把收到的字符转换成ASCII码进行显示
*/

void my_display_ASCIIdata(uint8_t *rsbuf)
{
//	int tt=0;
    int length = 0;
//	uint8_t *mypt=rsbuf;
//	uint8_t my_temp1,my_temp2;
    uint8_t desbuf[512] = {0};

    if(*rsbuf == 0x10) length = 6;
    else if(*rsbuf == 0x68) length = *(rsbuf + 1) + 6;


    HexToStr2(desbuf, rsbuf, length);
    USART_printf(&huart3, desbuf);

    /*
    for(tt=0;tt<length;tt++)
    {
    	my_temp1=*mypt/16;
    	my_temp2=*mypt%16;
    	USART_printf(&huart3,"%d%d-",my_temp1,my_temp2);
    mypt++;
    }
    */
    USART_printf(&huart3, "\r\n");

}

/*

功能：发送433模块被动收到的遥测数据，利用GPRS网络发送，进行透明传输，数据不进行存储
*/
uint8_t WDZ_GPRS_101transmit_analog_data(void)
{
    uint8_t rs_temp_buf22[512] = {0}; //测试使用
    uint8_t my_status = 0;
    uint8_t my_rxbuf[256] = "\x10\x80\x01\x00\x81\x16";
    uint8_t my_txbuf[256] = TX_GPRS_101_testdata;
    int ii = 0;

//----------------

    if(GPRS_Status == 1 && MY_MCU_getdata_status == 1)
        my_status = 1;
    else
        my_status = 0;

    if(my_status == 1)
    {
        wdz_GPRS_string_to_array(TX_GPRS_101_testdata, my_txbuf); //密码指令
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
    }
    if(my_status == 1)
    {



        my_buf1_to_buf2(MY_MCU_RsBuf, 0, MY_GPRS_MCU_RsBuf, 0, 8); //环境参数

        MY_GPRS_Call_Single_data_number = 0X12;
        MY_GPRS_Call_Analog_data_number = 0X24;
        for(ii = 0; ii < 40; ii++)
            MY_GPRS_Call_Single_data_buf[ii] = 0XEF; //模拟数据，用0XEF来表示
        for(ii = 0; ii < 110; ii++)
            MY_GPRS_Call_Analog_data_buf[ii] = my_433_anlag_buf[ii];	 //获取433被动接收的模拟量
        my_RTCtime_to_array(MY_GPRS_Call_Time_data_buf);

        //生成发送数据包，发送数据，先发遥信、再发遥测、最后发环境
        //遥信数据包 68 23 23 68 73 01 00 01 98 14 00 01 00 01 00 00 00 00 00 00 00 00 23 16
        my_gprs_generate_101single_data(1, my_txbuf);
        //发送遥信数据
        my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

    }
    if(my_status == 1)
    {

        //遥测数据包
        //68 53 53 68 53 01 00 09 98 14 00 01 00 01 40 00 00 00 00 00 00 00 00 00 00 4B 16
        my_gprs_generate_101analog_data(1, my_txbuf);
        //发送遥测数据
        my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);


        //测试使用，过程数据@@@@@@@
        HexToStr2(rs_temp_buf22, MY_GPRS_Call_Single_data_buf, 18);
        USART_printf(&huart3, rs_temp_buf22);
        USART_printf(&huart3, "\r\n");
        HexToStr2(rs_temp_buf22, MY_GPRS_Call_Analog_data_buf, 108);
        USART_printf(&huart3, rs_temp_buf22);
        USART_printf(&huart3, "\r\n");
        HexToStr2(rs_temp_buf22, MY_GPRS_Call_Time_data_buf, 7);
        USART_printf(&huart3, rs_temp_buf22);
        USART_printf(&huart3, "\r\n");

    }
    else
    {
        my_status = 0;
        USART_printf(&huart3, "GPRS ANALOG Sing data error\r\n"); //调试使用
    }

    //发送环境数据
    if(my_status == 1)
    {
        //环境数据包
        //68 53 53 68 53 01 00 09 98 14 00 01 00 00 41 00 00 00 00 00 00 00 00 00 00 4B 16
        my_gprs_generate_101MCU_data(1, my_txbuf);
        //my_at_senddata(my_txbuf);
        //新增
        //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500);
        my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

        //测试使用，过程数据@@@@@@@
        USART_printf(&huart3, "real ANALOG cycle data to show-MCU data\r\n");
        HexToStr2(rs_temp_buf22, MY_GPRS_MCU_RsBuf, 8);
        USART_printf(&huart3, rs_temp_buf22);
        USART_printf(&huart3, "\r\n");

    }
    else
    {
        my_status = 0;
        USART_printf(&huart3, "GPRS real ANALOG angale data error\r\n"); //调试使用
    }

    if(my_status == 0)
    {
        USART_printf(&huart3, "GPRS MCU ANALOG data error\r\n"); //调试使用
    }

    if(my_status == 1) //发送成功了，最终
    {

        USART_printf(&huart3, " Server real ANALOG Cycle data OK2*\r\n");

        // LED2_TOGGLE;
    }

//结束，进行返回状态处理
    MY_MCU_getdata_status = 0; //PW环境总召数据处理完成
    MY_433_Call_Status = 0;  //433模块总召数据处理完成

    return my_status;

}

/*
功能：发送AT+CSQ，发送信号质量

*/

extern uint8_t MY_AT_CSQ_Value;
int my_GPRS_AT_CSQ()
{

    uint8_t rs_temp_buf22[512] = {0}; //测试使用
    uint8_t my_status = 0;
    uint8_t my_rxbuf[256] = "\x10\x80\x01\x00\x81\x16";
    uint8_t my_txbuf[256] = TX_GPRS_101_testdata;


//----------------

    if(GPRS_Status == 1)
        my_status = 1;
    else
        my_status = 0;

    if(my_status == 1)
    {
        wdz_GPRS_string_to_array(TX_GPRS_101_testdata, my_txbuf); //密码指令
        my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);
    }



    //发送环境数据
    if(my_status == 1)
    {
        //环境数据包
        //68 53 53 68 53 01 00 09 98 14 00 01 00 00 41 00 00 00 00 00 00 00 00 00 00 4B 16
        my_gprs_generate_101CSQ_data(1, my_txbuf);
        //my_at_senddata(my_txbuf);
        //新增
        //my_status=WDZ_GPRS_101receive_testactive(0x80,00,00,1500);
        my_status = my_status = WDZ_GPRS_101transmint_commd_wait_commd(2, my_txbuf, 1, my_rxbuf);

        //测试使用，过程数据@@@@@@@
        USART_printf(&huart3, "GRPS CSQ:");
        HexToStr2(rs_temp_buf22, my_txbuf, 18);
        USART_printf(&huart3, rs_temp_buf22);
        USART_printf(&huart3, "\r\n");

    }
    else
    {
        my_status = 0;
        USART_printf(&huart3, "GPRS GRPS CSQ error1\r\n"); //调试使用
    }

    if(my_status == 0)
    {
        USART_printf(&huart3, "GPRS GRPS CSQ error2\r\n"); //调试使用
    }

    if(my_status == 1) //发送成功了，最终
    {

        USART_printf(&huart3, " GRPS CSQ OK1*\r\n");

        // LED2_TOGGLE;
    }

//结束，进行返回状态处理

    return my_status;

}

/*
功能：产生GPRS信号质量
*/
void my_gprs_generate_101CSQ_data(uint8_t temp, uint8_t *my_rsbuf)
{
    uint8_t length = 0;

    if(temp == 1) //生成数据包
    {
        length = 1;

        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length * 2 + 10;
        my_rsbuf[2] = length * 2 + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);


        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X09; //类型标识，带时标的单点信息，
        my_rsbuf[8] = length + 0x80; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;;

        my_rsbuf[12] = 0x00; //遥信信息体首地址
        my_rsbuf[13] = 0x42;

        //my_buf1_to_buf2(MY_GPRS_MCU_RsBuf,0,my_rsbuf,14,length*2);

        my_rsbuf[14] = MY_AT_CSQ_Value;
        my_rsbuf[15] = 0X00;

        //
        my_rsbuf[13 + length * 2 + 1] = 0XFF;
        my_rsbuf[13 + length * 2 + 1 + 1] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节



    }
    else if(temp == 0) //生成0数据体数据包
    {
        length = 2;
        my_rsbuf[0] = 0x68;
        my_rsbuf[3] = 0x68;
        my_rsbuf[1] = length + 10;
        my_rsbuf[2] = length + 10;

        my_rsbuf[4] = 0x73; //控制域码为53/73
        my_GPRS_101_geneate_FCBword(my_rsbuf);

        my_rsbuf[5] = DTU_ADDRESS & 0X00FF;
        my_rsbuf[6] = (DTU_ADDRESS >> 8) & 0X00FF;

        my_rsbuf[7] = 0X09; //类型标识，带时标的单点信息，
        my_rsbuf[8] = 0x84; //信息体个数
        my_rsbuf[9] = 0x14; //传输原因

        my_rsbuf[10] = DTU_ADDRESS & 0X00FF; //公共域地址
        my_rsbuf[11] = (DTU_ADDRESS >> 8) & 0X00FF;;

        my_rsbuf[12] = 0x00; //遥信信息体首地址
        my_rsbuf[13] = 0x42;

        my_rsbuf[14] = 0x00;
        my_rsbuf[15] = 0x00;

        my_rsbuf[16] = 0XFF;
        my_rsbuf[17] = 0x16;

        wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节
    }


}

/*
功能：遥测补充
*/
void my_gprs_generate_101yaoce2_data(uint8_t *my_rsbuf)
{
    uint8_t length = 12; //信息体个数
    uint8_t jj = 0;
    //帧头
    my_rsbuf[0] = 0x68;
    my_rsbuf[3] = 0x68;
    my_rsbuf[1] = length * 2 + 10;
    my_rsbuf[2] = length * 2 + 10;
    //控制域部分
    my_rsbuf[4] = 0x73; //控制域码为53/73
    //my_GPRS_101_geneate_FCBword(my_rsbuf);
    my_rsbuf[5] = DTU_ADDRESS;
    my_rsbuf[6] = (DTU_ADDRESS >> 8);
    my_rsbuf[7] = 0X09; //类型标识，带时标的单点信息，
    my_rsbuf[8] = length + 0x80; //信息体个数
    my_rsbuf[9] = 0x67; //传输原因
    my_rsbuf[10] = DTU_ADDRESS; //公共域地址
    my_rsbuf[11] = (DTU_ADDRESS >> 8);;
    my_rsbuf[12] = 0x01; //遥信信息体首地址
    my_rsbuf[13] = 0x42;
    //帧尾
    my_rsbuf[13 + length * 2 + 1] = 0XFF;
    my_rsbuf[13 + length * 2 + 1 + 1] = 0x16;
    //数据部分
    for(jj = 0; jj < 3; jj++)
    {   //1温度，2电源，3参考电压，4干电池，5线上电压，6太阳能，7锂电池
        my_rsbuf[14 + 8 * jj] = my_indicator_data[jj].DC_data_buf[5 * 2]; //6太阳能
        my_rsbuf[15 + 8 * jj] = my_indicator_data[jj].DC_data_buf[5 * 2 + 1];

        my_rsbuf[16 + 8 * jj] = my_indicator_data[jj].DC_data_buf[4 * 2]; //5线上电压
        my_rsbuf[17 + 8 * jj] = my_indicator_data[jj].DC_data_buf[4 * 2 + 1];


        my_rsbuf[18 + 8 * jj] = my_indicator_data[jj].DC_data_buf[3 * 2]; //4干电池
        my_rsbuf[19 + 8 * jj] = my_indicator_data[jj].DC_data_buf[3 * 2 + 1];


        my_rsbuf[20 + 8 * jj] = my_indicator_data[jj].AC_data_buf[4]; //半波电流值
        my_rsbuf[21 + 8 * jj] = my_indicator_data[jj].AC_data_buf[5];

    }

    wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节

}

/*
功能：遥测12T
*/
void my_gprs_generate_101yaoce12T_data(uint8_t *my_rsbuf)
{
    uint8_t length = 3; //信息体个数
    uint8_t jj = 0, ii = 0;
    //帧头
    my_rsbuf[0] = 0x68;
    my_rsbuf[3] = 0x68;
    my_rsbuf[1] = length * 48 + 10;
    my_rsbuf[2] = length * 48 + 10;
    //控制域部分
    my_rsbuf[4] = 0x73; //控制域码为53/73
    //my_GPRS_101_geneate_FCBword(my_rsbuf);
    my_rsbuf[5] = DTU_ADDRESS;
    my_rsbuf[6] = (DTU_ADDRESS >> 8);
    my_rsbuf[7] = 0X09; //类型标识，带时标的单点信息，
    my_rsbuf[8] = length + 0x80; //信息体个数
    my_rsbuf[9] = 0x68; //传输原因
    my_rsbuf[10] = DTU_ADDRESS; //公共域地址
    my_rsbuf[11] = (DTU_ADDRESS >> 8);;
    my_rsbuf[12] = 0x01; //遥信信息体首地址
    my_rsbuf[13] = 0x43;
    //帧尾
    my_rsbuf[13 + length * 2 + 1] = 0XFF;
    my_rsbuf[13 + length * 2 + 1 + 1] = 0x16;
    //数据部分
    for(jj = 0; jj < 3; jj++)
    {   //12T 电流+电场
        for(ii = 0; ii < 24; ii++)
        {
            my_rsbuf[14 + 48 * jj + ii] = my_indicator_data[jj].AC12T_ALL_Current_data_buf[ii ]; //电流
            
        }
				for(ii = 0; ii < 24; ii++)
        {
            my_rsbuf[14 + 48 * jj + ii +24] = my_indicator_data[jj].AC12T_ALL_dianchang_data_buf[ii]; //电场

        }

    }

    wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节

}
/*
功能：计数同步值
*/
extern uint16_t my_gprs_count_time; //GPRS通信，周期数据，传递给SERVER的DTU收到的zsq的计数值
extern uint8_t  my_gprs_RTC_buf[];

void my_gprs_generate_101yaoce1_COUNTSYN_data(uint8_t *my_rsbuf)
{
    uint8_t length = 1; //信息体个数

    //帧头
    my_rsbuf[0] = 0x68;
    my_rsbuf[3] = 0x68;
    my_rsbuf[1] = length * 9 + 10;
    my_rsbuf[2] = length * 9 + 10;
    //控制域部分
    my_rsbuf[4] = 0x73; //控制域码为53/73
    //my_GPRS_101_geneate_FCBword(my_rsbuf);
    my_rsbuf[5] = DTU_ADDRESS;
    my_rsbuf[6] = (DTU_ADDRESS >> 8);
    my_rsbuf[7] = 0XDC; //类型标识，带时标的单点信息，
    my_rsbuf[8] = length + 0x80; //信息体个数
    my_rsbuf[9] = 0x69; //传输原因
    my_rsbuf[10] = DTU_ADDRESS; //公共域地址
    my_rsbuf[11] = (DTU_ADDRESS >> 8);;
    my_rsbuf[12] = 0x01; //遥信信息体首地址
    my_rsbuf[13] = 0x4F;
    //帧尾
    my_rsbuf[13 + length * 9 + 1] = 0XFF;
    my_rsbuf[13 + length * 9 + 1 + 1] = 0x16;
    //数据部分


    my_rsbuf[14] = my_gprs_count_time;
    my_rsbuf[15] = (my_gprs_count_time>>8);

    my_rsbuf[16] = my_gprs_RTC_buf[0];
    my_rsbuf[17] = my_gprs_RTC_buf[1];
    my_rsbuf[18] = my_gprs_RTC_buf[2];
    my_rsbuf[19] = my_gprs_RTC_buf[3];
    my_rsbuf[20] = my_gprs_RTC_buf[4];
    my_rsbuf[21] = my_gprs_RTC_buf[5];
    my_rsbuf[22] = my_gprs_RTC_buf[6];




    wdz_GPRS_101check_generate(my_rsbuf); //生成校验字节

}


/*
功能：产生报警发送使用的12个字符
*/
extern uint8_t my_indicator_tx_index;

extern struct indicator_alarm_class my_indicator_alarm_data[];
void my_fun_gprs_generate_12T_data(uint8_t *txbuf)
{
	uint8_t ii=0,jj=0;
	
	wdz_GPRS_string_to_array(TX_GPRS_101_ALarm_single_12T_data, my_usart1_tx_buf1);
	for(ii=0;ii<3;ii++) //报警相的12T数据
	{
		if(ii==my_indicator_tx_index)
		{
			for(jj=0;jj<24;jj++)
			{
				my_usart1_tx_buf1[14+ii*48+jj]=my_indicator_alarm_data[ii].AC12T_ALL_Current_data_buf[jj];		//电流	
			}
				for(jj=0;jj<24;jj++)
			{
				my_usart1_tx_buf1[14+ii*48+jj+24]=my_indicator_alarm_data[ii].AC12T_ALL_dianchang_data_buf[jj];	//电场
			}
		}	
		else //其它相利用周期数据填充
		{
			for(jj=0;jj<24;jj++)
			{
				my_usart1_tx_buf1[14+ii*48+jj]=my_indicator_data[ii].AC12T_ALL_Current_data_buf[jj];		//电流	
			}
				for(jj=0;jj<24;jj++)
			{
				my_usart1_tx_buf1[14+ii*48+jj+24]=my_indicator_data[ii].AC12T_ALL_dianchang_data_buf[jj];			//电场
			}
			
			
			
		}

		
	}
	
	
	
	
	
	
	
}
