/*
 * Copyright (c) 2016 Intel Corporation + Padurariu Dumitru practica tuiasi 2024 
 *	Lucrare termostat 
 * SPDX-License-Identifier: Apache-2.0
 */
#include <math.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include "SSD1306OLED.h"
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/conn.h>
#include <bluetooth/services/lbs.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>
#include <dk_buttons_and_leds.h>
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000
#define SW0_NODE	DT_ALIAS(sw0) 
#define SW1_NODE	DT_ALIAS(sw1)
#define SW2_NODE	DT_ALIAS(sw2)
#define SW3_NODE	DT_ALIAS(sw3)
/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)
#define I2C0_NODE DT_NODELABEL(mysensor1)
#define GPIO0_LABEL DT_NODELABEL(gpio1)
#define PIN 5
#define I2C0_NODE1 DT_NODELABEL(mydisplay1)
#define STACKSIZE 1024
#define THREAD0_PRIORITY 7
LOG_MODULE_REGISTER(Lesson2_Exercise1, LOG_LEVEL_INF);
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
#define ds1624_CONFIG_REG               0xAC
#define COMPANY_ID_CODE            0x0059
#define USER_BUTTON DK_BTN4_MSK
//#define BT_DATA_NAME_COMPLETE1 0x0A
int  aT=0;
struct bt_conn *my_conn = NULL;
void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection error %d", err);
        return;
    }
    LOG_INF("Connected");
    my_conn = bt_conn_ref(conn);

    /* STEP 3.2  Turn the connection status LED on */
}


void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected. Reason %d", reason);
    bt_conn_unref(my_conn);

    /* STEP 3.3  Turn the connection status LED off */
}
struct bt_conn_cb connection_callbacks = {
    .connected              = on_connected,
    .disconnected           = on_disconnected,
};


typedef struct adv_mfg_data {
	uint16_t company_code; /* Company Identifier Code. */
	uint16_t data1; /* Number of times Button 1 is pressed */
} adv_mfg_data_type,adv_mfg_data1;
#define COMPANY_ID_CODE 0x0059










static adv_mfg_data_type adv_mfg_data = { COMPANY_ID_CODE,0x00 };
static const struct bt_data ad[] = {
	/* STEP 4.1.2 - Set the advertising flags */
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, (unsigned char *)&adv_mfg_data, sizeof(adv_mfg_data)),//temperatura 
	//BT_DATA(BT_DATA_NAME_COMPLETE1, (unsigned char *)&adv_mfg_data1, sizeof(adv_mfg_data1))
};
		//BT_DATA(BT_DATAMANUFACTURER_DATA, (unsigned char *)&aT, sizeof(aT)),


static unsigned char url_data[] = { 0x17, '/', '/', 'a', 'c', 'a', 'd', 'e', 'm',
				    'y',  '.', 'n', 'o', 'r', 'd', 'i', 'c', 's',
				    'e',  'm', 'i', '.', 'c', 'o', 'm' };
static const struct bt_data sd[] = {
	/* 4.2.3 Include the URL data in the scan response packet */
	BT_DATA_BYTES(BT_DATA_UUID128_ALL,
		      BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)),
};
static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(BT_LE_ADV_OPT_CONNECTABLE |
	 BT_LE_ADV_OPT_USE_IDENTITY), /* Connectable advertising and use identity address */
	BT_GAP_ADV_FAST_INT_MIN_1, /* 0x30 units, 48 units, 30ms */
	BT_GAP_ADV_FAST_INT_MAX_1, /* 0x60 units, 96 units, 60ms */
	NULL); /* Set to NULL for undirected advertising */



static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);
static const struct i2c_dt_spec dev_i2c1 = I2C_DT_SPEC_GET(I2C0_NODE1);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(SW1_NODE, gpios);
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET(SW2_NODE, gpios);
static const struct gpio_dt_spec button4 = GPIO_DT_SPEC_GET(SW3_NODE, gpios);
static struct gpio_callback button_cb_data;
static struct gpio_callback button_cb_data1;
static struct gpio_callback button_cb_data2;
static struct gpio_callback button_cb_data3;
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);


