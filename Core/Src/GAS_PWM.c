/*
 * GAS_PWM.c
 *
 *  Created on: Nov 6, 2020
 *      Author: Suprhimp
 *
 *      must calibrate the SeraRisingTime and Maxwidth by debugging.
 *      Check the Falling edge to find them!!!
 *      pwm1->R, pwm2->L
 */

#include "GAS_PWM.h"//test
#include "stdio.h"


//BatteryTemp_t R_BatteryTemp; //230104: defined in GAS_Can.c

pwmIn_t pwmIn15;
pwmIn_t pwmIn16;
pwmIn_t pwmIn17;

uint16_t per;
uint16_t fanPulse;
float floatFanPulse;
uint8_t BMSTemp;

volatile uint8_t pwmChangeFlag; //230104: not used in this file//230108: use again

void GAS_PWM_inputInit(void);
void GAS_PWM_outputInit(void);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
void GAS_PWM_Fan_run();
void GAS_PWM_Check(TIM_HandleTypeDef *htim, pwmIn_t *pwmIn);

void GAS_PWM_inputInit(void)
{

	HAL_TIM_IC_Start_IT(&htim15, TIM_CHANNEL_1);	//Fan_Tach_789
	HAL_TIM_IC_Start_IT(&htim16, TIM_CHANNEL_1);	//Fan_Tach_456
	HAL_TIM_IC_Start_IT(&htim17, TIM_CHANNEL_1);	//Fan_Tach_123


}

void GAS_PWM_outputInit(void)
{
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);

	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);//230105: why not existed?????

	/*Turn on the fan!*/
	HAL_GPIO_WritePin(Fan_On_GPIO_Port, Fan_On_Pin, GPIO_PIN_SET);
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	/*
	 * For PWM input function
	 * One PWM input per one htim channel(because of TIM counter)
	 * Period, width, duty cycle, frequency can be calculated.
	 */
	if(htim ->Instance == TIM15)
	{
		GAS_PWM_Check(htim, &pwmIn15);
	}
	else if(htim ->Instance == TIM16){
		GAS_PWM_Check(htim, &pwmIn16);
	}
	else if(htim ->Instance == TIM17){
		GAS_PWM_Check(htim, &pwmIn17);
	}
//	if(htim ->Instance == TIM4){
//		GAS_PWM_Check2(htim, &pwmIn3);
//		pwmChangeFlag = 1;
//	}
}

