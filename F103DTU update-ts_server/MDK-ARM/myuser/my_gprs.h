#include "stm32f1xx_hal.h"

#include "bsp_SysTick.h"
#include "bsp_usart1.h"
#include "bsp_spi_flash.h"
#include "wdz_eeprom.h"
#include "wdz_101.h"
#include "wdz_m35.h"
#include "wdz_rtc_time.h"
#include "wdz_MCU.h"



//------------GPRS 101模块通信指令
#define TX_GPRS_101_Linkquire_data "\x10\x49\x01\x00\x4A\x16";
#define TX_GPRS_101_Linkrest_data "\x10\x40\x01\x00\x41\x16"
#define TX_GPRS_101_Linkconfirm_data "\x10\x0B\x01\x00\x0C\x16"
#define TX_GPRS_101_OKdata "\x10\x00\x01\x00\x01\x16"

#define TX_GPRS_101_heartdata "\x10\xD2\x01\x00\xD3\x16";
#define TX_GPRS_101_testdata "\x68\x0C\x0C\x68\x73\x01\x00\x68\x01\x06\x00\x01\x00\x00\xAA\x55\xE3\x16"

#define TX_GPRS_101_restdata "\x68\x0B\x0B\x68\x73\x01\x00\x69\x01\x06\x01\x00\x00\x00\x01\xE6\x16"
#define TX_GPRS_101_calldata "\x68\x0B\x0B\x68\x73\x01\x00\x64\x01\x06\x01\x00\x00\x00\x14\xF4\x16"


#define TX_GPRS_101_delaytime_data "\x68\x0C\x0C\x68\x53\x01\x00\x6A\x01\x07\x01\x00\x00\x00\x3D\xC3\xC6\x16"
#define TX_GPRS_101_delaytime_burst_data "\x68\x0C\x0C\x68\x73\x01\x00\x67\x01\x07\x01\x00\x00\x00\xC4\x01\xA8\x16"
#define TX_GPRS_101_time_synchronization_data "\x68\x11\x11\x68\x73\x01\x00\x67\x01\x07\x01\x00\x00\x00\xB2\x9D\x0E\x0F\x05\x07\x0E\x6A\x16"
#define TX_GPRS_101_Calldata_over_data "\x68\x12\x12\x68\x73\x01\x00\x09\x84\x14\x01\x00\x00\x41\x68\x05\xA1\x00\x00\x6C\x00\x2D\xFE\x16"
#define TX_GPRS_101_ResetActive_data "\x68\x0B\x0B\x68\x53\x01\x00\x69\x01\x07\x01\x00\x00\x00\x01\xC7\x16"
#define TX_GPRS_101_Callhistory_over_data "\x68\x0B\x0B\x68\x53\x01\x00\x64\x01\x0A\x01\x00\x00\x00\x14\xFF\x16"


#define RX_GPRS_101_OKdata "\x10\x80\x01x00x81x16"
#define RX_GPRS_101_Linkrest_data "\x10\xC0\x01\x00\xC1\x16"
#define RX_GPRS_101_Linkover_data "\x68\x0B\x0B\x68\xF3\x01\x00\x46\x01\x04\x01\x00\x00\x00\x00\x40\x16"


#define TX_GPRS_101_count_syn_data "\x68\x0C\x0C\x68\x80\x01\x00\xDC\x01\x66\x01\x00\x01\x4F\x00\x00\xE3\x16"
#define TX_GPRS_101_RTC_data "\x68\x11\x11\x68\x73\x01\x00\x67\x01\x07\x01\x00\x00\x00\x01\x02\x03\x04\x05\x06\x07\xFF\x16"
#define TX_GPRS_101_CYC7_data  "\x68\x12\x12\x68\x73\x01\x00\x09\x84\x14\x01\x00\x00\x41\x68\x05\xA1\x00\x00\x6C\x00\x2D\xFE\x16"
#define TX_GPRS_101_RESET_ACK_data "\x68\x0B\x0B\x68\x53\x01\x00\x69\x01\x07\x01\x00\x00\x00\x01\xC7\x16"
#define TX_GPRS_101_changeparameter_ACK_2byte_data "\x68\x0C\x0C\x68\x00\x01\x00\x30\x01\x07\x01\x00\x01\x50\x03\x00\x02\x16"
#define TX_GPRS_101_changeparameter_ACK_6byte_data "\x68\x10\x10\x68\x00\x01\x00\x30\x01\x07\x01\x00\x04\x50\x11\x12\x13\x14\x15\x16\x79\x16"
#define TX_GPRS_101_changeparameter_ACK_4byte_data "\x68\x0E\x0E\x68\x00\x01\x00\x30\x01\x07\x01\x00\x05\x50\x11\x12\x13\x14\x79\x16"
#define TX_GPRS_101_turn_lend_data "\x68\x0E\x0E\x68\x00\x02\x00\x2D\x02\x07\x02\x00\x01\x60\x01\x04\x60\x00\x34\x16"
#define TX_GPRS_101_Chaxun_data "\x68\x0C\x0C\x68\x00\x03\x00\x30\x01\x07\x03\x00\x01\x50\x03\x00\x0E\x16"
#define TX_GPRS_101_Chaxun_6bye_data "\x68\x10\x10\x68\x00\x03\x00\x30\x01\x07\x03\x00\x01\x50\x01\x02\x03\x04\x05\x06\x0E\x16"
#define TX_GPRS_101_Chaxun_7bye_data "\x68\x11\x11\x68\x00\x03\x00\x30\x01\x07\x03\x00\x01\x50\x01\x02\x03\x04\x05\x06\x07\x0E\x16"