bool leds[4]={true,false,false,false};
double temp1;
double t_inf=20;//temperatura inferioara
double t_sup=45;//temperatura superioara
double temp_C,temp_f;//temperatura prezenta
bool setare[5]={true,false,false,false,false};//selectare mod setare tip temperatura ,interval masurare,rezolutie ,conf prag inf ,
// conf prag sup ;
bool tip_temp[2]={true,false};//tip temperatura
int int_mas=9;//interval masura setat pe 1 secunda;
int rezolutie_t=1;//cat sa afiseze 2.0,2.5,3.0
char aC[15];
char  aF[15];
int tempon=1;
void masurare_temperatura(){
	while (1){
		uint8_t temp_reading[2]= {0};
		uint8_t sensor_regs[1] ={0xAA};
		int ret = i2c_write_read_dt(&dev_i2c,&sensor_regs[0],1,temp_reading,2);
		if(ret != 0){
			printk("Failed to write/read I2C device address %x at Reg. %x \r\n", dev_i2c.addr,sensor_regs[0]);
		}
		
		printk("%d",temp_reading[0]);
		
		double  temp = temp_reading[0]  ;
		if(temp > 12711)
		{
			temp -= 4096;
			
		}
		
			temp=temp+(double )(temp_reading[1]>>4)*0.0625;
		
		// Convert to engineering units 
		temp_C  = temp ;
		temp_f= temp_C  * 1.8 + 32;
		printk("temp tc=%lf, tf=%lf \r\n", temp_C,temp_f);
		int cc=(int)temp_C;
		int ff=(int)temp_f;
		temp_C=(double)(cc)+(double)((int)((temp_C-cc)*pow(10,rezolutie_t)))/pow(10,rezolutie_t);
		temp_f=(double)(ff)+(double)((int)((temp_f-ff)*pow(10,rezolutie_t)))/pow(10,rezolutie_t);
		printk("temp tc=%lf, tf=%lf \r\n", temp_C,temp_f);
		const struct device *dev;
		dev = DEVICE_DT_GET(GPIO0_LABEL);
		if (temp_C>t_sup)
			gpio_pin_set(dev,PIN,1);
		else gpio_pin_set(dev, PIN, 0);
		tempon=0;
		k_msleep(int_mas*1000);
		
	 
	}

}
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{		
		for (int i=0;i<5;i++){
				if ( setare[i]==true)
					{if (i<4)
						printk("setare %d\n", i + 1);
					else 
						printk("setare %d\n ",0);
					setare[i]=false;
					if (i<4)
					{setare[i+1]=true;
						break;}
					else {setare[0]=true;
							break;}
		}}
		
		}
void button_pressed1(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
		k_msleep(30);
		for (int i=0;i<5;i++){
				if ( setare[i]==true)
					{switch (i)
					{
					case 0:
						
						tip_temp[0]=true;
						tip_temp[1]=false;
						
						printk(" masurare in grade fahrenheit");
						break;
					case 1:if (rezolutie_t<3)
							rezolutie_t+=1;
						printk(" marire rezolutie");
						break;
						
					case 2:
						//restructurare 
						if (int_mas<300){
							if(int_mas<60){
								int_mas+=1;
							}
							else int_mas+=10;
							
							}
							
						printk(" marire interval masurare");
						break;
					case 3:
						if (t_inf<(t_sup-2)){
							t_inf++;}
							printk("marire prag inf= %lf\n", t_inf);
						break;
					case 4:
						if (t_sup<123){
							t_sup++;}
							printk("marire prag superior=%lf", t_sup);
						break;
					default:
						break;
					}
						}
		}
		}
void button_pressed2(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
	
		
		for (int i=0;i<5;i++){
				if ( setare[i]==true)
					{
					if (i>0)
						printk("setare %d\n",i-1);
					else 
						printk("setare "+ 4);
					setare[i]=false;
					if (i>0)
					{setare[i-1]=true;
						break;}
					else {setare[4]=true;
							break;}
		}}
		
		}
