#include "stm32f10x.h"

#include "main.h"
#include "base.h"
#include "USART.h"
#include "timer.h"
#include "mpu9250.h"
#include "lt8910.h"
#include "set.h"
#include "angle.h"
#include "control.h"
#include "ms5611.h"
#include "hmc5883.h"


#include "SR04.h"
GPIO_InitTypeDef GPIO_InitStructure;
u8 TIM3CH1_CAPTURE_STA=0; //输入捕获状态
//bit7:捕获完成标志 
//bit6：捕获到高点平标志
//bit5~0：捕获到高电平后定时器溢出的次数
u32 TIM3CH1_CAPTURE_VAL;//输入捕获值


void main(){
	int temp;
	
	SystemInit();
	Usart_Init(USART,115200);
	
	uprintf(USART,"Hello,world!\r\n");

	delay_init(72);
	MPU_init();
	//ms5611_init();
	
	//hmc5883lInit();
	
	TIMER_Init(TIM6);
	if(CAP_TIM==TIM3){
		TIM3_Cap_Init();
	}
	SR04_Init();
	Brushless_Init();
	//Brush_Init();
	Load_Prams();
	pitch_target=-2.85;     //3.25
	roll_target=0.3;  //48.5
	Write_Prams();
	uprintf(USART,"in  it ok!\r\n");
	
	//uprintf(USART1,"mode=%d\r\nstate=%d\r\n",mode,state);
	
	//BLE_Send_Over();
	
	
	
	while(1){	
			//send_wave((int)(gravity_Y*100),(int)(pitch),(int)(kal_ac_y.kal_out*100),(int)(a_y*100));
		//send_wave((int)roll,(int)height*10,(int)angle_speed_X,(int)pitch);
		//uprintf(USART1,"%f\r\n",fly_speed);
		//send_wave((int)kal_gy_z.kal_out,(int)angle_speed_Z,(int)yaw,(int)pitch);
		//send_wave((int)s_x,(int)s_y,(int)v_x,(int)yaw);
		//uprintf(USART1,"%f\r\n",height);
		//uprintf(USART,"%f\r\n",pitch);
		//uprintf(USART,"%f\r\n",height);
		
		/*
		1、标定角度正方向  pitch前倾为负   roll左倾为正   顺时针为负
		2、标定角速度正方向  X左倾为正   Y前倾为正 Z 顺时针为正
		3、根据此正方向，修改PID中参数的符号
		*/
		
//				if(TIM3CH1_CAPTURE_STA&0X80)//成功捕获到了一次高电平
//		{
//			temp=TIM3CH1_CAPTURE_STA&0X3F;
//			temp*=65535;					//溢出时间总和
//			temp+=TIM3CH1_CAPTURE_VAL;		//得到总的高电平时间
//			uprintf(USART,"HIGH:%f cm\r\n",((float)temp)*0.5*34000/1000000/2);	//打印总的高点平时间
// 			TIM3CH1_CAPTURE_STA=0;			//开启下一次捕获
// 		}
		
			if(Send_Angle){
				//send_wave((int)(pitch*10),(int)(roll*10),(int)(accel_speed_Z*10),(int)(Roll_PID.i*10));//标定角度正方向
				//send_wave((int)(angle_speed_X*100),(int)(angle_speed_Y*100),(int)(angle_speed_Z*100),0);//标定角速度正方向
				    
				//send_wave((int)Angle_Speed_X_Out,(int)Angle_Speed_Y_Out,0,0);
				//send_wave((int)CH1_Out,(int)Pitch_Out,(int)Roll_PID.i,(int)Pitch_PID.i);
				//send_wave((int)CH1_Out,(int)CH2_Out,(int)CH3_Out,(int)CH4_Out);
				//send_wave((int)Angle_Speed_Y_Out,(int)Angle_Speed_X_Out,(int)pitch,(int)roll);
				//send_wave((int)Angle_Speed_X_Out,(int)(ANGLE_SPEED_X_PID.i*100),(int)(angle_speed_X*10),(int)roll);
				send_wave((int)(height),(int)(kal_acc_z.kal_out),(int)(Height_PID.i),(int)(SR04_V));
			}
			//uprintf(USART,"Height:%f\r\n",height);
	}

}

int overflow;//溢出计数

typedef enum{
	WAIT_RISING,
	WAIT_FALLING
}cap_state;