#define TX_GPRS_101_xinhaoqiangdu_data "\x68\x10\x10\x68\x73\x01\x00\x09\x02\x6f\x01\x00\x60\x50\x01\x02\x61\x50\x03\x04\x0E\x16"
#define TX_GPRS_101_count_time_data "\x68\x13\x13\x68\x73\x01\x00\x09\x01\x71\x01\x00\x70\x50\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0E\x16"
#define TX_GPRS_101_ALarm_single_notime_data "\x68\x0B\x0B\x68\x73\x01\x00\x01\x01\x03\x01\x00\x08\x00\x01\x83\x16"
#define TX_GPRS_101_ALarm_single_with_time_data "\x68\x12\x12\x68\x73\x01\x00\x1E\x01\x03\x01\x00\x08\x00\xFF\x01\x02\x03\x04\x05\x06\x07\x83\x16"
#define TX_GPRS_101_ALarm_single_AC_data "\x68\x1C\x1C\x68\x73\x01\x00\x09\x83\x6A\x01\x00\x01\x44\x01\x02\x03\x04\x05\x06\x01\x02\x03\x04\x05\x06\x01\x02\x03\x04\x05\x06\xFF\x16"
#define TX_GPRS_101_ALarm_single_12T_data "\x68\x9A\x9A\x68\x73\x01\x00\x09\x83\x6B\x01\x00\x01\x43\x01\x02\x03\x04\x05\x06\x01\x02\x03\x04\x05\x06\x01\x02\x03\x04\x05\x06\xFF\x16"
#define TX_GPRS_101_ALarm_single_countRTC_dadta "\x68\x13\x13\x68\x73\x01\x00\xDC\x01\x6C\x01\x00\x01\x4F\x01\x02\x03\x04\x05\x06\x07\x08\x09\xFF\x16"
#define TX_GPRS_101_Record_data "\x68\x13\x13\x68\x73\x01\x00\x64\x01\x72\x01\x00\x01\x45\x01\x02\x03\x04\x05\x06\x07\x08\x09\xFF\x16"

void wdz_GPRS_string_to_array(uint8_t *my_string,uint8_t *txbuf);  //把字符串写入数组中，即把默认指令写入数组中
uint8_t wdz_GPRS_101char_check(uint8_t *buffer);
void wdz_GPRS_101check_generate(uint8_t *buffer);

uint8_t my_usart_GPRS_101frame(uint8_t usart_port);
uint8_t WDZ_GPRS_101receive_testactive(uint8_t Link_control,uint8_t type_identification,uint8_t transmit_reason,uint16_t time);
uint8_t WDZ_GPRS_101transmint_commd_wait_commd(uint8_t type,uint8_t *txbuf,uint8_t address_type,uint8_t *rxbuf);
uint8_t WDZ_GPRS_101transmint_commd_wait_commd2(uint8_t type,uint8_t *txbuf,uint8_t address_type,uint8_t *rxbuf,uint16_t my_temp_dtu_address);  //type为类型，1为固定长度，2为非固定长度，txbuf为发送指令数组


void my_gprs_generate_101single_data(uint8_t temp,uint8_t *my_rsbuf); //生成 遥信发送数据包
void my_gprs_generate_101analog_data(uint8_t temp,uint8_t *my_rsbuf); //生成 遥测发送数据包
void my_gprs_generate_101MCU_data(uint8_t temp,uint8_t *my_rsbuf);    //生成 环境数据发送数据包
void my_gprs_generate_101Alarm_Notime_data(uint8_t temp,uint8_t *my_rsbuf);//生成，无时标的报警数据包
void my_gprs_generate_101Alarm_Time_data(uint8_t temp,uint8_t *my_rsbuf);// 生成，有时标的报警数据包

