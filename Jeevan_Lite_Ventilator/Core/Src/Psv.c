/*
 * Psv.c
 *
 *  Created on: Mar 18, 2022
 *      Author: asus
 */


#include "Psv.h"




extern uint16_t Dac1;
extern uint16_t Dac2;

extern uint8_t kp;
extern uint8_t ki;
extern double kd;
extern int Time;
extern int Speed;


extern int Initial_open_valve;
extern int Alert_error_count;
extern int now_open_valve;
extern int Apnea_Patient_circuit_disconnected_Flag;
int pressure_acheived=1;
uint16_t Apnea_Temp_DAC=0;
int Alert_Status_count=0;
int Alert_Data_send_count=0;


int Dac_control_count=0;
int Wait_Dac=1;
int Apnea_counter_trigger_Flag=1;
int initial_blower_trigger=1;
int initial_blower_trigger2=1;
int Start_check_time=0;
int Start_check=0;
int one_time_only=1;

extern uint16_t _60_Seconds;
extern uint8_t now_check_breath;


extern uint8_t Breath;

int pressure_high_error_count=0;      //new
int pressure_high_error_count2=0;     //new

void PSV_Mode_Task (void *argument)
{

	while(1)
	{



		  if(Initial_open_valve==1)
		  {





			  if(one_time_only==1)
			  {
				  //Initial_open_valve=1;
				 Alert_error_count=0;
				 Alert_Status_count=0;

				 Wait_Dac=1;
				 Apnea_counter_trigger_Flag=1;
				 now_update=1;
				 Blower_Signal(0);

				 initial_blower_trigger=1;
				 initial_blower_trigger2=1;
				 Start_check=0;
				 Alert_Data_send_count=0;
				 Alert_error_count=0;
				 Dac_control_count=0;
				 uwTick=0;




				 Dac1=300;
				 Dac2=2000;
				 kp=5;
				 ki=4;
				 kd=1;
				 Time=10;
				 Speed=30;


				 Trigger_Count=0;
				 _60_Seconds=60000;
				 now_check_breath=0;
				 Breath=0;


				 TempSetpoint = (P1.PEEP_PSV_Val );
				 PID(&TPID, &Temp, &PIDOut, &TempSetpoint, kp, ki, kd, _PID_P_ON_E, _PID_CD_DIRECT);
				 PID_SetMode(&TPID, _PID_MODE_AUTOMATIC);

				 PID_SetSampleTime(&TPID, Time);
				 PID_SetOutputLimits(&TPID, Dac1, Dac2);
				 pressure_acheived=1;


				 Parkar_valve_Signal(0);
				  O2.O2_process = 0;
				  O2.Servo=45;
				  Servo_Angle(O2.Servo);

				  pressure_high_error_count=0;        //new
				  pressure_high_error_count2=0;       //new



				 one_time_only=0;
		    }

			  else
			  {

				  Initial_open_valve=0;
				  ExpValve_OPEN();
				  vTaskDelay(700);
			  }


		  }


		if(now_update==1)
		{

			if(Pressure_sensor._Pressure_Val<=(P1.PEEP_PSV_Val-4))
			{
				ExpValve_CLOSE();
				P1._Apnea_counter_trigger_check_time=2000;
				P1._APNEA_COUNTER = (1000*P1._APNEA_TIME) ;
				ExpValve_CLOSE();





				uwTick=0;
				TPID.OutputSum=500;
				Speed=30;

				kp=5;
				ki=4;

				TempSetpoint = (P1.PEEP_PSV_Val );
				PID(&TPID, &Temp, &PIDOut, &TempSetpoint, kp, ki, kd, _PID_P_ON_E, _PID_CD_DIRECT);
				PID_SetMode(&TPID, _PID_MODE_AUTOMATIC);

				PID_SetSampleTime(&TPID, Time);
				PID_SetOutputLimits(&TPID, Dac1, Dac2);

				vol.Volume = 0;
				now_update=0;
			}
			else if(Pressure_sensor._Pressure_Val>(P1.PEEP_PSV_Val-4))
			{
				ExpValve_OPEN();
				vTaskDelay(600);
				vol.Volume = 0;
				P1.PEEP_PSV_DAC_Val=0;
				Blower_Signal(P1.PEEP_PSV_DAC_Val);
				P1._Apnea_counter_trigger_check_time=2000;
			}
	   }



/*****************************   air release    *****************************************/

		if(now_update==3)
		{

			if(Pressure_sensor._Pressure_Val<=(P1.PEEP_PSV_Val-4))
			{
				ExpValve_CLOSE();
				P1._Apnea_counter_trigger_check_time=2000;

				uwTick=0;
				TPID.OutputSum=500;
				Speed=30;

				kp=5;
				ki=4;

				TempSetpoint = (P1.PEEP_PSV_Val );
				PID(&TPID, &Temp, &PIDOut, &TempSetpoint, kp, ki, kd, _PID_P_ON_E, _PID_CD_DIRECT);
				PID_SetMode(&TPID, _PID_MODE_AUTOMATIC);

				PID_SetSampleTime(&TPID, Time);
				PID_SetOutputLimits(&TPID, Dac1, Dac2);

				now_update=0;
			}
			else if(Pressure_sensor._Pressure_Val>(P1.PEEP_PSV_Val-4))
			{
				ExpValve_OPEN();
				vTaskDelay(600);
			}
	   }

/*********************************************************************/

		if((P1._APNEA_COUNTER >0) && (Trigger==0) && (now_update==0))
		{

			_Control_Byte |= (uint8_t) 0x80;


			Alert_Data_send_count++;
			if(Alert_Data_send_count>1500)
			{
				A.Alert_Now=1;
				Alert_Data_send_count=0;
			}

			if(pressure_acheived == 0)
			{
				Dac_control_count++;
				if(Dac_control_count>250)
				{
					pressure_acheived=1;
					Dac_control_count=0;
					Wait_Dac=1;

				}
			}

			Alert_error_count++;

			Patient_Circuit_Disconnected_Alert_Func();
			Main_Supply_or_Battery_Indication();

			if( Alert_error_count >= 500)
			{
				if(Apnea_Patient_circuit_disconnected_Flag==1)
				{

						Alert_Status_count++;
						if(Alert_Status_count>1)
						{
							A.Alert_Now=1;
							Apnea_Patient_circuit_disconnected_Flag=0;
							Alert_Status_count=0;
							Alert_Data_send_count=0;
							Dac_control_count =0;
							Alert_error_count=0;
						}

				}
				else
				{
					A.Alert_Now=1;
					CLEAR_ALERT_BIT(FIRST_FRAME_UN,_ALERT_PATIENT_CIRCUIT);
				}
				now_open_valve=1;
				Alert_error_count=0;



			}


/*********************************************************************/



        	if((pressure_acheived == 1) && (Wait_Dac == 1))
        	{
					PID_Compute(&TPID);
					P1.PEEP_PSV_DAC_Val=(int16_t)PIDOut;
					Blower_Signal( P1.PEEP_PSV_DAC_Val);

        	}



        if((Flow_Sensor_cal._Flow_Val>0 )  &&  (pressure_acheived==0))
        {
        	if( P1._Apnea_counter_trigger_check_time==0)
        	{
        	     if(pressure_acheived==0)
        	      {
        	    	 if( (Start_check_time==0) && (Start_check==1))
        	    	 {
						 Blower_Signal(0);
						 Wait_Dac=0;
						 uwTick=0;
						 TPID.OutputSum=500;

						 Dac_control_count=0;
						 Alert_Data_send_count=0;
						 Alert_Status_count=0;
						 Alert_error_count=0;
        	    	 }



    	       }


        	 }
        }



        if(Pressure_sensor._Pressure_Val >= (P1.PEEP_PSV_Val))
        {
        	if((initial_blower_trigger==0))
        	{
        		if(Start_check_time==0)
        		{
					pressure_acheived=0;
					if(Apnea_counter_trigger_Flag==1)
					{
						if(Flow_Sensor_cal._Flow_Val==0)
						{
							Apnea_counter_trigger_Flag=0;
						}
					}
        		}
        	}
			 else
			 {
				 initial_blower_trigger=0;
				 Start_check=1;
				 Start_check_time=500;

			 }

        }

/****************  psv air release   ******************************/
        if(Pressure_sensor._Pressure_Val >= (P1.PEEP_PSV_Val +5))
        {
        	pressure_high_error_count++;
        	P1._Apnea_counter_trigger_check_time=1000;
        	if(pressure_high_error_count>1000)
        	{
        		now_update=3;
        		pressure_high_error_count=0;
        	}

        }

        else if(Pressure_sensor._Pressure_Val < (P1.PEEP_PSV_Val +5))
        {
        	pressure_high_error_count2++;
        	if(pressure_high_error_count2>1000)
        	{
        		pressure_high_error_count=0;
        		pressure_high_error_count2=0;
        	}

        }

/*********************************************************************/


			if( (P1._Apnea_counter_trigger_check_time==0) && (Apnea_counter_trigger_Flag==0))
			{
				if (P1._TRIG_TYPE == 1)
			    {
			        if((Pressure_sensor._Pressure_Val<(P1.simv_trigger_offset-P1._TRIG_LMT)))
			        {


								if (_CurrentBackupMode == PCCMV_BACKUP)
								{
									now_check_breath=1;
									_Control_Byte &= (uint8_t) (~(0x80));
									_I_TIMER = B2._I_TIMER_HOLD;
									P1.PEEP_PSV_DAC_Val=0;
									Blower_Signal(P1.PEEP_PSV_DAC_Val);
									vol.Volume = 0;
									ExpValve_CLOSE();

									Trigger_Count++;

									A.Alert_Now=0;

									uwTick=0;
									TPID.OutputSum=300;
									Speed=40;
									kp=5;
									ki=4;

									TempSetpoint = P1.PIP_PSV_Val;
									PID(&TPID, &Temp, &PIDOut, &TempSetpoint, kp, ki, kd, _PID_P_ON_E, _PID_CD_DIRECT);
									PID_SetMode(&TPID, _PID_MODE_AUTOMATIC);

									PID_SetSampleTime(&TPID, Time);
									PID_SetOutputLimits(&TPID, Dac1, Dac2);

									Trigger=1;
									vTaskResume(Psv_Pid_Handler);
									vTaskSuspend(Psv_Handler);

								 }
								else if (_CurrentBackupMode == VCCMV_BACKUP)
								{
									now_check_breath=1;
									_Control_Byte &= (uint8_t) (~(0x80));
									_I_TIMER = D2._I_TIMER_HOLD;
									P1.PEEP_PSV_DAC_Val=0;
									Blower_Signal(P1.PEEP_PSV_DAC_Val);
									vol.Volume = 0;
									ExpValve_CLOSE();

									Trigger_Count++;

									uwTick=0;
									TPID.OutputSum=300;
									Speed=40;
									kp=5;
									ki=4;

									TempSetpoint = P1.PIP_PSV_Val;
									PID(&TPID, &Temp, &PIDOut, &TempSetpoint, kp, ki, kd, _PID_P_ON_E, _PID_CD_DIRECT);
									PID_SetMode(&TPID, _PID_MODE_AUTOMATIC);
									PID_SetSampleTime(&TPID, Time);
									PID_SetOutputLimits(&TPID, Dac1, Dac2);

									Trigger=1;
									vTaskResume(Psv_Pid_Handler);
									vTaskSuspend(Psv_Handler);
								}
					    }
					}
				  else
				  {
					 if((Flow_Sensor_cal._Flow_Val>(P1.simv_trigger_offset2+P1._TRIG_LMT)))
					 {


						if (_CurrentBackupMode == PCCMV_BACKUP)
						{
							now_check_breath=1;
							_Control_Byte &= (uint8_t) (~(0x80));
							_I_TIMER = B2._I_TIMER_HOLD;
							P1.PEEP_PSV_DAC_Val = 0;
							Blower_Signal(P1.PEEP_PSV_DAC_Val);

							 TempSetpoint = P1.PIP_PSV_Val;
							 vol.Volume = 0;
							 ExpValve_CLOSE();

							 Trigger_Count++;

							 uwTick=0;
							 TPID.OutputSum=300;
							 Speed=40;
							 kp=5;
							 ki=4;

							 PID(&TPID, &Temp, &PIDOut, &TempSetpoint, kp, ki, kd, _PID_P_ON_E, _PID_CD_DIRECT);
							 PID_SetMode(&TPID, _PID_MODE_AUTOMATIC);
							 PID_SetSampleTime(&TPID, Time);
							 PID_SetOutputLimits(&TPID, Dac1, Dac2);

							Trigger = 1;
							vTaskResume(Psv_Pid_Handler);
							vTaskSuspend(Psv_Handler);

						}
						else if (_CurrentBackupMode == VCCMV_BACKUP)
						{
							now_check_breath=1;
							_Control_Byte &= (uint8_t) (~(0x80));
							_I_TIMER = D2._I_TIMER_HOLD;
							P1.PEEP_PSV_DAC_Val = 0;
							Blower_Signal(P1.PEEP_PSV_DAC_Val);
							vol.Volume = 0;

							ExpValve_CLOSE();
							Trigger_Count++;

							uwTick=0;
							TPID.OutputSum=300;
							Speed=40;
							kp=5;
							ki=4;

							TempSetpoint = P1.PIP_PSV_Val;
							PID(&TPID, &Temp, &PIDOut, &TempSetpoint, kp, ki, kd, _PID_P_ON_E, _PID_CD_DIRECT);
							PID_SetMode(&TPID, _PID_MODE_AUTOMATIC);
							PID_SetSampleTime(&TPID, Time);
							PID_SetOutputLimits(&TPID, Dac1, Dac2);

							Trigger = 1;
							vTaskResume(Psv_Pid_Handler);
							vTaskSuspend(Psv_Handler);
						}
					 }
					}



			   }




			   else
			   {
				       if(Flow_Sensor_cal._Flow_Val==0 || (Flow_Sensor_cal._Flow_Val>=(-8) && Flow_Sensor_cal._Flow_Val<3))
			         	{
			         		 P1.simv_trigger_offset = Pressure_sensor._Pressure_Val;
			         		 P1.simv_trigger_offset2=Flow_Sensor_cal._Flow_Val;
			         	}
			    }


			ExpValve_CLOSE();

/*********************************************************************/

		}


		else if(P1._APNEA_COUNTER == 0)
		{
			Alert_Receiving_Parameter();
			vTaskResume(alert_Handler);
			Trigger=0;
			SET_ALERT_BIT(FIRST_FRAME_UN,_ALERT_APNEA);
			A.Alert_Now=1;
			A.Apnea_UART_alert=1;
			A.Alert=1;
			A.Red_Led_Alert=1;
			Led_Alert();
			Trigger_Count=0;
			P1.PEEP_PSV_DAC_Val=0;
			Blower_Signal(P1.PEEP_PSV_DAC_Val);
			S5.peep_process_done=0;
			S5.lock=1;
			P1.Apnea_Mode=0;
			O2_Parameter();
			if(_CurrentBackupMode == PCCMV_BACKUP )
			{
				S5.Lock_delay=100;
				_CurrentWaveFormState = Generate_E_Wave;
				_CurrentComputationState = Compute_I_Wave ;
				vTaskResume(One_Time_Handler);
			    vTaskResume(Back_Up_PC_CMV_Mode_Handler);
			    vTaskResume(PID_Back_Up_PC_CMV_Mode_Handler);
			    if(O2._FIO2_Val>21)
			    {
			    	vTaskResume(Oxygen_Handler);
			    }
			    else if(O2._FIO2_Val<=21)
			    {
			    	O2.O2_DAC=0;
			    	Parkar_valve_Signal(0);
			    	vTaskSuspend(Oxygen_Handler);
			    }

			    vTaskSuspend(Psv_Handler);
			    vTaskSuspend(Psv_Pid_Handler);
			}


			if(_CurrentBackupMode == VCCMV_BACKUP )
			{
				S5.Lock_delay=10;
				S5._Set_Peep=D1._PEEP_Val;
				_CurrentWaveFormState = Generate_E_Wave;
				_CurrentComputationState = Compute_I_Wave ;
				vTaskResume(One_Time_Handler);
				vTaskResume(Back_Up_VC_CMV_Mode_Handler);
				vTaskResume(PID_Back_Up_VC_CMV_Mode_Handler);

				if(O2._FIO2_Val>21)
				{
					vTaskResume(Oxygen_Handler);
				}
				else if(O2._FIO2_Val<=21)
				{
					O2.O2_DAC=0;
					Parkar_valve_Signal(0);
					vTaskSuspend(Oxygen_Handler);
				}

				vTaskSuspend(Psv_Handler);
				vTaskSuspend(Psv_Pid_Handler);
			}
		}
		vTaskDelay(2);
/*********************************************************************/
	}

}