void GAS_PWM_Fan_run()
{
	/*
	 * PWM change duty cycle function
	 * Change duty cycle by changing register CCR directly
	 * 220108
	 * write 9 duties every time
	 * read only 3 fan duties at a time
	 */


	BMSTemp = R_BatteryTemp.B.HighestTemp; //230104: FIXED(/10 deleted)//230108: FIXED(uint16_t -> uint8_t)

	//230108: TC ordermode update
	uint16_t TCorder = 0;//R_TC_order.B.TCControlMode;

	if(TCorder == 1){
		fanPulse = 287*((float)(R_TC_order.B.TCFanDutyOrder/100));
	}else{
		per = BMSTemp*4-60;
		if (per<=20) per=20;					//230105: under 20 degree C, min duty
		else if (per>=100) per = 100;			//230105: over 40 degree C, Max duty
		floatFanPulse = per/100.0;				//per->60
		fanPulse = 287.0*floatFanPulse; 		//230105: not duty... its pulse!!!! => name change(duty->fanPulse)

	}

	/*
	per = BMSTemp*4-60;
	if (per<=20) per=20;					//230105: under 20 degree C, min duty
	else if (per>=100) per = 100;			//230105: over 40 degree C, Max duty
	floatFanPulse = per/100.0;				//per->60
	fanPulse = 287.0*floatFanPulse; 		//230105: not duty... its pulse!!!! => name change(duty->fanPulse)
*/
	//fanPulse = 140;

	htim1.Instance -> CCR1 = fanPulse;
	htim1.Instance -> CCR2 = fanPulse;
	htim1.Instance -> CCR3 = fanPulse;
	//HAL_TIM_GenerateEvent(&htim1, TIM_EventSource_Update);
	//HAL_Delay(1);
	htim2.Instance -> CCR1 = fanPulse;
	htim2.Instance -> CCR2 = fanPulse;
	htim2.Instance -> CCR3 = fanPulse;
	//HAL_TIM_GenerateEvent(&htim2, TIM_EventSource_Update);
	//HAL_Delay(1);
	htim3.Instance -> CCR1 = fanPulse;
	htim3.Instance -> CCR2 = fanPulse;
	htim3.Instance -> CCR3 = fanPulse;
	//HAL_TIM_GenerateEvent(&htim3, TIM_EventSource_Update);
	//HAL_Delay(1);

/*
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, fanPulse);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, fanPulse);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, fanPulse);
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, fanPulse);
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, fanPulse);
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, fanPulse);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, fanPulse);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, fanPulse);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, fanPulse);
*/

/*
	TIM1->CCR1=fanPulse;					//TIM1_CHANNEL1: fan control 3
	TIM1->CCR2=fanPulse;					//TIM1_CHANNEL2: fan control 2
	TIM1->CCR3=fanPulse;					//TIM1_CHANNEL3: fan control 1

	TIM2->CCR1=fanPulse;					//TIM1_CHANNEL1: fan control 4
	TIM2->CCR2=fanPulse;					//TIM1_CHANNEL2: fan control 5
	TIM2->CCR3=fanPulse;					//TIM1_CHANNEL3: fan control 6

	TIM3->CCR1=fanPulse;					//TIM1_CHANNEL1: fan control 7
	TIM3->CCR2=fanPulse;					//TIM1_CHANNEL2: fan control 8

	TIM3->CCR3=fanPulse;
	 				//230105: why not existed???: fan control 9
*/

	/*
	 * PWM input: select fan and calculate duty: 230108
	 */
	switch(pwmChangeFlag){
	case 1:
		HAL_GPIO_WritePin(Fan_Tach_S0_GPIO_Port, Fan_Tach_S0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(Fan_Tach_S1_GPIO_Port, Fan_Tach_S1_Pin, GPIO_PIN_SET);
		GAS_PWM_Check(&htim15, &pwmIn15); //read fan7
		GAS_PWM_Check(&htim16, &pwmIn16); //read fan4
		GAS_PWM_Check(&htim17, &pwmIn17); //read fan1
		pwmChangeFlag ++;
		break;

	case 2:
		HAL_GPIO_WritePin(Fan_Tach_S0_GPIO_Port, Fan_Tach_S0_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Fan_Tach_S1_GPIO_Port, Fan_Tach_S1_Pin, GPIO_PIN_SET);
		GAS_PWM_Check(&htim15, &pwmIn15); //read fan8
		GAS_PWM_Check(&htim16, &pwmIn16); //read fan5
		GAS_PWM_Check(&htim17, &pwmIn17); //read fan2
		pwmChangeFlag ++;
		break;

	case 3:
		HAL_GPIO_WritePin(Fan_Tach_S0_GPIO_Port, Fan_Tach_S0_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(Fan_Tach_S1_GPIO_Port, Fan_Tach_S1_Pin, GPIO_PIN_RESET);
		GAS_PWM_Check(&htim15, &pwmIn15); //read fan9
		GAS_PWM_Check(&htim16, &pwmIn16); //read fan6
		GAS_PWM_Check(&htim17, &pwmIn17); //read fan3
		pwmChangeFlag ++;
		break;
	default:
		pwmChangeFlag = 1;
	}



}


void GAS_PWM_Check(TIM_HandleTypeDef *htim, pwmIn_t *pwmIn){

			if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
			{
				pwmIn->RisingEdgeValue = HAL_TIM_ReadCapturedValue(htim,TIM_CHANNEL_1); //CCR1: pulse
				htim->Instance->CNT = 0;
				pwmIn->Period=pwmIn->RisingEdgeValue;
			}
			else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
			{
				pwmIn->FallingEdgeValue=HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
				pwmIn->Width=pwmIn->FallingEdgeValue;
				pwmIn->DutyCycle = (float)(pwmIn->FallingEdgeValue)/(float)(pwmIn->RisingEdgeValue)*100;
			}

}