//历史数据查询使用
//生成历史遥信数据1帧数据，temp为1表示正常，为0表示生成0数据包，my_buf为从EEPROM中读取到一帧周期数据，含遥信和遥测，环境参数，my_rsbuf为最终生成的指令响应帧
void my_gprs_generate_101single_history_data(uint8_t temp,uint8_t *my_buf,uint8_t *my_rsbuf);
void my_gprs_generate_101analog_history_data(uint8_t temp,uint8_t *my_buf,uint8_t *my_rsbuf);
void my_gprs_generate_101MCU_history_data(uint8_t temp,uint8_t *my_buf, uint8_t *my_rsbuf);
void my_gprs_generate_101Alarm_Time_history_data(uint8_t temp,uint8_t *my_buf,uint8_t *my_rsbuf);




void WDZ_GPRS_101Transmit_OKdata(void); //发送肯定确认

uint8_t WDZ_GPRS_101Transmit_Link_data(void);//发送建立链路数据
//uint8_t WDZ_GPRS_101transmit_heartdata(void);  //发送心跳数据
//uint8_t WDZ_GPRS_101transmit_Cycle_data(void);  //发送周期数据
uint8_t WDZ_GPRS_101Transmit_Alarm_Data(void);//主动发送报警数据

uint8_t WDZ_GPRS_101transmit_Cycle_realtime_data(void);//主动发送周期实时数据，如果没有历史数据就发送
uint8_t WDZ_GPRS_101transmit_Cycle_TH_data(void);

void my_re_transmit_alarmdata(void); //补发报警历史数据

void WDZ_GPRS_101Resiver_Analyse(void); //GPRS接收到的101指令进行分析，下面的程序都是他的子程序

uint8_t WDZ_GPRS_101Resiver_AdjustTime_data(void);  //接收校时数据包
uint8_t WDZ_GPRS_101Resiver_Call_Data(void); //接收总召数据包
uint8_t WDZ_GPRS_101Resiver_ResetMCU_Data(void);//接收复位进程命令，重启MCU
uint8_t WDZ_GPRS_101Resiver_Adjustvalue_data(void);//接收数据，设置参数
uint8_t WDZ_GPRS_101Resiver_CallHistory_data(void); //总召历史数据，把以前没有发送出去的数据补发出去
uint8_t WDZ_GPRS_101Resiver_AdjustTimeSimple_data(void);//接收时钟同步指令，简化版
uint8_t WDZ_GPRS_101Resiver_ControlMonitor_Data(void);  //翻牌指令
uint8_t WDZ_GPRS_101Resiver_Cycle_analog_time_Data(void); //设置指示器遥测数据周期时间

void my_GPRS_101_geneate_FCBword(uint8_t *my_rsbuf); //产生控制域码
void WDZ_GPRS_Adjust_value(uint8_t *rsbuf); //完成设置参数的各项功能,子函数利用GPRS收到的参数设置命令，设置对应参数

uint8_t WDZ_GPRS_Phonenumber_check(uint8_t *phone_source_buf1,uint8_t *phone_getnew_buf2);//进行电话号码在电话薄中检查
void my_save_PTTO_EEROM(uint32_t mypt,uint32_t tableaddress);  //存32位地址的低3个字节到eerpom中
uint8_t WDZ_GPRS_101transmit_analog_data(void);  //发送433模块收到的遥测数据


//-----测试用函数-----
void my_test_calldata_to_GPRS_array(void); //测试用，把总召数据，写入到GRPS发送数组中，然后生成发送数据帧，遥信，遥测、环境数据帧，发送到UART5

void my_reset_mcu(void); //重启MCU通过软命令

void my_display_ASCIIdata(uint8_t *rsbuf);

#define TX_History_cyc_data_record 5  //历史数据的发送个数!!!!


int my_GPRS_AT_CSQ(void); //发送信号质量
void my_gprs_generate_101CSQ_data(uint8_t temp,uint8_t *my_rsbuf);
void my_gprs_generate_101yaoce2_data(uint8_t *my_rsbuf);
void my_gprs_generate_101yaoce12T_data(uint8_t *my_rsbuf);
void my_gprs_generate_101yaoce1_COUNTSYN_data(uint8_t *my_rsbuf);
void my_fun_gprs_generate_12T_data(uint8_t *txbuf);