void PSV_PARAMETERS(SET_PARAM_CMD_PACKET *RX_PARAM_CMD_PACKET)
{
	//xSemaphoreTake(binarysem,portMAX_DELAY);
	     P1.PIP_PSV_Val = RX_PARAM_CMD_PACKET->_PIP;
	     P1.PEEP_PSV_Val = RX_PARAM_CMD_PACKET->_PEEP;
	     P1._Apnea_counter_trigger_check_time=2000;
		 P1._TRIG_TYPE = (0xF0 & RX_PARAM_CMD_PACKET->_TRIG_TYPE_TRIG_LMT)>>4;
		 P1._TRIG_LMT = 0x0F & (RX_PARAM_CMD_PACKET->_TRIG_TYPE_TRIG_LMT);
		 P1._TRIG_TIME = 0x0F & (RX_PARAM_CMD_PACKET->_RiseTime_TRIG_TIME);

		 if(_CurrentBackupMode == PCCMV_BACKUP)
		 {
			 P1._TOLERANCE_EWAVE = B2._E_TIMER_HOLD - P1._CALC_TRIG_VAL ;
		 }

		 else if(_CurrentBackupMode == VCCMV_BACKUP)
		 {

			 P1._TOLERANCE_EWAVE = D2._E_TIMER_HOLD - P1._CALC_TRIG_VAL ;
		 }
		 P1._TRIG_WINDOW = P1._TOLERANCE_EWAVE * (((float)P1._TRIG_TIME*10.00)/100.00) ;
		 P1._APNEA_TIME = RX_PARAM_CMD_PACKET->_APNEA;
		 P1._APNEA_COUNTER = (1000*P1._APNEA_TIME) ;

		 Initial_open_valve=1;
		 one_time_only=1;
		 ExpValve_CLOSE();

		 vTaskSuspend(pc_mode_Handler);
		 vTaskSuspend(Pc_cmv_Pid_Handler);

		 vTaskSuspend(Pc_simv_Mode_Handler);
		 vTaskSuspend(Pc_simv_Mode_Pid_Handler);

		 vTaskSuspend(Vc_mode_Handler);
		 vTaskSuspend(Vc_cmv_Pid_Handler);

		 vTaskSuspend(Vc_simv_mode_Handler);
		 vTaskSuspend(Vc_simv_Pid_Handler);

		 vTaskSuspend(Cpap_Handler);

		 vTaskSuspend(BiPap_Handler);
		 vTaskSuspend(BiPap_Pid_Handler);

		 vTaskSuspend(APRV_Handler);
		 vTaskSuspend(APRV_one_Handler);

		 vTaskSuspend(Back_Up_PC_CMV_Mode_Handler);
		 vTaskSuspend(PID_Back_Up_PC_CMV_Mode_Handler);

		 vTaskSuspend(Back_Up_VC_CMV_Mode_Handler);
		 vTaskSuspend(PID_Back_Up_VC_CMV_Mode_Handler);

		 vTaskSuspend(One_Time_Handler);

		 vTaskSuspend(Oxygen_Handler);

		 vTaskSuspend(alert_Handler);

		 vTaskSuspend(HFNC_Handler);
		 vTaskSuspend(Suction_Handler);


		 vTaskResume(Psv_Handler);



		 _CurrentMode = PSV;

		 if(_CurrentBackupMode == PCCMV_BACKUP)
		 {
			 S1._Mode_Val=10;
		 }
		 else if(_CurrentBackupMode == VCCMV_BACKUP)
		 {
			 S1._Mode_Val=11;
		 }
		// xSemaphoreGive(binarysem);
}




void PSV_PID_Task (void *argument)
{

	while(1)
	{
		if(_I_TIMER>0)
		{
			ExpValve_CLOSE();
			PID_Compute(&TPID);
			P1.PIP_PSV_DAC_Val=(int16_t)PIDOut;
			Blower_Signal( P1.PIP_PSV_DAC_Val);
			pressure_acheived=1;
			Wait_Dac=1;
			Apnea_counter_trigger_Flag=1;
		}
		else if(_I_TIMER==0)
		{

			P1.PIP_PSV_DAC_Val=0;
			Blower_Signal( P1.PIP_PSV_DAC_Val);
			Trigger=0;
			now_update=1;

			P1._Apnea_counter_trigger_check_time=2000;
			P1._APNEA_COUNTER = (1000*P1._APNEA_TIME) ;

			Apnea_counter_trigger_Flag=1;
			Alert_Data_send_count=0;
			Alert_error_count=0;
			Dac_control_count=0;
			Alert_error_count=0;


			  pressure_high_error_count=0;    //new
			  pressure_high_error_count2=0;   //new

			vTaskResume(Psv_Handler);
			vTaskSuspend(Psv_Pid_Handler);
		}


		vTaskDelay(2);

	}

}