void button_pressed3(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
		
		for (int i=0;i<5;i++){
				if ( setare[i]==true)
					{switch (i)
					{
					case 0:
						
						tip_temp[0]=false;
						tip_temp[1]=true;
						printk(" masurare in grade celsius");
						break;
					case 1:
						if (rezolutie_t>0)
							rezolutie_t-=1;
						printk(" scadere rezolutie");
						break;
					case 2:
						//restructurare 
						if (int_mas>1){
							if(int_mas<=61){
									int_mas-=1;
									printk(" timp%d",int_mas);
								}
								else int_mas-=10;
								
								}
							
						printk(" scadere interval masurare");
						break;
					case 3:
						if (t_inf>-55){
							t_inf--;}
							printk("scadere prag inf=%lf\n", t_inf);
						break;
					case 4:
						if (t_sup>t_inf+2){
							t_sup--;}
							
							printk("scadere prag superior=%lf\n", t_sup);
						break;
					default:
						break;
					}
						}
		}
		
		}
K_THREAD_DEFINE(thread0_id, STACKSIZE,masurare_temperatura, NULL, NULL, NULL,
		THREAD0_PRIORITY, 0, 0);		

int main(void)
{
	int ret;
	if (!gpio_is_ready_dt(&led)) {
		return 0;
	}
	if (!gpio_is_ready_dt(&led1)) {
		return 0;
	}
	if (!gpio_is_ready_dt(&led2)) {
		return 0;
	}
	if (!gpio_is_ready_dt(&led3)) {
		return 0;
	}
	if (!device_is_ready(button.port)) {
	return -1;
	}
	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		return -1;
	}
	if (!device_is_ready(button2.port)) {
	return -1;
	}
	ret = gpio_pin_configure_dt(&button2, GPIO_INPUT);
	if (ret < 0) {
		return -1;
	}
	if (!device_is_ready(button3.port)) {
	return -1;
	}
	const struct device *dev;
	dev = DEVICE_DT_GET(GPIO0_LABEL);
	ret = gpio_pin_configure(dev, PIN, GPIO_OUTPUT_ACTIVE);
	ret = gpio_pin_configure_dt(&button3, GPIO_INPUT);
	if (ret < 0) {
		return -1;
	}
	if (!device_is_ready(button4.port)) {
	return -1;
	}
	ret = gpio_pin_configure_dt(&button4, GPIO_INPUT);
	if (ret < 0) {
		return -1;
	}
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}
	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}
	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}
	ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}
	
	bt_conn_cb_register(&connection_callbacks);
	ret= bt_enable(NULL);
	if (ret) {
		LOG_ERR("Bluetooth init failed (err %d)\n", ret);
		return -1;
	}

	LOG_INF("Bluetooth initialized\n");

	/* STEP 6 - Start advertising */
	ret = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	if (ret) {
		LOG_ERR("Advertising failed to start (err %d)\n", ret);
		return -1;
	}
	gpio_pin_set_dt(&led,leds[0]);
	gpio_pin_set_dt(&led1,leds[1]);
	gpio_pin_set_dt(&led2,leds[2]);
	gpio_pin_set_dt(&led3,leds[3]);
	
	
	if (!device_is_ready(dev_i2c.bus)) {
		printk("I2C bus %s is not ready!\n\r",dev_i2c.bus->name);
		return -1;
		}
	if (!device_is_ready(dev_i2c1.bus)) {
		printk("I2C bus %s is not ready!\n\r",dev_i2c1.bus->name);
		return -1;
		}
	//afisare pornire 
	SSD1306_Begin( &dev_i2c1);
	SSD1306_ClearDisplay();
	SSD1306_DrawText(0, 0, "Incarcare", 2);
	SSD1306_DrawLine(0, 24, 127, 24, 0);
	SSD1306_Display();	

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
		gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin)); 	
		gpio_add_callback(button.port, &button_cb_data);
	ret = gpio_pin_interrupt_configure_dt(&button2, GPIO_INT_EDGE_TO_ACTIVE);
		gpio_init_callback(&button_cb_data1, button_pressed1, BIT(button2.pin)); 	
		gpio_add_callback(button2.port, &button_cb_data1);
	ret = gpio_pin_interrupt_configure_dt(&button3, GPIO_INT_EDGE_TO_ACTIVE);
		gpio_init_callback(&button_cb_data2, button_pressed2, BIT(button3.pin)); 	
		gpio_add_callback(button3.port, &button_cb_data2);
	ret = gpio_pin_interrupt_configure_dt(&button4, GPIO_INT_EDGE_TO_ACTIVE);
		gpio_init_callback(&button_cb_data3, button_pressed3, BIT(button4.pin)); 	
		gpio_add_callback(button4.port, &button_cb_data3);
		
		SSD1306_InvertDisplay( 1);
		SSD1306_Display();	
		uint8_t config[2] = {ds1624_CONFIG_REG ,0x0c};
		ret = i2c_write_dt(&dev_i2c, config, sizeof(config));
		if(ret != 0){
		printk("Failed to write to I2C device address %x at Reg. %x \n", dev_i2c.addr,config[0]);
		return -1;}
		config[0]=0x51;//start termometru 
		ret = i2c_write_dt(&dev_i2c, config, 1);
		
	while (1) {
		//masurare temperatura din  int_mas in int_mas 
		k_msleep(400);
		int cc=(int)temp_C;
		int ff=(int)temp_f;
		temp_C=(double)(cc)+(double)((int)((temp_C-cc)*pow(10,rezolutie_t)))/pow(10,rezolutie_t);
		temp_f=(double)(ff)+(double)((int)((temp_f-ff)*pow(10,rezolutie_t)))/pow(10,rezolutie_t);
		//printk("temp tc=%lf, tf=%lf \r\n", temp_C,temp_f);
		sprintf(aF,"%g",(temp_f));
		sprintf(aC,"%g",(temp_C));
		char a1[20]="Rez: ";
		char a2[2];
		sprintf(a2,"%d",(rezolutie_t));
		strcat(a1,a2);
		char* a;
		char* c=0;
		
		if (tip_temp[0]==true){
			a=aF;
			c="F";
		}
		else {
			a=aC;
			c="C";

		}	//afisare setari+ temperatura
		if (tempon==0)
		{	if (tip_temp[1]==true)
				adv_mfg_data.data1=(double)(cc)+(double)((int)((temp_C-cc)*pow(10,rezolutie_t)))/pow(10,rezolutie_t);
			else 
				adv_mfg_data.data1=(double)(ff)+(double)((int)((temp_f-ff)*pow(10,rezolutie_t)))/pow(10,rezolutie_t);
			bt_le_adv_update_data(ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
		
		tempon=0;
		}
		
		SSD1306_ClearDisplay();
		SSD1306_DrawText(2, 2, a, 2);
		SSD1306_DrawText(92, 2, c, 2);
		SSD1306_DrawCircle(88,4,2);
		if (setare[0])
			if (tip_temp[0]==true)
			SSD1306_DrawText(8, 19,"Fahrenheit", 2);
			else 
			SSD1306_DrawText(20, 19,"Celsius", 2);
		else
			
		if (setare[1])
			SSD1306_DrawText(10, 19,a1, 2);
		if (setare[2]){
			if (int_mas>60)
				{double b=(double)(int_mas)/60;
					char b1[20]="timp:";
					char b2[5];
					sprintf(b2,"%.1f",(b));
					strcat(b1,b2);
					strcat(b1,"m");
					SSD1306_DrawText(5, 19,b1, 2);
				}
			else {
				char b1[20]="timp:";
					char b2[11];
					sprintf(b2,"%d",(int_mas));
					strcat(b1,b2);
					strcat(b1,"sec");
					SSD1306_DrawText(5, 19,b1, 2);
			}}
		if (setare[3]){
			if (tip_temp[0]==true)
				{char b2[7];
				double tr=((double)t_inf)*1.8+32;
				sprintf(b2,"%.1f",(tr));
				char t[20]="t_inf:";
				strcat(t,b2);
				SSD1306_DrawText(5, 19,t, 2);
				}
				else {
					char b2[5];
				
				sprintf(b2,"%d",(int)(t_inf));
				char t[20]="t_inf:";
				strcat(t,b2);
				SSD1306_DrawText(5, 19,t, 2);
				}
		}
		if (setare[4]){
			if (tip_temp[0]==true)
				{char b2[7];
				double tr=((double)t_sup)*1.8+32;
				sprintf(b2,"%.1f",(tr));
				char t[20]="t_sup:";
				strcat(t,b2);
				SSD1306_DrawText(5, 19,t, 2);
				}
				else {
					char b2[5];
				
				sprintf(b2,"%d",(int)(t_sup));
				char t[20]="t_sup:";
				strcat(t,b2);
				SSD1306_DrawText(5, 19,t, 2);
				}
		}

		SSD1306_Display();

		
	}
	return 0;
}