cap_state state=WAIT_RISING;


/*
void TIM3_IRQHandler(void)
{ 

 	if((TIM3CH1_CAPTURE_STA&0X80)==0)//还未成功捕获	 与第七位与
	{	  
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)   //溢出标志
		 
		{	    
			if(TIM3CH1_CAPTURE_STA&0X40)//已经捕获到高电平了
			{
				if((TIM3CH1_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM3CH1_CAPTURE_STA|=0X80;//标记成功捕获了一次
					TIM3CH1_CAPTURE_VAL=0XFFFF;
				}else TIM3CH1_CAPTURE_STA++;
			}	 
		}
		if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)//捕获1发生捕获事件
			{	
				if(TIM3CH1_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
				{	  			
					TIM3CH1_CAPTURE_STA|=0X80;		//标记成功捕获到一次上升沿
					TIM3CH1_CAPTURE_VAL=TIM_GetCapture1(TIM3);
					TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
				}else  								//还未开始,第一次捕获上升沿
				{
					TIM3CH1_CAPTURE_STA=0;			//清空
					TIM3CH1_CAPTURE_VAL=0;
					TIM_SetCounter(TIM3,0);
					TIM3CH1_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
					TIM_OC1PolarityConfig(TIM3,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
				}		    
			}			     	    					   
 	}
 
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC1|TIM_IT_Update); //清除中断标志位
 
}*/



void TIM3_IRQHandler(void)  
{   
		float height_temp;
		float v_temp;
		if (TIM_GetITStatus(CAP_TIM, TIM_IT_Update) != RESET)  {         
				++overflow;  
				TIM_ClearITPendingBit(CAP_TIM, TIM_IT_Update); //清除中断  
		}  
			
		if (TIM_GetITStatus(CAP_TIM, TIM_IT_CC1) != RESET)//捕获事件  
		{     
			if(state == WAIT_RISING)      //初始状态,此时已经捕捉到了上升沿。             
				{     
						overflow = 0;  
						TIM_SetCounter(CAP_TIM,0);  
						state = WAIT_FALLING;    
						TIM_OC1PolarityConfig(CAP_TIM,TIM_ICPolarity_Falling);     //设置为下降沿捕获  
				}      
				else if(state==WAIT_FALLING)// wait falling  
				{  
						cap_time = TIM_GetCapture1(CAP_TIM);  
						cap_time+= overflow*65535;  
						
						height_temp=cap_time*0.5*34000/1000000;
						height_temp=KalMan(&kal_height,height_temp);
					
						if(height_temp>300)
							height_temp=height;
						v_temp=(height_temp-height)/0.015;    //单位 cm/s
						
						
						v_temp=KalMan(&kal_V,v_temp);
						if(v_temp>50||v_temp<-50){
							;
						}else{
							SR04_V=v_temp;
						}

						Limit(SR04_V,50);
						height=height_temp;
						overflow = 0;  
						TIM_SetCounter(CAP_TIM,0);  
						state = WAIT_RISING;  
						TIM_OC1PolarityConfig(CAP_TIM,TIM_ICPolarity_Rising); //设置为上升沿捕获  
							
				}  
				TIM_ClearITPendingBit(CAP_TIM, TIM_IT_CC1); //清除中断  
		}                                                  
   
    //TIM_ClearITPendingBit(CAP_TIM, TIM_IT_CC1|TIM_IT_Update); //清除中断  
}  

int count=0;
int save=0;

int temp;

void TIM6_IRQHandler(void){
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
		TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);   //关中断
		switch(count){
			case 1:
				Get_Angle_Speed();
				Get_Accel_Angle();
				break;
			case 2:
				IMU_Update(accel_speed_X,accel_speed_Y,accel_speed_Z,angle_speed_X,angle_speed_Y,angle_speed_Z);
				//AHR_Update(accel_speed_X,accel_speed_Y,accel_speed_Z,angle_speed_X,angle_speed_Y,angle_speed_Z,Mag_X,Mag_Y,Mag_Z);
				//Get_Angle();
				break;
			case 3:
				Fly_Control();
				//Adjust_Acc();
				//uprintf(USART,"test\r\n");
				break;
			case 4:
				//ms5611_get_height();
			temp++;
				if(temp>=3){
					SR04_Trig();
					//Get_Height();
					temp=0;
				}
				break;
			case 0:
				//HMC_Get_Mag();    //20ms 一次
				
				
				break;
			default:
				break;
		}
		
		count=(count+1)%5;
		TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	}	
}
void UART4_IRQHandler(void){
	char c;
	if(USART_GetITStatus(USART, USART_IT_RXNE) != RESET) {  
		USART_ClearITPendingBit(USART,USART_IT_RXNE); 
		c=USART_ReceiveData(USART);
		switch(c){
			case '1':
				Roll_PID.KI+=0.01;
				clear_i(Roll_PID);
				uprintf(USART,"Roll_PID.i=%f\r\n",Roll_PID.KI);
				break;
			case '2':
				Roll_PID.KI-=0.01;
				clear_i(Roll_PID);
				uprintf(USART,"Roll_PID.i=%f\r\n",Roll_PID.KI);
				break;
			case 't':
				Send_Angle=!Send_Angle;
				Height_PID.i=0;
				uprintf(USART,"SEND change!\r\n");
				break;
			case 'd':
				Debug=!Debug;
				uprintf(USART,"Debug=  %d\r\n",Debug);
				break;
			case 'f':
				Fly_Init();
				uprintf(USART,"start fly!\r\n");
				break;
			case 's':
				Fly_Stop();
				uprintf(USART,"stop fly!\r\n");
				break;
			case 'w':
				Fly_Up();
				uprintf(USART,"fly up !!\r\n");
				break;
			case 'p':
				Roll_PID.KP+=0.01;
				Pitch_PID.KP-=0.01;
				uprintf(USART,"pitch_P=%f    roll_P=%f \r\n",Pitch_PID.KP,Roll_PID.KP);
			
				break;
			case 'q':
				Roll_PID.KP-=0.01;
				Pitch_PID.KP+=0.01;
				uprintf(USART,"pitch_P=%f    roll_P=%f \r\n",Pitch_PID.KP,Roll_PID.KP);
				break;
			case 'a':

//				Set_Brushless_Speed(1,10);
//				Set_Brushless_Speed(2,10);
//				Set_Brushless_Speed(3,10);
//				Set_Brushless_Speed(4,10);
				base_duty+=0.3;
				uprintf(USART,"base_duty=%f\r\n",base_duty);
//				uprintf(USART,"CCR=%d\r\n",TIM1->CCR1);
				break;
			case 'b':
//				Brushless_Stop();
//				uprintf(USART,"CCR2=%d\r\n",TIM1->CCR2);
				base_duty-=0.3;
				uprintf(USART,"base_duty=%f\r\n",base_duty);
				break;
			case 'i':
				ANGLE_SPEED_X_PID.KD+=0.1;
				ANGLE_SPEED_Y_PID.KD+=0.1;
				uprintf(USART,"ANGLE_SPEED_X_PID.KD=%f\r\n",ANGLE_SPEED_X_PID.KD);
				break;
			case 'o':
				ANGLE_SPEED_X_PID.KD-=0.1;
				ANGLE_SPEED_Y_PID.KD-=0.1;
				uprintf(USART,"ANGLE_SPEED_X_PID.KD=%f\r\n",ANGLE_SPEED_X_PID.KD);
				break;	
			case 'e':
				roll_target+=0.1;
				uprintf(USART,"roll_target=%f\r\n",roll_target);
				break;					
			case 'r':
				roll_target-=0.1;
				uprintf(USART,"roll_target=%f\r\n",roll_target);
				break;
			case 'z':
				ANGLE_SPEED_X_PID.KI-=0.1;
				ANGLE_SPEED_Y_PID.KI-=0.1;
				clear_i(ANGLE_SPEED_X_PID);
				clear_i(ANGLE_SPEED_Y_PID);
				uprintf(USART,"angle_speed_ki=%f\r\n",ANGLE_SPEED_X_PID.KI);
				break;
			case 'x':
				ANGLE_SPEED_X_PID.KI+=0.1;
				ANGLE_SPEED_Y_PID.KI+=0.1;
				clear_i(ANGLE_SPEED_X_PID);
				clear_i(ANGLE_SPEED_Y_PID);
				uprintf(USART,"angle_speed_ki=%f\r\n",ANGLE_SPEED_X_PID.KI);
				break;
			case 'c':
				ANGLE_SPEED_Z_PID.KP+=0.1;
				uprintf(USART,"yaw_KP=%f\r\n",ANGLE_SPEED_Z_PID.KP);
				break;
			case 'v':
				ANGLE_SPEED_Z_PID.KP-=0.1;
				uprintf(USART,"yaw_KP=%f\r\n",ANGLE_SPEED_Z_PID.KP);
				break;				
			case 'n':
				Velocity_PID.KI+=0.001;
				clear_i(Velocity_PID);
				uprintf(USART,"Velocity_PID.KI=%f\r\n",Velocity_PID.KI);
				break;
			case 'm':
				Velocity_PID.KI-=0.001;
				clear_i(Velocity_PID);
				uprintf(USART,"Velocity_PID.KI=%f\r\n",Velocity_PID.KI);
				break;

			case 'k':
				Velocity_PID.KP+=0.01;
				uprintf(USART,"V_PID.KP=%f\r\n",Velocity_PID.KP);
//				Height_PID.KP+=0.1;
//				uprintf(USART,"Height_PID.KP=%f\r\n",Height_PID.KP);
				break;
			case 'l':
				Velocity_PID.KP-=0.01;
				uprintf(USART,"V_PID.KP=%f\r\n",Velocity_PID.KP);
//				Height_PID.KP-=0.1;
//				uprintf(USART,"Height_PID.KP=%f\r\n",Height_PID.KP);
				break;		
			case 'g':
				Velocity_PID.KD+=0.1;
				uprintf(USART,"Velocity_PID.KD=%f\r\n",Velocity_PID.KD);
				break;	
			case 'h':
				Velocity_PID.KD-=0.1;
				uprintf(USART,"Velocity_PID.KD=%f\r\n",Velocity_PID.KD);
				break;	
			case 'y':
				ANGLE_SPEED_X_PID.KP-=0.1;	//内环P
				ANGLE_SPEED_Y_PID.KP-=0.1;
				uprintf(USART,"ANGLE_SPEED_X_PID.KP=%f\r\n",ANGLE_SPEED_X_PID.KP);
				break;
			case 'u':
				ANGLE_SPEED_X_PID.KP+=0.1;  //内环P
				ANGLE_SPEED_Y_PID.KP+=0.1;
				uprintf(USART,"ANGLE_SPEED_X_PID.KP=%f\r\n",ANGLE_SPEED_X_PID.KP);
				break;
	
			case '3':
				Adjust_Gyro();   //取陀螺仪零偏
				
				break;
			case '4':
				Load_Prams();
				break;
			case '5':
				Write_Prams();  //写参数 
				break;
			

			case '7':
				Height_Out=15;
				//roll_target=10;
				uprintf(USART,"height_out =15!!\r\n");
				break;
			case '8':
				Height_Out=0;
				uprintf(USART,"Height_Out =0!!\r\n");
				break;
			case '9':
				Roll_Out=0;
				uprintf(USART,"roll_out =0!!\r\n");
				break;
			case '0':
				Roll_Out=5;
				uprintf(USART,"roll_out =5!!\r\n");
				break;

			
			case 'A':  //手柄左边 上
				pitch_target-=0.05;
				uprintf(USART,"%f\r\n",pitch_target);
				break;
			case 'B':    //下
				pitch_target+=0.05;
			uprintf(USART,"%f\r\n",pitch_target);
				break;
			case 'C':   //左
				roll_target+=0.05;
				uprintf(USART,"%f\r\n",roll_target);
				break;
			case 'D':		//右
				roll_target-=0.05;
			uprintf(USART,"%f\r\n",roll_target);
				break;		
			
			case 'I':
				base_duty+=1;
				break;
			
			case 'J':
				base_duty-=1;
				break;
			case 'K':
				base_duty=35;
				break;
			case 'L':
				Fly=0;
				break;
			case 'G':   //select
				Fly=1;
				break;
			case 'E':    //左边的1
				save=1;
				break;
			case 'N':		//右边的2
				if(save){
					Write_Prams(); 
					save=0;
				}
				break;
			
			case 'P':
				Height_PID.KP+=0.1;
				uprintf(USART,"height_PID.KP=%f\r\n",Height_PID.KP);
				break;
			case 'Q':
				Height_PID.KP-=0.1;
				uprintf(USART,"height_PID.KP=%f\r\n",Height_PID.KP);
				break;
			default:
				break;
		} 
	}
}
